// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_RAPPOR_TEST_RAPPOR_SERVICE_H_
#define COMPONENTS_RAPPOR_TEST_RAPPOR_SERVICE_H_

#include <string>

#include "base/prefs/testing_pref_service.h"
#include "components/rappor/rappor_service.h"
#include "components/rappor/test_log_uploader.h"

namespace rappor {

// This class provides a simple instance that can be instantiated by tests
// and examined to check that metrics were recorded.  It assumes the most
// permissive settings so that any metric can be recorded.
class TestRapporService : public RapporService {
 public:
  TestRapporService();

  ~TestRapporService() override;

  // Gets the number of reports that would be uploaded by this service.
  // This also clears the internal map of metrics as a biproduct, so if
  // comparing numbers of reports, the comparison should be from the last time
  // GetReportsCount() was called (not from the beginning of the test).
  int GetReportsCount();

  // Gets the reports proto that would be uploaded.
  // This clears the internal map of metrics.
  void GetReports(RapporReports* reports);

  void set_is_incognito(bool is_incognito) { is_incognito_ = is_incognito; }

  TestingPrefServiceSimple* test_prefs() { return &test_prefs_; }

  TestLogUploader* test_uploader() { return test_uploader_; }

  base::TimeDelta next_rotation() { return next_rotation_; }

 protected:
  // Cancels the next call to OnLogInterval.
  void CancelNextLogRotation() override;

  // Schedules the next call to OnLogInterval.
  void ScheduleNextLogRotation(base::TimeDelta interval) override;

 private:
  TestingPrefServiceSimple test_prefs_;

  // Holds a weak ref to the uploader_ object.
  TestLogUploader* test_uploader_;

  // The last scheduled log rotation.
  base::TimeDelta next_rotation_;

  // Sets this to true to mock incognito state.
  bool is_incognito_;

  DISALLOW_COPY_AND_ASSIGN(TestRapporService);
};

}  // namespace rappor

#endif  // COMPONENTS_RAPPOR_TEST_RAPPOR_SERVICE_H_
