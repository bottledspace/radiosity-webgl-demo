#pragma once
#include <Windows.h>
#include <renderdoc_app.h>
#include <iostream>

// A helper singleton that integrates with RenderDoc if available for capturing
// specific intervals, or otherwise does nothing.
class RenderDoc {
private:
    static auto api() {
        static HMODULE hmod = GetModuleHandleA("renderdoc.dll");
        static RENDERDOC_API_1_1_2 *api = nullptr;

        // Can we initialize and havent yet?
        if (hmod && !api) {
            std::cerr << "RenderDoc detected, integrating..." << std::endl;
            auto RENDERDOC_GetAPI = (pRENDERDOC_GetAPI)GetProcAddress(hmod, "RENDERDOC_GetAPI");
            int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_1_2, (void **)&api);
            assert(ret == 1);
        }
        return api;
    }
public:
    static void start_capture()
        { if (api()) api()->StartFrameCapture(NULL, NULL); }
    static void end_capture()
        { if (api()) api()->EndFrameCapture(NULL, NULL); }
};
