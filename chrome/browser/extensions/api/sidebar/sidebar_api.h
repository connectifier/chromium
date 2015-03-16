// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_EXTENSIONS_EXTENSION_SIDEBAR_API_H_
#define CHROME_BROWSER_EXTENSIONS_EXTENSION_SIDEBAR_API_H_

#include <string>
#include "chrome/browser/extensions/chrome_extension_function.h"

class Profile;

namespace base {
class DictionaryValue;
}

namespace content {
class WebContents;
}

namespace extension_sidebar_constants {
extern const char kActiveState[]; // TODO(kfowler) remove
extern const char kHiddenState[]; // TODO(kfowler) remove
extern const char kShownState[]; // TODO(kfowler) remove
extern const char kShownFlag[];
extern const char kPinnedFlag[];
}  // namespace extension_sidebar_constants

// Event router class for events related to the sidebar API.
class ExtensionSidebarEventRouter {
 public:
  // Sidebar state changed.
  static void OnStateChanged(
      Profile* profile, content::WebContents* tab,
      const std::string& content_id, const std::string& state);

 private:
  DISALLOW_COPY_AND_ASSIGN(ExtensionSidebarEventRouter);
};

// Base class for sidebar function APIs.
class SidebarFunction : public ChromeSyncExtensionFunction {
 public:
  bool RunSync() override;
 protected:
  ~SidebarFunction() override {};

 private:
  virtual bool RunImpl(content::WebContents* tab,
                       const std::string& content_id,
                       const base::DictionaryValue& details) = 0;
};

class SidebarGetStateFunction : public SidebarFunction {
 private:
  bool RunImpl(content::WebContents* tab,
                       const std::string& content_id,
                       const base::DictionaryValue& details) override;
protected:
  ~SidebarGetStateFunction() override {} ;

  DECLARE_EXTENSION_FUNCTION("sidebar.getState", SIDEBAR_GETSTATE);
};

class SidebarHideFunction : public SidebarFunction {
 private:
  bool RunImpl(content::WebContents* tab,
                       const std::string& content_id,
                       const base::DictionaryValue& details) override;
protected:
  ~SidebarHideFunction() override {} ;

  DECLARE_EXTENSION_FUNCTION("sidebar.hide", SIDEBAR_HIDE);
};

class SidebarShowFunction : public SidebarFunction {
 private:
  bool RunImpl(content::WebContents* tab,
                       const std::string& content_id,
                       const base::DictionaryValue& details) override;
protected:
  ~SidebarShowFunction() override {} ;

  DECLARE_EXTENSION_FUNCTION("sidebar.show", SIDEBAR_SHOW);
};

#endif  // CHROME_BROWSER_EXTENSIONS_EXTENSION_SIDEBAR_API_H_
