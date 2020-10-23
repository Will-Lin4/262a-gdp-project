﻿/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#pragma once
#include <aws/kafka/Kafka_EXPORTS.h>
#include <aws/core/utils/memory/stl/AWSString.h>
#include <utility>

namespace Aws
{
template<typename RESULT_TYPE>
class AmazonWebServiceResult;

namespace Utils
{
namespace Json
{
  class JsonValue;
} // namespace Json
} // namespace Utils
namespace Kafka
{
namespace Model
{
  class AWS_KAFKA_API GetBootstrapBrokersResult
  {
  public:
    GetBootstrapBrokersResult();
    GetBootstrapBrokersResult(const Aws::AmazonWebServiceResult<Aws::Utils::Json::JsonValue>& result);
    GetBootstrapBrokersResult& operator=(const Aws::AmazonWebServiceResult<Aws::Utils::Json::JsonValue>& result);


    /**
     * 
            <p>A string containing one or more hostname:port pairs.</p>
       
     *  
     */
    inline const Aws::String& GetBootstrapBrokerString() const{ return m_bootstrapBrokerString; }

    /**
     * 
            <p>A string containing one or more hostname:port pairs.</p>
       
     *  
     */
    inline void SetBootstrapBrokerString(const Aws::String& value) { m_bootstrapBrokerString = value; }

    /**
     * 
            <p>A string containing one or more hostname:port pairs.</p>
       
     *  
     */
    inline void SetBootstrapBrokerString(Aws::String&& value) { m_bootstrapBrokerString = std::move(value); }

    /**
     * 
            <p>A string containing one or more hostname:port pairs.</p>
       
     *  
     */
    inline void SetBootstrapBrokerString(const char* value) { m_bootstrapBrokerString.assign(value); }

    /**
     * 
            <p>A string containing one or more hostname:port pairs.</p>
       
     *  
     */
    inline GetBootstrapBrokersResult& WithBootstrapBrokerString(const Aws::String& value) { SetBootstrapBrokerString(value); return *this;}

    /**
     * 
            <p>A string containing one or more hostname:port pairs.</p>
       
     *  
     */
    inline GetBootstrapBrokersResult& WithBootstrapBrokerString(Aws::String&& value) { SetBootstrapBrokerString(std::move(value)); return *this;}

    /**
     * 
            <p>A string containing one or more hostname:port pairs.</p>
       
     *  
     */
    inline GetBootstrapBrokersResult& WithBootstrapBrokerString(const char* value) { SetBootstrapBrokerString(value); return *this;}


    /**
     * 
            <p>A string containing one or more DNS names (or IP) and TLS port
     * pairs.</p>
         
     */
    inline const Aws::String& GetBootstrapBrokerStringTls() const{ return m_bootstrapBrokerStringTls; }

    /**
     * 
            <p>A string containing one or more DNS names (or IP) and TLS port
     * pairs.</p>
         
     */
    inline void SetBootstrapBrokerStringTls(const Aws::String& value) { m_bootstrapBrokerStringTls = value; }

    /**
     * 
            <p>A string containing one or more DNS names (or IP) and TLS port
     * pairs.</p>
         
     */
    inline void SetBootstrapBrokerStringTls(Aws::String&& value) { m_bootstrapBrokerStringTls = std::move(value); }

    /**
     * 
            <p>A string containing one or more DNS names (or IP) and TLS port
     * pairs.</p>
         
     */
    inline void SetBootstrapBrokerStringTls(const char* value) { m_bootstrapBrokerStringTls.assign(value); }

    /**
     * 
            <p>A string containing one or more DNS names (or IP) and TLS port
     * pairs.</p>
         
     */
    inline GetBootstrapBrokersResult& WithBootstrapBrokerStringTls(const Aws::String& value) { SetBootstrapBrokerStringTls(value); return *this;}

    /**
     * 
            <p>A string containing one or more DNS names (or IP) and TLS port
     * pairs.</p>
         
     */
    inline GetBootstrapBrokersResult& WithBootstrapBrokerStringTls(Aws::String&& value) { SetBootstrapBrokerStringTls(std::move(value)); return *this;}

    /**
     * 
            <p>A string containing one or more DNS names (or IP) and TLS port
     * pairs.</p>
         
     */
    inline GetBootstrapBrokersResult& WithBootstrapBrokerStringTls(const char* value) { SetBootstrapBrokerStringTls(value); return *this;}


    /**
     * 
            <p>A string containing one or more DNS names (or IP) and Sasl Scram
     * port pairs.</p>
         
     */
    inline const Aws::String& GetBootstrapBrokerStringSaslScram() const{ return m_bootstrapBrokerStringSaslScram; }

    /**
     * 
            <p>A string containing one or more DNS names (or IP) and Sasl Scram
     * port pairs.</p>
         
     */
    inline void SetBootstrapBrokerStringSaslScram(const Aws::String& value) { m_bootstrapBrokerStringSaslScram = value; }

    /**
     * 
            <p>A string containing one or more DNS names (or IP) and Sasl Scram
     * port pairs.</p>
         
     */
    inline void SetBootstrapBrokerStringSaslScram(Aws::String&& value) { m_bootstrapBrokerStringSaslScram = std::move(value); }

    /**
     * 
            <p>A string containing one or more DNS names (or IP) and Sasl Scram
     * port pairs.</p>
         
     */
    inline void SetBootstrapBrokerStringSaslScram(const char* value) { m_bootstrapBrokerStringSaslScram.assign(value); }

    /**
     * 
            <p>A string containing one or more DNS names (or IP) and Sasl Scram
     * port pairs.</p>
         
     */
    inline GetBootstrapBrokersResult& WithBootstrapBrokerStringSaslScram(const Aws::String& value) { SetBootstrapBrokerStringSaslScram(value); return *this;}

    /**
     * 
            <p>A string containing one or more DNS names (or IP) and Sasl Scram
     * port pairs.</p>
         
     */
    inline GetBootstrapBrokersResult& WithBootstrapBrokerStringSaslScram(Aws::String&& value) { SetBootstrapBrokerStringSaslScram(std::move(value)); return *this;}

    /**
     * 
            <p>A string containing one or more DNS names (or IP) and Sasl Scram
     * port pairs.</p>
         
     */
    inline GetBootstrapBrokersResult& WithBootstrapBrokerStringSaslScram(const char* value) { SetBootstrapBrokerStringSaslScram(value); return *this;}

  private:

    Aws::String m_bootstrapBrokerString;

    Aws::String m_bootstrapBrokerStringTls;

    Aws::String m_bootstrapBrokerStringSaslScram;
  };

} // namespace Model
} // namespace Kafka
} // namespace Aws
