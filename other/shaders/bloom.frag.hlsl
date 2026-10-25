// OmniaFramework - built-in bloom passes (ogfx::BloomEffect).
// Copyright (C) 2026  OmniaX-Dev - GPLv3, see LICENSE.
//
// Two modes, both working on premultiplied-alpha render target contents:
//   mode 0 (extract):   keeps the parts brighter than `threshold`, rescaled to 0..1. The
//                       result is then blurred by BasicRenderer2D::blurRenderTarget().
//   mode 1 (composite): outputs the (blurred) glow * intensity with alpha 0. Drawn with the
//                       premultiplied blend mode that render targets use
//                       (dst = src + dst * (1 - src.a)), alpha 0 turns it into an additive blend.
// See blur.frag.hlsl for the general interface of an ogfx fragment shader.

cbuffer BloomParams : register(b0, space3)
{
    float threshold;  // 0..1, brightness where the glow starts
    float intensity;  // glow strength (composite)
    float mode;       // 0 = extract, 1 = composite
    float padding;
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

PSOutput main(PSInput input)
{
    PSOutput output;
    const float4 c = u_texture.Sample(u_sampler, input.v_uv);
    if (mode < 0.5)
    {
        const float brightest = max(c.r, max(c.g, c.b));
        const float w = saturate((brightest - threshold) / max(1.0 - threshold, 1e-4));
        output.o_color = c * w * input.v_color;
    }
    else
    {
        output.o_color = float4(c.rgb * intensity, 0.0) * input.v_color;
    }
    return output;
}
