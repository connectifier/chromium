// Copyright (c) 2009 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef VIEWS_CONTROLS_TABBED_PANE_NATIVE_TABBED_PANE_WIN_H_
#define VIEWS_CONTROLS_TABBED_PANE_NATIVE_TABBED_PANE_WIN_H_

#include "views/controls/native_control_win.h"
#include "views/controls/tabbed_pane/native_tabbed_pane_wrapper.h"

namespace views {

class WidgetWin;

class NativeTabbedPaneWin : public NativeControlWin,
                            public NativeTabbedPaneWrapper {
 public:
  explicit NativeTabbedPaneWin(TabbedPane* tabbed_pane);
  virtual ~NativeTabbedPaneWin();

  // NativeTabbedPaneWrapper implementation:
  virtual void AddTab(const std::wstring& title, View* contents);
  virtual void AddTabAtIndex(int index,
                             const std::wstring& title,
                             View* contents,
                             bool select_if_first_tab);
  virtual View* RemoveTabAtIndex(int index);
  virtual void SelectTabAt(int index);
  virtual int GetTabCount();
  virtual int GetSelectedTabIndex();
  virtual View* GetSelectedTab();
  virtual View* GetView();
  virtual void SetFocus();
  virtual gfx::NativeView GetTestingHandle() const;

  // NativeControlWin overrides.
  virtual void CreateNativeControl();
  virtual bool ProcessMessage(UINT message,
                              WPARAM w_param,
                              LPARAM l_param,
                              LRESULT* result);

  // View overrides:
  virtual void Layout();
  virtual FocusTraversable* GetFocusTraversable();
  virtual void ViewHierarchyChanged(bool is_add, View *parent, View *child);

 private:
  // Changes the contents view to the view associated with the tab at |index|.
  void DoSelectTabAt(int index);

  // Resizes the HWND control to macth the size of the containing view.
  void ResizeContents();

  // The tabbed-pane we are bound to.
  TabbedPane* tabbed_pane_;

  // The views associated with the different tabs.
  std::vector<View*> tab_views_;

  // The window displayed in the tab.
  WidgetWin* content_window_;

  DISALLOW_COPY_AND_ASSIGN(NativeTabbedPaneWin);
};

}  // namespace views

#endif  // VIEWS_CONTROLS_TABBED_PANE_NATIVE_TABBED_PANE_WIN_H_
