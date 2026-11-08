// SPDX-License-Identifier: GPL-3.0-or-later
#include "context.hpp"
#include <SDL_vulkan.h>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

int main(int argc, char** argv)
{
    try {
        unsigned frameLimit = 180;
        bool validation = false, exercise = false;
        for (int i = 1; i < argc; ++i) {
            const std::string arg = argv[i];
            if (arg == "--help") {
                std::cout << "Machines Vulkan presentation milestone (not the game)\n"
                    "--frames N          Present N frames, then exit (default 180)\n"
                    "--validation        Require Khronos validation layers\n"
                    "--exercise-window   Exercise resize and minimize/restore\n";
                return EXIT_SUCCESS;
            } else if (arg == "--validation") validation = true;
            else if (arg == "--exercise-window") exercise = true;
            else if (arg == "--frames" && i + 1 < argc) {
                const std::string value = argv[++i];
                if (value.empty() || value.find_first_not_of("0123456789") != std::string::npos)
                    throw std::runtime_error("--frames must be an integer between 1 and 1000000.");
                const auto parsed = std::stoul(value);
                if (!parsed || parsed > 1000000) throw std::runtime_error("--frames must be between 1 and 1000000.");
                frameLimit = static_cast<unsigned>(parsed);
            } else throw std::runtime_error("Unknown or incomplete argument: " + arg);
        }
        if (exercise && frameLimit < 100) throw std::runtime_error("--exercise-window requires at least 100 frames.");
        SDL_SetMainReady();
        if (SDL_Init(SDL_INIT_VIDEO) != 0) throw std::runtime_error(SDL_GetError());
        struct QuitSDL { ~QuitSDL() { SDL_Quit(); } } quit;
        std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> window(
            SDL_CreateWindow("Machines - Vulkan presentation milestone", SDL_WINDOWPOS_CENTERED,
                SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN), SDL_DestroyWindow);
        if (!window) throw std::runtime_error(std::string("Create Vulkan window: ") + SDL_GetError());
        RenVulkanContext context(window.get(), validation);
        unsigned frames = 0;
        bool running = true, resized = false, restored = false;
        while (running && frames < frameLimit) {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) running = false;
                if (event.type == SDL_WINDOWEVENT && (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED ||
                    event.window.event == SDL_WINDOWEVENT_RESTORED)) context.invalidateSwapchain();
            }
            if (!running) break;
            if (exercise && frames >= 30 && !resized) {
                SDL_SetWindowSize(window.get(), 1024, 720);
                context.invalidateSwapchain();
                resized = true;
            }
            if (exercise && frames >= 60 && !restored) {
                SDL_MinimizeWindow(window.get());
                // Submit no GPU work while minimized; restore from the same
                // event loop rather than depending on frame count advancing.
                if (context.presentClear(0.0f, 0.0f, 0.0f))
                    std::cout << "Window manager did not minimize the window synchronously.\n";
                SDL_Delay(100);
                SDL_RestoreWindow(window.get());
                context.invalidateSwapchain();
                restored = true;
            }
            if (context.presentClear(0.035f, 0.055f, 0.08f)) ++frames;
            else SDL_Delay(10);
        }
        if (frames != frameLimit) throw std::runtime_error("Window closed before requested presentation test completed.");
        if (exercise && context.swapchainGeneration() < 3)
            throw std::runtime_error("Window exercise did not recreate the swapchain as expected.");
        context.shutdown();
        if (context.validationErrors()) throw std::runtime_error("Vulkan validation reported errors.");
        std::cout << "PASS: " << frames << " Vulkan frames presented; " << context.swapchainGeneration()
                  << " swapchain generations; validation " << (validation ? "enabled" : "not enabled") << '.' << std::endl;
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << "FAIL: " << error.what() << std::endl;
        return EXIT_FAILURE;
    }
}
