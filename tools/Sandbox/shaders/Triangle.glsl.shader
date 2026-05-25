#version 410 core

// ============================================================================
// Triangle.glsl.shader
//
// Sandbox triangle, GLSL 410 Core variant.
//
// This file is consumed by:
//   1. The ShaderCompiler `Parser` to discover META_VS / META_PS markers and
//      produce a 2-stage .shadercache entry.
//   2. The OpenGL runtime, which compiles the same source twice -- once as VS
//      and once as PS. ShaderCompilerGLSL injects TK_SHADER_STAGE_VERTEX (or
//      TK_SHADER_STAGE_PIXEL) right after this `#version` directive so the
//      two compilations diverge via the #ifdef gates below.
//
// The META_VS / META_PS preprocessor stubs below are no-ops for GLSL; they
// only exist so the GLSL compiler doesn't choke on the markers the Parser
// needs to see.
// ============================================================================

#define META_VS(visible, level)
#define META_PS(visible, level)
#define META_CB_BEGIN(slot, name)
#define META_CB_END
#define META_FLAG(flag)

META_VS(true, FEATURE_LEVEL_SM5)
void VSMain() {}

META_PS(true, FEATURE_LEVEL_SM5)
void PSMain() {}

#ifdef TK_SHADER_STAGE_VERTEX

out vec4 vColor;

const vec2 kPositions[3] = vec2[3](
    vec2( 0.00,  0.65),
    vec2( 0.65, -0.55),
    vec2(-0.65, -0.55)
);

const vec4 kColors[3] = vec4[3](
    vec4(1.00, 0.12, 0.10, 1.0),
    vec4(0.10, 0.80, 0.25, 1.0),
    vec4(0.15, 0.35, 1.00, 1.0)
);

void main()
{
    gl_Position = vec4(kPositions[gl_VertexID], 0.0, 1.0);
    vColor      = kColors[gl_VertexID];
}

#endif

#ifdef TK_SHADER_STAGE_PIXEL

in vec4 vColor;
out vec4 FragColor;

void main()
{
    FragColor = vColor;
}

#endif
