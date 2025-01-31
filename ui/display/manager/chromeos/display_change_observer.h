// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_DISPLAY_MANAGER_CHROMEOS_DISPLAY_CHANGE_OBSERVER_H_
#define UI_DISPLAY_MANAGER_CHROMEOS_DISPLAY_CHANGE_OBSERVER_H_

#include <stdint.h>

#include <memory>
#include <vector>

#include "base/macros.h"
#include "ui/display/manager/chromeos/display_configurator.h"
#include "ui/display/manager/display_manager_export.h"
#include "ui/display/manager/managed_display_info.h"
#include "ui/events/devices/input_device_event_observer.h"

namespace ui {
class DisplaySnapshot;
}

namespace display {

class DisplayManager;

// An object that observes changes in display configuration and updates
// DisplayManager.
class DISPLAY_MANAGER_EXPORT DisplayChangeObserver
    : public ui::DisplayConfigurator::StateController,
      public ui::DisplayConfigurator::Observer,
      public ui::InputDeviceEventObserver {
 public:
  // Returns the mode list for internal display.
  DISPLAY_EXPORT static ManagedDisplayInfo::ManagedDisplayModeList
  GetInternalManagedDisplayModeList(const ManagedDisplayInfo& display_info,
                                    const ui::DisplaySnapshot& output);

  // Returns the resolution list.
  DISPLAY_EXPORT static ManagedDisplayInfo::ManagedDisplayModeList
  GetExternalManagedDisplayModeList(const ui::DisplaySnapshot& output);

  DisplayChangeObserver(ui::DisplayConfigurator* display_configurator,
                        display::DisplayManager* display_manager);
  ~DisplayChangeObserver() override;

  // ui::DisplayConfigurator::StateController overrides:
  ui::MultipleDisplayState GetStateForDisplayIds(
      const ui::DisplayConfigurator::DisplayStateList& outputs) const override;
  bool GetResolutionForDisplayId(int64_t display_id,
                                 gfx::Size* size) const override;

  // Overriden from ui::DisplayConfigurator::Observer:
  void OnDisplayModeChanged(
      const ui::DisplayConfigurator::DisplayStateList& outputs) override;
  void OnDisplayModeChangeFailed(
      const ui::DisplayConfigurator::DisplayStateList& displays,
      ui::MultipleDisplayState failed_new_state) override;

  // Overriden from ui::InputDeviceEventObserver:
  void OnTouchscreenDeviceConfigurationChanged() override;

  // Exposed for testing.
  DISPLAY_EXPORT static float FindDeviceScaleFactor(float dpi);

 private:
  // Both |display_configurator_| and |display_manager_| are not owned and must
  // outlive DisplayChangeObserver.
  ui::DisplayConfigurator* display_configurator_;
  DisplayManager* display_manager_;

  DISALLOW_COPY_AND_ASSIGN(DisplayChangeObserver);
};

}  // namespace display

#endif  // UI_DISPLAY_MANAGER_CHROMEOS_DISPLAY_CHANGE_OBSERVER_H_
