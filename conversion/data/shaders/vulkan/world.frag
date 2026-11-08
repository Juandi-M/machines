#version 450
layout(location = 0) in vec2 UV;
layout(location = 1) in vec4 fragmentColor;
layout(location = 2) in vec4 viewSpace;
layout(location = 3) in vec4 fragmentFogColour;
layout(location = 4) in vec3 fragmentFogParams;
layout(set = 0, binding = 1) uniform sampler2D textureSampler;
layout(location = 0) out vec4 color;
void main()
{
    // Keep the original 4D length and alpha-preserving fog equation.
    float dist = length(viewSpace);
    float fogStart = fragmentFogParams.x;
    float fogEnd = fragmentFogParams.y;
    float fogFactor = 0.0;
    if (fogEnd > 0.0) fogFactor = clamp((fogStart - dist) / (fogStart - fogEnd), 0.0, 1.0);
    color = texture(textureSampler, UV) * fragmentColor;
    float alpha = color.a;
    color = color * (1.0 - fogFactor) + fragmentFogColour * fogFactor;
    color.a = alpha;
}
