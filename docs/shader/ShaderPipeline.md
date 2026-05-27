# Shader Pipeline — state of `.tkshader` / `.tksd` / job cache

Snapshot of where the shader compiler stack is as of Phase 10 verification. The
goal of this document is to make it obvious which seams are load-bearing, which
are stubs, and which knobs you can turn during testing without breaking the
data flow.

## 1. Layered overview

```
.tkshader source
    │
    ▼
lexer / parser / sema                  modules/GraphicsKit/src/toolkit/shader/{lexer,parser,sema}/
    │   produces ShaderDecl
    ▼
ShaderManifestCompiler                 modules/GraphicsKit/src/toolkit/shader/manifest/
    │   iterates (pass × profile)
    │   resolves source / entry / includes
    │   runs DependencyScanner for cache key + asset closure
    ▼
ShaderJobCache (v2)                    modules/GraphicsKit/src/toolkit/shader/cache/
    ├── HIT  → CachedShaderJobResult (bytecode + reflection + deps)
    └── MISS → runs ShadersCompilation::compile, then stores the result
    │
    ▼
ShadersCompilation                     modules/GraphicsKit/src/toolkit/shader/core/Compilation.cpp
    │   creates ShaderCompilationContext
    │   dispatches to the right backend
    ▼
backend (one of)                       modules/GraphicsKit/src/toolkit/shader/backends/
    ├── ShaderCompilerD3D     — fxc / D3DCompiler, SM4 + SM5
    ├── ShaderCompilerDX      — DXC, SM6
    ├── ShaderCompilerVulkan  — glslang HLSL → SPIR-V
    └── ShaderCompilerGLSL    — glslang HLSL → SPIR-V → SPIRV-Cross → GLSL 410
                                  (also accepts native GLSL passthrough)
    │
    ▼
ShaderCompilationContext (per-compile workspace)
    │   .output       MemoryOStream (legacy `.shadercache`-wire bytes)
    │   .reflectionBindings / reflectionVertexInputs (written by backends)
    ▼
ShadersCompilation copies context.reflection → options.reflection
    │
    ▼
ShaderManifestCompiler folds bytecode + reflection + deps into ShaderAsset
    │
    ▼
TksdWriter                             modules/GraphicsKit/src/toolkit/shader/format/
    │   .tksd container with chunked metadata + bytecode
    ▼
Runtime: GfxShaderAssetLoader → GfxShaderAsset → LegacyShaderCacheBuilder → GfxShader
```

`TkGraphics` (the runtime library) only links `format/`, `common.hpp`, and the
runtime-facing bits — it does not pull in glslang, DXC, SPIRV-Cross or any
backend. `TkShaderCompilerCore` is the offline-tool library that owns the
backends and the manifest compiler. Both are built from the same source tree;
[`modules/GraphicsKit/CMakeLists.txt`](../../modules/GraphicsKit/CMakeLists.txt)
does the split via a `list(FILTER ... EXCLUDE REGEX ...)`.

## 2. `.tkshader` source language

First-version grammar accepts:

- `Shader "name" { ... }`
- `Properties { Name(Type, "label") = default; ... }`
- `Tags { key = value }`
- `Variants { NAME = [0, 1] }` — parsed; **expansion is a v1 stub** (see
  [`compilegraph/VariantExpander.cpp`](../../modules/GraphicsKit/src/toolkit/shader/compilegraph/VariantExpander.cpp)).
  ShaderManifestCompiler emits a `TKSHD0011` warning when the manifest declares
  variants; it then compiles the all-off permutation only.
