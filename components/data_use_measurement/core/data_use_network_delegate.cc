// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/data_use_measurement/core/data_use_network_delegate.h"

#include <memory>
#include <utility>

#include "components/data_use_measurement/core/data_use_ascriber.h"
#include "components/data_use_measurement/core/url_request_classifier.h"
#include "net/url_request/url_request.h"

namespace data_use_measurement {
DataUseNetworkDelegate::DataUseNetworkDelegate(
    std::unique_ptr<NetworkDelegate> nested_network_delegate,
    DataUseAscriber* ascriber,
    std::unique_ptr<URLRequestClassifier> url_request_classifier,
    const metrics::UpdateUsagePrefCallbackType& metrics_data_use_forwarder)
    : net::LayeredNetworkDelegate(std::move(nested_network_delegate)),
      ascriber_(ascriber),
      data_use_measurement_(std::move(url_request_classifier),
                            metrics_data_use_forwarder) {
  DCHECK(ascriber);
}

DataUseNetworkDelegate::~DataUseNetworkDelegate() {}

void DataUseNetworkDelegate::OnBeforeURLRequestInternal(
    net::URLRequest* request,
    const net::CompletionCallback& callback,
    GURL* new_url) {
  ascriber_->OnBeforeUrlRequest(request);
  data_use_measurement_.OnBeforeURLRequest(request);
}

void DataUseNetworkDelegate::OnBeforeRedirectInternal(
    net::URLRequest* request,
    const GURL& new_location) {
  data_use_measurement_.OnBeforeRedirect(*request, new_location);
}

void DataUseNetworkDelegate::OnNetworkBytesReceivedInternal(
    net::URLRequest* request,
    int64_t bytes_received) {
  ascriber_->OnNetworkBytesReceived(request, bytes_received);
  data_use_measurement_.OnNetworkBytesReceived(*request, bytes_received);
}

void DataUseNetworkDelegate::OnNetworkBytesSentInternal(
    net::URLRequest* request,
    int64_t bytes_sent) {
  ascriber_->OnNetworkBytesSent(request, bytes_sent);
  data_use_measurement_.OnNetworkBytesSent(*request, bytes_sent);
}

void DataUseNetworkDelegate::OnCompletedInternal(net::URLRequest* request,
                                                 bool started) {
  data_use_measurement_.OnCompleted(*request, started);
}

void DataUseNetworkDelegate::OnURLRequestDestroyedInternal(
    net::URLRequest* request) {
  ascriber_->OnUrlRequestDestroyed(request);
}

}  // namespace data_use_measurement
