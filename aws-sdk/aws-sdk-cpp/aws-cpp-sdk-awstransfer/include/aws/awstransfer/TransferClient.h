﻿/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#pragma once
#include <aws/awstransfer/Transfer_EXPORTS.h>
#include <aws/awstransfer/TransferErrors.h>
#include <aws/core/client/AWSError.h>
#include <aws/core/client/ClientConfiguration.h>
#include <aws/core/client/AWSClient.h>
#include <aws/core/utils/memory/stl/AWSString.h>
#include <aws/core/utils/json/JsonSerializer.h>
#include <aws/awstransfer/model/CreateServerResult.h>
#include <aws/awstransfer/model/CreateUserResult.h>
#include <aws/awstransfer/model/DescribeSecurityPolicyResult.h>
#include <aws/awstransfer/model/DescribeServerResult.h>
#include <aws/awstransfer/model/DescribeUserResult.h>
#include <aws/awstransfer/model/ImportSshPublicKeyResult.h>
#include <aws/awstransfer/model/ListSecurityPoliciesResult.h>
#include <aws/awstransfer/model/ListServersResult.h>
#include <aws/awstransfer/model/ListTagsForResourceResult.h>
#include <aws/awstransfer/model/ListUsersResult.h>
#include <aws/awstransfer/model/TestIdentityProviderResult.h>
#include <aws/awstransfer/model/UpdateServerResult.h>
#include <aws/awstransfer/model/UpdateUserResult.h>
#include <aws/core/NoResult.h>
#include <aws/core/client/AsyncCallerContext.h>
#include <aws/core/http/HttpTypes.h>
#include <future>
#include <functional>

namespace Aws
{

namespace Http
{
  class HttpClient;
  class HttpClientFactory;
} // namespace Http

namespace Utils
{
  template< typename R, typename E> class Outcome;
namespace Threading
{
  class Executor;
} // namespace Threading
} // namespace Utils

namespace Auth
{
  class AWSCredentials;
  class AWSCredentialsProvider;
} // namespace Auth

namespace Client
{
  class RetryStrategy;
} // namespace Client

namespace Transfer
{

namespace Model
{
        class CreateServerRequest;
        class CreateUserRequest;
        class DeleteServerRequest;
        class DeleteSshPublicKeyRequest;
        class DeleteUserRequest;
        class DescribeSecurityPolicyRequest;
        class DescribeServerRequest;
        class DescribeUserRequest;
        class ImportSshPublicKeyRequest;
        class ListSecurityPoliciesRequest;
        class ListServersRequest;
        class ListTagsForResourceRequest;
        class ListUsersRequest;
        class StartServerRequest;
        class StopServerRequest;
        class TagResourceRequest;
        class TestIdentityProviderRequest;
        class UntagResourceRequest;
        class UpdateServerRequest;
        class UpdateUserRequest;

        typedef Aws::Utils::Outcome<CreateServerResult, TransferError> CreateServerOutcome;
        typedef Aws::Utils::Outcome<CreateUserResult, TransferError> CreateUserOutcome;
        typedef Aws::Utils::Outcome<Aws::NoResult, TransferError> DeleteServerOutcome;
        typedef Aws::Utils::Outcome<Aws::NoResult, TransferError> DeleteSshPublicKeyOutcome;
        typedef Aws::Utils::Outcome<Aws::NoResult, TransferError> DeleteUserOutcome;
        typedef Aws::Utils::Outcome<DescribeSecurityPolicyResult, TransferError> DescribeSecurityPolicyOutcome;
        typedef Aws::Utils::Outcome<DescribeServerResult, TransferError> DescribeServerOutcome;
        typedef Aws::Utils::Outcome<DescribeUserResult, TransferError> DescribeUserOutcome;
        typedef Aws::Utils::Outcome<ImportSshPublicKeyResult, TransferError> ImportSshPublicKeyOutcome;
        typedef Aws::Utils::Outcome<ListSecurityPoliciesResult, TransferError> ListSecurityPoliciesOutcome;
        typedef Aws::Utils::Outcome<ListServersResult, TransferError> ListServersOutcome;
        typedef Aws::Utils::Outcome<ListTagsForResourceResult, TransferError> ListTagsForResourceOutcome;
        typedef Aws::Utils::Outcome<ListUsersResult, TransferError> ListUsersOutcome;
        typedef Aws::Utils::Outcome<Aws::NoResult, TransferError> StartServerOutcome;
        typedef Aws::Utils::Outcome<Aws::NoResult, TransferError> StopServerOutcome;
        typedef Aws::Utils::Outcome<Aws::NoResult, TransferError> TagResourceOutcome;
        typedef Aws::Utils::Outcome<TestIdentityProviderResult, TransferError> TestIdentityProviderOutcome;
        typedef Aws::Utils::Outcome<Aws::NoResult, TransferError> UntagResourceOutcome;
        typedef Aws::Utils::Outcome<UpdateServerResult, TransferError> UpdateServerOutcome;
        typedef Aws::Utils::Outcome<UpdateUserResult, TransferError> UpdateUserOutcome;

        typedef std::future<CreateServerOutcome> CreateServerOutcomeCallable;
        typedef std::future<CreateUserOutcome> CreateUserOutcomeCallable;
        typedef std::future<DeleteServerOutcome> DeleteServerOutcomeCallable;
        typedef std::future<DeleteSshPublicKeyOutcome> DeleteSshPublicKeyOutcomeCallable;
        typedef std::future<DeleteUserOutcome> DeleteUserOutcomeCallable;
        typedef std::future<DescribeSecurityPolicyOutcome> DescribeSecurityPolicyOutcomeCallable;
        typedef std::future<DescribeServerOutcome> DescribeServerOutcomeCallable;
        typedef std::future<DescribeUserOutcome> DescribeUserOutcomeCallable;
        typedef std::future<ImportSshPublicKeyOutcome> ImportSshPublicKeyOutcomeCallable;
        typedef std::future<ListSecurityPoliciesOutcome> ListSecurityPoliciesOutcomeCallable;
        typedef std::future<ListServersOutcome> ListServersOutcomeCallable;
        typedef std::future<ListTagsForResourceOutcome> ListTagsForResourceOutcomeCallable;
        typedef std::future<ListUsersOutcome> ListUsersOutcomeCallable;
        typedef std::future<StartServerOutcome> StartServerOutcomeCallable;
        typedef std::future<StopServerOutcome> StopServerOutcomeCallable;
        typedef std::future<TagResourceOutcome> TagResourceOutcomeCallable;
        typedef std::future<TestIdentityProviderOutcome> TestIdentityProviderOutcomeCallable;
        typedef std::future<UntagResourceOutcome> UntagResourceOutcomeCallable;
        typedef std::future<UpdateServerOutcome> UpdateServerOutcomeCallable;
        typedef std::future<UpdateUserOutcome> UpdateUserOutcomeCallable;
} // namespace Model

  class TransferClient;

