// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_SERVICE_WORKER_SERVICE_WORKER_CONTEXT_WRAPPER_H_
#define CONTENT_BROWSER_SERVICE_WORKER_SERVICE_WORKER_CONTEXT_WRAPPER_H_

#include <vector>

#include "base/files/file_path.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "content/browser/service_worker/service_worker_context_core.h"
#include "content/common/content_export.h"
#include "content/public/browser/service_worker_context.h"

namespace base {
class FilePath;
class SequencedTaskRunner;
}

namespace quota {
class QuotaManagerProxy;
}

namespace content {

class BrowserContext;
class ServiceWorkerContextCore;
class ServiceWorkerContextObserver;

// A refcounted wrapper class for our core object. Higher level content lib
// classes keep references to this class on mutliple threads. The inner core
// instance is strictly single threaded and is not refcounted, the core object
// is what is used internally in the service worker lib.
class CONTENT_EXPORT ServiceWorkerContextWrapper
    : NON_EXPORTED_BASE(public ServiceWorkerContext),
      public base::RefCountedThreadSafe<ServiceWorkerContextWrapper> {
 public:
  typedef base::Callback<void(base::WeakPtr<ServiceWorkerContextCore>)> ServiceWorkerContextCoreCallback;

  ServiceWorkerContextWrapper(BrowserContext* browser_context);

  // Init and Shutdown are for use on the UI thread when the profile,
  // storagepartition is being setup and torn down.
  void Init(const base::FilePath& user_data_directory,
            quota::QuotaManagerProxy* quota_manager_proxy);
  void Shutdown();

  // The core context is only for use on the IO thread.
  ServiceWorkerContextCore* context();

  // ServiceWorkerContext implementation:
  virtual void RegisterServiceWorker(
      const Scope& scope,
      const GURL& script_url,
      ServiceWorkerHostClient* client,
      const ServiceWorkerHostCallback& callback) OVERRIDE;
  virtual void UnregisterServiceWorker(const GURL& pattern,
                                       const ResultCallback& callback) OVERRIDE;
  virtual void GetServiceWorkerHost(
      const Scope& scope,
      ServiceWorkerHostClient* client,
      const ServiceWorkerHostCallback& callback) OVERRIDE;

  void AddObserver(ServiceWorkerContextObserver* observer);
  void RemoveObserver(ServiceWorkerContextObserver* observer);

 private:
  friend class base::RefCountedThreadSafe<ServiceWorkerContextWrapper>;
  friend class EmbeddedWorkerTestHelper;
  friend class ServiceWorkerProcessManager;
  virtual ~ServiceWorkerContextWrapper();

  void InitForTesting(const base::FilePath& user_data_directory,
                      base::SequencedTaskRunner* database_task_runner,
                      quota::QuotaManagerProxy* quota_manager_proxy);
  void InitInternal(const base::FilePath& user_data_directory,
                    base::SequencedTaskRunner* database_task_runner,
                    quota::QuotaManagerProxy* quota_manager_proxy);

  // Completes the registration process on IO thread.
  void FinishRegistrationOnIO(const Scope& scope,
                              ServiceWorkerHostClient* client,
                              const ServiceWorkerHostCallback& callback,
                              ServiceWorkerStatusCode status,
                              int64 registration_id,
                              int64 version_id);

  const scoped_refptr<ObserverListThreadSafe<ServiceWorkerContextObserver> >
      observer_list_;
  // Cleared in Shutdown():
  BrowserContext* browser_context_;
  scoped_ptr<ServiceWorkerContextCore> context_core_;
};

}  // namespace content

#endif  // CONTENT_BROWSER_SERVICE_WORKER_SERVICE_WORKER_CONTEXT_WRAPPER_H_
