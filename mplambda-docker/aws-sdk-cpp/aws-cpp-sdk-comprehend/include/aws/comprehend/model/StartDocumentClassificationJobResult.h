﻿/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#pragma once
#include <aws/comprehend/Comprehend_EXPORTS.h>
#include <aws/core/utils/memory/stl/AWSString.h>
#include <aws/comprehend/model/JobStatus.h>
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
namespace Comprehend
{
namespace Model
{
  class AWS_COMPREHEND_API StartDocumentClassificationJobResult
  {
  public:
    StartDocumentClassificationJobResult();
    StartDocumentClassificationJobResult(const Aws::AmazonWebServiceResult<Aws::Utils::Json::JsonValue>& result);
    StartDocumentClassificationJobResult& operator=(const Aws::AmazonWebServiceResult<Aws::Utils::Json::JsonValue>& result);


    /**
     * <p>The identifier generated for the job. To get the status of the job, use this
     * identifier with the operation.</p>
     */
    inline const Aws::String& GetJobId() const{ return m_jobId; }

    /**
     * <p>The identifier generated for the job. To get the status of the job, use this
     * identifier with the operation.</p>
     */
    inline void SetJobId(const Aws::String& value) { m_jobId = value; }

    /**
     * <p>The identifier generated for the job. To get the status of the job, use this
     * identifier with the operation.</p>
     */
    inline void SetJobId(Aws::String&& value) { m_jobId = std::move(value); }

    /**
     * <p>The identifier generated for the job. To get the status of the job, use this
     * identifier with the operation.</p>
     */
    inline void SetJobId(const char* value) { m_jobId.assign(value); }

    /**
     * <p>The identifier generated for the job. To get the status of the job, use this
     * identifier with the operation.</p>
     */
    inline StartDocumentClassificationJobResult& WithJobId(const Aws::String& value) { SetJobId(value); return *this;}

    /**
     * <p>The identifier generated for the job. To get the status of the job, use this
     * identifier with the operation.</p>
     */
    inline StartDocumentClassificationJobResult& WithJobId(Aws::String&& value) { SetJobId(std::move(value)); return *this;}

    /**
     * <p>The identifier generated for the job. To get the status of the job, use this
     * identifier with the operation.</p>
     */
    inline StartDocumentClassificationJobResult& WithJobId(const char* value) { SetJobId(value); return *this;}


    /**
     * <p>The status of the job:</p> <ul> <li> <p>SUBMITTED - The job has been received
     * and queued for processing.</p> </li> <li> <p>IN_PROGRESS - Amazon Comprehend is
     * processing the job.</p> </li> <li> <p>COMPLETED - The job was successfully
     * completed and the output is available.</p> </li> <li> <p>FAILED - The job did
     * not complete. For details, use the operation.</p> </li> <li> <p>STOP_REQUESTED -
     * Amazon Comprehend has received a stop request for the job and is processing the
     * request.</p> </li> <li> <p>STOPPED - The job was successfully stopped without
     * completing.</p> </li> </ul>
     */
    inline const JobStatus& GetJobStatus() const{ return m_jobStatus; }

    /**
     * <p>The status of the job:</p> <ul> <li> <p>SUBMITTED - The job has been received
     * and queued for processing.</p> </li> <li> <p>IN_PROGRESS - Amazon Comprehend is
     * processing the job.</p> </li> <li> <p>COMPLETED - The job was successfully
     * completed and the output is available.</p> </li> <li> <p>FAILED - The job did
     * not complete. For details, use the operation.</p> </li> <li> <p>STOP_REQUESTED -
     * Amazon Comprehend has received a stop request for the job and is processing the
     * request.</p> </li> <li> <p>STOPPED - The job was successfully stopped without
     * completing.</p> </li> </ul>
     */
    inline void SetJobStatus(const JobStatus& value) { m_jobStatus = value; }

    /**
     * <p>The status of the job:</p> <ul> <li> <p>SUBMITTED - The job has been received
     * and queued for processing.</p> </li> <li> <p>IN_PROGRESS - Amazon Comprehend is
     * processing the job.</p> </li> <li> <p>COMPLETED - The job was successfully
     * completed and the output is available.</p> </li> <li> <p>FAILED - The job did
     * not complete. For details, use the operation.</p> </li> <li> <p>STOP_REQUESTED -
     * Amazon Comprehend has received a stop request for the job and is processing the
     * request.</p> </li> <li> <p>STOPPED - The job was successfully stopped without
     * completing.</p> </li> </ul>
     */
    inline void SetJobStatus(JobStatus&& value) { m_jobStatus = std::move(value); }

    /**
     * <p>The status of the job:</p> <ul> <li> <p>SUBMITTED - The job has been received
     * and queued for processing.</p> </li> <li> <p>IN_PROGRESS - Amazon Comprehend is
     * processing the job.</p> </li> <li> <p>COMPLETED - The job was successfully
     * completed and the output is available.</p> </li> <li> <p>FAILED - The job did
     * not complete. For details, use the operation.</p> </li> <li> <p>STOP_REQUESTED -
     * Amazon Comprehend has received a stop request for the job and is processing the
     * request.</p> </li> <li> <p>STOPPED - The job was successfully stopped without
     * completing.</p> </li> </ul>
     */
    inline StartDocumentClassificationJobResult& WithJobStatus(const JobStatus& value) { SetJobStatus(value); return *this;}

    /**
     * <p>The status of the job:</p> <ul> <li> <p>SUBMITTED - The job has been received
     * and queued for processing.</p> </li> <li> <p>IN_PROGRESS - Amazon Comprehend is
     * processing the job.</p> </li> <li> <p>COMPLETED - The job was successfully
     * completed and the output is available.</p> </li> <li> <p>FAILED - The job did
     * not complete. For details, use the operation.</p> </li> <li> <p>STOP_REQUESTED -
     * Amazon Comprehend has received a stop request for the job and is processing the
     * request.</p> </li> <li> <p>STOPPED - The job was successfully stopped without
     * completing.</p> </li> </ul>
     */
    inline StartDocumentClassificationJobResult& WithJobStatus(JobStatus&& value) { SetJobStatus(std::move(value)); return *this;}

  private:

    Aws::String m_jobId;

    JobStatus m_jobStatus;
  };

} // namespace Model
} // namespace Comprehend
} // namespace Aws
