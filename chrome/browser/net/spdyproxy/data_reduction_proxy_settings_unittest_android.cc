// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/net/spdyproxy/data_reduction_proxy_settings_android.h"

#include "base/android/jni_android.h"
#include "base/android/jni_string.h"
#include "base/android/scoped_java_ref.h"
#include "base/base64.h"
#include "base/command_line.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "base/prefs/pref_service.h"
#include "base/test/test_simple_task_runner.h"
#include "base/time/time.h"
#include "chrome/browser/net/spdyproxy/data_reduction_proxy_chrome_settings.h"
#include "chrome/browser/prefs/proxy_prefs.h"
#include "chrome/common/chrome_switches.h"
#include "chrome/common/pref_names.h"
#include "components/data_reduction_proxy/core/browser/data_reduction_proxy_config_test_utils.h"
#include "components/data_reduction_proxy/core/browser/data_reduction_proxy_settings_test_utils.h"
#include "components/data_reduction_proxy/core/browser/data_reduction_proxy_test_utils.h"
#include "components/data_reduction_proxy/core/common/data_reduction_proxy_params.h"
#include "components/data_reduction_proxy/core/common/data_reduction_proxy_params_test_utils.h"
#include "net/proxy/proxy_server.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "url/gurl.h"

using testing::_;
using testing::AnyNumber;
using testing::Return;

const char kDataReductionProxyDev[] = "foo-dev.com:80";

using data_reduction_proxy::DataReductionProxySettings;

// Used for testing the DataReductionProxySettingsAndroid class.
class TestDataReductionProxySettingsAndroid
    : public DataReductionProxySettingsAndroid {
 public:
  // Constructs an Android settings object for test that wraps the provided
  // settings object.
  explicit TestDataReductionProxySettingsAndroid(
      DataReductionProxySettings* settings)
      : DataReductionProxySettingsAndroid(),
        settings_(settings) {}

  // Returns the provided setting object. Used by wrapping methods.
  DataReductionProxySettings* Settings() override { return settings_; }

  // The wrapped settings object.
  DataReductionProxySettings* settings_;
};

template <class C>
void data_reduction_proxy::DataReductionProxySettingsTestBase::ResetSettings(
    bool allowed,
    bool fallback_allowed,
    bool alt_allowed,
    bool promo_allowed,
    bool holdback) {
  int flags = 0;
  if (allowed)
    flags |= DataReductionProxyParams::kAllowed;
  if (fallback_allowed)
    flags |= DataReductionProxyParams::kFallbackAllowed;
  if (alt_allowed)
    flags |= DataReductionProxyParams::kAlternativeAllowed;
  if (promo_allowed)
    flags |= DataReductionProxyParams::kPromoAllowed;
  if (holdback)
    flags |= DataReductionProxyParams::kHoldback;
  MockDataReductionProxySettings<C>* settings =
      new MockDataReductionProxySettings<C>();
  settings->config_ = test_context_->config();
  settings->data_reduction_proxy_service_ =
      test_context_->CreateDataReductionProxyService();
  test_context_->config()->ResetParamFlagsForTest(flags);
  settings->UpdateConfigValues();
  EXPECT_CALL(*settings, GetOriginalProfilePrefs())
      .Times(AnyNumber())
      .WillRepeatedly(Return(test_context_->pref_service()));
  EXPECT_CALL(*settings, GetLocalStatePrefs())
      .Times(AnyNumber())
      .WillRepeatedly(Return(test_context_->pref_service()));
  settings_.reset(settings);
}

template void
data_reduction_proxy::DataReductionProxySettingsTestBase::ResetSettings<
    DataReductionProxyChromeSettings>(bool allowed,
                                       bool fallback_allowed,
                                       bool alt_allowed,
                                       bool promo_allowed,
                                       bool holdback);

class DataReductionProxySettingsAndroidTest
    : public data_reduction_proxy::ConcreteDataReductionProxySettingsTest<
          DataReductionProxyChromeSettings> {
 public:
  // DataReductionProxySettingsTest implementation:
  void SetUp() override {
    env_ = base::android::AttachCurrentThread();
    DataReductionProxySettingsAndroid::Register(env_);
    DataReductionProxySettingsTestBase::SetUp();
    ResetSettingsAndroid();
  }

  void ResetSettingsAndroid() {
    settings_android_.reset(new TestDataReductionProxySettingsAndroid(
        settings_.get()));
  }

  DataReductionProxySettings* Settings() {
    return settings_.get();
  }

  DataReductionProxySettingsAndroid* SettingsAndroid() {
    return settings_android_.get();
  }

  scoped_ptr<DataReductionProxySettingsAndroid> settings_android_;
  JNIEnv* env_;
};

TEST_F(DataReductionProxySettingsAndroidTest, TestGetDataReductionProxyOrigin) {
  // SetUp() adds the origin to the command line, which should be returned here.
  ScopedJavaLocalRef<jstring> result =
      SettingsAndroid()->GetDataReductionProxyOrigin(env_, NULL);
  ASSERT_TRUE(result.obj());
  const base::android::JavaRef<jstring>& str_ref = result;
  EXPECT_EQ(test_context_->config()->test_params()->DefaultOrigin(),
            ConvertJavaStringToUTF8(str_ref));
}

TEST_F(DataReductionProxySettingsAndroidTest,
       TestGetDataReductionProxyDevOrigin) {
  base::CommandLine::ForCurrentProcess()->AppendSwitchASCII(
      data_reduction_proxy::switches::kDataReductionProxyDev,
      kDataReductionProxyDev);
  ResetSettings(true, true, false, true, false);
  ResetSettingsAndroid();
  ScopedJavaLocalRef<jstring> result =
      SettingsAndroid()->GetDataReductionProxyOrigin(env_, NULL);
  ASSERT_TRUE(result.obj());
  const base::android::JavaRef<jstring>& str_ref = result;
  EXPECT_EQ(kDataReductionProxyDev,
            ConvertJavaStringToUTF8(str_ref));
}

TEST_F(DataReductionProxySettingsAndroidTest, TestGetDailyContentLengths) {
  ScopedJavaLocalRef<jlongArray> result =
      SettingsAndroid()->GetDailyContentLengths(
          env_, data_reduction_proxy::prefs::kDailyHttpOriginalContentLength);
  ASSERT_TRUE(result.obj());

  jsize java_array_len = env_->GetArrayLength(result.obj());
  ASSERT_EQ(static_cast<jsize>(data_reduction_proxy::kNumDaysInHistory),
            java_array_len);

  jlong value;
  for (size_t i = 0; i < data_reduction_proxy::kNumDaysInHistory; ++i) {
    env_->GetLongArrayRegion(result.obj(), i, 1, &value);
    ASSERT_EQ(
        static_cast<long>(
            (data_reduction_proxy::kNumDaysInHistory - 1 - i) * 2), value);
  }
}

