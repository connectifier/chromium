// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_PUBLIC_COMMON_MANIFEST_H_
#define CONTENT_PUBLIC_COMMON_MANIFEST_H_

#include "base/strings/nullable_string16.h"
#include "content/common/content_export.h"
#include "third_party/WebKit/public/platform/WebScreenOrientationLockType.h"
#include "url/gurl.h"

namespace content {

// The Manifest structure is an internal representation of the Manifest file
// described in the "Manifest for Web Application" document:
// http://w3c.github.io/manifest/
struct CONTENT_EXPORT Manifest {
  enum DisplayMode {
    DISPLAY_MODE_UNSPECIFIED,
    DISPLAY_MODE_FULLSCREEN,
    DISPLAY_MODE_STANDALONE,
    DISPLAY_MODE_MINIMAL_UI,
    DISPLAY_MODE_BROWSER
  };

  Manifest();
  ~Manifest();

  // Returns whether this Manifest had no attribute set. A newly created
  // Manifest is always empty.
  bool IsEmpty() const;

  // Null if the parsing failed or the field was not present.
  base::NullableString16 name;

  // Null if the parsing failed or the field was not present.
  base::NullableString16 short_name;

  // Empty if the parsing failed or the field was not present.
  GURL start_url;

  // Set to DISPLAY_MODE_UNSPECIFIED if the parsing failed or the field was not
  // present.
  DisplayMode display;

  // Set to blink::WebScreenOrientationLockDefault if the parsing failed or the
  // field was not present.
  blink::WebScreenOrientationLockType orientation;

  // Maximum length for all the strings inside the Manifest when it is sent over
  // IPC. The renderer process should truncate the strings before sending the
  // Manifest and the browser process must do the same when receiving it.
  static const size_t kMaxIPCStringLength;
};

} // namespace content

#endif // CONTENT_PUBLIC_COMMON_MANIFEST_H_
