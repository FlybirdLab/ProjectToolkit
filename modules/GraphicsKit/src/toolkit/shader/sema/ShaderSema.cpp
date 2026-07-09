/*
 * @Author: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include <toolkit/shader/sema/ShaderSema.hpp>
#include <toolkit/shader/diagnostics/DiagnosticSink.hpp>

namespace toolkit
{
    namespace
    {
        bool isKnownBackend(const TkString &name)
        {
            return name == "dx11" || name == "dx12"
                || name == "vulkan" || name == "opengl"
                || name == "metal";
        }

        // Profile strings the manifest is allowed to declare per backend.
        // These are advisory -- the actual GfxShaderProfile used to compile is
        // chosen by the CLI's --profile flag (see ShaderManifestCompiler's
        // manifestBackendFor), but validating the strings here catches typos
        // like "d3d_sm99" or "vk_sm9" at sema time instead of letting them
        // slip through silently. Keep in sync with ShaderManifestCompiler's
        // mapping and the GfxShaderProfile enum.
        bool isKnownProfileForBackend(const TkString &backend, const TkString &profile)
        {
            if (backend == "dx11")
                return profile == "d3d_sm4" || profile == "d3d_sm5";
            if (backend == "dx12")
                return profile == "dx_sm6";
            if (backend == "vulkan")
                return profile == "vk_sm5";
            if (backend == "opengl")
                return profile == "glsl_410" || profile == "glsl_440";
            if (backend == "metal")
                return profile == "msl_2_0" || profile == "msl_2_1" || profile == "msl_2_2";
            return false;
        }

        // Render-state value validity table. The first column is the state
        // name; the rest are accepted single-token values. We accept "Off" on
        // every state so users can disable a feature without thinking about
        // its specific value set.
        struct RenderStateRule
        {
            const char *name;
            const char *values[12];
        };

        constexpr RenderStateRule kRenderStateRules[] = {
            {"Blend",  {"Off", "Alpha", "Additive", "Multiply", "Opaque", nullptr}},
            {"Cull",   {"Off", "Front", "Back", "None", nullptr}},
            {"ZWrite", {"On", "Off", nullptr}},
            {"ZTest",  {"Less", "LessEqual", "Equal", "GreaterEqual", "Greater",
                       "NotEqual", "Always", "Never", "Off", nullptr}},
        };

        const RenderStateRule *findRule(const TkString &name)
        {
            for (const auto &r : kRenderStateRules)
            {
                if (name == r.name)
                    return &r;
            }
            return nullptr;
        }

        bool ruleAccepts(const RenderStateRule &rule, const TkString &value)
        {
            for (const char *const *p = rule.values; *p != nullptr; p++)
            {
                if (value == *p)
                    return true;
            }
            return false;
        }

        bool languageWorksForBackend(ShaderSourceLanguage lang, const TkString &backend)
        {
            switch (lang)
            {
            case ShaderSourceLanguage::GLSL:
                return backend == "opengl";
            case ShaderSourceLanguage::HLSL:
                return backend == "dx11" || backend == "dx12" || backend == "vulkan";
            case ShaderSourceLanguage::MSL:
                return backend == "metal";
            default:
                return false;
            }
        }
    }

    ShaderSema::ShaderSema(DiagnosticSink *sink)
        : m_sink(sink)
    {
    }

    void ShaderSema::error(const SourceRange &range, TkStringView code, TkStringView message)
    {
        if (m_sink != nullptr)
            m_sink->error(range, code, message);
    }

    void ShaderSema::error(const SourceRange &range, TkStringView code, const TkString &message)
    {
        error(range, code, TkStringView(message));
    }

    void ShaderSema::warning(const SourceRange &range, TkStringView code, TkStringView message)
    {
        if (m_sink != nullptr)
            m_sink->warning(range, code, message);
    }

    void ShaderSema::warning(const SourceRange &range, TkStringView code, const TkString &message)
    {
        warning(range, code, TkStringView(message));
    }

    bool ShaderSema::validate(const ShaderDecl &shader)
    {
        bool ok = true;

        if (shader.name.isEmpty())
        {
            error(shader.range, TkStringView("TKSHD1001"),
                  TkStringView("shader must declare a non-empty name"));
            ok = false;
        }

        if (shader.subShaders.isEmpty())
        {
            error(shader.range, TkStringView("TKSHD1001"),
                  TkStringView("shader must declare at least one SubShader"));
            ok = false;
        }

        for (i32 i = 0; i < shader.subShaders.count(); i++)
        {
            if (!validateSubShader(shader.subShaders[i], i))
                ok = false;
        }
        return ok;
    }

    bool ShaderSema::validateSubShader(const SubShaderDecl &sub, i32 index)
    {
        bool ok = true;

        if (!sub.hasTarget || sub.target.entries.isEmpty())
        {
            error(sub.range, TkStringView("TKSHD1006"),
                  TkString::format("SubShader %i is missing a Target block", index));
            ok = false;
        }
        else if (!validateTarget(sub.target))
        {
            ok = false;
        }

        if (sub.passes.isEmpty())
        {
            error(sub.range, TkStringView("TKSHD1005"),
                  TkString::format("SubShader %i must contain at least one Pass", index));
            ok = false;
        }

        for (i32 i = 0; i < sub.passes.count(); i++)
        {
            if (!validatePass(sub.passes[i], index, i))
                ok = false;
            if (sub.hasTarget && !checkLanguageMatchesTarget(sub.passes[i], sub.target))
                ok = false;
        }
        return ok;
    }

    bool ShaderSema::validateTarget(const TargetDecl &target)
    {
        bool ok = true;
        for (i32 i = 0; i < target.entries.count(); i++)
        {
            const TargetEntryDecl &entry = target.entries[i];
            if (!isKnownBackend(entry.backend))
            {
                error(entry.range, TkStringView("TKSHD1006"),
                      TkString::format("unknown backend '%s' (expected dx11|dx12|vulkan|opengl|metal)",
                                       entry.backend.get()));
                ok = false;
            }
            if (entry.profile.isEmpty())
            {
                error(entry.range, TkStringView("TKSHD1006"),
                      TkString::format("backend '%s' has empty profile string", entry.backend.get()));
                ok = false;
            }
            else if (isKnownBackend(entry.backend) && !isKnownProfileForBackend(entry.backend, entry.profile))
            {
                // Profile string is parsed-only today, but mis-typed strings
                // are a common error and worth surfacing here so users don't
                // wonder why a SubShader matches the backend yet still
                // produces unexpected output.
                warning(entry.range, TkStringView("TKSHD1006"),
                        TkString::format(
                            "backend '%s' has unrecognized profile '%s' "
                            "(expected e.g. d3d_sm5 / dx_sm6 / vk_sm5 / glsl_410)",
                            entry.backend.get(), entry.profile.get()));
            }
        }
        return ok;
    }

    bool ShaderSema::validatePass(const PassDecl &pass, i32 subIndex, i32 passIndex)
    {
        bool ok = true;

        // Either external Vertex/Pixel pair OR at least one embedded source.
        bool hasVertex = false;
        bool hasPixel = false;
        for (i32 i = 0; i < pass.stages.count(); i++)
        {
            const StageDecl &s = pass.stages[i];
            if (s.stage == ShaderStage::kVertex)
            {
                if (hasVertex)
                {
                    error(s.range, TkStringView("TKSHD1010"),
                          TkStringView("duplicate Vertex stage declaration"));
                    ok = false;
                }
                hasVertex = true;
            }
            else if (s.stage == ShaderStage::kPixel)
            {
                if (hasPixel)
                {
                    error(s.range, TkStringView("TKSHD1010"),
                          TkStringView("duplicate Pixel stage declaration"));
                    ok = false;
                }
                hasPixel = true;
            }
        }

        const bool hasExternalPair = hasVertex && hasPixel;
        const bool hasEmbedded = !pass.embeddedSources.isEmpty();
        if (!hasExternalPair && !hasEmbedded)
        {
            error(pass.range, TkStringView("TKSHD1008"),
                  TkString::format(
                      "Pass '%s' (SubShader %i) must declare Vertex+Pixel stages or an embedded source block",
                      pass.name.isEmpty() ? "<unnamed>" : pass.name.get(), subIndex));
            ok = false;
        }
        (void)passIndex;

        if (!validateRenderStates(pass))
            ok = false;

        return ok;
    }

    bool ShaderSema::validateRenderStates(const PassDecl &pass)
    {
        bool ok = true;
        for (i32 i = 0; i < pass.renderStates.count(); i++)
        {
            const RenderStateDecl &rs = pass.renderStates[i];
            const RenderStateRule *rule = findRule(rs.name);
            if (rule == nullptr)
            {
                error(rs.range, TkStringView("TKSHD1007"),
                      TkString::format("unknown render state '%s'", rs.name.get()));
                ok = false;
                continue;
            }
            if (!ruleAccepts(*rule, rs.value))
            {
                error(rs.range, TkStringView("TKSHD1007"),
                      TkString::format("invalid value '%s' for render state '%s'",
                                       rs.value.get(), rs.name.get()));
                ok = false;
            }
        }
        return ok;
    }

    bool ShaderSema::checkLanguageMatchesTarget(const PassDecl &pass, const TargetDecl &target)
    {
        if (pass.embeddedSources.isEmpty())
            return true;

        bool ok = true;
        for (i32 i = 0; i < pass.embeddedSources.count(); i++)
        {
            const EmbeddedSourceDecl &emb = pass.embeddedSources[i];
            // We require at least one Target entry to be compatible with the
            // embedded language. Per-backend selection happens at compile time.
            bool any = false;
            for (i32 t = 0; t < target.entries.count(); t++)
            {
                if (languageWorksForBackend(emb.language, target.entries[t].backend))
                {
                    any = true;
                    break;
                }
            }
            if (!any)
            {
                error(emb.range, TkStringView("TKSHD1009"),
                      TkString::format("embedded %s block has no compatible Target backend",
                                       toString(emb.language)));
                ok = false;
            }
        }
        return ok;
    }
}
