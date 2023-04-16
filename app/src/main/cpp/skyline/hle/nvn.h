// SPDX-License-Identifier: MPL-2.0
// Copyright © 2022 Skyline Team and Contributors (https://github.com/skyline-emu/)

#pragma once

#include <common.h>

namespace skyline::hle {
    void SetNvnHleTlsRestorer(void *(*function)());

    /**
     * @brief Resolves a guest-callable symbol for the requested NVN functio
     */
    void *NvnBootstrapLoader(const char *name);
}
