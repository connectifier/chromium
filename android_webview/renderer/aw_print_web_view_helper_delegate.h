// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ANDROID_WEBVIEW_RENDERER_AW_PRINT_WEB_VIEW_HELPER_DELEGATE_H_
#define ANDROID_WEBVIEW_RENDERER_AW_PRINT_WEB_VIEW_HELPER_DELEGATE_H_

#include "components/printing/renderer/print_web_view_helper.h"

namespace android_webview {

class AwPrintWebViewHelperDelegate
    : public printing::PrintWebViewHelper::Delegate {
 public:
  ~AwPrintWebViewHelperDelegate() override;
  bool CancelPrerender(content::RenderView* render_view,
                       int routing_id) override;
  blink::WebElement GetPdfElement(blink::WebLocalFrame* frame) override;
  bool IsOutOfProcessPdfEnabled() override;
  bool IsPrintPreviewEnabled() override;
  bool IsAskPrintSettingsEnabled() override;
  bool IsScriptedPrintEnabled() override;
  bool OverridePrint(blink::WebLocalFrame* frame) override;
};  // class AwPrintWebViewHelperDelegate

}  // namespace android_webview

#endif  // ANDROID_WEBVIEW_RENDERER_AW_PRINT_WEB_VIEW_HELPER_DELEGATE_H_
