// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/browser/media/session/mock_media_session_player_observer.h"

#include "testing/gtest/include/gtest/gtest.h"

namespace content {

MockMediaSessionPlayerObserver::MockMediaSessionPlayerObserver() = default;

MockMediaSessionPlayerObserver::~MockMediaSessionPlayerObserver() = default;

void MockMediaSessionPlayerObserver::OnSuspend(int player_id) {
  EXPECT_GE(player_id, 0);
  EXPECT_GT(players_.size(), static_cast<size_t>(player_id));

  ++received_suspend_calls_;
  players_[player_id].is_playing_ = false;
}

void MockMediaSessionPlayerObserver::OnResume(int player_id) {
  EXPECT_GE(player_id, 0);
  EXPECT_GT(players_.size(), static_cast<size_t>(player_id));

  ++received_resume_calls_;
  players_[player_id].is_playing_ = true;
}

void MockMediaSessionPlayerObserver::OnSetVolumeMultiplier(
    int player_id,
    double volume_multiplier) {
  EXPECT_GE(player_id, 0);
  EXPECT_GT(players_.size(), static_cast<size_t>(player_id));

  EXPECT_GE(volume_multiplier, 0.0f);
  EXPECT_LE(volume_multiplier, 1.0f);

  players_[player_id].volume_multiplier_ = volume_multiplier;
}

RenderFrameHost* MockMediaSessionPlayerObserver::GetRenderFrameHost() const {
  return nullptr;
}

int MockMediaSessionPlayerObserver::StartNewPlayer() {
  players_.push_back(MockPlayer(true, 1.0f));
  return players_.size() - 1;
}

bool MockMediaSessionPlayerObserver::IsPlaying(size_t player_id) {
  EXPECT_GT(players_.size(), player_id);
  return players_[player_id].is_playing_;
}

double MockMediaSessionPlayerObserver::GetVolumeMultiplier(size_t player_id) {
  EXPECT_GT(players_.size(), player_id);
  return players_[player_id].volume_multiplier_;
}

void MockMediaSessionPlayerObserver::SetPlaying(size_t player_id,
                                                bool playing) {
  EXPECT_GT(players_.size(), player_id);
  players_[player_id].is_playing_ = playing;
}

int MockMediaSessionPlayerObserver::received_suspend_calls() const {
  return received_suspend_calls_;
}

int MockMediaSessionPlayerObserver::received_resume_calls() const {
  return received_resume_calls_;
}

}  // namespace content
