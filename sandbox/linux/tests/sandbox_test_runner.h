// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SANDBOX_LINUX_TESTS_SANDBOX_TEST_RUNNER_H_
#define SANDBOX_LINUX_TESTS_SANDBOX_TEST_RUNNER_H_

#include "base/basictypes.h"

namespace sandbox {

// A simple "runner" class to implement tests.
class SandboxTestRunner {
 public:
  SandboxTestRunner() {}
  virtual ~SandboxTestRunner() {}
  virtual void Run() = 0;

 private:
  DISALLOW_COPY_AND_ASSIGN(SandboxTestRunner);
};

}  // namespace sandbox

#endif  // SANDBOX_LINUX_TESTS_SANDBOX_TEST_RUNNER_H_
