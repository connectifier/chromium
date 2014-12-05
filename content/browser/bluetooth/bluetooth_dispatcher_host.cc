// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/browser/bluetooth/bluetooth_dispatcher_host.h"

#include "content/common/bluetooth/bluetooth_messages.h"
#include "device/bluetooth/bluetooth_adapter.h"
#include "device/bluetooth/bluetooth_adapter_factory.h"
#include "device/bluetooth/bluetooth_device.h"

#if defined(OS_CHROMEOS)
#include "chromeos/dbus/dbus_thread_manager.h"
#endif  // defined(OS_CHROMEOS)

using device::BluetoothAdapter;
using device::BluetoothAdapterFactory;
using device::BluetoothDevice;

namespace content {

BluetoothDispatcherHost::BluetoothDispatcherHost()
    : BrowserMessageFilter(BluetoothMsgStart),
      bluetooth_mock_data_set_(MockData::NOT_MOCKING),
      bluetooth_request_device_reject_type_(BluetoothError::NOT_FOUND) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);

//
//
// remove, probably not needed
//
//
#if defined(OS_CHROMEOS)
  // GetAdapter must wait for DBusThreadManager::IsInitialized();
  DCHECK(chromeos::DBusThreadManager::IsInitialized());
#endif
  //
  //
  //
  //
  //
  fprintf(stderr, "%s:%s:%d \n", __FILE__, __FUNCTION__, __LINE__);
  if (BluetoothAdapterFactory::IsBluetoothAdapterAvailable())
    BluetoothAdapterFactory::GetAdapter(
        base::Bind(&BluetoothDispatcherHost::set_adapter, this));
}

BluetoothDispatcherHost::~BluetoothDispatcherHost() {
  // Clear adapter, releasing observer references.
  set_adapter(scoped_refptr<device::BluetoothAdapter>());
}

void BluetoothDispatcherHost::set_adapter(
    scoped_refptr<device::BluetoothAdapter> adapter) {
  if (adapter_.get())
    adapter_->RemoveObserver(this);
  fprintf(stderr, "%s:%s:%d \n", __FILE__, __FUNCTION__, __LINE__);
  adapter_ = adapter;
  if (adapter_.get())
    adapter_->AddObserver(this);
}

bool BluetoothDispatcherHost::OnMessageReceived(const IPC::Message& message) {
  DCHECK_CURRENTLY_ON(BrowserThread::IO);
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(BluetoothDispatcherHost, message)
  IPC_MESSAGE_HANDLER(BluetoothHostMsg_RequestDevice, OnRequestDevice)
  IPC_MESSAGE_HANDLER(BluetoothHostMsg_SetBluetoothMockDataSetForTesting,
                      OnSetBluetoothMockDataSetForTesting)
  IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()
  return handled;
}

void BluetoothDispatcherHost::OnRequestDevice(int thread_id, int request_id) {
  DCHECK_CURRENTLY_ON(BrowserThread::IO);
  // Mock implementation util a more complete implementation is built out.
  switch (bluetooth_mock_data_set_) {
    case MockData::NOT_MOCKING: {
      BluetoothAdapter::DeviceList devices = adapter_->GetDevices();
      if (devices.begin() == devices.end()) {
      Send(new BluetoothMsg_RequestDeviceError(thread_id, request_id,
                                               BluetoothError::NOT_FOUND));
      return;
      }
      BluetoothDevice* device = *devices.begin();
      Send(new BluetoothMsg_RequestDeviceSuccess(thread_id, request_id,
                                                 device->GetAddress()));
    }
    case MockData::REJECT: {
      Send(new BluetoothMsg_RequestDeviceError(
          thread_id, request_id, bluetooth_request_device_reject_type_));
      return;
    }
    case MockData::RESOLVE: {
      Send(new BluetoothMsg_RequestDeviceSuccess(thread_id, request_id,
                                                 "Empty Mock deviceId"));
      return;
    }
  }
  NOTREACHED();
}

void BluetoothDispatcherHost::OnSetBluetoothMockDataSetForTesting(
    const std::string& name) {
  DCHECK_CURRENTLY_ON(BrowserThread::IO);
  if (name == "RejectRequestDevice_NotFoundError") {
    bluetooth_mock_data_set_ = MockData::REJECT;
    bluetooth_request_device_reject_type_ = BluetoothError::NOT_FOUND;
  } else if (name == "RejectRequestDevice_SecurityError") {
    bluetooth_mock_data_set_ = MockData::REJECT;
    bluetooth_request_device_reject_type_ = BluetoothError::SECURITY;
  } else if (name == "ResolveRequestDevice_Empty" ||  // TODO(scheib): Remove.
             name == "Single Empty Device") {
    bluetooth_mock_data_set_ = MockData::RESOLVE;
  } else {
    bluetooth_mock_data_set_ = MockData::NOT_MOCKING;
  }
}

}  // namespace content
