// SPDX-License-Identifier: GPL-3.0-or-later
#include "context.hpp"
#include <SDL_vulkan.h>
#include <algorithm>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {
void check(VkResult result, const char* operation)
{
    if (result != VK_SUCCESS)
        throw std::runtime_error(std::string(operation) + " failed (VkResult " + std::to_string(result) + ")");
}
template<typename T> T info(VkStructureType type)
{
    T value = {};
    value.sType = type;
    return value;
}
}

VKAPI_ATTR VkBool32 VKAPI_CALL RenVulkanContext::debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT,
    const VkDebugUtilsMessengerCallbackDataEXT* data, void* user)
{
    if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        ++static_cast<RenVulkanContext*>(user)->validationErrors_;
    std::cerr << "Vulkan validation: " << data->pMessage << '\n';
    return VK_FALSE;
}

RenVulkanContext::RenVulkanContext(SDL_Window* window, bool validation) : window_(window)
{
    try {
        check(volkInitialize(), "Load Vulkan driver (install the GPU vendor's driver if unavailable)");
        createInstance(validation);
        if (!SDL_Vulkan_CreateSurface(window_, instance_, &surface_))
            throw std::runtime_error(std::string("SDL_Vulkan_CreateSurface: ") + SDL_GetError());
        selectDevice();
        auto pool = info<VkCommandPoolCreateInfo>(VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO);
        pool.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        pool.queueFamilyIndex = graphicsFamily_;
        check(vkCreateCommandPool(device_, &pool, nullptr, &commandPool_), "Create command pool");
        auto allocation = info<VkCommandBufferAllocateInfo>(VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO);
        allocation.commandPool = commandPool_;
        allocation.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocation.commandBufferCount = 1;
        check(vkAllocateCommandBuffers(device_, &allocation, &command_), "Allocate command buffer");
        auto semaphore = info<VkSemaphoreCreateInfo>(VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO);
        check(vkCreateSemaphore(device_, &semaphore, nullptr, &imageAvailable_), "Create acquire semaphore");
        auto fence = info<VkFenceCreateInfo>(VK_STRUCTURE_TYPE_FENCE_CREATE_INFO);
        fence.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        check(vkCreateFence(device_, &fence, nullptr, &frameFence_), "Create frame fence");
        recreateSwapchain();
    } catch (...) {
        cleanup();
        throw;
    }
}

