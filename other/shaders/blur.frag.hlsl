// OmniaFramework - built-in separable Gaussian blur (one direction per pass).
// Copyright (C) 2026  OmniaX-Dev - GPLv3, see LICENSE.
//
// ---------------------------------------------------------------------------
// Interface of a custom fragment shader for SDL's GPU renderer (SDL_GPURenderState),
// which ogfx::Shader uses. The vertex shader and vertex format are SDL's own:
//   - inputs:   COLOR0    float4  vertex color   (SPIR-V location 0)
//               TEXCOORD0 float2  texture coords (SPIR-V location 1)
//   - texture:  the draw's texture + sampler at t0/s0 in space2 (SPIR-V set 2, binding 0)
//   - uniforms: cbuffers in space3 (SPIR-V set 3); the register index is the slot passed
//               to ogfx::Shader::setUniforms(data, slot). No slot is reserved by SDL.
//
// Every ogfx draw is textured: flat shapes sample the opaque white texel of the glyph
// atlas, text samples the glyph atlas, images/targets their own texture. So a custom
// shader must output (texture sample) * (vertex color), like SDL's default shader does -
// otherwise shapes and text drawn while the shader is active come out wrong.
//
// Render target contents are premultiplied alpha; blurring premultiplied colors is what
// avoids dark halos, and the result stays premultiplied.
// ---------------------------------------------------------------------------

cbuffer BlurParams : register(b0, space3)
{
    float2 texel_size;  // 1 / source texture size, in pixels
    float2 direction;   // (1, 0) horizontal pass, (0, 1) vertical pass
    float  sigma;       // Gaussian standard deviation, in source pixels
    int    radius;      // taps on each side of the center (clamped to MAX_RADIUS)
    float2 padding;
};

Texture2D u_texture : register(t0, space2);
SamplerState u_sampler : register(s0, space2);

struct PSInput
{
    float4 v_color : COLOR0;
    float2 v_uv : TEXCOORD0;
};

struct PSOutput
{
    float4 o_color : SV_Target;
};

static const int MAX_RADIUS = 96;

PSOutput main(PSInput input)
{
    PSOutput output;
    float4 sum = u_texture.Sample(u_sampler, input.v_uv);
    float total = 1.0;
    int taps = min(radius, MAX_RADIUS);
    if (taps > 0 && sigma > 0.0)
    {
        float k = -0.5 / (sigma * sigma);
        // Two neighbouring taps per sample, using the bilinear filter to weigh them
        // (halves the number of texture reads).
        [loop]
        for (int i = 1; i <= taps; i += 2)
        {
            float x1 = float(i);
            float x2 = float(i + 1);
            float w1 = exp(k * x1 * x1);
            float w2 = (i + 1 <= taps) ? exp(k * x2 * x2) : 0.0;
            float w = w1 + w2;
            float offset = (x1 * w1 + x2 * w2) / w;
            float2 d = direction * texel_size * offset;
            sum += (u_texture.Sample(u_sampler, input.v_uv + d) + u_texture.Sample(u_sampler, input.v_uv - d)) * w;
            total += 2.0 * w;
        }
    }
    output.o_color = (sum / total) * input.v_color;
    return output;
}
