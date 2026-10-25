// OmniaFramework - built-in CRT monitor effect (ogfx::CRTShader).
// Copyright (C) 2026  OmniaX-Dev - GPLv3, see LICENSE.
//
// Applied while drawing a low-resolution image (e.g. a PixelCanvas) stretched over a larger
// area: the texture coordinates must cover the whole texture (0..1), and the texture should
// use linear filtering: it is sampled "sharp bilinear" (crisp pixels, but evenly sized at
// non-integer scales, and smooth under the curvature). Simulates the curved
// glass, scanlines between the source rows, an aperture grille, a vignette, a slight
// convergence error (chromatic aberration), flicker and noise. The area outside the curved
// screen comes out opaque black, with softly rounded corners.
// See blur.frag.hlsl for the general interface of an ogfx fragment shader.

cbuffer CRTParams : register(b0, space3)
{
    float2 source_size;         // texture size in pixels (the emulated resolution)
    float2 output_size;         // size of the drawn area in screen pixels
    float  curvature;           // barrel distortion, 0 = flat
    float  scanline_intensity;  // 0..1, darkening between source rows
    float  mask_intensity;      // 0..1, RGB aperture grille
    float  vignette;            // 0..1, darkening towards the edges
    float  chromatic;           // convergence error, in source pixels
    float  flicker;             // 0..1, brightness flicker amplitude
    float  noise;               // 0..1, animated noise amount
    float  brightness;          // overall gain (compensates for the darkening)
    float  time;                // seconds, animates flicker and noise
    float  corner_radius;       // rounded screen corners, fraction of the screen size
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

static const float PI = 3.14159265;

float2 warp(float2 uv)
{
    float2 c = uv * 2.0 - 1.0;
    c *= 1.0 + curvature * (c.yx * c.yx);
    return c * 0.5 + 0.5;
}

// Sharp bilinear: within each source pixel the coordinate snaps to the pixel's center, and
// only the last output pixel before the next source pixel is interpolated.
float2 sharp_uv(float2 uv)
{
    const float2 scale = max(output_size / source_size, float2(1.0, 1.0));
    const float2 texel = uv * source_size;
    const float2 base = floor(texel);
    const float2 offset = frac(texel) - 0.5;
    const float2 range = 0.5 - 0.5 / scale;
    const float2 f = (offset - clamp(offset, -range, range)) * scale + 0.5;
    return (base + f) / source_size;
}

float hash(float2 p)
{
    p = frac(p * float2(123.34, 456.21));
    p += dot(p, p + 45.32);
    return frac(p.x * p.y);
}

PSOutput main(PSInput input)
{
    PSOutput output;
    const float2 uv = warp(input.v_uv);

    // Outside the glass: black, with a soft rounded edge.
    const float2 centered = abs(uv * 2.0 - 1.0);
    const float2 corner = max(centered - (1.0 - corner_radius), 0.0) / max(corner_radius, 1e-4);
    const float2 pixel = 2.0 / max(output_size, float2(1.0, 1.0));
    float edge = 1.0;
    if (corner_radius > 0.0)
        edge = saturate((1.0 - length(corner)) * corner_radius / max(pixel.x, pixel.y));
    edge *= saturate((1.0 - centered.x) / pixel.x) * saturate((1.0 - centered.y) / pixel.y);
    if (edge <= 0.0)
    {
        output.o_color = float4(0.0, 0.0, 0.0, 1.0) * input.v_color;
        return output;
    }

    // Convergence error: red and blue sampled slightly apart, horizontally.
    const float2 shift = float2(chromatic / source_size.x, 0.0);
    const float4 center = u_texture.Sample(u_sampler, sharp_uv(uv));
    float3 color;
    color.r = u_texture.Sample(u_sampler, sharp_uv(uv + shift)).r;
    color.g = center.g;
    color.b = u_texture.Sample(u_sampler, sharp_uv(uv - shift)).b;
    const float alpha = center.a;

    // Scanlines: dark between source rows. Faded out when a row is drawn less than ~2 pixels
    // tall, where they would only alias.
    const float rows_per_pixel = output_size.y / source_size.y;
    const float scan_strength = scanline_intensity * saturate(rows_per_pixel - 1.0);
    const float d = abs(frac(uv.y * source_size.y) - 0.5) * 2.0;
    color *= 1.0 - scan_strength * d * d;

    // Aperture grille: R, G and B stripes, one screen pixel each.
    const float stripe = fmod(floor(input.v_uv.x * output_size.x), 3.0);
    float3 mask = float3(1.0 - mask_intensity, 1.0 - mask_intensity, 1.0 - mask_intensity);
    if (stripe < 0.5)
        mask.r = 1.0;
    else if (stripe < 1.5)
        mask.g = 1.0;
    else
        mask.b = 1.0;
    color *= mask;

    // Vignette, flicker and noise.
    const float v = 16.0 * uv.x * uv.y * (1.0 - uv.x) * (1.0 - uv.y);
    color *= lerp(1.0, pow(saturate(v), 0.35), vignette);
    color *= 1.0 - flicker * (0.5 + 0.5 * sin(time * 2.0 * PI * 8.0));
    color += (hash(input.v_uv * output_size + frac(time) * 100.0) - 0.5) * noise;

    color = max(color * brightness, 0.0) * edge;
    output.o_color = float4(color, lerp(1.0, alpha, edge)) * input.v_color;
    return output;
}
