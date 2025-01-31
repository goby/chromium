// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_DISPLAY_MANAGER_CHROMEOS_TEST_ACTION_LOGGER_UTIL_H_
#define UI_DISPLAY_MANAGER_CHROMEOS_TEST_ACTION_LOGGER_UTIL_H_

#include <stdint.h>

#include <string>
#include <vector>

#include "ui/display/types/display_constants.h"

namespace gfx {
class Point;
class Size;
}  // namespace gfx

namespace ui {

struct GammaRampRGBEntry;
class DisplayMode;
class DisplaySnapshot;

namespace test {

// Strings returned by TestNativeDisplayDelegate::GetActionsAndClear() to
// describe various actions that were performed.
const char kInitXRandR[] = "init";
const char kGrab[] = "grab";
const char kUngrab[] = "ungrab";
const char kSync[] = "sync";
const char kForceDPMS[] = "dpms";
const char kTakeDisplayControl[] = "take";
const char kRelinquishDisplayControl[] = "relinquish";

// String returned by TestNativeDisplayDelegate::GetActionsAndClear() if no
// actions were requested.
const char kNoActions[] = "";

std::string DisplaySnapshotToString(const DisplaySnapshot& output);

// Returns a string describing a TestNativeDisplayDelegate::SetBackgroundColor()
// call.
std::string GetBackgroundAction(uint32_t color_argb);

// Returns a string describing a TestNativeDisplayDelegate::AddOutputMode()
// call.
std::string GetAddOutputModeAction(const DisplaySnapshot& output,
                                   const DisplayMode* mode);

// Returns a string describing a TestNativeDisplayDelegate::Configure()
// call.
std::string GetCrtcAction(const DisplaySnapshot& output,
                          const DisplayMode* mode,
                          const gfx::Point& origin);

// Returns a string describing a TestNativeDisplayDelegate::CreateFramebuffer()
// call.
std::string GetFramebufferAction(const gfx::Size& size,
                                 const DisplaySnapshot* out1,
                                 const DisplaySnapshot* out2);

// Returns a string describing a TestNativeDisplayDelegate::SetHDCPState() call.
std::string GetSetHDCPStateAction(const DisplaySnapshot& output,
                                  HDCPState state);

// Returns a string describing a TestNativeDisplayDelegate::SetColorCorrection()
// call;
std::string SetColorCorrectionAction(
    const ui::DisplaySnapshot& output,
    const std::vector<GammaRampRGBEntry>& degamma_lut,
    const std::vector<GammaRampRGBEntry>& gamma_lut,
    const std::vector<float>& correction_matrix);
// Joins a sequence of strings describing actions (e.g. kScreenDim) such
// that they can be compared against a string returned by
// ActionLogger::GetActionsAndClear().  The list of actions must be
// terminated by a NULL pointer.
std::string JoinActions(const char* action, ...);

}  // namespace test
}  // namespace ui

#endif  // UI_DISPLAY_MANAGER_CHROMEOS_TEST_ACTION_LOGGER_UTIL_H_
