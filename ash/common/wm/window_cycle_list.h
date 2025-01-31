// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ASH_COMMON_WM_WINDOW_CYCLE_LIST_H_
#define ASH_COMMON_WM_WINDOW_CYCLE_LIST_H_

#include <memory>
#include <vector>

#include "ash/ash_export.h"
#include "ash/common/wm/window_cycle_controller.h"
#include "ash/common/wm_window_observer.h"
#include "base/macros.h"
#include "base/scoped_observer.h"
#include "base/timer/timer.h"
#include "ui/display/display_observer.h"

namespace display {
class Screen;
}

namespace views {
class Widget;
}

namespace ash {

class ScopedShowWindow;
class WindowCycleView;

// Tracks a set of Windows that can be stepped through. This class is used by
// the WindowCycleController.
class ASH_EXPORT WindowCycleList : public WmWindowObserver,
                                   public display::DisplayObserver {
 public:
  using WindowList = std::vector<WmWindow*>;

  explicit WindowCycleList(const WindowList& windows);
  ~WindowCycleList() override;

  bool empty() const { return windows_.empty(); }

  // Cycles to the next or previous window based on |direction|.
  void Step(WindowCycleController::Direction direction);

  int current_index() const { return current_index_; }

 private:
  friend class WindowCycleControllerTest;

  static void DisableInitialDelayForTesting();

  const WindowList& windows() const { return windows_; }

  // WmWindowObserver overrides:
  // There is a chance a window is destroyed, for example by JS code. We need to
  // take care of that even if it is not intended for the user to close a window
  // while window cycling.
  void OnWindowDestroying(WmWindow* window) override;

  // display::DisplayObserver overrides:
  void OnDisplayAdded(const display::Display& new_display) override;
  void OnDisplayRemoved(const display::Display& old_display) override;
  void OnDisplayMetricsChanged(const display::Display& display,
                               uint32_t changed_metrics) override;

  // Returns true if the window list overlay should be shown.
  bool ShouldShowUi();

  // Initializes and shows |cycle_view_|.
  void InitWindowCycleView();

  // List of weak pointers to windows to use while cycling with the keyboard.
  // List is built when the user initiates the gesture (i.e. hits alt-tab the
  // first time) and is emptied when the gesture is complete (i.e. releases the
  // alt key).
  WindowList windows_;

  // Current position in the |windows_|. Can be used to query selection depth,
  // i.e., the position of an active window in a global MRU ordering.
  int current_index_;

  // Wrapper for the window brought to the front.
  // TODO(estade): remove ScopedShowWindow when we know we are happy launching
  // the |cycle_view_| version.
  std::unique_ptr<ScopedShowWindow> showing_window_;

  // The top level View for the window cycle UI. May be null if the UI is not
  // showing.
  WindowCycleView* cycle_view_;

  // The widget that hosts the window cycle UI.
  views::Widget* cycle_ui_widget_;

  // The window list will dismiss if the display metrics change.
  ScopedObserver<display::Screen, display::DisplayObserver> screen_observer_;

  // A timer to delay showing the UI. Quick Alt+Tab should not flash a UI.
  base::OneShotTimer show_ui_timer_;

  DISALLOW_COPY_AND_ASSIGN(WindowCycleList);
};

}  // namespace ash

#endif  // ASH_COMMON_WM_WINDOW_CYCLE_LIST_H_
