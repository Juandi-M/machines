#version 450
// Mirrors 2DShading.vxgls. Positive Vulkan viewport height maps -Y to the top.
layout(location = 0) in vec2 vertexPosition_screenspace;
layout(location = 1) in vec2 vertexUV;
layout(location = 2) in vec4 vertexColor;
layout(set = 0, binding = 0, std140) uniform GuiUniforms { vec2 screenspace; } gui;
layout(location = 0) out vec2 UV;
layout(location = 1) out vec4 fragmentColor;
void main()
{
    vec2 halfScreen = gui.screenspace / 2.0;
    vec2 position = (vertexPosition_screenspace - abs(halfScreen)) / halfScreen;
    gl_Position = vec4(position, 0.5, 1.0);
    UV = vertexUV;
    fragmentColor = vertexColor.bgra;
}