    typedef std::function<void(const TransferClient*, const Model::CreateServerRequest&, const Model::CreateServerOutcome&, const std::shared_ptr<const Aws::Client::AsyncCallerContext>&) > CreateServerResponseReceivedHandler;
    typedef std::function<void(const TransferClient*, const Model::CreateUserRequest&, const Model::CreateUserOutcome&, const std::shared_ptr<const Aws::Client::AsyncCallerContext>&) > CreateUserResponseReceivedHandler;
    typedef std::function<void(const TransferClient*, const Model::DeleteServerRequest&, const Model::DeleteServerOutcome&, const std::shared_ptr<const Aws::Client::AsyncCallerContext>&) > DeleteServerResponseReceivedHandler;
    typedef std::function<void(const TransferClient*, const Model::DeleteSshPublicKeyRequest&, const Model::DeleteSshPublicKeyOutcome&, const std::shared_ptr<const Aws::Client::AsyncCallerContext>&) > DeleteSshPublicKeyResponseReceivedHandler;
    typedef std::function<void(const TransferClient*, const Model::DeleteUserRequest&, const Model::DeleteUserOutcome&, const std::shared_ptr<const Aws::Client::AsyncCallerContext>&) > DeleteUserResponseReceivedHandler;
    typedef std::function<void(const TransferClient*, const Model::DescribeSecurityPolicyRequest&, const Model::DescribeSecurityPolicyOutcome&, const std::shared_ptr<const Aws::Client::AsyncCallerContext>&) > DescribeSecurityPolicyResponseReceivedHandler;
    typedef std::function<void(const TransferClient*, const Model::DescribeServerRequest&, const Model::DescribeServerOutcome&, const std::shared_ptr<const Aws::Client::AsyncCallerContext>&) > DescribeServerResponseReceivedHandler;
    typedef std::function<void(const TransferClient*, const Model::DescribeUserRequest&, const Model::DescribeUserOutcome&, const std::shared_ptr<const Aws::Client::AsyncCallerContext>&) > DescribeUserResponseReceivedHandler;
    typedef std::function<void(const TransferClient*, const Model::ImportSshPublicKeyRequest&, const Model::ImportSshPublicKeyOutcome&, const std::shared_ptr<const Aws::Client::AsyncCallerContext>&) > ImportSshPublicKeyResponseReceivedHandler;
    typedef std::function<void(const TransferClient*, const Model::ListSecurityPoliciesRequest&, const Model::ListSecurityPoliciesOutcome&, const std::shared_ptr<const Aws::Client::AsyncCallerContext>&) > ListSecurityPoliciesResponseReceivedHandler;
    typedef std::function<void(const TransferClient*, const Model::ListServersRequest&, const Model::ListServersOutcome&, const std::shared_ptr<const Aws::Client::AsyncCallerContext>&) > ListServersResponseReceivedHandler;
    typedef std::function<void(const TransferClient*, const Model::ListTagsForResourceRequest&, const Model::ListTagsForResourceOutcome&, const std::shared_ptr<const Aws::Client::AsyncCallerContext>&) > ListTagsForResourceResponseReceivedHandler;
    typedef std::function<void(const TransferClient*, const Model::ListUsersRequest&, const Model::ListUsersOutcome&, const std::shared_ptr<const Aws::Client::AsyncCallerContext>&) > ListUsersResponseReceivedHandler;
    typedef std::function<void(const TransferClient*, const Model::StartServerRequest&, const Model::StartServerOutcome&, const std::shared_ptr<const Aws::Client::AsyncCallerContext>&) > StartServerResponseReceivedHandler;
    typedef std::function<void(const TransferClient*, const Model::StopServerRequest&, const Model::StopServerOutcome&, const std::shared_ptr<const Aws::Client::AsyncCallerContext>&) > StopServerResponseReceivedHandler;
    typedef std::function<void(const TransferClient*, const Model::TagResourceRequest&, const Model::TagResourceOutcome&, const std::shared_ptr<const Aws::Client::AsyncCallerContext>&) > TagResourceResponseReceivedHandler;
    typedef std::function<void(const TransferClient*, const Model::TestIdentityProviderRequest&, const Model::TestIdentityProviderOutcome&, const std::shared_ptr<const Aws::Client::AsyncCallerContext>&) > TestIdentityProviderResponseReceivedHandler;
    typedef std::function<void(const TransferClient*, const Model::UntagResourceRequest&, const Model::UntagResourceOutcome&, const std::shared_ptr<const Aws::Client::AsyncCallerContext>&) > UntagResourceResponseReceivedHandler;
    typedef std::function<void(const TransferClient*, const Model::UpdateServerRequest&, const Model::UpdateServerOutcome&, const std::shared_ptr<const Aws::Client::AsyncCallerContext>&) > UpdateServerResponseReceivedHandler;
    typedef std::function<void(const TransferClient*, const Model::UpdateUserRequest&, const Model::UpdateUserOutcome&, const std::shared_ptr<const Aws::Client::AsyncCallerContext>&) > UpdateUserResponseReceivedHandler;

  /**
   * <p>AWS Transfer Family is a fully managed service that enables the transfer of
   * files over the File Transfer Protocol (FTP), File Transfer Protocol over SSL
   * (FTPS), or Secure Shell (SSH) File Transfer Protocol (SFTP) directly into and
   * out of Amazon Simple Storage Service (Amazon S3). AWS helps you seamlessly
   * migrate your file transfer workflows to AWS Transfer Family by integrating with
   * existing authentication systems, and providing DNS routing with Amazon Route 53
   * so nothing changes for your customers and partners, or their applications. With
   * your data in Amazon S3, you can use it with AWS services for processing,
   * analytics, machine learning, and archiving. Getting started with AWS Transfer
   * Family is easy since there is no infrastructure to buy and set up.</p>
   */
  class AWS_TRANSFER_API TransferClient : public Aws::Client::AWSJsonClient
  {
    public:
      typedef Aws::Client::AWSJsonClient BASECLASS;

       /**
        * Initializes client to use DefaultCredentialProviderChain, with default http client factory, and optional client config. If client config
        * is not specified, it will be initialized to default values.
        */
        TransferClient(const Aws::Client::ClientConfiguration& clientConfiguration = Aws::Client::ClientConfiguration());

       /**
        * Initializes client to use SimpleAWSCredentialsProvider, with default http client factory, and optional client config. If client config
        * is not specified, it will be initialized to default values.
        */
        TransferClient(const Aws::Auth::AWSCredentials& credentials, const Aws::Client::ClientConfiguration& clientConfiguration = Aws::Client::ClientConfiguration());

       /**
        * Initializes client to use specified credentials provider with specified client config. If http client factory is not supplied,
        * the default http client factory will be used
        */
        TransferClient(const std::shared_ptr<Aws::Auth::AWSCredentialsProvider>& credentialsProvider,
            const Aws::Client::ClientConfiguration& clientConfiguration = Aws::Client::ClientConfiguration());

        virtual ~TransferClient();


