/*
 * @Author: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include <toolkit/shader/compilegraph/DependencyScanner.hpp>
#include <toolkit/shader/source/IncludeResolver.hpp>
#include <toolkit/shader/source/SourceManager.hpp>
#include <toolkit/shader/source/SourceFile.hpp>
#include <toolkit/shader/cache/ShaderHash.hpp>

#include <toolkit/platforms/FileSystem.hpp>

namespace toolkit
{
    namespace
    {
        // Skip ASCII whitespace including tabs but stop at newlines so we
        // don't accidentally cross-line a malformed directive.
        i32 skipInlineSpace(TkStringView s, i32 i)
        {
            while (i < s.length() && (s[i] == ' ' || s[i] == '\t'))
                i++;
            return i;
        }

        // Match the literal `#include` at position `i`. Returns the index
        // just past the directive, or -1 if it didn't match.
        i32 matchInclude(TkStringView s, i32 i)
        {
            if (i + 8 > s.length()) return -1;
            if (s[i] != '#') return -1;
            i32 j = skipInlineSpace(s, i + 1);
            const char *kw = "include";
            for (i32 k = 0; k < 7; k++)
            {
                if (j + k >= s.length() || s[j + k] != kw[k])
                    return -1;
            }
            return j + 7;
        }

        struct IncludeRef
        {
            TkString rawName;  // exactly as written between quotes/angles
        };

        // Walk `sourceText` once and return every line-start `#include "..."`
        // / `#include <...>` literal in document order. Comments and string
        // literals embedded in code are skipped only insofar as we honour
        // the `at-line-start` rule -- a full lexer would do more, but this
        // matches what real users expect from a dependency scanner.
        void collectIncludeRefs(TkStringView sourceText, TkArray<IncludeRef> &out)
        {
            const i32 length = sourceText.length();
            bool atLineStart = true;
            for (i32 i = 0; i < length; )
            {
                const char c = sourceText[i];

                if (c == '\n')
                {
                    atLineStart = true;
                    i++;
                    continue;
                }
                if (atLineStart && (c == ' ' || c == '\t'))
                {
                    i++;
                    continue;
                }

                if (atLineStart && c == '#')
                {
                    const i32 after = matchInclude(sourceText, i);
                    if (after >= 0)
                    {
                        i32 j = skipInlineSpace(sourceText, after);
                        if (j < length && (sourceText[j] == '"' || sourceText[j] == '<'))
                        {
                            const char closer = sourceText[j] == '"' ? '"' : '>';
                            const i32 startName = j + 1;
                            i32 endName = startName;
                            while (endName < length
                                   && sourceText[endName] != closer
                                   && sourceText[endName] != '\n')
                                endName++;
                            if (endName < length && sourceText[endName] == closer && endName > startName)
                            {
                                IncludeRef ref;
                                ref.rawName = TkString(sourceText.get() + startName, endName - startName);
                                out.add(ref);
                                i = endName + 1;
                                atLineStart = false;
                                continue;
                            }
                        }
                    }
                }

                atLineStart = false;
                i++;
            }
        }

        // Try to read the contents of `path` from disk. Falls back to
        // SourceManager so that virtual files (e.g. wrapped embedded GLSL)
        // also resolve correctly. The caller has already resolved `path`
        // through IncludeResolver, so we trust it here.
        bool readSourceForScan(
            SourceManager &sources,
            TkStringView resolvedPath,
            TkString &outContent)
        {
            // Prefer the SourceManager cache so we don't pay a second
            // filesystem read when the file is also being compiled.
            for (i32 i = 1; i < sources.getFileCount(); i++)
            {
                const SourceFile *f = sources.getFile(static_cast<u32>(i));
                if (f != nullptr && f->getPath() == TkString(resolvedPath))
                {
                    outContent = f->getContent();
                    return true;
                }
            }
            // Fall through to disk -- the scanner intentionally re-reads here
            // so that a freshly-edited include is reflected in the hash even
            // if no SourceManager entry exists yet.
            const TkString path(resolvedPath);
            OpenFileParams params{
                .path = path,
                .mode = FileMode::kOpenExisting,
                .access = FileAccess::kRead,
                .share = FileShare::kRead,
            };
            FileHandle h = TkFileSystem::openFile(params);
            if (!TkFileSystem::isFileOpened(h))
                return false;
            const bool ok = TkFileSystem::readAsText(h, outContent);
            TkFileSystem::closeFile(h);
            return ok;
        }
    }

    bool DependencyScanner::scanFlat(
        TkStringView sourceText,
        TkStringView anchorFile,
        const IncludeResolver &resolver,
        TkArray<TkString> &outResolvedPaths,
        TkArray<TkString> &outMissing) const
    {
        bool ok = true;
        TkArray<IncludeRef> refs;
        collectIncludeRefs(sourceText, refs);
        for (i32 i = 0; i < refs.count(); i++)
        {
            TkString resolved;
            if (resolver.resolve(TkStringView(refs[i].rawName), anchorFile, resolved))
            {
                bool already = false;
                for (i32 k = 0; k < outResolvedPaths.count(); k++)
                {
                    if (outResolvedPaths[k] == resolved)
                    {
                        already = true;
                        break;
                    }
                }
                if (!already)
                    outResolvedPaths.add(resolved);
            }
            else
            {
                outMissing.add(refs[i].rawName);
                ok = false;
            }
        }
        return ok;
    }

    DependencyScanResult DependencyScanner::scan(
        SourceManager &sourceManager,
        IncludeResolver &includeResolver,
        TkStringView primaryPath,
        TkStringView primaryContent) const
    {
        DependencyScanResult result;

        // Visited set keeps de-dup deterministic regardless of include order.
        TkArray<TkString> visited;
        // Stack tracks the current DFS path so we can spot cycles cheaply.
        TkArray<TkString> stack;

        auto scanOne = [&](auto &&self, const TkString &anchor, const TkString &content) -> void
        {
            TkArray<IncludeRef> refs;
            collectIncludeRefs(TkStringView(content), refs);

            for (i32 i = 0; i < refs.count(); i++)
            {
                TkString resolved;
                if (!includeResolver.resolve(
                        TkStringView(refs[i].rawName),
                        TkStringView(anchor),
                        resolved))
                {
                    // Record the missing include exactly once so the printer
                    // doesn't repeat the same line N times when several
                    // sources include the same missing header.
                    bool already = false;
                    for (i32 k = 0; k < result.missingIncludes.count(); k++)
                    {
                        if (result.missingIncludes[k] == refs[i].rawName)
                        {
                            already = true;
                            break;
                        }
                    }
                    if (!already)
                        result.missingIncludes.add(refs[i].rawName);
                    result.success = false;
                    continue;
                }

                // Cycle detection: if `resolved` is already on the DFS stack
                // we've looped back into a parent. Record the cycle chain
                // (stack contents joined with " -> ") then drop this branch.
                bool cycle = false;
                for (i32 s = 0; s < stack.count(); s++)
                {
                    if (stack[s] == resolved)
                    {
                        TkString chain;
                        for (i32 j = s; j < stack.count(); j++)
                        {
                            chain += stack[j];
                            chain += TkString(" -> ");
                        }
                        chain += resolved;
                        bool dupChain = false;
                        for (i32 k = 0; k < result.includeCycles.count(); k++)
                        {
                            if (result.includeCycles[k] == chain)
                            {
                                dupChain = true;
                                break;
                            }
                        }
                        if (!dupChain)
                            result.includeCycles.add(chain);
                        cycle = true;
                        result.success = false;
                        break;
                    }
                }
                if (cycle)
                    continue;

                // De-dup: if we already produced this resolved path, skip
                // descending into it again. The dependency entry is recorded
                // only once.
                bool alreadyVisited = false;
                for (i32 v = 0; v < visited.count(); v++)
                {
                    if (visited[v] == resolved)
                    {
                        alreadyVisited = true;
                        break;
                    }
                }
                if (alreadyVisited)
                    continue;
                visited.add(resolved);
                stack.add(resolved);

                // Read content + record dependency entry.
                TkString includeContent;
                if (!readSourceForScan(sourceManager, TkStringView(resolved), includeContent))
                {
                    // Resolver said it exists but we couldn't read it; treat
                    // as a missing-include diagnostic so the user knows.
                    result.missingIncludes.add(resolved);
                    result.success = false;
                    stack.removeAt(stack.count() - 1);
                    continue;
                }

                ShaderDependency dep;
                dep.resolvedPath = resolved;
                dep.contentHash = ShaderHash::hashString(TkStringView(includeContent));
                result.dependencies.add(dep);

                self(self, resolved, includeContent);
                stack.removeAt(stack.count() - 1);
            }
        };

        const TkString seedAnchor(primaryPath);
        const TkString seedContent(primaryContent);
        if (!seedAnchor.isEmpty())
            stack.add(seedAnchor);
        scanOne(scanOne, seedAnchor, seedContent);
        if (!seedAnchor.isEmpty())
        {
            TK_ASSERT(!stack.isEmpty());
            stack.removeAt(stack.count() - 1);
        }

        // Final dependency-hash mix. Sort first so two different scan orders
        // over the same set of (path, contentHash) pairs produce the same
        // hash; that's what makes the cache stable across rebuilds.
        // TkArray has no sort(); do an insertion sort on the small set.
        for (i32 i = 1; i < result.dependencies.count(); i++)
        {
            ShaderDependency tmp = result.dependencies[i];
            i32 j = i - 1;
            while (j >= 0 && result.dependencies[j].resolvedPath > tmp.resolvedPath)
            {
                result.dependencies[j + 1] = result.dependencies[j];
                j--;
            }
            result.dependencies[j + 1] = tmp;
        }

        RollingStableHasher hasher;
        hasher.begin();
        const u32 count = static_cast<u32>(result.dependencies.count());
        hasher.update(&count, sizeof(count));
        for (i32 i = 0; i < result.dependencies.count(); i++)
        {
            const ShaderDependency &d = result.dependencies[i];
            const u32 plen = static_cast<u32>(d.resolvedPath.length());
            hasher.update(&plen, sizeof(plen));
            if (plen > 0)
                hasher.update(d.resolvedPath.get(), plen);
            hasher.update(&d.contentHash.low, sizeof(d.contentHash.low));
            hasher.update(&d.contentHash.high, sizeof(d.contentHash.high));
        }
        result.dependencyHash.low = hasher.end64().getHashValue();
        result.dependencyHash.high = 0;
        return result;
    }
}
