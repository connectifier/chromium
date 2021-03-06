// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_OZONE_PLATFORM_DRI_GBM_DEVICE_H_
#define UI_OZONE_PLATFORM_DRI_GBM_DEVICE_H_

#include "ui/ozone/platform/dri/drm_device.h"

struct gbm_device;

namespace ui {

class GbmDevice : public DrmDevice {
 public:
  GbmDevice(const base::FilePath& device_path);
  GbmDevice(const base::FilePath& device_path, base::File file);

  gbm_device* device() const { return device_; }

  // DrmDevice implementation:
  bool Initialize() override;

 private:
  ~GbmDevice() override;

  gbm_device* device_;

  DISALLOW_COPY_AND_ASSIGN(GbmDevice);
};

}  // namespace ui

#endif  // UI_OZONE_PLATFORM_DRI_GBM_DEVICE_H_