        /**
         * <p>Instantiates an autoscaling virtual server based on the selected file
         * transfer protocol in AWS. When you make updates to your file transfer
         * protocol-enabled server or when you work with users, use the service-generated
         * <code>ServerId</code> property that is assigned to the newly created
         * server.</p><p><h3>See Also:</h3>   <a
         * href="http://docs.aws.amazon.com/goto/WebAPI/transfer-2018-11-05/CreateServer">AWS
         * API Reference</a></p>
         */
        virtual Model::CreateServerOutcome CreateServer(const Model::CreateServerRequest& request) const;

        /**
         * <p>Instantiates an autoscaling virtual server based on the selected file
         * transfer protocol in AWS. When you make updates to your file transfer
         * protocol-enabled server or when you work with users, use the service-generated
         * <code>ServerId</code> property that is assigned to the newly created
         * server.</p><p><h3>See Also:</h3>   <a
         * href="http://docs.aws.amazon.com/goto/WebAPI/transfer-2018-11-05/CreateServer">AWS
         * API Reference</a></p>
         *
         * returns a future to the operation so that it can be executed in parallel to other requests.
         */
        virtual Model::CreateServerOutcomeCallable CreateServerCallable(const Model::CreateServerRequest& request) const;

        /**
         * <p>Instantiates an autoscaling virtual server based on the selected file
         * transfer protocol in AWS. When you make updates to your file transfer
         * protocol-enabled server or when you work with users, use the service-generated
         * <code>ServerId</code> property that is assigned to the newly created
         * server.</p><p><h3>See Also:</h3>   <a
         * href="http://docs.aws.amazon.com/goto/WebAPI/transfer-2018-11-05/CreateServer">AWS
         * API Reference</a></p>
         *
         * Queues the request into a thread executor and triggers associated callback when operation has finished.
         */
        virtual void CreateServerAsync(const Model::CreateServerRequest& request, const CreateServerResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context = nullptr) const;

        /**
         * <p>Creates a user and associates them with an existing file transfer
         * protocol-enabled server. You can only create and associate users with servers
         * that have the <code>IdentityProviderType</code> set to
         * <code>SERVICE_MANAGED</code>. Using parameters for <code>CreateUser</code>, you
         * can specify the user name, set the home directory, store the user's public key,
         * and assign the user's AWS Identity and Access Management (IAM) role. You can
         * also optionally add a scope-down policy, and assign metadata with tags that can
         * be used to group and search for users.</p><p><h3>See Also:</h3>   <a
         * href="http://docs.aws.amazon.com/goto/WebAPI/transfer-2018-11-05/CreateUser">AWS
         * API Reference</a></p>
         */
        virtual Model::CreateUserOutcome CreateUser(const Model::CreateUserRequest& request) const;

        /**
         * <p>Creates a user and associates them with an existing file transfer
         * protocol-enabled server. You can only create and associate users with servers
         * that have the <code>IdentityProviderType</code> set to
         * <code>SERVICE_MANAGED</code>. Using parameters for <code>CreateUser</code>, you
         * can specify the user name, set the home directory, store the user's public key,
         * and assign the user's AWS Identity and Access Management (IAM) role. You can
         * also optionally add a scope-down policy, and assign metadata with tags that can
         * be used to group and search for users.</p><p><h3>See Also:</h3>   <a
         * href="http://docs.aws.amazon.com/goto/WebAPI/transfer-2018-11-05/CreateUser">AWS
         * API Reference</a></p>
         *
         * returns a future to the operation so that it can be executed in parallel to other requests.
         */
        virtual Model::CreateUserOutcomeCallable CreateUserCallable(const Model::CreateUserRequest& request) const;

        /**
         * <p>Creates a user and associates them with an existing file transfer
         * protocol-enabled server. You can only create and associate users with servers
         * that have the <code>IdentityProviderType</code> set to
         * <code>SERVICE_MANAGED</code>. Using parameters for <code>CreateUser</code>, you
         * can specify the user name, set the home directory, store the user's public key,
         * and assign the user's AWS Identity and Access Management (IAM) role. You can
         * also optionally add a scope-down policy, and assign metadata with tags that can
         * be used to group and search for users.</p><p><h3>See Also:</h3>   <a
         * href="http://docs.aws.amazon.com/goto/WebAPI/transfer-2018-11-05/CreateUser">AWS
         * API Reference</a></p>
         *
         * Queues the request into a thread executor and triggers associated callback when operation has finished.
         */
        virtual void CreateUserAsync(const Model::CreateUserRequest& request, const CreateUserResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context = nullptr) const;

        /**
         * <p>Deletes the file transfer protocol-enabled server that you specify.</p> <p>No
         * response returns from this operation.</p><p><h3>See Also:</h3>   <a
         * href="http://docs.aws.amazon.com/goto/WebAPI/transfer-2018-11-05/DeleteServer">AWS
         * API Reference</a></p>
         */
        virtual Model::DeleteServerOutcome DeleteServer(const Model::DeleteServerRequest& request) const;

        /**
         * <p>Deletes the file transfer protocol-enabled server that you specify.</p> <p>No
         * response returns from this operation.</p><p><h3>See Also:</h3>   <a
         * href="http://docs.aws.amazon.com/goto/WebAPI/transfer-2018-11-05/DeleteServer">AWS
         * API Reference</a></p>
         *
         * returns a future to the operation so that it can be executed in parallel to other requests.
         */
        virtual Model::DeleteServerOutcomeCallable DeleteServerCallable(const Model::DeleteServerRequest& request) const;

        /**
         * <p>Deletes the file transfer protocol-enabled server that you specify.</p> <p>No
         * response returns from this operation.</p><p><h3>See Also:</h3>   <a
         * href="http://docs.aws.amazon.com/goto/WebAPI/transfer-2018-11-05/DeleteServer">AWS
         * API Reference</a></p>
         *
         * Queues the request into a thread executor and triggers associated callback when operation has finished.
         */
        virtual void DeleteServerAsync(const Model::DeleteServerRequest& request, const DeleteServerResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context = nullptr) const;

        /**
         * <p>Deletes a user's Secure Shell (SSH) public key.</p> <p>No response is
         * returned from this operation.</p><p><h3>See Also:</h3>   <a
         * href="http://docs.aws.amazon.com/goto/WebAPI/transfer-2018-11-05/DeleteSshPublicKey">AWS
         * API Reference</a></p>
         */
        virtual Model::DeleteSshPublicKeyOutcome DeleteSshPublicKey(const Model::DeleteSshPublicKeyRequest& request) const;

        /**
         * <p>Deletes a user's Secure Shell (SSH) public key.</p> <p>No response is
         * returned from this operation.</p><p><h3>See Also:</h3>   <a
         * href="http://docs.aws.amazon.com/goto/WebAPI/transfer-2018-11-05/DeleteSshPublicKey">AWS
         * API Reference</a></p>
         *
         * returns a future to the operation so that it can be executed in parallel to other requests.
         */
        virtual Model::DeleteSshPublicKeyOutcomeCallable DeleteSshPublicKeyCallable(const Model::DeleteSshPublicKeyRequest& request) const;