void RenVulkanContext::createInstance(bool validation)
{
    unsigned count = 0;
    if (!SDL_Vulkan_GetInstanceExtensions(window_, &count, nullptr))
        throw std::runtime_error(SDL_GetError());
    std::vector<const char*> extensions(count);
    if (!SDL_Vulkan_GetInstanceExtensions(window_, &count, extensions.data()))
        throw std::runtime_error(SDL_GetError());

    const char* layer = "VK_LAYER_KHRONOS_validation";
    if (validation) {
        uint32_t layerCount = 0;
        check(vkEnumerateInstanceLayerProperties(&layerCount, nullptr), "Enumerate validation layers");
        std::vector<VkLayerProperties> layers(layerCount);
        check(vkEnumerateInstanceLayerProperties(&layerCount, layers.data()), "Read validation layers");
        bool found = false;
        for (const auto& candidate : layers)
            found = found || std::strcmp(candidate.layerName, layer) == 0;
        if (!found)
            throw std::runtime_error("--validation requires VK_LAYER_KHRONOS_validation (install Vulkan SDK/validation layers).");
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    auto app = info<VkApplicationInfo>(VK_STRUCTURE_TYPE_APPLICATION_INFO);
    app.pApplicationName = "Machines Vulkan presentation milestone";
    app.pEngineName = "Machines";
    app.apiVersion = VK_API_VERSION_1_1;
    auto debug = info<VkDebugUtilsMessengerCreateInfoEXT>(VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT);
    debug.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debug.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debug.pfnUserCallback = debugCallback;
    debug.pUserData = this;
    auto instance = info<VkInstanceCreateInfo>(VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO);
    instance.pApplicationInfo = &app;
    instance.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    instance.ppEnabledExtensionNames = extensions.data();
    if (validation) {
        instance.enabledLayerCount = 1;
        instance.ppEnabledLayerNames = &layer;
        instance.pNext = &debug;
    }
    check(vkCreateInstance(&instance, nullptr, &instance_), "Create Vulkan 1.1 instance");
    volkLoadInstance(instance_);
    if (validation)
        check(vkCreateDebugUtilsMessengerEXT(instance_, &debug, nullptr, &messenger_), "Create validation messenger");
}

void RenVulkanContext::selectDevice()
{
    uint32_t count = 0;
    check(vkEnumeratePhysicalDevices(instance_, &count, nullptr), "Enumerate GPUs");
    std::vector<VkPhysicalDevice> devices(count);
    check(vkEnumeratePhysicalDevices(instance_, &count, devices.data()), "Read GPUs");
    int bestScore = -1;
    for (auto candidate : devices) {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(candidate, &properties);
        if (properties.apiVersion < VK_API_VERSION_1_1) continue;
        uint32_t extensionCount = 0;
        check(vkEnumerateDeviceExtensionProperties(candidate, nullptr, &extensionCount, nullptr), "Count device extensions");
        std::vector<VkExtensionProperties> extensions(extensionCount);
        check(vkEnumerateDeviceExtensionProperties(candidate, nullptr, &extensionCount, extensions.data()), "Read device extensions");
        bool swapchain = false;
        for (const auto& extension : extensions)
            swapchain = swapchain || std::strcmp(extension.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0;
        if (!swapchain) continue;
        uint32_t familyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(candidate, &familyCount, nullptr);
        std::vector<VkQueueFamilyProperties> families(familyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(candidate, &familyCount, families.data());
        uint32_t graphics = UINT32_MAX, present = UINT32_MAX;
        for (uint32_t i = 0; i < familyCount; ++i) {
            VkBool32 supported = VK_FALSE;
            check(vkGetPhysicalDeviceSurfaceSupportKHR(candidate, i, surface_, &supported), "Check presentation support");
            if (families[i].queueCount && (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)) graphics = i;
            if (families[i].queueCount && supported) present = i;
            if (graphics == i && present == i) break;
        }
        uint32_t formatCount = 0, modeCount = 0;
        check(vkGetPhysicalDeviceSurfaceFormatsKHR(candidate, surface_, &formatCount, nullptr), "Count surface formats");
        check(vkGetPhysicalDeviceSurfacePresentModesKHR(candidate, surface_, &modeCount, nullptr), "Count presentation modes");
        if (graphics == UINT32_MAX || present == UINT32_MAX || !formatCount || !modeCount) continue;
        const int score = properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ? 100 :
            properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU ? 50 : 0;
        if (score > bestScore) {
            bestScore = score;
            physicalDevice_ = candidate;
            graphicsFamily_ = graphics;
            presentFamily_ = present;
        }
    }
    if (!physicalDevice_) throw std::runtime_error("No Vulkan 1.1 GPU with graphics, swapchain and window presentation support.");
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(physicalDevice_, &properties);
    std::cout << "GPU: " << properties.deviceName << "; Vulkan "
        << VK_VERSION_MAJOR(properties.apiVersion) << '.' << VK_VERSION_MINOR(properties.apiVersion) << '.'
        << VK_VERSION_PATCH(properties.apiVersion) << "; graphics queue " << graphicsFamily_
        << "; present queue " << presentFamily_ << std::endl;

    const float priority = 1.0f;
    std::vector<VkDeviceQueueCreateInfo> queues;
    for (auto family : {graphicsFamily_, presentFamily_}) {
        if (!queues.empty() && queues.front().queueFamilyIndex == family) continue;
        auto queue = info<VkDeviceQueueCreateInfo>(VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO);
        queue.queueFamilyIndex = family;
        queue.queueCount = 1;
        queue.pQueuePriorities = &priority;
        queues.push_back(queue);
    }
    const char* extension = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
    auto device = info<VkDeviceCreateInfo>(VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO);
    device.queueCreateInfoCount = static_cast<uint32_t>(queues.size());
    device.pQueueCreateInfos = queues.data();
    device.enabledExtensionCount = 1;
    device.ppEnabledExtensionNames = &extension;
    check(vkCreateDevice(physicalDevice_, &device, nullptr, &device_), "Create Vulkan device");
    volkLoadDevice(device_);
    vkGetDeviceQueue(device_, graphicsFamily_, 0, &graphicsQueue_);
    vkGetDeviceQueue(device_, presentFamily_, 0, &presentQueue_);
}

bool RenVulkanContext::recreateSwapchain()
{
    int width, height;
    SDL_Vulkan_GetDrawableSize(window_, &width, &height);
    if (width <= 0 || height <= 0 || (SDL_GetWindowFlags(window_) & SDL_WINDOW_MINIMIZED)) return false;
    check(vkDeviceWaitIdle(device_), "Wait before replacing swapchain");
    // Recorded command buffers reference the old framebuffers.
    check(vkResetCommandPool(device_, commandPool_, 0), "Reset commands before replacing swapchain");
    destroySwapchain();

    VkSurfaceCapabilitiesKHR caps;
    check(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice_, surface_, &caps), "Read surface capabilities");
    if (!(caps.supportedUsageFlags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT))
        throw std::runtime_error("Surface does not support color attachments.");
    extent_ = caps.currentExtent;
    if (extent_.width == UINT32_MAX) {
        extent_.width = std::max(caps.minImageExtent.width, std::min(caps.maxImageExtent.width, static_cast<uint32_t>(width)));
        extent_.height = std::max(caps.minImageExtent.height, std::min(caps.maxImageExtent.height, static_cast<uint32_t>(height)));
    }
    if (!extent_.width || !extent_.height) return false;
    uint32_t count = 0;
    check(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice_, surface_, &count, nullptr), "Count formats");
    std::vector<VkSurfaceFormatKHR> formats(count);
    check(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice_, surface_, &count, formats.data()), "Read formats");
    if (formats.empty()) throw std::runtime_error("Surface has no available formats.");
    auto format = formats.front();
    if (formats.size() == 1 && format.format == VK_FORMAT_UNDEFINED) format.format = VK_FORMAT_B8G8R8A8_UNORM;
    for (const auto& option : formats)
        if (option.format == VK_FORMAT_B8G8R8A8_UNORM && option.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) format = option;
    auto swapchain = info<VkSwapchainCreateInfoKHR>(VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR);
    swapchain.surface = surface_;
    swapchain.minImageCount = caps.minImageCount + 1;
    if (caps.maxImageCount && swapchain.minImageCount > caps.maxImageCount) swapchain.minImageCount = caps.maxImageCount;
    swapchain.imageFormat = format.format;
    swapchain.imageColorSpace = format.colorSpace;
    swapchain.imageExtent = extent_;
    swapchain.imageArrayLayers = 1;
    swapchain.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    const uint32_t families[] = {graphicsFamily_, presentFamily_};
    swapchain.imageSharingMode = graphicsFamily_ == presentFamily_ ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT;
    if (swapchain.imageSharingMode == VK_SHARING_MODE_CONCURRENT) {
        swapchain.queueFamilyIndexCount = 2;
        swapchain.pQueueFamilyIndices = families;
    }
    swapchain.preTransform = caps.currentTransform;
    for (auto alpha : {VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR, VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
                      VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR, VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR})
        if (caps.supportedCompositeAlpha & alpha) { swapchain.compositeAlpha = alpha; break; }
    swapchain.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    swapchain.clipped = VK_TRUE;
    check(vkCreateSwapchainKHR(device_, &swapchain, nullptr, &swapchain_), "Create swapchain");
    check(vkGetSwapchainImagesKHR(device_, swapchain_, &count, nullptr), "Count swapchain images");
    std::vector<VkImage> images(count);
    check(vkGetSwapchainImagesKHR(device_, swapchain_, &count, images.data()), "Read swapchain images");

    VkAttachmentDescription color = {};
    color.format = format.format;
    color.samples = VK_SAMPLE_COUNT_1_BIT;
    color.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    VkAttachmentReference reference = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &reference;
    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    auto pass = info<VkRenderPassCreateInfo>(VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO);
    pass.attachmentCount = 1;
    pass.pAttachments = &color;
    pass.subpassCount = 1;
    pass.pSubpasses = &subpass;
    pass.dependencyCount = 1;
    pass.pDependencies = &dependency;
    check(vkCreateRenderPass(device_, &pass, nullptr, &renderPass_), "Create render pass");
    for (auto image : images) {
        auto view = info<VkImageViewCreateInfo>(VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO);
        view.image = image;
        view.viewType = VK_IMAGE_VIEW_TYPE_2D;
        view.format = format.format;
        view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        view.subresourceRange.levelCount = 1;
        view.subresourceRange.layerCount = 1;
        VkImageView handle;
        check(vkCreateImageView(device_, &view, nullptr, &handle), "Create image view");
        views_.push_back(handle);
        auto framebuffer = info<VkFramebufferCreateInfo>(VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO);
        framebuffer.renderPass = renderPass_;
        framebuffer.attachmentCount = 1;
        framebuffer.pAttachments = &views_.back();
        framebuffer.width = extent_.width;
        framebuffer.height = extent_.height;
        framebuffer.layers = 1;
        VkFramebuffer target;
        check(vkCreateFramebuffer(device_, &framebuffer, nullptr, &target), "Create framebuffer");
        framebuffers_.push_back(target);
        auto semaphore = info<VkSemaphoreCreateInfo>(VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO);
        VkSemaphore finished;
        check(vkCreateSemaphore(device_, &semaphore, nullptr, &finished), "Create presentation semaphore");
        renderFinished_.push_back(finished);
    }
    resized_ = false;
    ++generation_;
    std::cout << "Swapchain " << generation_ << ": " << extent_.width << 'x' << extent_.height
              << ", " << images.size() << " images, FIFO" << std::endl;
    return true;
}

