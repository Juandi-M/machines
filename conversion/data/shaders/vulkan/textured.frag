#version 450
// Mirrors 2DShading.fggls; also used by billboards.
layout(location = 0) in vec2 UV;
layout(location = 1) in vec4 fragmentColor;
layout(set = 0, binding = 1) uniform sampler2D textureSampler;
layout(location = 0) out vec4 color;
void main() { color = texture(textureSampler, UV) * fragmentColor; }