        /**
         * <p>Deletes a user's Secure Shell (SSH) public key.</p> <p>No response is
         * returned from this operation.</p><p><h3>See Also:</h3>   <a
         * href="http://docs.aws.amazon.com/goto/WebAPI/transfer-2018-11-05/DeleteSshPublicKey">AWS
         * API Reference</a></p>
         *
         * Queues the request into a thread executor and triggers associated callback when operation has finished.
         */
        virtual void DeleteSshPublicKeyAsync(const Model::DeleteSshPublicKeyRequest& request, const DeleteSshPublicKeyResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context = nullptr) const;

        /**
         * <p>Deletes the user belonging to a file transfer protocol-enabled server you
         * specify.</p> <p>No response returns from this operation.</p>  <p>When you
         * delete a user from a server, the user's information is lost.</p>
         * <p><h3>See Also:</h3>   <a
         * href="http://docs.aws.amazon.com/goto/WebAPI/transfer-2018-11-05/DeleteUser">AWS
         * API Reference</a></p>
         */
        virtual Model::DeleteUserOutcome DeleteUser(const Model::DeleteUserRequest& request) const;

        /**
         * <p>Deletes the user belonging to a file transfer protocol-enabled server you
         * specify.</p> <p>No response returns from this operation.</p>  <p>When you
         * delete a user from a server, the user's information is lost.</p>
         * <p><h3>See Also:</h3>   <a
         * href="http://docs.aws.amazon.com/goto/WebAPI/transfer-2018-11-05/DeleteUser">AWS
         * API Reference</a></p>
         *
         * returns a future to the operation so that it can be executed in parallel to other requests.
         */
        virtual Model::DeleteUserOutcomeCallable DeleteUserCallable(const Model::DeleteUserRequest& request) const;

        /**
         * <p>Deletes the user belonging to a file transfer protocol-enabled server you
         * specify.</p> <p>No response returns from this operation.</p>  <p>When you
         * delete a user from a server, the user's information is lost.</p>
         * <p><h3>See Also:</h3>   <a
         * href="http://docs.aws.amazon.com/goto/WebAPI/transfer-2018-11-05/DeleteUser">AWS
         * API Reference</a></p>
         *
         * Queues the request into a thread executor and triggers associated callback when operation has finished.
         */
        virtual void DeleteUserAsync(const Model::DeleteUserRequest& request, const DeleteUserResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context = nullptr) const;

        /**
         * <p>Describes the security policy that is attached to your file transfer
         * protocol-enabled server. The response contains a description of the security
         * policy's properties. For more information about security policies, see <a
         * href="https://docs.aws.amazon.com/transfer/latest/userguide/security-policies.html">Working
         * with security policies</a>.</p><p><h3>See Also:</h3>   <a
         * href="http://docs.aws.amazon.com/goto/WebAPI/transfer-2018-11-05/DescribeSecurityPolicy">AWS
         * API Reference</a></p>
         */
        virtual Model::DescribeSecurityPolicyOutcome DescribeSecurityPolicy(const Model::DescribeSecurityPolicyRequest& request) const;

        /**
         * <p>Describes the security policy that is attached to your file transfer
         * protocol-enabled server. The response contains a description of the security
         * policy's properties. For more information about security policies, see <a
         * href="https://docs.aws.amazon.com/transfer/latest/userguide/security-policies.html">Working
         * with security policies</a>.</p><p><h3>See Also:</h3>   <a
         * href="http://docs.aws.amazon.com/goto/WebAPI/transfer-2018-11-05/DescribeSecurityPolicy">AWS
         * API Reference</a></p>
         *
         * returns a future to the operation so that it can be executed in parallel to other requests.
         */
        virtual Model::DescribeSecurityPolicyOutcomeCallable DescribeSecurityPolicyCallable(const Model::DescribeSecurityPolicyRequest& request) const;

        /**
         * <p>Describes the security policy that is attached to your file transfer
         * protocol-enabled server. The response contains a description of the security
         * policy's properties. For more information about security policies, see <a
         * href="https://docs.aws.amazon.com/transfer/latest/userguide/security-policies.html">Working
         * with security policies</a>.</p><p><h3>See Also:</h3>   <a
         * href="http://docs.aws.amazon.com/goto/WebAPI/transfer-2018-11-05/DescribeSecurityPolicy">AWS
         * API Reference</a></p>
         *
         * Queues the request into a thread executor and triggers associated callback when operation has finished.
         */
        virtual void DescribeSecurityPolicyAsync(const Model::DescribeSecurityPolicyRequest& request, const DescribeSecurityPolicyResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context = nullptr) const;

        /**
         * <p>Describes a file transfer protocol-enabled server that you specify by passing
         * the <code>ServerId</code> parameter.</p> <p>The response contains a description
         * of a server's properties. When you set <code>EndpointType</code> to VPC, the
         * response will contain the <code>EndpointDetails</code>.</p><p><h3>See Also:</h3>
         * <a
         * href="http://docs.aws.amazon.com/goto/WebAPI/transfer-2018-11-05/DescribeServer">AWS
         * API Reference</a></p>
         */
        virtual Model::DescribeServerOutcome DescribeServer(const Model::DescribeServerRequest& request) const;

        /**
         * <p>Describes a file transfer protocol-enabled server that you specify by passing
         * the <code>ServerId</code> parameter.</p> <p>The response contains a description
         * of a server's properties. When you set <code>EndpointType</code> to VPC, the
         * response will contain the <code>EndpointDetails</code>.</p><p><h3>See Also:</h3>
         * <a
         * href="http://docs.aws.amazon.com/goto/WebAPI/transfer-2018-11-05/DescribeServer">AWS
         * API Reference</a></p>
         *
         * returns a future to the operation so that it can be executed in parallel to other requests.
         */
        virtual Model::DescribeServerOutcomeCallable DescribeServerCallable(const Model::DescribeServerRequest& request) const;

        /**
         * <p>Describes a file transfer protocol-enabled server that you specify by passing
         * the <code>ServerId</code> parameter.</p> <p>The response contains a description
         * of a server's properties. When you set <code>EndpointType</code> to VPC, the
         * response will contain the <code>EndpointDetails</code>.</p><p><h3>See Also:</h3>
         * <a
         * href="http://docs.aws.amazon.com/goto/WebAPI/transfer-2018-11-05/DescribeServer">AWS
         * API Reference</a></p>
         *
         * Queues the request into a thread executor and triggers associated callback when operation has finished.
         */
        virtual void DescribeServerAsync(const Model::DescribeServerRequest& request, const DescribeServerResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context = nullptr) const;

        /**
         * <p>Describes the user assigned to the specific file transfer protocol-enabled
         * server, as identified by its <code>ServerId</code> property.</p> <p>The response
         * from this call returns the properties of the user associated with the
         * <code>ServerId</code> value that was specified.</p><p><h3>See Also:</h3>   <a
         * href="http://docs.aws.amazon.com/goto/WebAPI/transfer-2018-11-05/DescribeUser">AWS
         * API Reference</a></p>
         */
        virtual Model::DescribeUserOutcome DescribeUser(const Model::DescribeUserRequest& request) const;

