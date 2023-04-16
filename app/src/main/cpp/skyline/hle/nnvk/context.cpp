#include <vulkan/vulkan_raii.hpp>
#include "types.h"
#include "nnvk.h"

namespace nnvk {
    Context::Context(PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr, void *adrenotoolsImportHandle, bool enableValidation)
        : vkCore{vkGetInstanceProcAddr, adrenotoolsImportHandle, vk::ApplicationInfo{
            .pApplicationName = "nnvk",
            .applicationVersion = VK_MAKE_API_VERSION(0, 0, 1, 0),
            .pEngineName = "nnvk",
            .engineVersion = VK_MAKE_API_VERSION(0, 0, 1, 0),
            .apiVersion = VK_API_VERSION_1_1,
          }, enableValidation} {}

    void Context::SetApiVersion(i32 major, i32 minor) {
        apiVersion = {major, minor};
    }
}