bool RenVulkanContext::presentClear(float red, float green, float blue)
{
    if ((SDL_GetWindowFlags(window_) & SDL_WINDOW_MINIMIZED)) return false;
    if (resized_ || !swapchain_) { recreateSwapchain(); return false; }
    check(vkWaitForFences(device_, 1, &frameFence_, VK_TRUE, UINT64_MAX), "Wait for frame");
    uint32_t image = 0;
    const auto acquired = vkAcquireNextImageKHR(device_, swapchain_, UINT64_MAX, imageAvailable_, VK_NULL_HANDLE, &image);
    if (acquired == VK_ERROR_OUT_OF_DATE_KHR) { recreateSwapchain(); return false; }
    if (acquired != VK_SUBOPTIMAL_KHR) check(acquired, "Acquire image");
    check(vkResetCommandBuffer(command_, 0), "Reset frame commands");
    auto begin = info<VkCommandBufferBeginInfo>(VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO);
    begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    check(vkBeginCommandBuffer(command_, &begin), "Begin frame commands");
    VkClearValue clear = {};
    clear.color.float32[0] = red;
    clear.color.float32[1] = green;
    clear.color.float32[2] = blue;
    clear.color.float32[3] = 1.0f;
    auto pass = info<VkRenderPassBeginInfo>(VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO);
    pass.renderPass = renderPass_;
    pass.framebuffer = framebuffers_.at(image);
    pass.renderArea.extent = extent_;
    pass.clearValueCount = 1;
    pass.pClearValues = &clear;
    vkCmdBeginRenderPass(command_, &pass, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdEndRenderPass(command_);
    check(vkEndCommandBuffer(command_), "End frame commands");
    const VkPipelineStageFlags stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    auto submit = info<VkSubmitInfo>(VK_STRUCTURE_TYPE_SUBMIT_INFO);
    submit.waitSemaphoreCount = 1;
    submit.pWaitSemaphores = &imageAvailable_;
    submit.pWaitDstStageMask = &stage;
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &command_;
    submit.signalSemaphoreCount = 1;
    submit.pSignalSemaphores = &renderFinished_.at(image);
    // Reset only after a successful acquire; an out-of-date swapchain must not
    // leave an unsignaled fence that deadlocks the following frame.
    check(vkResetFences(device_, 1, &frameFence_), "Reset frame fence");
    check(vkQueueSubmit(graphicsQueue_, 1, &submit, frameFence_), "Submit frame");
    auto present = info<VkPresentInfoKHR>(VK_STRUCTURE_TYPE_PRESENT_INFO_KHR);
    present.waitSemaphoreCount = 1;
    present.pWaitSemaphores = &renderFinished_.at(image);
    present.swapchainCount = 1;
    present.pSwapchains = &swapchain_;
    present.pImageIndices = &image;
    const auto result = vkQueuePresentKHR(presentQueue_, &present);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || acquired == VK_SUBOPTIMAL_KHR)
        resized_ = true;
    else check(result, "Present frame");
    return true;
}

