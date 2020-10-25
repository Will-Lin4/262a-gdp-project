﻿/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#include <aws/core/utils/Outcome.h>
#include <aws/core/auth/AWSAuthSigner.h>
#include <aws/core/client/CoreErrors.h>
#include <aws/core/client/RetryStrategy.h>
#include <aws/core/http/HttpClient.h>
#include <aws/core/http/HttpResponse.h>
#include <aws/core/http/HttpClientFactory.h>
#include <aws/core/auth/AWSCredentialsProviderChain.h>
#include <aws/core/utils/json/JsonSerializer.h>
#include <aws/core/utils/memory/stl/AWSStringStream.h>
#include <aws/core/utils/threading/Executor.h>
#include <aws/core/utils/DNS.h>
#include <aws/core/utils/logging/LogMacros.h>

#include <aws/eventbridge/EventBridgeClient.h>
#include <aws/eventbridge/EventBridgeEndpoint.h>
#include <aws/eventbridge/EventBridgeErrorMarshaller.h>
#include <aws/eventbridge/model/ActivateEventSourceRequest.h>
#include <aws/eventbridge/model/CreateEventBusRequest.h>
#include <aws/eventbridge/model/CreatePartnerEventSourceRequest.h>
#include <aws/eventbridge/model/DeactivateEventSourceRequest.h>
#include <aws/eventbridge/model/DeleteEventBusRequest.h>
#include <aws/eventbridge/model/DeletePartnerEventSourceRequest.h>
#include <aws/eventbridge/model/DeleteRuleRequest.h>
#include <aws/eventbridge/model/DescribeEventBusRequest.h>
#include <aws/eventbridge/model/DescribeEventSourceRequest.h>
#include <aws/eventbridge/model/DescribePartnerEventSourceRequest.h>
#include <aws/eventbridge/model/DescribeRuleRequest.h>
#include <aws/eventbridge/model/DisableRuleRequest.h>
#include <aws/eventbridge/model/EnableRuleRequest.h>
#include <aws/eventbridge/model/ListEventBusesRequest.h>
#include <aws/eventbridge/model/ListEventSourcesRequest.h>
#include <aws/eventbridge/model/ListPartnerEventSourceAccountsRequest.h>
#include <aws/eventbridge/model/ListPartnerEventSourcesRequest.h>
#include <aws/eventbridge/model/ListRuleNamesByTargetRequest.h>
#include <aws/eventbridge/model/ListRulesRequest.h>
#include <aws/eventbridge/model/ListTagsForResourceRequest.h>
#include <aws/eventbridge/model/ListTargetsByRuleRequest.h>
#include <aws/eventbridge/model/PutEventsRequest.h>
#include <aws/eventbridge/model/PutPartnerEventsRequest.h>
#include <aws/eventbridge/model/PutPermissionRequest.h>
#include <aws/eventbridge/model/PutRuleRequest.h>
#include <aws/eventbridge/model/PutTargetsRequest.h>
#include <aws/eventbridge/model/RemovePermissionRequest.h>
#include <aws/eventbridge/model/RemoveTargetsRequest.h>
#include <aws/eventbridge/model/TagResourceRequest.h>
#include <aws/eventbridge/model/TestEventPatternRequest.h>
#include <aws/eventbridge/model/UntagResourceRequest.h>

using namespace Aws;
using namespace Aws::Auth;
using namespace Aws::Client;
using namespace Aws::EventBridge;
using namespace Aws::EventBridge::Model;
using namespace Aws::Http;
using namespace Aws::Utils::Json;

static const char* SERVICE_NAME = "events";
static const char* ALLOCATION_TAG = "EventBridgeClient";


EventBridgeClient::EventBridgeClient(const Client::ClientConfiguration& clientConfiguration) :
  BASECLASS(clientConfiguration,
    Aws::MakeShared<AWSAuthV4Signer>(ALLOCATION_TAG, Aws::MakeShared<DefaultAWSCredentialsProviderChain>(ALLOCATION_TAG),
        SERVICE_NAME, Aws::Region::ComputeSignerRegion(clientConfiguration.region)),
    Aws::MakeShared<EventBridgeErrorMarshaller>(ALLOCATION_TAG)),
    m_executor(clientConfiguration.executor)
{
  init(clientConfiguration);
}

EventBridgeClient::EventBridgeClient(const AWSCredentials& credentials, const Client::ClientConfiguration& clientConfiguration) :
  BASECLASS(clientConfiguration,
    Aws::MakeShared<AWSAuthV4Signer>(ALLOCATION_TAG, Aws::MakeShared<SimpleAWSCredentialsProvider>(ALLOCATION_TAG, credentials),
         SERVICE_NAME, Aws::Region::ComputeSignerRegion(clientConfiguration.region)),
    Aws::MakeShared<EventBridgeErrorMarshaller>(ALLOCATION_TAG)),
    m_executor(clientConfiguration.executor)
{
  init(clientConfiguration);
}

EventBridgeClient::EventBridgeClient(const std::shared_ptr<AWSCredentialsProvider>& credentialsProvider,
  const Client::ClientConfiguration& clientConfiguration) :
  BASECLASS(clientConfiguration,
    Aws::MakeShared<AWSAuthV4Signer>(ALLOCATION_TAG, credentialsProvider,
         SERVICE_NAME, Aws::Region::ComputeSignerRegion(clientConfiguration.region)),
    Aws::MakeShared<EventBridgeErrorMarshaller>(ALLOCATION_TAG)),
    m_executor(clientConfiguration.executor)
{
  init(clientConfiguration);
}

EventBridgeClient::~EventBridgeClient()
{
}

void EventBridgeClient::init(const ClientConfiguration& config)
{
  SetServiceClientName("EventBridge");
  m_configScheme = SchemeMapper::ToString(config.scheme);
  if (config.endpointOverride.empty())
  {
      m_uri = m_configScheme + "://" + EventBridgeEndpoint::ForRegion(config.region, config.useDualStack);
  }
  else
  {
      OverrideEndpoint(config.endpointOverride);
  }
}

