// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/browser/permissions/permission_service_impl.h"

#include <stddef.h>

#include <memory>
#include <utility>

#include "base/bind.h"
#include "base/memory/ptr_util.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/permission_manager.h"
#include "content/public/browser/permission_type.h"

using blink::mojom::PermissionDescriptorPtr;
using blink::mojom::PermissionName;
using blink::mojom::PermissionStatus;

namespace content {

namespace {

PermissionType PermissionDescriptorToPermissionType(
    const PermissionDescriptorPtr& descriptor) {
  switch (descriptor->name) {
    case PermissionName::GEOLOCATION:
      return PermissionType::GEOLOCATION;
    case PermissionName::NOTIFICATIONS:
      return PermissionType::NOTIFICATIONS;
    case PermissionName::PUSH_NOTIFICATIONS:
      return PermissionType::PUSH_MESSAGING;
    case PermissionName::MIDI: {
      if (descriptor->extension && descriptor->extension->is_midi() &&
          descriptor->extension->get_midi()->sysex) {
        return PermissionType::MIDI_SYSEX;
      }
      return PermissionType::MIDI;
    }
    case PermissionName::PROTECTED_MEDIA_IDENTIFIER:
      return PermissionType::PROTECTED_MEDIA_IDENTIFIER;
    case PermissionName::DURABLE_STORAGE:
      return PermissionType::DURABLE_STORAGE;
    case PermissionName::AUDIO_CAPTURE:
      return PermissionType::AUDIO_CAPTURE;
    case PermissionName::VIDEO_CAPTURE:
      return PermissionType::VIDEO_CAPTURE;
    case PermissionName::BACKGROUND_SYNC:
      return PermissionType::BACKGROUND_SYNC;
  }

  NOTREACHED();
  return PermissionType::NUM;
}

// This function allows the usage of the the multiple request map
// with single requests.
void PermissionRequestResponseCallbackWrapper(
    const base::Callback<void(PermissionStatus)>& callback,
    const std::vector<PermissionStatus>& vector) {
  DCHECK_EQ(vector.size(), 1ul);
  callback.Run(vector[0]);
}

} // anonymous namespace

PermissionServiceImpl::PendingRequest::PendingRequest(
    const RequestPermissionsCallback& callback,
    int request_count)
    : callback(callback),
      request_count(request_count) {
}

PermissionServiceImpl::PendingRequest::~PendingRequest() {
  if (callback.is_null())
    return;

  std::vector<PermissionStatus> result(request_count, PermissionStatus::DENIED);
  callback.Run(result);
}

PermissionServiceImpl::PendingSubscription::PendingSubscription(
    PermissionType permission,
    const url::Origin& origin,
    const PermissionStatusCallback& callback)
    : id(-1), permission(permission), origin(origin), callback(callback) {}

PermissionServiceImpl::PendingSubscription::~PendingSubscription() {
  if (!callback.is_null())
    callback.Run(PermissionStatus::ASK);
}

PermissionServiceImpl::PermissionServiceImpl(
    PermissionServiceContext* context,
    mojo::InterfaceRequest<blink::mojom::PermissionService> request)
    : context_(context),
      binding_(this, std::move(request)),
      weak_factory_(this) {
  binding_.set_connection_error_handler(
      base::Bind(&PermissionServiceImpl::OnConnectionError,
                 base::Unretained(this)));
}

PermissionServiceImpl::~PermissionServiceImpl() {
  DCHECK(pending_requests_.IsEmpty());
}

void PermissionServiceImpl::OnConnectionError() {
  CancelPendingOperations();
  context_->ServiceHadConnectionError(this);
  // After that call, |this| will be deleted.
}

void PermissionServiceImpl::RequestPermission(
    PermissionDescriptorPtr permission,
    const url::Origin& origin,
    bool user_gesture,
    const PermissionStatusCallback& callback) {
  // This condition is valid if the call is coming from a ChildThread instead of
  // a RenderFrame. Some consumers of the service run in Workers and some in
  // Frames. In the context of a Worker, it is not possible to show a
  // permission prompt because there is no tab. In the context of a Frame, we
  // can. Even if the call comes from a context where it is not possible to show
  // any UI, we want to still return something relevant so the current
  // permission status is returned.
  BrowserContext* browser_context = context_->GetBrowserContext();
  DCHECK(browser_context);
  if (!context_->render_frame_host() ||
      !browser_context->GetPermissionManager()) {
    callback.Run(GetPermissionStatus(permission, origin));
    return;
  }

  int pending_request_id =
      pending_requests_.Add(base::MakeUnique<PendingRequest>(
          base::Bind(&PermissionRequestResponseCallbackWrapper, callback), 1));
  int id = browser_context->GetPermissionManager()->RequestPermission(
      PermissionDescriptorToPermissionType(permission),
      context_->render_frame_host(), origin.GetURL(), user_gesture,
      base::Bind(&PermissionServiceImpl::OnRequestPermissionResponse,
                 weak_factory_.GetWeakPtr(), pending_request_id));

  // Check if the request still exists. It might have been removed by the
  // callback if it was run synchronously.
  PendingRequest* pending_request = pending_requests_.Lookup(
      pending_request_id);
  if (!pending_request)
      return;
  pending_request->id = id;
}

void PermissionServiceImpl::OnRequestPermissionResponse(
    int pending_request_id,
    PermissionStatus status) {
  OnRequestPermissionsResponse(pending_request_id,
                               std::vector<PermissionStatus>(1, status));
}

void PermissionServiceImpl::RequestPermissions(
    std::vector<PermissionDescriptorPtr> permissions,
    const url::Origin& origin,
    bool user_gesture,
    const RequestPermissionsCallback& callback) {
  // This condition is valid if the call is coming from a ChildThread instead of
  // a RenderFrame. Some consumers of the service run in Workers and some in
  // Frames. In the context of a Worker, it is not possible to show a
  // permission prompt because there is no tab. In the context of a Frame, we
  // can. Even if the call comes from a context where it is not possible to show
  // any UI, we want to still return something relevant so the current
  // permission status is returned for each permission.
  BrowserContext* browser_context = context_->GetBrowserContext();
  DCHECK(browser_context);
  if (!context_->render_frame_host() ||
      !browser_context->GetPermissionManager()) {
    std::vector<PermissionStatus> result(permissions.size());
    for (size_t i = 0; i < permissions.size(); ++i)
      result[i] = GetPermissionStatus(permissions[i], origin);
    callback.Run(result);
    return;
  }

  std::vector<PermissionType> types(permissions.size());
  for (size_t i = 0; i < types.size(); ++i)
    types[i] = PermissionDescriptorToPermissionType(permissions[i]);

  int pending_request_id = pending_requests_.Add(
      base::MakeUnique<PendingRequest>(callback, permissions.size()));
  int id = browser_context->GetPermissionManager()->RequestPermissions(
      types, context_->render_frame_host(), origin.GetURL(), user_gesture,
      base::Bind(&PermissionServiceImpl::OnRequestPermissionsResponse,
                 weak_factory_.GetWeakPtr(), pending_request_id));

  // Check if the request still exists. It may have been removed by the
  // the response callback.
  PendingRequest* pending_request = pending_requests_.Lookup(
      pending_request_id);
  if (!pending_request)
      return;
  pending_request->id = id;
}

void PermissionServiceImpl::OnRequestPermissionsResponse(
    int pending_request_id,
    const std::vector<PermissionStatus>& result) {
  PendingRequest* request = pending_requests_.Lookup(pending_request_id);
  RequestPermissionsCallback callback(request->callback);
  request->callback.Reset();
  pending_requests_.Remove(pending_request_id);
  callback.Run(result);
}

void PermissionServiceImpl::CancelPendingOperations() {
  DCHECK(context_->GetBrowserContext());

  PermissionManager* permission_manager =
      context_->GetBrowserContext()->GetPermissionManager();
  if (!permission_manager)
    return;

  // Cancel pending requests.
  for (RequestsMap::Iterator<PendingRequest> it(&pending_requests_);
       !it.IsAtEnd(); it.Advance()) {
    permission_manager->CancelPermissionRequest(
        it.GetCurrentValue()->id);
  }
  pending_requests_.Clear();

  // Cancel pending subscriptions.
  for (SubscriptionsMap::Iterator<PendingSubscription>
          it(&pending_subscriptions_); !it.IsAtEnd(); it.Advance()) {
    it.GetCurrentValue()->callback.Run(GetPermissionStatusFromType(
        it.GetCurrentValue()->permission, it.GetCurrentValue()->origin));
    it.GetCurrentValue()->callback.Reset();
    permission_manager->UnsubscribePermissionStatusChange(
        it.GetCurrentValue()->id);
  }
  pending_subscriptions_.Clear();
}

void PermissionServiceImpl::HasPermission(
    PermissionDescriptorPtr permission,
    const url::Origin& origin,
    const PermissionStatusCallback& callback) {
  callback.Run(GetPermissionStatus(permission, origin));
}

void PermissionServiceImpl::RevokePermission(
    PermissionDescriptorPtr permission,
    const url::Origin& origin,
    const PermissionStatusCallback& callback) {
  PermissionType permission_type =
      PermissionDescriptorToPermissionType(permission);
  PermissionStatus status =
      GetPermissionStatusFromType(permission_type, origin);

  // Resetting the permission should only be possible if the permission is
  // already granted.
  if (status != PermissionStatus::GRANTED) {
    callback.Run(status);
    return;
  }

  ResetPermissionStatus(permission_type, origin);

  callback.Run(GetPermissionStatusFromType(permission_type, origin));
}

void PermissionServiceImpl::GetNextPermissionChange(
    PermissionDescriptorPtr permission,
    const url::Origin& origin,
    PermissionStatus last_known_status,
    const PermissionStatusCallback& callback) {
  PermissionStatus current_status = GetPermissionStatus(permission, origin);
  if (current_status != last_known_status) {
    callback.Run(current_status);
    return;
  }

  BrowserContext* browser_context = context_->GetBrowserContext();
  DCHECK(browser_context);
  if (!browser_context->GetPermissionManager()) {
    callback.Run(current_status);
    return;
  }

  PermissionType permission_type =
      PermissionDescriptorToPermissionType(permission);

  // We need to pass the id of PendingSubscription in pending_subscriptions_
  // to the callback but SubscribePermissionStatusChange() will also return an
  // id which is different.
  auto subscription =
      base::MakeUnique<PendingSubscription>(permission_type, origin, callback);
  PendingSubscription* subscription_raw = subscription.get();
  int pending_subscription_id =
      pending_subscriptions_.Add(std::move(subscription));

  GURL requesting_origin(origin.Serialize());
  GURL embedding_origin = context_->GetEmbeddingOrigin();
  subscription_raw->id =
      browser_context->GetPermissionManager()->SubscribePermissionStatusChange(
          permission_type, requesting_origin,
          // If the embedding_origin is empty, we,ll use the |origin| instead.
          embedding_origin.is_empty() ? requesting_origin : embedding_origin,
          base::Bind(&PermissionServiceImpl::OnPermissionStatusChanged,
                     weak_factory_.GetWeakPtr(), pending_subscription_id));
}

PermissionStatus PermissionServiceImpl::GetPermissionStatus(
    const PermissionDescriptorPtr& permission,
    const url::Origin& origin) {
  return GetPermissionStatusFromType(
      PermissionDescriptorToPermissionType(permission), origin);
}

PermissionStatus PermissionServiceImpl::GetPermissionStatusFromType(
    PermissionType type,
    const url::Origin& origin) {
  BrowserContext* browser_context = context_->GetBrowserContext();
  DCHECK(browser_context);
  if (!browser_context->GetPermissionManager())
    return PermissionStatus::DENIED;

  GURL requesting_origin(origin.Serialize());
  // If the embedding_origin is empty we'll use |origin| instead.
  GURL embedding_origin = context_->GetEmbeddingOrigin();
  return browser_context->GetPermissionManager()->GetPermissionStatus(
      type, requesting_origin,
      embedding_origin.is_empty() ? requesting_origin : embedding_origin);
}

void PermissionServiceImpl::ResetPermissionStatus(PermissionType type,
                                                  const url::Origin& origin) {
  BrowserContext* browser_context = context_->GetBrowserContext();
  DCHECK(browser_context);
  if (!browser_context->GetPermissionManager())
    return;

  GURL requesting_origin(origin.Serialize());
  // If the embedding_origin is empty we'll use |origin| instead.
  GURL embedding_origin = context_->GetEmbeddingOrigin();
  browser_context->GetPermissionManager()->ResetPermission(
      type, requesting_origin,
      embedding_origin.is_empty() ? requesting_origin : embedding_origin);
}

void PermissionServiceImpl::OnPermissionStatusChanged(
    int pending_subscription_id,
    PermissionStatus status) {
  PendingSubscription* subscription =
      pending_subscriptions_.Lookup(pending_subscription_id);

  BrowserContext* browser_context = context_->GetBrowserContext();
  DCHECK(browser_context);
  if (browser_context->GetPermissionManager()) {
    browser_context->GetPermissionManager()->UnsubscribePermissionStatusChange(
        subscription->id);
  }

  PermissionStatusCallback callback = subscription->callback;

  subscription->callback.Reset();
  pending_subscriptions_.Remove(pending_subscription_id);

  callback.Run(status);
}

}  // namespace content