        /**
         * <p>Describes the user assigned to the specific file transfer protocol-enabled
         * server, as identified by its <code>ServerId</code> property.</p> <p>The response
         * from this call returns the properties of the user associated with the
         * <code>ServerId</code> value that was specified.</p><p><h3>See Also:</h3>   <a
         * href="http://docs.aws.amazon.com/goto/WebAPI/transfer-2018-11-05/DescribeUser">AWS
         * API Reference</a></p>
         *
         * returns a future to the operation so that it can be executed in parallel to other requests.
         */
        virtual Model::DescribeUserOutcomeCallable DescribeUserCallable(const Model::DescribeUserRequest& request) const;

        /**
         * <p>Describes the user assigned to the specific file transfer protocol-enabled
         * server, as identified by its <code>ServerId</code> property.</p> <p>The response
         * from this call returns the properties of the user associated with the
         * <code>ServerId</code> value that was specified.</p><p><h3>See Also:</h3>   <a
         * href="http://docs.aws.amazon.com/goto/WebAPI/transfer-2018-11-05/DescribeUser">AWS
         * API Reference</a></p>
         *
         * Queues the request into a thread executor and triggers associated callback when operation has finished.
         */
        virtual void DescribeUserAsync(const Model::DescribeUserRequest& request, const DescribeUserResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context = nullptr) const;

        /**
         * <p>Adds a Secure Shell (SSH) public key to a user account identified by a
         * <code>UserName</code> value assigned to the specific file transfer
         * protocol-enabled server, identified by <code>ServerId</code>.</p> <p>The
         * response returns the <code>UserName</code> value, the <code>ServerId</code>
         * value, and the name of the <code>SshPublicKeyId</code>.</p><p><h3>See Also:</h3>
         * <a
         * href="http://docs.aws.amazon.com/goto/WebAPI/transfer-2018-11-05/ImportSshPublicKey">AWS
         * API Reference</a></p>
         */
        virtual Model::ImportSshPublicKeyOutcome ImportSshPublicKey(const Model::ImportSshPublicKeyRequest& request) const;

        /**
         * <p>Adds a Secure Shell (SSH) public key to a user account identified by a
         * <code>UserName</code> value assigned to the specific file transfer
         * protocol-enabled server, identified by <code>ServerId</code>.</p> <p>The
         * response returns the <code>UserName</code> value, the <code>ServerId</code>
         * value, and the name of the <code>SshPublicKeyId</code>.</p><p><h3>See Also:</h3>
         * <a
         * href="http://docs.aws.amazon.com/goto/WebAPI/transfer-2018-11-05/ImportSshPublicKey">AWS
         * API Reference</a></p>
         *
         * returns a future to the operation so that it can be executed in parallel to other requests.
         */
        virtual Model::ImportSshPublicKeyOutcomeCallable ImportSshPublicKeyCallable(const Model::ImportSshPublicKeyRequest& request) const;

        /**
         * <p>Adds a Secure Shell (SSH) public key to a user account identified by a
         * <code>UserName</code> value assigned to the specific file transfer
         * protocol-enabled server, identified by <code>ServerId</code>.</p> <p>The
         * response returns the <code>UserName</code> value, the <code>ServerId</code>
         * value, and the name of the <code>SshPublicKeyId</code>.</p><p><h3>See Also:</h3>
         * <a
         * href="http://docs.aws.amazon.com/goto/WebAPI/transfer-2018-11-05/ImportSshPublicKey">AWS
         * API Reference</a></p>
         *
         * Queues the request into a thread executor and triggers associated callback when operation has finished.
         */
        virtual void ImportSshPublicKeyAsync(const Model::ImportSshPublicKeyRequest& request, const ImportSshPublicKeyResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context = nullptr) const;

        /**
         * <p>Lists the security policies that are attached to your file transfer
         * protocol-enabled servers.</p><p><h3>See Also:</h3>   <a
         * href="http://docs.aws.amazon.com/goto/WebAPI/transfer-2018-11-05/ListSecurityPolicies">AWS
         * API Reference</a></p>
         */
        virtual Model::ListSecurityPoliciesOutcome ListSecurityPolicies(const Model::ListSecurityPoliciesRequest& request) const;

        /**
         * <p>Lists the security policies that are attached to your file transfer
         * protocol-enabled servers.</p><p><h3>See Also:</h3>   <a
         * href="http://docs.aws.amazon.com/goto/WebAPI/transfer-2018-11-05/ListSecurityPolicies">AWS
         * API Reference</a></p>
         *
         * returns a future to the operation so that it can be executed in parallel to other requests.
         */
        virtual Model::ListSecurityPoliciesOutcomeCallable ListSecurityPoliciesCallable(const Model::ListSecurityPoliciesRequest& request) const;

        /**
         * <p>Lists the security policies that are attached to your file transfer
         * protocol-enabled servers.</p><p><h3>See Also:</h3>   <a
         * href="http://docs.aws.amazon.com/goto/WebAPI/transfer-2018-11-05/ListSecurityPolicies">AWS
         * API Reference</a></p>
         *
         * Queues the request into a thread executor and triggers associated callback when operation has finished.
         */
        virtual void ListSecurityPoliciesAsync(const Model::ListSecurityPoliciesRequest& request, const ListSecurityPoliciesResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context = nullptr) const;

        /**
         * <p>Lists the file transfer protocol-enabled servers that are associated with
         * your AWS account.</p><p><h3>See Also:</h3>   <a
         * href="http://docs.aws.amazon.com/goto/WebAPI/transfer-2018-11-05/ListServers">AWS
         * API Reference</a></p>
         */
        virtual Model::ListServersOutcome ListServers(const Model::ListServersRequest& request) const;

        /**
         * <p>Lists the file transfer protocol-enabled servers that are associated with
         * your AWS account.</p><p><h3>See Also:</h3>   <a
         * href="http://docs.aws.amazon.com/goto/WebAPI/transfer-2018-11-05/ListServers">AWS
         * API Reference</a></p>
         *
         * returns a future to the operation so that it can be executed in parallel to other requests.
         */
        virtual Model::ListServersOutcomeCallable ListServersCallable(const Model::ListServersRequest& request) const;

        /**
         * <p>Lists the file transfer protocol-enabled servers that are associated with
         * your AWS account.</p><p><h3>See Also:</h3>   <a
         * href="http://docs.aws.amazon.com/goto/WebAPI/transfer-2018-11-05/ListServers">AWS
         * API Reference</a></p>
         *
         * Queues the request into a thread executor and triggers associated callback when operation has finished.
         */
        virtual void ListServersAsync(const Model::ListServersRequest& request, const ListServersResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context = nullptr) const;

        /**
         * <p>Lists all of the tags associated with the Amazon Resource Number (ARN) you
         * specify. The resource can be a user, server, or role.</p><p><h3>See Also:</h3>  
         * <a
         * href="http://docs.aws.amazon.com/goto/WebAPI/transfer-2018-11-05/ListTagsForResource">AWS
         * API Reference</a></p>
         */
        virtual Model::ListTagsForResourceOutcome ListTagsForResource(const Model::ListTagsForResourceRequest& request) const;