void RenVulkanContext::destroySwapchain()
{
    for (auto framebuffer : framebuffers_) vkDestroyFramebuffer(device_, framebuffer, nullptr);
    framebuffers_.clear();
    for (auto view : views_) vkDestroyImageView(device_, view, nullptr);
    views_.clear();
    for (auto semaphore : renderFinished_) vkDestroySemaphore(device_, semaphore, nullptr);
    renderFinished_.clear();
    if (renderPass_) vkDestroyRenderPass(device_, renderPass_, nullptr);
    renderPass_ = VK_NULL_HANDLE;
    if (swapchain_) vkDestroySwapchainKHR(device_, swapchain_, nullptr);
    swapchain_ = VK_NULL_HANDLE;
}

void RenVulkanContext::cleanup()
{
    if (device_) {
        vkDeviceWaitIdle(device_);
        if (commandPool_) vkDestroyCommandPool(device_, commandPool_, nullptr);
        destroySwapchain();
        if (imageAvailable_) vkDestroySemaphore(device_, imageAvailable_, nullptr);
        if (frameFence_) vkDestroyFence(device_, frameFence_, nullptr);
        vkDestroyDevice(device_, nullptr);
    }
    if (instance_ && surface_) vkDestroySurfaceKHR(instance_, surface_, nullptr);
    if (instance_ && messenger_) vkDestroyDebugUtilsMessengerEXT(instance_, messenger_, nullptr);
    if (instance_) vkDestroyInstance(instance_, nullptr);
    device_ = VK_NULL_HANDLE;
    instance_ = VK_NULL_HANDLE;
    surface_ = VK_NULL_HANDLE;
    messenger_ = VK_NULL_HANDLE;
}

RenVulkanContext::~RenVulkanContext() { cleanup(); }

