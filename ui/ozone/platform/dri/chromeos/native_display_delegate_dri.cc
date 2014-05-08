// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/ozone/platform/dri/chromeos/native_display_delegate_dri.h"

#include "base/bind.h"
#include "ui/display/types/chromeos/native_display_observer.h"
#include "ui/events/ozone/device/device_event.h"
#include "ui/events/ozone/device/device_manager.h"
#include "ui/ozone/platform/dri/chromeos/display_mode_dri.h"
#include "ui/ozone/platform/dri/chromeos/display_snapshot_dri.h"
#include "ui/ozone/platform/dri/dri_surface_factory.h"
#include "ui/ozone/platform/dri/dri_util.h"
#include "ui/ozone/platform/dri/dri_wrapper.h"

namespace ui {

namespace {
const size_t kMaxDisplayCount = 2;
}  // namespace

NativeDisplayDelegateDri::NativeDisplayDelegateDri(
    DriSurfaceFactory* surface_factory, DeviceManager* device_manager)
    : surface_factory_(surface_factory),
      device_manager_(device_manager) {}

NativeDisplayDelegateDri::~NativeDisplayDelegateDri() {
  device_manager_->RemoveObserver(this);
}

void NativeDisplayDelegateDri::Initialize() {
  gfx::SurfaceFactoryOzone::HardwareState state =
      surface_factory_->InitializeHardware();

  CHECK_EQ(gfx::SurfaceFactoryOzone::INITIALIZED, state)
      << "Failed to initialize hardware";

  device_manager_->AddObserver(this);
}

void NativeDisplayDelegateDri::GrabServer() {}

void NativeDisplayDelegateDri::UngrabServer() {}

void NativeDisplayDelegateDri::SyncWithServer() {}

void NativeDisplayDelegateDri::SetBackgroundColor(uint32_t color_argb) {
  NOTIMPLEMENTED();
}

void NativeDisplayDelegateDri::ForceDPMSOn() {
  for (size_t i = 0; i < cached_displays_.size(); ++i) {
    DisplaySnapshotDri* dri_output = cached_displays_[i];
    if (dri_output->dpms_property())
      surface_factory_->drm()->SetProperty(
          dri_output->connector(),
          dri_output->dpms_property()->prop_id,
          DRM_MODE_DPMS_ON);
  }
}

std::vector<DisplaySnapshot*> NativeDisplayDelegateDri::GetDisplays() {
  ScopedVector<DisplaySnapshotDri> old_displays(cached_displays_.Pass());
  cached_modes_.clear();

  drmModeRes* resources = drmModeGetResources(
      surface_factory_->drm()->get_fd());
  DCHECK(resources) << "Failed to get DRM resources";
  ScopedVector<HardwareDisplayControllerInfo> displays =
      GetAvailableDisplayControllerInfos(
          surface_factory_->drm()->get_fd(),
          resources);
  for (size_t i = 0;
       i < displays.size() && cached_displays_.size() < kMaxDisplayCount; ++i) {
    DisplaySnapshotDri* display = new DisplaySnapshotDri(
        surface_factory_->drm(),
        displays[i]->connector(),
        displays[i]->crtc(),
        i);
    cached_displays_.push_back(display);
    // Modes can be shared between different displays, so we need to keep track
    // of them independently for cleanup.
    cached_modes_.insert(cached_modes_.end(),
                         display->modes().begin(),
                         display->modes().end());
  }

  drmModeFreeResources(resources);

  for (size_t i = 0; i < old_displays.size(); ++i) {
    bool found = false;
    for (size_t j = 0; j < cached_displays_.size(); ++j) {
      if (old_displays[i]->connector() == cached_displays_[j]->connector() &&
          old_displays[i]->crtc() == cached_displays_[j]->crtc()) {
        found = true;
        break;
      }
    }

    if (!found) {
      surface_factory_->DestroyHardwareDisplayController(
          old_displays[i]->connector(), old_displays[i]->crtc());
    }
  }

  std::vector<DisplaySnapshot*> generic_displays(cached_displays_.begin(),
                                                 cached_displays_.end());
  return generic_displays;
}

void NativeDisplayDelegateDri::AddMode(const DisplaySnapshot& output,
                                       const DisplayMode* mode) {}

bool NativeDisplayDelegateDri::Configure(const DisplaySnapshot& output,
                                         const DisplayMode* mode,
                                         const gfx::Point& origin) {
  const DisplaySnapshotDri& dri_output =
      static_cast<const DisplaySnapshotDri&>(output);

  VLOG(1) << "DRM configuring: crtc=" << dri_output.crtc()
          << " connector=" << dri_output.connector()
          << " origin=" << origin.ToString()
          << " size=" << mode->size().ToString();

  if (mode) {
    if (!surface_factory_->CreateHardwareDisplayController(
        dri_output.connector(),
        dri_output.crtc(),
        static_cast<const DisplayModeDri*>(mode)->mode_info())) {
      VLOG(1) << "Failed to configure: crtc=" << dri_output.crtc()
              << " connector=" << dri_output.connector();
      return false;
    }
  } else {
    if (!surface_factory_->DisableHardwareDisplayController(
        dri_output.crtc())) {
      VLOG(1) << "Failed to disable crtc=" << dri_output.crtc();
      return false;
    }
  }

  return true;
}

void NativeDisplayDelegateDri::CreateFrameBuffer(const gfx::Size& size) {}

bool NativeDisplayDelegateDri::GetHDCPState(const DisplaySnapshot& output,
                                            HDCPState* state) {
  NOTIMPLEMENTED();
  return false;
}

bool NativeDisplayDelegateDri::SetHDCPState(const DisplaySnapshot& output,
                                            HDCPState state) {
  NOTIMPLEMENTED();
  return false;
}

std::vector<ui::ColorCalibrationProfile>
NativeDisplayDelegateDri::GetAvailableColorCalibrationProfiles(
    const ui::DisplaySnapshot& output) {
  NOTIMPLEMENTED();
  return std::vector<ui::ColorCalibrationProfile>();
}

bool NativeDisplayDelegateDri::SetColorCalibrationProfile(
    const ui::DisplaySnapshot& output,
    ui::ColorCalibrationProfile new_profile) {
  NOTIMPLEMENTED();
  return false;
}

void NativeDisplayDelegateDri::AddObserver(NativeDisplayObserver* observer) {
  observers_.AddObserver(observer);
}

void NativeDisplayDelegateDri::RemoveObserver(NativeDisplayObserver* observer) {
  observers_.RemoveObserver(observer);
}

void NativeDisplayDelegateDri::OnDeviceEvent(const DeviceEvent& event) {
  if (event.device_type() != DeviceEvent::DISPLAY)
    return;

  if (event.action_type() == DeviceEvent::CHANGE) {
    VLOG(1) << "Got display changed event";
    FOR_EACH_OBSERVER(
        NativeDisplayObserver, observers_, OnConfigurationChanged());
  }
}

}  // namespace ui