        /**
         * <p>Lists all of the tags associated with the Amazon Resource Number (ARN) you
         * specify. The resource can be a user, server, or role.</p><p><h3>See Also:</h3>  
         * <a
         * href="http://docs.aws.amazon.com/goto/WebAPI/transfer-2018-11-05/ListTagsForResource">AWS
         * API Reference</a></p>
         *
         * returns a future to the operation so that it can be executed in parallel to other requests.
         */
        virtual Model::ListTagsForResourceOutcomeCallable ListTagsForResourceCallable(const Model::ListTagsForResourceRequest& request) const;

        /**
         * <p>Lists all of the tags associated with the Amazon Resource Number (ARN) you
         * specify. The resource can be a user, server, or role.</p><p><h3>See Also:</h3>  
         * <a
         * href="http://docs.aws.amazon.com/goto/WebAPI/transfer-2018-11-05/ListTagsForResource">AWS
         * API Reference</a></p>
         *
         * Queues the request into a thread executor and triggers associated callback when operation has finished.
         */
        virtual void ListTagsForResourceAsync(const Model::ListTagsForResourceRequest& request, const ListTagsForResourceResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context = nullptr) const;

        /**
         * <p>Lists the users for a file transfer protocol-enabled server that you specify
         * by passing the <code>ServerId</code> parameter.</p><p><h3>See Also:</h3>   <a
         * href="http://docs.aws.amazon.com/goto/WebAPI/transfer-2018-11-05/ListUsers">AWS
         * API Reference</a></p>
         */
        virtual Model::ListUsersOutcome ListUsers(const Model::ListUsersRequest& request) const;

        /**
         * <p>Lists the users for a file transfer protocol-enabled server that you specify
         * by passing the <code>ServerId</code> parameter.</p><p><h3>See Also:</h3>   <a
         * href="http://docs.aws.amazon.com/goto/WebAPI/transfer-2018-11-05/ListUsers">AWS
         * API Reference</a></p>
         *
         * returns a future to the operation so that it can be executed in parallel to other requests.
         */
        virtual Model::ListUsersOutcomeCallable ListUsersCallable(const Model::ListUsersRequest& request) const;

        /**
         * <p>Lists the users for a file transfer protocol-enabled server that you specify
         * by passing the <code>ServerId</code> parameter.</p><p><h3>See Also:</h3>   <a
         * href="http://docs.aws.amazon.com/goto/WebAPI/transfer-2018-11-05/ListUsers">AWS
         * API Reference</a></p>
         *
         * Queues the request into a thread executor and triggers associated callback when operation has finished.
         */
        virtual void ListUsersAsync(const Model::ListUsersRequest& request, const ListUsersResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context = nullptr) const;

        /**
         * <p>Changes the state of a file transfer protocol-enabled server from
         * <code>OFFLINE</code> to <code>ONLINE</code>. It has no impact on a server that
         * is already <code>ONLINE</code>. An <code>ONLINE</code> server can accept and
         * process file transfer jobs.</p> <p>The state of <code>STARTING</code> indicates
         * that the server is in an intermediate state, either not fully able to respond,
         * or not fully online. The values of <code>START_FAILED</code> can indicate an
         * error condition.</p> <p>No response is returned from this call.</p><p><h3>See
         * Also:</h3>   <a
         * href="http://docs.aws.amazon.com/goto/WebAPI/transfer-2018-11-05/StartServer">AWS
         * API Reference</a></p>
         */
        virtual Model::StartServerOutcome StartServer(const Model::StartServerRequest& request) const;

        /**
         * <p>Changes the state of a file transfer protocol-enabled server from
         * <code>OFFLINE</code> to <code>ONLINE</code>. It has no impact on a server that
         * is already <code>ONLINE</code>. An <code>ONLINE</code> server can accept and
         * process file transfer jobs.</p> <p>The state of <code>STARTING</code> indicates
         * that the server is in an intermediate state, either not fully able to respond,
         * or not fully online. The values of <code>START_FAILED</code> can indicate an
         * error condition.</p> <p>No response is returned from this call.</p><p><h3>See
         * Also:</h3>   <a
         * href="http://docs.aws.amazon.com/goto/WebAPI/transfer-2018-11-05/StartServer">AWS
         * API Reference</a></p>
         *
         * returns a future to the operation so that it can be executed in parallel to other requests.
         */
        virtual Model::StartServerOutcomeCallable StartServerCallable(const Model::StartServerRequest& request) const;

        /**
         * <p>Changes the state of a file transfer protocol-enabled server from
         * <code>OFFLINE</code> to <code>ONLINE</code>. It has no impact on a server that
         * is already <code>ONLINE</code>. An <code>ONLINE</code> server can accept and
         * process file transfer jobs.</p> <p>The state of <code>STARTING</code> indicates
         * that the server is in an intermediate state, either not fully able to respond,
         * or not fully online. The values of <code>START_FAILED</code> can indicate an
         * error condition.</p> <p>No response is returned from this call.</p><p><h3>See
         * Also:</h3>   <a
         * href="http://docs.aws.amazon.com/goto/WebAPI/transfer-2018-11-05/StartServer">AWS
         * API Reference</a></p>
         *
         * Queues the request into a thread executor and triggers associated callback when operation has finished.
         */
        virtual void StartServerAsync(const Model::StartServerRequest& request, const StartServerResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context = nullptr) const;

        /**
         * <p>Changes the state of a file transfer protocol-enabled server from
         * <code>ONLINE</code> to <code>OFFLINE</code>. An <code>OFFLINE</code> server
         * cannot accept and process file transfer jobs. Information tied to your server,
         * such as server and user properties, are not affected by stopping your
         * server.</p>  <p>Stopping the server will not reduce or impact your file
         * transfer protocol endpoint billing; you must delete the server to stop being
         * billed.</p>  <p>The state of <code>STOPPING</code> indicates that the
         * server is in an intermediate state, either not fully able to respond, or not
         * fully offline. The values of <code>STOP_FAILED</code> can indicate an error
         * condition.</p> <p>No response is returned from this call.</p><p><h3>See
         * Also:</h3>   <a
         * href="http://docs.aws.amazon.com/goto/WebAPI/transfer-2018-11-05/StopServer">AWS
         * API Reference</a></p>
         */
        virtual Model::StopServerOutcome StopServer(const Model::StopServerRequest& request) const;

