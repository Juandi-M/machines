#version 450
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec2 vertexUV;
layout(location = 2) in vec4 vertexColor;
layout(set = 0, binding = 0, std140) uniform WorldUniforms {
    mat4 model;
    mat4 view;
    mat4 projection;
    vec4 fogColour;
    vec4 fogParams;
} world;
layout(location = 0) out vec2 UV;
layout(location = 1) out vec4 fragmentColor;
layout(location = 2) out vec4 viewSpace;
layout(location = 3) out vec4 fragmentFogColour;
layout(location = 4) out vec3 fragmentFogParams;
void main()
{
    viewSpace = world.view * world.model * vec4(vertexPosition_modelspace, 1.0);
    vec4 clip = world.projection * viewSpace;
    // Preserve the legacy GLM matrices: convert GL clip coordinates here.
    gl_Position = vec4(clip.x, -clip.y, (clip.z + clip.w) * 0.5, clip.w);
    UV = vertexUV;
    fragmentColor = vertexColor.bgra;
    fragmentFogColour = world.fogColour;
    fragmentFogParams = world.fogParams.xyz;
}