void EventBridgeClient::OverrideEndpoint(const Aws::String& endpoint)
{
  if (endpoint.compare(0, 7, "http://") == 0 || endpoint.compare(0, 8, "https://") == 0)
  {
      m_uri = endpoint;
  }
  else
  {
      m_uri = m_configScheme + "://" + endpoint;
  }
}

ActivateEventSourceOutcome EventBridgeClient::ActivateEventSource(const ActivateEventSourceRequest& request) const
{
  Aws::Http::URI uri = m_uri;
  Aws::StringStream ss;
  ss << "/";
  uri.SetPath(uri.GetPath() + ss.str());
  return ActivateEventSourceOutcome(MakeRequest(uri, request, Aws::Http::HttpMethod::HTTP_POST, Aws::Auth::SIGV4_SIGNER));
}

ActivateEventSourceOutcomeCallable EventBridgeClient::ActivateEventSourceCallable(const ActivateEventSourceRequest& request) const
{
  auto task = Aws::MakeShared< std::packaged_task< ActivateEventSourceOutcome() > >(ALLOCATION_TAG, [this, request](){ return this->ActivateEventSource(request); } );
  auto packagedFunction = [task]() { (*task)(); };
  m_executor->Submit(packagedFunction);
  return task->get_future();
}

void EventBridgeClient::ActivateEventSourceAsync(const ActivateEventSourceRequest& request, const ActivateEventSourceResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const
{
  m_executor->Submit( [this, request, handler, context](){ this->ActivateEventSourceAsyncHelper( request, handler, context ); } );
}

void EventBridgeClient::ActivateEventSourceAsyncHelper(const ActivateEventSourceRequest& request, const ActivateEventSourceResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const
{
  handler(this, request, ActivateEventSource(request), context);
}

CreateEventBusOutcome EventBridgeClient::CreateEventBus(const CreateEventBusRequest& request) const
{
  Aws::Http::URI uri = m_uri;
  Aws::StringStream ss;
  ss << "/";
  uri.SetPath(uri.GetPath() + ss.str());
  return CreateEventBusOutcome(MakeRequest(uri, request, Aws::Http::HttpMethod::HTTP_POST, Aws::Auth::SIGV4_SIGNER));
}

CreateEventBusOutcomeCallable EventBridgeClient::CreateEventBusCallable(const CreateEventBusRequest& request) const
{
  auto task = Aws::MakeShared< std::packaged_task< CreateEventBusOutcome() > >(ALLOCATION_TAG, [this, request](){ return this->CreateEventBus(request); } );
  auto packagedFunction = [task]() { (*task)(); };
  m_executor->Submit(packagedFunction);
  return task->get_future();
}

void EventBridgeClient::CreateEventBusAsync(const CreateEventBusRequest& request, const CreateEventBusResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const
{
  m_executor->Submit( [this, request, handler, context](){ this->CreateEventBusAsyncHelper( request, handler, context ); } );
}

void EventBridgeClient::CreateEventBusAsyncHelper(const CreateEventBusRequest& request, const CreateEventBusResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const
{
  handler(this, request, CreateEventBus(request), context);
}

CreatePartnerEventSourceOutcome EventBridgeClient::CreatePartnerEventSource(const CreatePartnerEventSourceRequest& request) const
{
  Aws::Http::URI uri = m_uri;
  Aws::StringStream ss;
  ss << "/";
  uri.SetPath(uri.GetPath() + ss.str());
  return CreatePartnerEventSourceOutcome(MakeRequest(uri, request, Aws::Http::HttpMethod::HTTP_POST, Aws::Auth::SIGV4_SIGNER));
}

CreatePartnerEventSourceOutcomeCallable EventBridgeClient::CreatePartnerEventSourceCallable(const CreatePartnerEventSourceRequest& request) const
{
  auto task = Aws::MakeShared< std::packaged_task< CreatePartnerEventSourceOutcome() > >(ALLOCATION_TAG, [this, request](){ return this->CreatePartnerEventSource(request); } );
  auto packagedFunction = [task]() { (*task)(); };
  m_executor->Submit(packagedFunction);
  return task->get_future();
}

void EventBridgeClient::CreatePartnerEventSourceAsync(const CreatePartnerEventSourceRequest& request, const CreatePartnerEventSourceResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const
{
  m_executor->Submit( [this, request, handler, context](){ this->CreatePartnerEventSourceAsyncHelper( request, handler, context ); } );
}

void EventBridgeClient::CreatePartnerEventSourceAsyncHelper(const CreatePartnerEventSourceRequest& request, const CreatePartnerEventSourceResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const
{
  handler(this, request, CreatePartnerEventSource(request), context);
}

DeactivateEventSourceOutcome EventBridgeClient::DeactivateEventSource(const DeactivateEventSourceRequest& request) const
{
  Aws::Http::URI uri = m_uri;
  Aws::StringStream ss;
  ss << "/";
  uri.SetPath(uri.GetPath() + ss.str());
  return DeactivateEventSourceOutcome(MakeRequest(uri, request, Aws::Http::HttpMethod::HTTP_POST, Aws::Auth::SIGV4_SIGNER));
}

DeactivateEventSourceOutcomeCallable EventBridgeClient::DeactivateEventSourceCallable(const DeactivateEventSourceRequest& request) const
{
  auto task = Aws::MakeShared< std::packaged_task< DeactivateEventSourceOutcome() > >(ALLOCATION_TAG, [this, request](){ return this->DeactivateEventSource(request); } );
  auto packagedFunction = [task]() { (*task)(); };
  m_executor->Submit(packagedFunction);
  return task->get_future();
}

void EventBridgeClient::DeactivateEventSourceAsync(const DeactivateEventSourceRequest& request, const DeactivateEventSourceResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const
{
  m_executor->Submit( [this, request, handler, context](){ this->DeactivateEventSourceAsyncHelper( request, handler, context ); } );
}

void EventBridgeClient::DeactivateEventSourceAsyncHelper(const DeactivateEventSourceRequest& request, const DeactivateEventSourceResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const
{
  handler(this, request, DeactivateEventSource(request), context);
}

DeleteEventBusOutcome EventBridgeClient::DeleteEventBus(const DeleteEventBusRequest& request) const
{
  Aws::Http::URI uri = m_uri;
  Aws::StringStream ss;
  ss << "/";
  uri.SetPath(uri.GetPath() + ss.str());
  return DeleteEventBusOutcome(MakeRequest(uri, request, Aws::Http::HttpMethod::HTTP_POST, Aws::Auth::SIGV4_SIGNER));
}

DeleteEventBusOutcomeCallable EventBridgeClient::DeleteEventBusCallable(const DeleteEventBusRequest& request) const
{
  auto task = Aws::MakeShared< std::packaged_task< DeleteEventBusOutcome() > >(ALLOCATION_TAG, [this, request](){ return this->DeleteEventBus(request); } );
  auto packagedFunction = [task]() { (*task)(); };
  m_executor->Submit(packagedFunction);
  return task->get_future();
}

void EventBridgeClient::DeleteEventBusAsync(const DeleteEventBusRequest& request, const DeleteEventBusResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const
{
  m_executor->Submit( [this, request, handler, context](){ this->DeleteEventBusAsyncHelper( request, handler, context ); } );
}

void EventBridgeClient::DeleteEventBusAsyncHelper(const DeleteEventBusRequest& request, const DeleteEventBusResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const
{
  handler(this, request, DeleteEventBus(request), context);
}

DeletePartnerEventSourceOutcome EventBridgeClient::DeletePartnerEventSource(const DeletePartnerEventSourceRequest& request) const
{
  Aws::Http::URI uri = m_uri;
  Aws::StringStream ss;
  ss << "/";
  uri.SetPath(uri.GetPath() + ss.str());
  return DeletePartnerEventSourceOutcome(MakeRequest(uri, request, Aws::Http::HttpMethod::HTTP_POST, Aws::Auth::SIGV4_SIGNER));
}

DeletePartnerEventSourceOutcomeCallable EventBridgeClient::DeletePartnerEventSourceCallable(const DeletePartnerEventSourceRequest& request) const
{
  auto task = Aws::MakeShared< std::packaged_task< DeletePartnerEventSourceOutcome() > >(ALLOCATION_TAG, [this, request](){ return this->DeletePartnerEventSource(request); } );
  auto packagedFunction = [task]() { (*task)(); };
  m_executor->Submit(packagedFunction);
  return task->get_future();
}

void EventBridgeClient::DeletePartnerEventSourceAsync(const DeletePartnerEventSourceRequest& request, const DeletePartnerEventSourceResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const
{
  m_executor->Submit( [this, request, handler, context](){ this->DeletePartnerEventSourceAsyncHelper( request, handler, context ); } );
}

void EventBridgeClient::DeletePartnerEventSourceAsyncHelper(const DeletePartnerEventSourceRequest& request, const DeletePartnerEventSourceResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const
{
  handler(this, request, DeletePartnerEventSource(request), context);
}

DeleteRuleOutcome EventBridgeClient::DeleteRule(const DeleteRuleRequest& request) const
{
  Aws::Http::URI uri = m_uri;
  Aws::StringStream ss;
  ss << "/";
  uri.SetPath(uri.GetPath() + ss.str());
  return DeleteRuleOutcome(MakeRequest(uri, request, Aws::Http::HttpMethod::HTTP_POST, Aws::Auth::SIGV4_SIGNER));
}

DeleteRuleOutcomeCallable EventBridgeClient::DeleteRuleCallable(const DeleteRuleRequest& request) const
{
  auto task = Aws::MakeShared< std::packaged_task< DeleteRuleOutcome() > >(ALLOCATION_TAG, [this, request](){ return this->DeleteRule(request); } );
  auto packagedFunction = [task]() { (*task)(); };
  m_executor->Submit(packagedFunction);
  return task->get_future();
}

void EventBridgeClient::DeleteRuleAsync(const DeleteRuleRequest& request, const DeleteRuleResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const
{
  m_executor->Submit( [this, request, handler, context](){ this->DeleteRuleAsyncHelper( request, handler, context ); } );
}

void EventBridgeClient::DeleteRuleAsyncHelper(const DeleteRuleRequest& request, const DeleteRuleResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const
{
  handler(this, request, DeleteRule(request), context);
}

DescribeEventBusOutcome EventBridgeClient::DescribeEventBus(const DescribeEventBusRequest& request) const
{
  Aws::Http::URI uri = m_uri;
  Aws::StringStream ss;
  ss << "/";
  uri.SetPath(uri.GetPath() + ss.str());
  return DescribeEventBusOutcome(MakeRequest(uri, request, Aws::Http::HttpMethod::HTTP_POST, Aws::Auth::SIGV4_SIGNER));
}

DescribeEventBusOutcomeCallable EventBridgeClient::DescribeEventBusCallable(const DescribeEventBusRequest& request) const
{
  auto task = Aws::MakeShared< std::packaged_task< DescribeEventBusOutcome() > >(ALLOCATION_TAG, [this, request](){ return this->DescribeEventBus(request); } );
  auto packagedFunction = [task]() { (*task)(); };
  m_executor->Submit(packagedFunction);
  return task->get_future();
}

void EventBridgeClient::DescribeEventBusAsync(const DescribeEventBusRequest& request, const DescribeEventBusResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const
{
  m_executor->Submit( [this, request, handler, context](){ this->DescribeEventBusAsyncHelper( request, handler, context ); } );
}

void EventBridgeClient::DescribeEventBusAsyncHelper(const DescribeEventBusRequest& request, const DescribeEventBusResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const
{
  handler(this, request, DescribeEventBus(request), context);
}

DescribeEventSourceOutcome EventBridgeClient::DescribeEventSource(const DescribeEventSourceRequest& request) const
{
  Aws::Http::URI uri = m_uri;
  Aws::StringStream ss;
  ss << "/";
  uri.SetPath(uri.GetPath() + ss.str());
  return DescribeEventSourceOutcome(MakeRequest(uri, request, Aws::Http::HttpMethod::HTTP_POST, Aws::Auth::SIGV4_SIGNER));
}

DescribeEventSourceOutcomeCallable EventBridgeClient::DescribeEventSourceCallable(const DescribeEventSourceRequest& request) const
{
  auto task = Aws::MakeShared< std::packaged_task< DescribeEventSourceOutcome() > >(ALLOCATION_TAG, [this, request](){ return this->DescribeEventSource(request); } );
  auto packagedFunction = [task]() { (*task)(); };
  m_executor->Submit(packagedFunction);
  return task->get_future();
}

void EventBridgeClient::DescribeEventSourceAsync(const DescribeEventSourceRequest& request, const DescribeEventSourceResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const
{
  m_executor->Submit( [this, request, handler, context](){ this->DescribeEventSourceAsyncHelper( request, handler, context ); } );
}

void EventBridgeClient::DescribeEventSourceAsyncHelper(const DescribeEventSourceRequest& request, const DescribeEventSourceResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const
{
  handler(this, request, DescribeEventSource(request), context);
}

DescribePartnerEventSourceOutcome EventBridgeClient::DescribePartnerEventSource(const DescribePartnerEventSourceRequest& request) const
{
  Aws::Http::URI uri = m_uri;
  Aws::StringStream ss;
  ss << "/";
  uri.SetPath(uri.GetPath() + ss.str());
  return DescribePartnerEventSourceOutcome(MakeRequest(uri, request, Aws::Http::HttpMethod::HTTP_POST, Aws::Auth::SIGV4_SIGNER));
}

DescribePartnerEventSourceOutcomeCallable EventBridgeClient::DescribePartnerEventSourceCallable(const DescribePartnerEventSourceRequest& request) const
{
  auto task = Aws::MakeShared< std::packaged_task< DescribePartnerEventSourceOutcome() > >(ALLOCATION_TAG, [this, request](){ return this->DescribePartnerEventSource(request); } );
  auto packagedFunction = [task]() { (*task)(); };
  m_executor->Submit(packagedFunction);
  return task->get_future();
}

void EventBridgeClient::DescribePartnerEventSourceAsync(const DescribePartnerEventSourceRequest& request, const DescribePartnerEventSourceResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const
{
  m_executor->Submit( [this, request, handler, context](){ this->DescribePartnerEventSourceAsyncHelper( request, handler, context ); } );
}

void EventBridgeClient::DescribePartnerEventSourceAsyncHelper(const DescribePartnerEventSourceRequest& request, const DescribePartnerEventSourceResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const
{
  handler(this, request, DescribePartnerEventSource(request), context);
}

DescribeRuleOutcome EventBridgeClient::DescribeRule(const DescribeRuleRequest& request) const
{
  Aws::Http::URI uri = m_uri;
  Aws::StringStream ss;
  ss << "/";
  uri.SetPath(uri.GetPath() + ss.str());
  return DescribeRuleOutcome(MakeRequest(uri, request, Aws::Http::HttpMethod::HTTP_POST, Aws::Auth::SIGV4_SIGNER));
}

DescribeRuleOutcomeCallable EventBridgeClient::DescribeRuleCallable(const DescribeRuleRequest& request) const
{
  auto task = Aws::MakeShared< std::packaged_task< DescribeRuleOutcome() > >(ALLOCATION_TAG, [this, request](){ return this->DescribeRule(request); } );
  auto packagedFunction = [task]() { (*task)(); };
  m_executor->Submit(packagedFunction);
  return task->get_future();
}

void EventBridgeClient::DescribeRuleAsync(const DescribeRuleRequest& request, const DescribeRuleResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const
{
  m_executor->Submit( [this, request, handler, context](){ this->DescribeRuleAsyncHelper( request, handler, context ); } );
}

void EventBridgeClient::DescribeRuleAsyncHelper(const DescribeRuleRequest& request, const DescribeRuleResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const
{
  handler(this, request, DescribeRule(request), context);
}

DisableRuleOutcome EventBridgeClient::DisableRule(const DisableRuleRequest& request) const
{
  Aws::Http::URI uri = m_uri;
  Aws::StringStream ss;
  ss << "/";
  uri.SetPath(uri.GetPath() + ss.str());
  return DisableRuleOutcome(MakeRequest(uri, request, Aws::Http::HttpMethod::HTTP_POST, Aws::Auth::SIGV4_SIGNER));
}

DisableRuleOutcomeCallable EventBridgeClient::DisableRuleCallable(const DisableRuleRequest& request) const
{
  auto task = Aws::MakeShared< std::packaged_task< DisableRuleOutcome() > >(ALLOCATION_TAG, [this, request](){ return this->DisableRule(request); } );
  auto packagedFunction = [task]() { (*task)(); };
  m_executor->Submit(packagedFunction);
  return task->get_future();
}

void EventBridgeClient::DisableRuleAsync(const DisableRuleRequest& request, const DisableRuleResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const
{
  m_executor->Submit( [this, request, handler, context](){ this->DisableRuleAsyncHelper( request, handler, context ); } );
}

void EventBridgeClient::DisableRuleAsyncHelper(const DisableRuleRequest& request, const DisableRuleResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const
{
  handler(this, request, DisableRule(request), context);
}

EnableRuleOutcome EventBridgeClient::EnableRule(const EnableRuleRequest& request) const
{
  Aws::Http::URI uri = m_uri;
  Aws::StringStream ss;
  ss << "/";
  uri.SetPath(uri.GetPath() + ss.str());
  return EnableRuleOutcome(MakeRequest(uri, request, Aws::Http::HttpMethod::HTTP_POST, Aws::Auth::SIGV4_SIGNER));
}

EnableRuleOutcomeCallable EventBridgeClient::EnableRuleCallable(const EnableRuleRequest& request) const
{
  auto task = Aws::MakeShared< std::packaged_task< EnableRuleOutcome() > >(ALLOCATION_TAG, [this, request](){ return this->EnableRule(request); } );
  auto packagedFunction = [task]() { (*task)(); };
  m_executor->Submit(packagedFunction);
  return task->get_future();
}

void EventBridgeClient::EnableRuleAsync(const EnableRuleRequest& request, const EnableRuleResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const
{
  m_executor->Submit( [this, request, handler, context](){ this->EnableRuleAsyncHelper( request, handler, context ); } );
}

void EventBridgeClient::EnableRuleAsyncHelper(const EnableRuleRequest& request, const EnableRuleResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const
{
  handler(this, request, EnableRule(request), context);
}

ListEventBusesOutcome EventBridgeClient::ListEventBuses(const ListEventBusesRequest& request) const
{
  Aws::Http::URI uri = m_uri;
  Aws::StringStream ss;
  ss << "/";
  uri.SetPath(uri.GetPath() + ss.str());
  return ListEventBusesOutcome(MakeRequest(uri, request, Aws::Http::HttpMethod::HTTP_POST, Aws::Auth::SIGV4_SIGNER));
}

ListEventBusesOutcomeCallable EventBridgeClient::ListEventBusesCallable(const ListEventBusesRequest& request) const
{
  auto task = Aws::MakeShared< std::packaged_task< ListEventBusesOutcome() > >(ALLOCATION_TAG, [this, request](){ return this->ListEventBuses(request); } );
  auto packagedFunction = [task]() { (*task)(); };
  m_executor->Submit(packagedFunction);
  return task->get_future();
}

void EventBridgeClient::ListEventBusesAsync(const ListEventBusesRequest& request, const ListEventBusesResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const
{
  m_executor->Submit( [this, request, handler, context](){ this->ListEventBusesAsyncHelper( request, handler, context ); } );
}

void EventBridgeClient::ListEventBusesAsyncHelper(const ListEventBusesRequest& request, const ListEventBusesResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const
{
  handler(this, request, ListEventBuses(request), context);
}

ListEventSourcesOutcome EventBridgeClient::ListEventSources(const ListEventSourcesRequest& request) const
{
  Aws::Http::URI uri = m_uri;
  Aws::StringStream ss;
  ss << "/";
  uri.SetPath(uri.GetPath() + ss.str());
  return ListEventSourcesOutcome(MakeRequest(uri, request, Aws::Http::HttpMethod::HTTP_POST, Aws::Auth::SIGV4_SIGNER));
}

ListEventSourcesOutcomeCallable EventBridgeClient::ListEventSourcesCallable(const ListEventSourcesRequest& request) const
{
  auto task = Aws::MakeShared< std::packaged_task< ListEventSourcesOutcome() > >(ALLOCATION_TAG, [this, request](){ return this->ListEventSources(request); } );
  auto packagedFunction = [task]() { (*task)(); };
  m_executor->Submit(packagedFunction);
  return task->get_future();
}

void EventBridgeClient::ListEventSourcesAsync(const ListEventSourcesRequest& request, const ListEventSourcesResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const
{
  m_executor->Submit( [this, request, handler, context](){ this->ListEventSourcesAsyncHelper( request, handler, context ); } );
}

void EventBridgeClient::ListEventSourcesAsyncHelper(const ListEventSourcesRequest& request, const ListEventSourcesResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const
{
  handler(this, request, ListEventSources(request), context);
}

ListPartnerEventSourceAccountsOutcome EventBridgeClient::ListPartnerEventSourceAccounts(const ListPartnerEventSourceAccountsRequest& request) const
{
  Aws::Http::URI uri = m_uri;
  Aws::StringStream ss;
  ss << "/";
  uri.SetPath(uri.GetPath() + ss.str());
  return ListPartnerEventSourceAccountsOutcome(MakeRequest(uri, request, Aws::Http::HttpMethod::HTTP_POST, Aws::Auth::SIGV4_SIGNER));
}

ListPartnerEventSourceAccountsOutcomeCallable EventBridgeClient::ListPartnerEventSourceAccountsCallable(const ListPartnerEventSourceAccountsRequest& request) const
{
  auto task = Aws::MakeShared< std::packaged_task< ListPartnerEventSourceAccountsOutcome() > >(ALLOCATION_TAG, [this, request](){ return this->ListPartnerEventSourceAccounts(request); } );
  auto packagedFunction = [task]() { (*task)(); };
  m_executor->Submit(packagedFunction);
  return task->get_future();
}

void EventBridgeClient::ListPartnerEventSourceAccountsAsync(const ListPartnerEventSourceAccountsRequest& request, const ListPartnerEventSourceAccountsResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const
{
  m_executor->Submit( [this, request, handler, context](){ this->ListPartnerEventSourceAccountsAsyncHelper( request, handler, context ); } );
}

void EventBridgeClient::ListPartnerEventSourceAccountsAsyncHelper(const ListPartnerEventSourceAccountsRequest& request, const ListPartnerEventSourceAccountsResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const
{
  handler(this, request, ListPartnerEventSourceAccounts(request), context);
}

ListPartnerEventSourcesOutcome EventBridgeClient::ListPartnerEventSources(const ListPartnerEventSourcesRequest& request) const
{
  Aws::Http::URI uri = m_uri;
  Aws::StringStream ss;
  ss << "/";
  uri.SetPath(uri.GetPath() + ss.str());
  return ListPartnerEventSourcesOutcome(MakeRequest(uri, request, Aws::Http::HttpMethod::HTTP_POST, Aws::Auth::SIGV4_SIGNER));
}

ListPartnerEventSourcesOutcomeCallable EventBridgeClient::ListPartnerEventSourcesCallable(const ListPartnerEventSourcesRequest& request) const
{
  auto task = Aws::MakeShared< std::packaged_task< ListPartnerEventSourcesOutcome() > >(ALLOCATION_TAG, [this, request](){ return this->ListPartnerEventSources(request); } );
  auto packagedFunction = [task]() { (*task)(); };
  m_executor->Submit(packagedFunction);
  return task->get_future();
}

void EventBridgeClient::ListPartnerEventSourcesAsync(const ListPartnerEventSourcesRequest& request, const ListPartnerEventSourcesResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const
{
  m_executor->Submit( [this, request, handler, context](){ this->ListPartnerEventSourcesAsyncHelper( request, handler, context ); } );
}

void EventBridgeClient::ListPartnerEventSourcesAsyncHelper(const ListPartnerEventSourcesRequest& request, const ListPartnerEventSourcesResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const
{
  handler(this, request, ListPartnerEventSources(request), context);
}

ListRuleNamesByTargetOutcome EventBridgeClient::ListRuleNamesByTarget(const ListRuleNamesByTargetRequest& request) const
{
  Aws::Http::URI uri = m_uri;
  Aws::StringStream ss;
  ss << "/";
  uri.SetPath(uri.GetPath() + ss.str());
  return ListRuleNamesByTargetOutcome(MakeRequest(uri, request, Aws::Http::HttpMethod::HTTP_POST, Aws::Auth::SIGV4_SIGNER));
}

ListRuleNamesByTargetOutcomeCallable EventBridgeClient::ListRuleNamesByTargetCallable(const ListRuleNamesByTargetRequest& request) const
{
  auto task = Aws::MakeShared< std::packaged_task< ListRuleNamesByTargetOutcome() > >(ALLOCATION_TAG, [this, request](){ return this->ListRuleNamesByTarget(request); } );
  auto packagedFunction = [task]() { (*task)(); };
  m_executor->Submit(packagedFunction);
  return task->get_future();
}

void EventBridgeClient::ListRuleNamesByTargetAsync(const ListRuleNamesByTargetRequest& request, const ListRuleNamesByTargetResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const
{
  m_executor->Submit( [this, request, handler, context](){ this->ListRuleNamesByTargetAsyncHelper( request, handler, context ); } );
}

void EventBridgeClient::ListRuleNamesByTargetAsyncHelper(const ListRuleNamesByTargetRequest& request, const ListRuleNamesByTargetResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const
{
  handler(this, request, ListRuleNamesByTarget(request), context);
}

ListRulesOutcome EventBridgeClient::ListRules(const ListRulesRequest& request) const
{
  Aws::Http::URI uri = m_uri;
  Aws::StringStream ss;
  ss << "/";
  uri.SetPath(uri.GetPath() + ss.str());
  return ListRulesOutcome(MakeRequest(uri, request, Aws::Http::HttpMethod::HTTP_POST, Aws::Auth::SIGV4_SIGNER));
}

ListRulesOutcomeCallable EventBridgeClient::ListRulesCallable(const ListRulesRequest& request) const
{
  auto task = Aws::MakeShared< std::packaged_task< ListRulesOutcome() > >(ALLOCATION_TAG, [this, request](){ return this->ListRules(request); } );
  auto packagedFunction = [task]() { (*task)(); };
  m_executor->Submit(packagedFunction);
  return task->get_future();
}

void EventBridgeClient::ListRulesAsync(const ListRulesRequest& request, const ListRulesResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const
{
  m_executor->Submit( [this, request, handler, context](){ this->ListRulesAsyncHelper( request, handler, context ); } );
}

void EventBridgeClient::ListRulesAsyncHelper(const ListRulesRequest& request, const ListRulesResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const
{
  handler(this, request, ListRules(request), context);
}

ListTagsForResourceOutcome EventBridgeClient::ListTagsForResource(const ListTagsForResourceRequest& request) const
{
  Aws::Http::URI uri = m_uri;
  Aws::StringStream ss;
  ss << "/";
  uri.SetPath(uri.GetPath() + ss.str());
  return ListTagsForResourceOutcome(MakeRequest(uri, request, Aws::Http::HttpMethod::HTTP_POST, Aws::Auth::SIGV4_SIGNER));
}

ListTagsForResourceOutcomeCallable EventBridgeClient::ListTagsForResourceCallable(const ListTagsForResourceRequest& request) const
{
  auto task = Aws::MakeShared< std::packaged_task< ListTagsForResourceOutcome() > >(ALLOCATION_TAG, [this, request](){ return this->ListTagsForResource(request); } );
  auto packagedFunction = [task]() { (*task)(); };
  m_executor->Submit(packagedFunction);
  return task->get_future();
}

void EventBridgeClient::ListTagsForResourceAsync(const ListTagsForResourceRequest& request, const ListTagsForResourceResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const
{
  m_executor->Submit( [this, request, handler, context](){ this->ListTagsForResourceAsyncHelper( request, handler, context ); } );
}

void EventBridgeClient::ListTagsForResourceAsyncHelper(const ListTagsForResourceRequest& request, const ListTagsForResourceResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const
{
  handler(this, request, ListTagsForResource(request), context);
}

ListTargetsByRuleOutcome EventBridgeClient::ListTargetsByRule(const ListTargetsByRuleRequest& request) const
{
  Aws::Http::URI uri = m_uri;
  Aws::StringStream ss;
  ss << "/";
  uri.SetPath(uri.GetPath() + ss.str());
  return ListTargetsByRuleOutcome(MakeRequest(uri, request, Aws::Http::HttpMethod::HTTP_POST, Aws::Auth::SIGV4_SIGNER));
}

ListTargetsByRuleOutcomeCallable EventBridgeClient::ListTargetsByRuleCallable(const ListTargetsByRuleRequest& request) const
{
  auto task = Aws::MakeShared< std::packaged_task< ListTargetsByRuleOutcome() > >(ALLOCATION_TAG, [this, request](){ return this->ListTargetsByRule(request); } );
  auto packagedFunction = [task]() { (*task)(); };
  m_executor->Submit(packagedFunction);
  return task->get_future();
}

void EventBridgeClient::ListTargetsByRuleAsync(const ListTargetsByRuleRequest& request, const ListTargetsByRuleResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const
{
  m_executor->Submit( [this, request, handler, context](){ this->ListTargetsByRuleAsyncHelper( request, handler, context ); } );
}

void EventBridgeClient::ListTargetsByRuleAsyncHelper(const ListTargetsByRuleRequest& request, const ListTargetsByRuleResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const
{
  handler(this, request, ListTargetsByRule(request), context);
}

PutEventsOutcome EventBridgeClient::PutEvents(const PutEventsRequest& request) const
{
  Aws::Http::URI uri = m_uri;
  Aws::StringStream ss;
  ss << "/";
  uri.SetPath(uri.GetPath() + ss.str());
  return PutEventsOutcome(MakeRequest(uri, request, Aws::Http::HttpMethod::HTTP_POST, Aws::Auth::SIGV4_SIGNER));
}

PutEventsOutcomeCallable EventBridgeClient::PutEventsCallable(const PutEventsRequest& request) const
{
  auto task = Aws::MakeShared< std::packaged_task< PutEventsOutcome() > >(ALLOCATION_TAG, [this, request](){ return this->PutEvents(request); } );
  auto packagedFunction = [task]() { (*task)(); };
  m_executor->Submit(packagedFunction);
  return task->get_future();
}

void EventBridgeClient::PutEventsAsync(const PutEventsRequest& request, const PutEventsResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const
{
  m_executor->Submit( [this, request, handler, context](){ this->PutEventsAsyncHelper( request, handler, context ); } );
}

void EventBridgeClient::PutEventsAsyncHelper(const PutEventsRequest& request, const PutEventsResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const
{
  handler(this, request, PutEvents(request), context);
}

PutPartnerEventsOutcome EventBridgeClient::PutPartnerEvents(const PutPartnerEventsRequest& request) const
{
  Aws::Http::URI uri = m_uri;
  Aws::StringStream ss;
  ss << "/";
  uri.SetPath(uri.GetPath() + ss.str());
  return PutPartnerEventsOutcome(MakeRequest(uri, request, Aws::Http::HttpMethod::HTTP_POST, Aws::Auth::SIGV4_SIGNER));
}

PutPartnerEventsOutcomeCallable EventBridgeClient::PutPartnerEventsCallable(const PutPartnerEventsRequest& request) const
{
  auto task = Aws::MakeShared< std::packaged_task< PutPartnerEventsOutcome() > >(ALLOCATION_TAG, [this, request](){ return this->PutPartnerEvents(request); } );
  auto packagedFunction = [task]() { (*task)(); };
  m_executor->Submit(packagedFunction);
  return task->get_future();
}

void EventBridgeClient::PutPartnerEventsAsync(const PutPartnerEventsRequest& request, const PutPartnerEventsResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const
{
  m_executor->Submit( [this, request, handler, context](){ this->PutPartnerEventsAsyncHelper( request, handler, context ); } );
}

void EventBridgeClient::PutPartnerEventsAsyncHelper(const PutPartnerEventsRequest& request, const PutPartnerEventsResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const
{
  handler(this, request, PutPartnerEvents(request), context);
}

PutPermissionOutcome EventBridgeClient::PutPermission(const PutPermissionRequest& request) const
{
  Aws::Http::URI uri = m_uri;
  Aws::StringStream ss;
  ss << "/";
  uri.SetPath(uri.GetPath() + ss.str());
  return PutPermissionOutcome(MakeRequest(uri, request, Aws::Http::HttpMethod::HTTP_POST, Aws::Auth::SIGV4_SIGNER));
}

PutPermissionOutcomeCallable EventBridgeClient::PutPermissionCallable(const PutPermissionRequest& request) const
{
  auto task = Aws::MakeShared< std::packaged_task< PutPermissionOutcome() > >(ALLOCATION_TAG, [this, request](){ return this->PutPermission(request); } );
  auto packagedFunction = [task]() { (*task)(); };
  m_executor->Submit(packagedFunction);
  return task->get_future();
}

void EventBridgeClient::PutPermissionAsync(const PutPermissionRequest& request, const PutPermissionResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const
{
  m_executor->Submit( [this, request, handler, context](){ this->PutPermissionAsyncHelper( request, handler, context ); } );
}

void EventBridgeClient::PutPermissionAsyncHelper(const PutPermissionRequest& request, const PutPermissionResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const
{
  handler(this, request, PutPermission(request), context);
}

PutRuleOutcome EventBridgeClient::PutRule(const PutRuleRequest& request) const
{
  Aws::Http::URI uri = m_uri;
  Aws::StringStream ss;
  ss << "/";
  uri.SetPath(uri.GetPath() + ss.str());
  return PutRuleOutcome(MakeRequest(uri, request, Aws::Http::HttpMethod::HTTP_POST, Aws::Auth::SIGV4_SIGNER));
}

PutRuleOutcomeCallable EventBridgeClient::PutRuleCallable(const PutRuleRequest& request) const
{
  auto task = Aws::MakeShared< std::packaged_task< PutRuleOutcome() > >(ALLOCATION_TAG, [this, request](){ return this->PutRule(request); } );
  auto packagedFunction = [task]() { (*task)(); };
  m_executor->Submit(packagedFunction);
  return task->get_future();
}

void EventBridgeClient::PutRuleAsync(const PutRuleRequest& request, const PutRuleResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const
{
  m_executor->Submit( [this, request, handler, context](){ this->PutRuleAsyncHelper( request, handler, context ); } );
}

void EventBridgeClient::PutRuleAsyncHelper(const PutRuleRequest& request, const PutRuleResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const
{
  handler(this, request, PutRule(request), context);
}

PutTargetsOutcome EventBridgeClient::PutTargets(const PutTargetsRequest& request) const
{
  Aws::Http::URI uri = m_uri;
  Aws::StringStream ss;
  ss << "/";
  uri.SetPath(uri.GetPath() + ss.str());
  return PutTargetsOutcome(MakeRequest(uri, request, Aws::Http::HttpMethod::HTTP_POST, Aws::Auth::SIGV4_SIGNER));
}

PutTargetsOutcomeCallable EventBridgeClient::PutTargetsCallable(const PutTargetsRequest& request) const
{
  auto task = Aws::MakeShared< std::packaged_task< PutTargetsOutcome() > >(ALLOCATION_TAG, [this, request](){ return this->PutTargets(request); } );
  auto packagedFunction = [task]() { (*task)(); };
  m_executor->Submit(packagedFunction);
  return task->get_future();
}

void EventBridgeClient::PutTargetsAsync(const PutTargetsRequest& request, const PutTargetsResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const
{
  m_executor->Submit( [this, request, handler, context](){ this->PutTargetsAsyncHelper( request, handler, context ); } );
}

void EventBridgeClient::PutTargetsAsyncHelper(const PutTargetsRequest& request, const PutTargetsResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const
{
  handler(this, request, PutTargets(request), context);
}

RemovePermissionOutcome EventBridgeClient::RemovePermission(const RemovePermissionRequest& request) const
{
  Aws::Http::URI uri = m_uri;
  Aws::StringStream ss;
  ss << "/";
  uri.SetPath(uri.GetPath() + ss.str());
  return RemovePermissionOutcome(MakeRequest(uri, request, Aws::Http::HttpMethod::HTTP_POST, Aws::Auth::SIGV4_SIGNER));
}

RemovePermissionOutcomeCallable EventBridgeClient::RemovePermissionCallable(const RemovePermissionRequest& request) const
{
  auto task = Aws::MakeShared< std::packaged_task< RemovePermissionOutcome() > >(ALLOCATION_TAG, [this, request](){ return this->RemovePermission(request); } );
  auto packagedFunction = [task]() { (*task)(); };
  m_executor->Submit(packagedFunction);
  return task->get_future();
}

void EventBridgeClient::RemovePermissionAsync(const RemovePermissionRequest& request, const RemovePermissionResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const
{
  m_executor->Submit( [this, request, handler, context](){ this->RemovePermissionAsyncHelper( request, handler, context ); } );
}

void EventBridgeClient::RemovePermissionAsyncHelper(const RemovePermissionRequest& request, const RemovePermissionResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const
{
  handler(this, request, RemovePermission(request), context);
}

RemoveTargetsOutcome EventBridgeClient::RemoveTargets(const RemoveTargetsRequest& request) const
{
  Aws::Http::URI uri = m_uri;
  Aws::StringStream ss;
  ss << "/";
  uri.SetPath(uri.GetPath() + ss.str());
  return RemoveTargetsOutcome(MakeRequest(uri, request, Aws::Http::HttpMethod::HTTP_POST, Aws::Auth::SIGV4_SIGNER));
}

RemoveTargetsOutcomeCallable EventBridgeClient::RemoveTargetsCallable(const RemoveTargetsRequest& request) const
{
  auto task = Aws::MakeShared< std::packaged_task< RemoveTargetsOutcome() > >(ALLOCATION_TAG, [this, request](){ return this->RemoveTargets(request); } );
  auto packagedFunction = [task]() { (*task)(); };
  m_executor->Submit(packagedFunction);
  return task->get_future();
}

void EventBridgeClient::RemoveTargetsAsync(const RemoveTargetsRequest& request, const RemoveTargetsResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const
{
  m_executor->Submit( [this, request, handler, context](){ this->RemoveTargetsAsyncHelper( request, handler, context ); } );
}

void EventBridgeClient::RemoveTargetsAsyncHelper(const RemoveTargetsRequest& request, const RemoveTargetsResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const
{
  handler(this, request, RemoveTargets(request), context);
}

TagResourceOutcome EventBridgeClient::TagResource(const TagResourceRequest& request) const
{
  Aws::Http::URI uri = m_uri;
  Aws::StringStream ss;
  ss << "/";
  uri.SetPath(uri.GetPath() + ss.str());
  return TagResourceOutcome(MakeRequest(uri, request, Aws::Http::HttpMethod::HTTP_POST, Aws::Auth::SIGV4_SIGNER));
}

TagResourceOutcomeCallable EventBridgeClient::TagResourceCallable(const TagResourceRequest& request) const
{
  auto task = Aws::MakeShared< std::packaged_task< TagResourceOutcome() > >(ALLOCATION_TAG, [this, request](){ return this->TagResource(request); } );
  auto packagedFunction = [task]() { (*task)(); };
  m_executor->Submit(packagedFunction);
  return task->get_future();
}

void EventBridgeClient::TagResourceAsync(const TagResourceRequest& request, const TagResourceResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const
{
  m_executor->Submit( [this, request, handler, context](){ this->TagResourceAsyncHelper( request, handler, context ); } );
}

void EventBridgeClient::TagResourceAsyncHelper(const TagResourceRequest& request, const TagResourceResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const
{
  handler(this, request, TagResource(request), context);
}

TestEventPatternOutcome EventBridgeClient::TestEventPattern(const TestEventPatternRequest& request) const
{
  Aws::Http::URI uri = m_uri;
  Aws::StringStream ss;
  ss << "/";
  uri.SetPath(uri.GetPath() + ss.str());
  return TestEventPatternOutcome(MakeRequest(uri, request, Aws::Http::HttpMethod::HTTP_POST, Aws::Auth::SIGV4_SIGNER));
}

TestEventPatternOutcomeCallable EventBridgeClient::TestEventPatternCallable(const TestEventPatternRequest& request) const
{
  auto task = Aws::MakeShared< std::packaged_task< TestEventPatternOutcome() > >(ALLOCATION_TAG, [this, request](){ return this->TestEventPattern(request); } );
  auto packagedFunction = [task]() { (*task)(); };
  m_executor->Submit(packagedFunction);
  return task->get_future();
}

void EventBridgeClient::TestEventPatternAsync(const TestEventPatternRequest& request, const TestEventPatternResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const
{
  m_executor->Submit( [this, request, handler, context](){ this->TestEventPatternAsyncHelper( request, handler, context ); } );
}

void EventBridgeClient::TestEventPatternAsyncHelper(const TestEventPatternRequest& request, const TestEventPatternResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const
{
  handler(this, request, TestEventPattern(request), context);
}

UntagResourceOutcome EventBridgeClient::UntagResource(const UntagResourceRequest& request) const
{
  Aws::Http::URI uri = m_uri;
  Aws::StringStream ss;
  ss << "/";
  uri.SetPath(uri.GetPath() + ss.str());
  return UntagResourceOutcome(MakeRequest(uri, request, Aws::Http::HttpMethod::HTTP_POST, Aws::Auth::SIGV4_SIGNER));
}

UntagResourceOutcomeCallable EventBridgeClient::UntagResourceCallable(const UntagResourceRequest& request) const
{
  auto task = Aws::MakeShared< std::packaged_task< UntagResourceOutcome() > >(ALLOCATION_TAG, [this, request](){ return this->UntagResource(request); } );
  auto packagedFunction = [task]() { (*task)(); };
  m_executor->Submit(packagedFunction);
  return task->get_future();
}

void EventBridgeClient::UntagResourceAsync(const UntagResourceRequest& request, const UntagResourceResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const
{
  m_executor->Submit( [this, request, handler, context](){ this->UntagResourceAsyncHelper( request, handler, context ); } );
}

void EventBridgeClient::UntagResourceAsyncHelper(const UntagResourceRequest& request, const UntagResourceResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const
{
  handler(this, request, UntagResource(request), context);
}

