// Copyright (c) 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DEVICE_BLUETOOTH_DEVICE_BLUETOOTH_EXPORT_H_
#define DEVICE_BLUETOOTH_DEVICE_BLUETOOTH_EXPORT_H_

#if defined(COMPONENT_BUILD)
#if defined(WIN32)

#if defined(DEVICE_BLUETOOTH_IMPLEMENTATION)
#define DEVICE_BLUETOOTH_EXPORT __declspec(dllexport)
#else
#define DEVICE_BLUETOOTH_EXPORT __declspec(dllimport)
#endif  // defined(DEVICE_BLUETOOTH_IMPLEMENTATION)

#else  // defined(WIN32)

#if defined(DEVICE_BLUETOOTH_IMPLEMENTATION)
#define DEVICE_BLUETOOTH_EXPORT __attribute__((visibility("default")))
#else
#define DEVICE_BLUETOOTH_EXPORT
#endif

#endif  // defined(WIN32)

#else  // defined(COMPONENT_BUILD)

#define DEVICE_BLUETOOTH_EXPORT

#endif  // defined(COMPONENT_BUILD)

#endif  // DEVICE_BLUETOOTH_DEVICE_BLUETOOTH_EXPORT_H_