        /**
         * <p>Changes the state of a file transfer protocol-enabled server from
         * <code>ONLINE</code> to <code>OFFLINE</code>. An <code>OFFLINE</code> server
         * cannot accept and process file transfer jobs. Information tied to your server,
         * such as server and user properties, are not affected by stopping your
         * server.</p>  <p>Stopping the server will not reduce or impact your file
         * transfer protocol endpoint billing; you must delete the server to stop being
         * billed.</p>  <p>The state of <code>STOPPING</code> indicates that the
         * server is in an intermediate state, either not fully able to respond, or not
         * fully offline. The values of <code>STOP_FAILED</code> can indicate an error
         * condition.</p> <p>No response is returned from this call.</p><p><h3>See
         * Also:</h3>   <a
         * href="http://docs.aws.amazon.com/goto/WebAPI/transfer-2018-11-05/StopServer">AWS
         * API Reference</a></p>
         *
         * returns a future to the operation so that it can be executed in parallel to other requests.
         */
        virtual Model::StopServerOutcomeCallable StopServerCallable(const Model::StopServerRequest& request) const;

        /**
         * <p>Changes the state of a file transfer protocol-enabled server from
         * <code>ONLINE</code> to <code>OFFLINE</code>. An <code>OFFLINE</code> server
         * cannot accept and process file transfer jobs. Information tied to your server,
         * such as server and user properties, are not affected by stopping your
         * server.</p>  <p>Stopping the server will not reduce or impact your file
         * transfer protocol endpoint billing; you must delete the server to stop being
         * billed.</p>  <p>The state of <code>STOPPING</code> indicates that the
         * server is in an intermediate state, either not fully able to respond, or not
         * fully offline. The values of <code>STOP_FAILED</code> can indicate an error
         * condition.</p> <p>No response is returned from this call.</p><p><h3>See
         * Also:</h3>   <a
         * href="http://docs.aws.amazon.com/goto/WebAPI/transfer-2018-11-05/StopServer">AWS
         * API Reference</a></p>
         *
         * Queues the request into a thread executor and triggers associated callback when operation has finished.
         */
        virtual void StopServerAsync(const Model::StopServerRequest& request, const StopServerResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context = nullptr) const;

        /**
         * <p>Attaches a key-value pair to a resource, as identified by its Amazon Resource
         * Name (ARN). Resources are users, servers, roles, and other entities.</p>
         * <p>There is no response returned from this call.</p><p><h3>See Also:</h3>   <a
         * href="http://docs.aws.amazon.com/goto/WebAPI/transfer-2018-11-05/TagResource">AWS
         * API Reference</a></p>
         */
        virtual Model::TagResourceOutcome TagResource(const Model::TagResourceRequest& request) const;

        /**
         * <p>Attaches a key-value pair to a resource, as identified by its Amazon Resource
         * Name (ARN). Resources are users, servers, roles, and other entities.</p>
         * <p>There is no response returned from this call.</p><p><h3>See Also:</h3>   <a
         * href="http://docs.aws.amazon.com/goto/WebAPI/transfer-2018-11-05/TagResource">AWS
         * API Reference</a></p>
         *
         * returns a future to the operation so that it can be executed in parallel to other requests.
         */
        virtual Model::TagResourceOutcomeCallable TagResourceCallable(const Model::TagResourceRequest& request) const;

        /**
         * <p>Attaches a key-value pair to a resource, as identified by its Amazon Resource
         * Name (ARN). Resources are users, servers, roles, and other entities.</p>
         * <p>There is no response returned from this call.</p><p><h3>See Also:</h3>   <a
         * href="http://docs.aws.amazon.com/goto/WebAPI/transfer-2018-11-05/TagResource">AWS
         * API Reference</a></p>
         *
         * Queues the request into a thread executor and triggers associated callback when operation has finished.
         */
        virtual void TagResourceAsync(const Model::TagResourceRequest& request, const TagResourceResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context = nullptr) const;

        /**
         * <p>If the <code>IdentityProviderType</code> of a file transfer protocol-enabled
         * server is <code>API_Gateway</code>, tests whether your API Gateway is set up
         * successfully. We highly recommend that you call this operation to test your
         * authentication method as soon as you create your server. By doing so, you can
         * troubleshoot issues with the API Gateway integration to ensure that your users
         * can successfully use the service.</p><p><h3>See Also:</h3>   <a
         * href="http://docs.aws.amazon.com/goto/WebAPI/transfer-2018-11-05/TestIdentityProvider">AWS
         * API Reference</a></p>
         */
        virtual Model::TestIdentityProviderOutcome TestIdentityProvider(const Model::TestIdentityProviderRequest& request) const;

        /**
         * <p>If the <code>IdentityProviderType</code> of a file transfer protocol-enabled
         * server is <code>API_Gateway</code>, tests whether your API Gateway is set up
         * successfully. We highly recommend that you call this operation to test your
         * authentication method as soon as you create your server. By doing so, you can
         * troubleshoot issues with the API Gateway integration to ensure that your users
         * can successfully use the service.</p><p><h3>See Also:</h3>   <a
         * href="http://docs.aws.amazon.com/goto/WebAPI/transfer-2018-11-05/TestIdentityProvider">AWS
         * API Reference</a></p>
         *
         * returns a future to the operation so that it can be executed in parallel to other requests.
         */
        virtual Model::TestIdentityProviderOutcomeCallable TestIdentityProviderCallable(const Model::TestIdentityProviderRequest& request) const;

        /**
         * <p>If the <code>IdentityProviderType</code> of a file transfer protocol-enabled
         * server is <code>API_Gateway</code>, tests whether your API Gateway is set up
         * successfully. We highly recommend that you call this operation to test your
         * authentication method as soon as you create your server. By doing so, you can
         * troubleshoot issues with the API Gateway integration to ensure that your users
         * can successfully use the service.</p><p><h3>See Also:</h3>   <a
         * href="http://docs.aws.amazon.com/goto/WebAPI/transfer-2018-11-05/TestIdentityProvider">AWS
         * API Reference</a></p>
         *
         * Queues the request into a thread executor and triggers associated callback when operation has finished.
         */
        virtual void TestIdentityProviderAsync(const Model::TestIdentityProviderRequest& request, const TestIdentityProviderResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context = nullptr) const;

        /**
         * <p>Detaches a key-value pair from a resource, as identified by its Amazon
         * Resource Name (ARN). Resources are users, servers, roles, and other
         * entities.</p> <p>No response is returned from this call.</p><p><h3>See
         * Also:</h3>   <a
         * href="http://docs.aws.amazon.com/goto/WebAPI/transfer-2018-11-05/UntagResource">AWS
         * API Reference</a></p>
         */
        virtual Model::UntagResourceOutcome UntagResource(const Model::UntagResourceRequest& request) const;

        /**
         * <p>Detaches a key-value pair from a resource, as identified by its Amazon
         * Resource Name (ARN). Resources are users, servers, roles, and other
         * entities.</p> <p>No response is returned from this call.</p><p><h3>See
         * Also:</h3>   <a
         * href="http://docs.aws.amazon.com/goto/WebAPI/transfer-2018-11-05/UntagResource">AWS
         * API Reference</a></p>
         *
         * returns a future to the operation so that it can be executed in parallel to other requests.
         */
        virtual Model::UntagResourceOutcomeCallable UntagResourceCallable(const Model::UntagResourceRequest& request) const;

