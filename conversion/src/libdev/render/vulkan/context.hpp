// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef MACHINES_RENDER_VULKAN_CONTEXT_HPP
#define MACHINES_RENDER_VULKAN_CONTEXT_HPP

#include <volk.h>
#include <SDL.h>
#include <cstdint>
#include <vector>

// Presentation foundation. The legacy RenDevice is not connected to this yet.
// SDL owns the window; this object must be destroyed before that window.
class RenVulkanContext
{
public:
    RenVulkanContext(SDL_Window* window, bool validation);
    ~RenVulkanContext();
    RenVulkanContext(const RenVulkanContext&) = delete;
    RenVulkanContext& operator=(const RenVulkanContext&) = delete;

    // False means the window is minimized or its swapchain was recreated.
    bool presentClear(float red, float green, float blue);
    // Explicit shutdown allows the caller to inspect validation errors from
    // resource destruction as well as from frame submission.
    void shutdown() { cleanup(); }
    void invalidateSwapchain() { resized_ = true; }
    unsigned validationErrors() const { return validationErrors_; }
    unsigned swapchainGeneration() const { return generation_; }

private:
    void createInstance(bool validation);
    void selectDevice();
    bool recreateSwapchain();
    void destroySwapchain();
    void cleanup();
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT severity,
        VkDebugUtilsMessageTypeFlagsEXT type,
        const VkDebugUtilsMessengerCallbackDataEXT* data, void* user);

    SDL_Window* window_;
    VkInstance instance_ = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT messenger_ = VK_NULL_HANDLE;
    VkSurfaceKHR surface_ = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice_ = VK_NULL_HANDLE;
    VkDevice device_ = VK_NULL_HANDLE;
    VkQueue graphicsQueue_ = VK_NULL_HANDLE;
    VkQueue presentQueue_ = VK_NULL_HANDLE;
    uint32_t graphicsFamily_ = UINT32_MAX;
    uint32_t presentFamily_ = UINT32_MAX;
    VkSwapchainKHR swapchain_ = VK_NULL_HANDLE;
    VkExtent2D extent_ = {};
    VkRenderPass renderPass_ = VK_NULL_HANDLE;
    VkCommandPool commandPool_ = VK_NULL_HANDLE;
    VkCommandBuffer command_ = VK_NULL_HANDLE;
    VkSemaphore imageAvailable_ = VK_NULL_HANDLE;
    VkFence frameFence_ = VK_NULL_HANDLE;
    std::vector<VkImageView> views_;
    std::vector<VkFramebuffer> framebuffers_;
    // One semaphore per swapchain image, not per frame: presentation can outlive
    // the graphics fence. Reacquiring that image makes reuse safe.
    std::vector<VkSemaphore> renderFinished_;
    bool resized_ = false;
    unsigned validationErrors_ = 0;
    unsigned generation_ = 0;
};
#endif