- `SubShader { Target { ... } Pass { ... } ... }`
- `Target { dx11 = "d3d_sm5" dx12 = "dx_sm6" vulkan = "vk_sm5" opengl = "glsl_410" }`
  — the **backend name** drives SubShader pickability; the **profile string**
  is advisory (CLI's `--profile` decides what actually compiles), but
  `ShaderSema::isKnownProfileForBackend` validates it to catch typos.
- `Pass "name" { Cull / ZWrite / ZTest / Blend ... Vertex { Source / Entry } Pixel { Source / Entry } }`
- `HLSLPROGRAM ... ENDHLSL`, `GLSLPROGRAM ... ENDGLSL`, `MSLPROGRAM ... ENDMSL`
  — embedded source. **Embedded HLSL is not supported in v1**
  (`TKSHD1009` error from manifest compiler); embedded GLSL works but is
  off the recommended path now that HLSL → GLSL cross-compile is wired.

Recommended authoring shape (single HLSL source, all four backends):

```
Shader "Sandbox/Triangle"
{
    SubShader
    {
        Target { dx11 = "d3d_sm5" dx12 = "dx_sm6" vulkan = "vk_sm5" opengl = "glsl_410" }
        Pass "Forward"
        {
            Cull Off; ZWrite On; ZTest LessEqual;
            Vertex { Source = "Triangle.hlsl"; Entry = "VSMain"; }
            Pixel  { Source = "Triangle.hlsl"; Entry = "PSMain"; }
        }
    }
}
```

Constraints worth knowing:

1. Stages in one Pass must share a single Source file (first-version policy).
2. PrebuiltShaderEntry (`Vertex { Entry = "..." }`) is now the **only** way the
   compiler discovers function names — the legacy `META_VS` / `META_PS` token
   scanner was removed; manifests without explicit `Entry = ...` get a
   `TKSHD_PARSE_FAILED` fail-fast diagnostic.

## 3. `.tksd` container

Binary layout in [`format/TksdFormat.hpp`](../../modules/GraphicsKit/src/toolkit/shader/format/TksdFormat.hpp).
Header → chunk table → 8-byte-aligned chunk payloads. Chunks currently emitted:

| Chunk          | Contents                                                                                                  |
|----------------|-----------------------------------------------------------------------------------------------------------|
| Metadata       | asset name, pass/stage counts (advisory)                                                                  |
| Properties     | `(type, name, default)` triples from manifest                                                             |
| Tags           | global `(key, value)` tags                                                                                |
| Passes         | per-pass `(name, tags, renderStates, stageIndices[])`                                                     |
| StageBytecode  | per `(pass, profile)`: `(kind = TKSD_STAGE_KIND_RAW, stage, profile, entry, bytes...)`                    |
| Bindings       | reflection: `(name, slot, stage, type)` — type tag is `"CB"` / `"SRV"` / `"UAV"` / `"Sampler"`            |
| VertexInput    | reflection: `(semantic, location, format, slot, offset)` — VS stage inputs only                           |
| Dependencies   | resolved absolute paths the asset was built against                                                       |

### Transitional notes

- `TKSD_STAGE_KIND_RAW` is the only stage kind today. The payload is **still**
  a self-contained `.shadercache`-wire-format blob covering every stage of the
  pass — it goes through `LegacyShaderCacheBuilder` at runtime to land in
  `GfxShader::create(MemoryIStream&)`. The kind tag is a u32 so a future
  per-stage split can reserve a second value without a format-major bump.
- `LegacyShaderCacheBuilder` is an identity pass in v1.
- Reflection is populated **only** by the OpenGL backend (`ShaderCompilerGLSL`)
  via SPIRV-Cross. dx11 / dx12 / vulkan currently write 0 bindings / 0 vertex
  inputs — true cross-backend reflection unification is Phase 11+.
- The Dependencies chunk stores paths only. A future schema bump can extend
  each entry to `(path, contentHash, sourceKind)` for incremental builds.
- Hash slots in the header (`sourceHash` / `dependencyHash` / `compilerHash` /
  `reflectionHash` / `payloadHash`) are reserved but zero in v1 — Phase 11
  drops real hashes in for deterministic-build verification.

Reader is in [`format/TksdReader.cpp`](../../modules/GraphicsKit/src/toolkit/shader/format/TksdReader.cpp).
Unknown chunks are silently skipped (the format guarantee), so a v1 reader can
ingest a v1.x file even if new chunks land later.

## 4. Job cache

On-disk: `<cache-dir>/<32-hex-hash>.jobcache`. File format in
[`cache/ShaderJobCache.cpp`](../../modules/GraphicsKit/src/toolkit/shader/cache/ShaderJobCache.cpp);
hash function in [`cache/ShaderHash.cpp`](../../modules/GraphicsKit/src/toolkit/shader/cache/ShaderHash.cpp).

Version 2 payload (current):

```
u32 magic 'SJCB'
u32 version = 2
u64 hashLow, u64 hashHigh
u32 blobSize, byte[blobSize]
u32 bindingCount, { str name, u32 slot, u32 stage, str type } × N
u32 vertexInputCount, { str semantic, u32 location, u32 format, u32 slot, u32 offset } × N
u32 dependencyCount, { str path } × N
```

v1 files (blob-only) silently MISS — they get overwritten on the next miss.

The hash mixes everything that influences output bytecode:

- shader name, pass name, stage, profile, entry name, variant key
- source path, virtual source name
- source text (post-include expansion)
- defines, include dirs
- dependency closure (paths + content hashes from `DependencyScanner`)
- compiler version tag

Editing any included header therefore invalidates the cache. Editing a comment
in the primary source still rewrites the hash; that is by design (any byte
change in source = miss).

**Hit/miss invariant:** `tryLoad → CachedShaderJobResult` always carries
bytecode AND reflection AND deps. Cache hit produces a `.tksd` with the same
Bindings / VertexInput / Dependencies chunks as a fresh compile against the
same source. The earlier v1 bug — where hit emitted empty reflection chunks —
no longer reproduces.

Verbose logging (`--show-stats` on the CLI) prints:

```
[ShaderJobCache] tryLoad job='Sandbox/Triangle/Forward' profile=4 stage=0 hash=...
[ShaderJobCache] tryLoad path='.shader-cache/....jobcache'
[ShaderJobCache] tryLoad: HIT  (12345 bytes blob + 2 bindings + 1 vertex inputs + 3 deps)
[ShaderJobCache] store:   OK   (16384 bytes written)
```

## 5. Backend reflection: data flow

Backends never touch `options` mutably. The data flow is one-way:

```
SPIRV-Cross get_shader_resources()
  → ShaderCompilationContext.reflectionBindings / reflectionVertexInputs   (during compile)
  → ShadersCompilation::compile copies context → options.reflection        (post backend)
  → ShaderManifestCompiler reads options.reflection, mirrors into          (per cached entry)
    CachedShaderJobResult.reflectionBindings / reflectionVertexInputs
  → ShaderJobCache.store writes it to disk; tryLoad pulls it back out
  → ShaderAsset.bindings / vertexInputs (deduped union across pass × profile)
  → TksdWriter Bindings / VertexInput chunks
  → GfxShaderAsset.bindings() / vertexInputs() at runtime
```

The const_cast back-channel that an earlier prototype used is gone — backends
write to context (their owned workspace), the compile entry point copies into
options after the backend returns. Adding reflection to the DX / Vulkan
backends is a matter of plumbing the same `(context.reflectionBindings,
context.reflectionVertexInputs)` outputs from those compile paths; no surface
changes needed elsewhere.

## 6. Runtime side

[`format/GfxShaderAsset.hpp`](../../modules/GraphicsKit/src/toolkit/shader/format/GfxShaderAsset.hpp)
parses a `.tksd` and hands out `GfxShader` instances per pass.

```cpp
GfxShaderAsset asset;
asset.loadFromFile("Triangle.tksd");
GfxShader *shader = asset.createShaderForDefaultPass(device); // nullptr on failure
asset.bindings();        // editor-side ABI lookup
asset.vertexInputs();    // editor-side vertex layout hints
asset.dependencies();    // file-watcher hookup
```

`createShaderForPass` now returns `nullptr` on failure (releasing the
half-initialized `GfxShader` via `GFX_DEL_RESOURCE` internally). Earlier
versions returned the broken object with "caller is responsible" — that
contract was easy to miss; callers now get a clean nullptr-or-valid result.

For convenience there is also `GfxShaderAssetLoader::loadShaderFromTksd(...)`
which folds the parse + per-pass shader creation into one call.

## 7. CLI

`tools/ShaderCompiler` is the offline driver. Common invocations:

```powershell
# Compile one manifest for all four backends, with verbose cache stats.
.\build\tools\ShaderCompiler\ShaderCompiler.exe `
    --file .\tools\Sandbox\shaders\Triangle.tkshader `
    --backend dx11,dx12,vulkan,opengl `
    --out shader-output `
    --cache-dir .shader-cache `
    --show-stats

# Just OpenGL, dump the intermediate GLSL SPIRV-Cross emitted.
$env:TK_SHADER_DUMP_INTERMEDIATE = "E:\Projects\ToolkitProject\shader-dump"
.\build\tools\ShaderCompiler\ShaderCompiler.exe `
    --file .\tools\Sandbox\shaders\Triangle.tkshader `
    --backend opengl `
    --out shader-output `
    --cache-dir .shader-cache `
    --show-stats

# Verify cache hit / miss is metadata-consistent.
Remove-Item -Recurse -Force .\.shader-cache -ErrorAction SilentlyContinue
# first run -> miss + store
.\build\tools\ShaderCompiler\ShaderCompiler.exe --file ... --show-stats
# second run -> hit, same .tksd content
.\build\tools\ShaderCompiler\ShaderCompiler.exe --file ... --show-stats
```

## 8. What is NOT done

Items punted on purpose — these are not bugs, they're known scope deferrals.

- **Variants expansion.** Manifest parses `Variants { ... }` but only the
  all-off permutation is compiled. `TKSHD0011` warns the user.
- **Per-stage bytecode in `.tksd`.** `TKSD_STAGE_KIND_RAW` payload is still a
  per-pass legacy bundle. Splitting it touches the four backends + the runtime
  builder; the chunk kind tag is reserved so the format-major doesn't move.
- **Cross-backend reflection.** Only OpenGL / SPIRV-Cross fills the reflection
  arrays today. DX / Vulkan need their own extraction paths (DXC reflection
  API, glslang SPIR-V reflection on the Vulkan side).
- **Dependency content hashes in `.tksd`.** v1 stores paths only. The cache
  key already mixes content hashes, but the on-disk asset doesn't surface them
  yet.
- **Real parallelism in the compile graph.** `ShaderCompileGraph` /
  `CompilerScheduler` exist as data structures and a single-threaded driver;
  job-level parallelism is a Phase 12+ task.
- **LSP / VSCode plugin.** The diagnostics infrastructure (DiagnosticSink,
  SourceManager, source ranges) is shaped to support an LSP, but no protocol
  server is wired up yet.

## 9. File map cheat sheet

```
modules/GraphicsKit/src/toolkit/shader/
├── common.hpp                  ShaderStage / ShaderMacro / GfxShaderProfile / GfxShaderFlags
├── core/
│   ├── common.hpp              ShaderCompilationOptions, PrebuiltShaderEntry
│   ├── Context.hpp             ShaderCompilationContext (per-compile workspace)
│   ├── Compiler.{hpp,cpp}      ShaderCompiler base + include resolution
│   ├── Compilation.{hpp,cpp}   ShadersCompilation entry point + compiler pool
│   ├── ShaderMeta.hpp          VertexShaderMeta / PixelShaderMeta / ... permutations
│   ├── DebugDataExporter.hpp   optional Shaders/Debug/*.txt dump
│   └── environment.hpp         INPUT_LAYOUT_ELEMENT_* constants
├── backends/
│   ├── ShaderCompilerD3D.*     fxc / D3DCompiler — SM4 + SM5
│   ├── ShaderCompilerDX.*      DXC — SM6
│   ├── ShaderCompilerVulkan.*  glslang HLSL → SPIR-V
│   └── ShaderCompilerGLSL.*    glslang HLSL → SPIR-V → SPIRV-Cross → GLSL 410
├── source/                     SourceManager / SourceFile / IncludeResolver
├── diagnostics/                DiagnosticSink + Text/Json printers + TKSHD codes
├── lexer/, parser/, sema/      .tkshader frontend
├── manifest/                   ShaderManifestCompiler — orchestrator
├── compilegraph/               DependencyScanner, ShaderCompileJob, VariantExpander (stub)
├── cache/                      ShaderJobCache (v2), ShaderHash (XXH3 rolling)
└── format/                     .tksd reader/writer + ShaderAsset + LegacyShaderCacheBuilder
                                + GfxShaderAsset/GfxShaderAssetLoader (runtime entry points)
```
