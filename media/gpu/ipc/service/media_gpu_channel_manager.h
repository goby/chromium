// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MEDIA_GPU_IPC_SERVICE_MEDIA_GPU_CHANNEL_MANAGER_H_
#define MEDIA_GPU_IPC_SERVICE_MEDIA_GPU_CHANNEL_MANAGER_H_

#include <stdint.h>

#include <memory>

#include "base/containers/scoped_ptr_hash_map.h"
#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "ipc/ipc_listener.h"
#include "ipc/ipc_sender.h"
#include "media/video/video_decode_accelerator.h"

namespace gpu {
class GpuChannelManager;
}

namespace media {

class MediaGpuChannel;

class MediaGpuChannelManager
    : public base::SupportsWeakPtr<MediaGpuChannelManager> {
 public:
  explicit MediaGpuChannelManager(gpu::GpuChannelManager* channel_manager);
  ~MediaGpuChannelManager();

  void AddChannel(int32_t client_id);
  void RemoveChannel(int32_t client_id);
  void DestroyAllChannels();

 private:
  gpu::GpuChannelManager* const channel_manager_;
  base::ScopedPtrHashMap<int32_t, std::unique_ptr<MediaGpuChannel>>
      media_gpu_channels_;
  DISALLOW_COPY_AND_ASSIGN(MediaGpuChannelManager);
};

}  // namespace media

#endif  // MEDIA_GPU_IPC_SERVICE_MEDIA_GPU_CHANNEL_MANAGER_H_
