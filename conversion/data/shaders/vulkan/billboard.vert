#version 450
layout(location = 0) in vec4 vertexPosition_Billboard;
layout(location = 1) in vec2 vertexUV;
layout(location = 2) in vec4 vertexColor;
layout(set = 0, binding = 0, std140) uniform BillboardUniforms { mat4 viewProjection; } billboard;
layout(location = 0) out vec2 UV;
layout(location = 1) out vec4 fragmentColor;
void main()
{
    vec4 clip = billboard.viewProjection * vec4(vertexPosition_Billboard.xyz, 1.0);
    gl_Position = vec4(clip.x, -clip.y, (clip.z + clip.w) * 0.5, clip.w);
    UV = vertexUV;
    fragmentColor = vertexColor.bgra;
}
