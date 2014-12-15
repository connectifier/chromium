// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_HOST_ZOOM_LEVEL_CONTEXT_H_
#define CONTENT_BROWSER_HOST_ZOOM_LEVEL_CONTEXT_H_

#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "content/browser/host_zoom_map_impl.h"
#include "content/public/browser/zoom_level_delegate.h"

class PrefService;

namespace content {
struct HostZoomLevelContextDeleter;

// This class manages a HostZoomMap and associates it with a ZoomLevelDelegate,
// if one is provided. It also serves to keep the zoom level machinery details
// separate from the owning StoragePartitionImpl.
class HostZoomLevelContext
    : public base::RefCountedThreadSafe<
        HostZoomLevelContext, DeleteOnCorrectThreadRefCountedThreadSafeTraits> {
 public:
  explicit HostZoomLevelContext(
      scoped_ptr<ZoomLevelDelegate> zoom_level_delegate);

  HostZoomMap* GetHostZoomMap() const { return host_zoom_map_impl_.get(); }
  ZoomLevelDelegate* GetZoomLevelDelegate() const {
    return zoom_level_delegate_.get();
  }

 protected:
  virtual ~HostZoomLevelContext();

 private:
  friend class base::DeleteHelper<HostZoomLevelContext>;
  friend class base::RefCountedThreadSafe<
      HostZoomLevelContext, DeleteOnCorrectThreadRefCountedThreadSafeTraits>;
  friend struct DeleteOnCorrectThreadRefCountedThreadSafeTraits;

  void DeleteOnCorrectThread() const;

  scoped_ptr<HostZoomMapImpl> host_zoom_map_impl_;
  // Release the delegate before the HostZoomMap, in case it is carrying
  // any HostZoomMap::Subscription pointers.
  scoped_ptr<ZoomLevelDelegate> zoom_level_delegate_;

  DISALLOW_COPY_AND_ASSIGN(HostZoomLevelContext);
};

}  // namespace content

#endif  // CONTENT_BROWSER_HOST_ZOOM_LEVEL_CONTEXT_H_