        /**
         * <p>Detaches a key-value pair from a resource, as identified by its Amazon
         * Resource Name (ARN). Resources are users, servers, roles, and other
         * entities.</p> <p>No response is returned from this call.</p><p><h3>See
         * Also:</h3>   <a
         * href="http://docs.aws.amazon.com/goto/WebAPI/transfer-2018-11-05/UntagResource">AWS
         * API Reference</a></p>
         *
         * Queues the request into a thread executor and triggers associated callback when operation has finished.
         */
        virtual void UntagResourceAsync(const Model::UntagResourceRequest& request, const UntagResourceResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context = nullptr) const;

        /**
         * <p>Updates the file transfer protocol-enabled server's properties after that
         * server has been created.</p> <p>The <code>UpdateServer</code> call returns the
         * <code>ServerId</code> of the server you updated.</p><p><h3>See Also:</h3>   <a
         * href="http://docs.aws.amazon.com/goto/WebAPI/transfer-2018-11-05/UpdateServer">AWS
         * API Reference</a></p>
         */
        virtual Model::UpdateServerOutcome UpdateServer(const Model::UpdateServerRequest& request) const;

        /**
         * <p>Updates the file transfer protocol-enabled server's properties after that
         * server has been created.</p> <p>The <code>UpdateServer</code> call returns the
         * <code>ServerId</code> of the server you updated.</p><p><h3>See Also:</h3>   <a
         * href="http://docs.aws.amazon.com/goto/WebAPI/transfer-2018-11-05/UpdateServer">AWS
         * API Reference</a></p>
         *
         * returns a future to the operation so that it can be executed in parallel to other requests.
         */
        virtual Model::UpdateServerOutcomeCallable UpdateServerCallable(const Model::UpdateServerRequest& request) const;

        /**
         * <p>Updates the file transfer protocol-enabled server's properties after that
         * server has been created.</p> <p>The <code>UpdateServer</code> call returns the
         * <code>ServerId</code> of the server you updated.</p><p><h3>See Also:</h3>   <a
         * href="http://docs.aws.amazon.com/goto/WebAPI/transfer-2018-11-05/UpdateServer">AWS
         * API Reference</a></p>
         *
         * Queues the request into a thread executor and triggers associated callback when operation has finished.
         */
        virtual void UpdateServerAsync(const Model::UpdateServerRequest& request, const UpdateServerResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context = nullptr) const;

        /**
         * <p>Assigns new properties to a user. Parameters you pass modify any or all of
         * the following: the home directory, role, and policy for the
         * <code>UserName</code> and <code>ServerId</code> you specify.</p> <p>The response
         * returns the <code>ServerId</code> and the <code>UserName</code> for the updated
         * user.</p><p><h3>See Also:</h3>   <a
         * href="http://docs.aws.amazon.com/goto/WebAPI/transfer-2018-11-05/UpdateUser">AWS
         * API Reference</a></p>
         */
        virtual Model::UpdateUserOutcome UpdateUser(const Model::UpdateUserRequest& request) const;

        /**
         * <p>Assigns new properties to a user. Parameters you pass modify any or all of
         * the following: the home directory, role, and policy for the
         * <code>UserName</code> and <code>ServerId</code> you specify.</p> <p>The response
         * returns the <code>ServerId</code> and the <code>UserName</code> for the updated
         * user.</p><p><h3>See Also:</h3>   <a
         * href="http://docs.aws.amazon.com/goto/WebAPI/transfer-2018-11-05/UpdateUser">AWS
         * API Reference</a></p>
         *
         * returns a future to the operation so that it can be executed in parallel to other requests.
         */
        virtual Model::UpdateUserOutcomeCallable UpdateUserCallable(const Model::UpdateUserRequest& request) const;

        /**
         * <p>Assigns new properties to a user. Parameters you pass modify any or all of
         * the following: the home directory, role, and policy for the
         * <code>UserName</code> and <code>ServerId</code> you specify.</p> <p>The response
         * returns the <code>ServerId</code> and the <code>UserName</code> for the updated
         * user.</p><p><h3>See Also:</h3>   <a
         * href="http://docs.aws.amazon.com/goto/WebAPI/transfer-2018-11-05/UpdateUser">AWS
         * API Reference</a></p>
         *
         * Queues the request into a thread executor and triggers associated callback when operation has finished.
         */
        virtual void UpdateUserAsync(const Model::UpdateUserRequest& request, const UpdateUserResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context = nullptr) const;


      void OverrideEndpoint(const Aws::String& endpoint);
    private:
      void init(const Aws::Client::ClientConfiguration& clientConfiguration);
        void CreateServerAsyncHelper(const Model::CreateServerRequest& request, const CreateServerResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const;
        void CreateUserAsyncHelper(const Model::CreateUserRequest& request, const CreateUserResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const;
        void DeleteServerAsyncHelper(const Model::DeleteServerRequest& request, const DeleteServerResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const;
        void DeleteSshPublicKeyAsyncHelper(const Model::DeleteSshPublicKeyRequest& request, const DeleteSshPublicKeyResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const;
        void DeleteUserAsyncHelper(const Model::DeleteUserRequest& request, const DeleteUserResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const;
        void DescribeSecurityPolicyAsyncHelper(const Model::DescribeSecurityPolicyRequest& request, const DescribeSecurityPolicyResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const;
        void DescribeServerAsyncHelper(const Model::DescribeServerRequest& request, const DescribeServerResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const;
        void DescribeUserAsyncHelper(const Model::DescribeUserRequest& request, const DescribeUserResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const;
        void ImportSshPublicKeyAsyncHelper(const Model::ImportSshPublicKeyRequest& request, const ImportSshPublicKeyResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const;
        void ListSecurityPoliciesAsyncHelper(const Model::ListSecurityPoliciesRequest& request, const ListSecurityPoliciesResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const;
        void ListServersAsyncHelper(const Model::ListServersRequest& request, const ListServersResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const;
        void ListTagsForResourceAsyncHelper(const Model::ListTagsForResourceRequest& request, const ListTagsForResourceResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const;
        void ListUsersAsyncHelper(const Model::ListUsersRequest& request, const ListUsersResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const;
        void StartServerAsyncHelper(const Model::StartServerRequest& request, const StartServerResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const;
        void StopServerAsyncHelper(const Model::StopServerRequest& request, const StopServerResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const;
        void TagResourceAsyncHelper(const Model::TagResourceRequest& request, const TagResourceResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const;
        void TestIdentityProviderAsyncHelper(const Model::TestIdentityProviderRequest& request, const TestIdentityProviderResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const;
        void UntagResourceAsyncHelper(const Model::UntagResourceRequest& request, const UntagResourceResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const;
        void UpdateServerAsyncHelper(const Model::UpdateServerRequest& request, const UpdateServerResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const;
        void UpdateUserAsyncHelper(const Model::UpdateUserRequest& request, const UpdateUserResponseReceivedHandler& handler, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) const;

      Aws::String m_uri;
      Aws::String m_configScheme;
      std::shared_ptr<Aws::Utils::Threading::Executor> m_executor;
  };

} // namespace Transfer
} // namespace Aws
