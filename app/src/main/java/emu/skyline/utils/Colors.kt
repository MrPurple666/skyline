/*
 * SPDX-License-Identifier: MPL-2.0
 * Copyright © 2023 Skyline Team and Contributors (https://github.com/skyline-emu/)
 */

package emu.skyline.utils

import android.graphics.Color

enum class SwitchColors(val color : Int) {
    GRAY(Color.GRAY),
    BLACK(Color.rgb(180, 0, 0, 0)),
    WHITE(Color.rgb(180, 255, 255, 255)),
    C1(Color.argb(180, 255,105,180)),
    C2(Color.argb(180, 128, 112, 203)),
    C3(Color.argb(180, 252, 236, 82)),
    C4(Color.argb(180, 93, 46, 140)),
    C5(Color.argb(180, 46, 196, 182)),
    C6(Color.argb(180, 0, 117, 162)),
    C7(Color.argb(180, 235, 50, 95));

    companion object {
        val colors get() = SwitchColors.values().map { clr -> clr.color }
    }
}
