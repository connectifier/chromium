// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/web_resource/eula_accepted_notifier.h"

#include "base/bind.h"
#include "base/logging.h"
#include "base/prefs/pref_service.h"
#include "chrome/common/pref_names.h"

EulaAcceptedNotifier::EulaAcceptedNotifier(PrefService* local_state)
    : local_state_(local_state), observer_(NULL) {
}

EulaAcceptedNotifier::~EulaAcceptedNotifier() {
}

void EulaAcceptedNotifier::Init(Observer* observer) {
  DCHECK(!observer_ && observer);
  observer_ = observer;
}

bool EulaAcceptedNotifier::IsEulaAccepted() {
  if (local_state_->GetBoolean(prefs::kEulaAccepted))
    return true;

  // Register for the notification, if this is the first time.
  if (registrar_.IsEmpty()) {
    registrar_.Init(local_state_);
    registrar_.Add(prefs::kEulaAccepted,
                   base::Bind(&EulaAcceptedNotifier::OnPrefChanged,
                              base::Unretained(this)));
  }
  return false;
}

// static
EulaAcceptedNotifier* EulaAcceptedNotifier::Create(PrefService* local_state) {
  // First run EULA only exists on ChromeOS, Android and iOS. On ChromeOS, it is
  // only shown in official builds.
#if (defined(OS_CHROMEOS) && defined(GOOGLE_CHROME_BUILD)) || \
    defined(OS_ANDROID) || defined(OS_IOS)
  // Tests that use higher-level classes that use EulaAcceptNotifier may not
  // have local state or may not register this pref. Return NULL to indicate not
  // needing to check the EULA.
  if (!local_state || !local_state->FindPreference(prefs::kEulaAccepted))
    return NULL;
  return new EulaAcceptedNotifier(local_state);
#else
  return NULL;
#endif
}

void EulaAcceptedNotifier::NotifyObserver() {
  observer_->OnEulaAccepted();
}

void EulaAcceptedNotifier::OnPrefChanged() {
  DCHECK(!registrar_.IsEmpty());
  registrar_.RemoveAll();

  DCHECK(local_state_->GetBoolean(prefs::kEulaAccepted));
  observer_->OnEulaAccepted();
}

