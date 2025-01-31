// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_AURA_MUS_MUS_CONTEXT_FACTORY_H_
#define UI_AURA_MUS_MUS_CONTEXT_FACTORY_H_

#include <stdint.h>

#include "base/macros.h"
#include "cc/surfaces/surface_manager.h"
#include "services/ui/public/cpp/raster_thread_helper.h"
#include "services/ui/public/interfaces/window_tree.mojom.h"
#include "ui/aura/aura_export.h"
#include "ui/compositor/compositor.h"

namespace ui {
class Gpu;
}

namespace aura {

// ContextFactory implementation that can be used with Mus.
class AURA_EXPORT MusContextFactory : public ui::ContextFactory {
 public:
  explicit MusContextFactory(ui::Gpu* gpu);
  ~MusContextFactory() override;

 private:
  // ContextFactory:
  void CreateCompositorFrameSink(
      base::WeakPtr<ui::Compositor> compositor) override;
  std::unique_ptr<ui::Reflector> CreateReflector(
      ui::Compositor* mirrored_compositor,
      ui::Layer* mirroring_layer) override;
  void RemoveReflector(ui::Reflector* reflector) override;
  scoped_refptr<cc::ContextProvider> SharedMainThreadContextProvider() override;
  void RemoveCompositor(ui::Compositor* compositor) override;
  bool DoesCreateTestContexts() override;
  uint32_t GetImageTextureTarget(gfx::BufferFormat format,
                                 gfx::BufferUsage usage) override;
  gpu::GpuMemoryBufferManager* GetGpuMemoryBufferManager() override;
  cc::TaskGraphRunner* GetTaskGraphRunner() override;
  cc::FrameSinkId AllocateFrameSinkId() override;
  cc::SurfaceManager* GetSurfaceManager() override;
  void SetDisplayVisible(ui::Compositor* compositor, bool visible) override;
  void ResizeDisplay(ui::Compositor* compositor,
                     const gfx::Size& size) override;
  void SetDisplayColorSpace(ui::Compositor* compositor,
                            const gfx::ColorSpace& color_space) override {}
  void SetAuthoritativeVSyncInterval(ui::Compositor* compositor,
                                     base::TimeDelta interval) override {}
  void SetDisplayVSyncParameters(ui::Compositor* compositor,
                                 base::TimeTicks timebase,
                                 base::TimeDelta interval) override {}
  void SetOutputIsSecure(ui::Compositor* compositor, bool secure) override {}
  void AddObserver(ui::ContextFactoryObserver* observer) override {}
  void RemoveObserver(ui::ContextFactoryObserver* observer) override {}

  cc::SurfaceManager surface_manager_;
  uint32_t next_sink_id_;
  ui::RasterThreadHelper raster_thread_helper_;
  ui::Gpu* gpu_;

  DISALLOW_COPY_AND_ASSIGN(MusContextFactory);
};

}  // namespace aura

#endif  // UI_AURA_MUS_MUS_CONTEXT_FACTORY_H_
