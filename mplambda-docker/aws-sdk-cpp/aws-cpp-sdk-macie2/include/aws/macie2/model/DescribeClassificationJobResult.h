﻿/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#pragma once
#include <aws/macie2/Macie2_EXPORTS.h>
#include <aws/core/utils/memory/stl/AWSString.h>
#include <aws/core/utils/DateTime.h>
#include <aws/core/utils/memory/stl/AWSVector.h>
#include <aws/macie2/model/JobStatus.h>
#include <aws/macie2/model/JobType.h>
#include <aws/macie2/model/S3JobDefinition.h>
#include <aws/macie2/model/JobScheduleFrequency.h>
#include <aws/macie2/model/Statistics.h>
#include <aws/core/utils/memory/stl/AWSMap.h>
#include <aws/macie2/model/UserPausedDetails.h>
#include <utility>
#include <aws/core/utils/UUID.h>

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
namespace Macie2
{
namespace Model
{
  class AWS_MACIE2_API DescribeClassificationJobResult
  {
  public:
    DescribeClassificationJobResult();
    DescribeClassificationJobResult(const Aws::AmazonWebServiceResult<Aws::Utils::Json::JsonValue>& result);
    DescribeClassificationJobResult& operator=(const Aws::AmazonWebServiceResult<Aws::Utils::Json::JsonValue>& result);


    /**
     * <p>The token that was provided to ensure the idempotency of the request to
     * create the job.</p>
     */
    inline const Aws::String& GetClientToken() const{ return m_clientToken; }

    /**
     * <p>The token that was provided to ensure the idempotency of the request to
     * create the job.</p>
     */
    inline void SetClientToken(const Aws::String& value) { m_clientToken = value; }

    /**
     * <p>The token that was provided to ensure the idempotency of the request to
     * create the job.</p>
     */
    inline void SetClientToken(Aws::String&& value) { m_clientToken = std::move(value); }

    /**
     * <p>The token that was provided to ensure the idempotency of the request to
     * create the job.</p>
     */
    inline void SetClientToken(const char* value) { m_clientToken.assign(value); }

    /**
     * <p>The token that was provided to ensure the idempotency of the request to
     * create the job.</p>
     */
    inline DescribeClassificationJobResult& WithClientToken(const Aws::String& value) { SetClientToken(value); return *this;}

    /**
     * <p>The token that was provided to ensure the idempotency of the request to
     * create the job.</p>
     */
    inline DescribeClassificationJobResult& WithClientToken(Aws::String&& value) { SetClientToken(std::move(value)); return *this;}

    /**
     * <p>The token that was provided to ensure the idempotency of the request to
     * create the job.</p>
     */
    inline DescribeClassificationJobResult& WithClientToken(const char* value) { SetClientToken(value); return *this;}


    /**
     * <p>The date and time, in UTC and extended ISO 8601 format, when the job was
     * created.</p>
     */
    inline const Aws::Utils::DateTime& GetCreatedAt() const{ return m_createdAt; }

    /**
     * <p>The date and time, in UTC and extended ISO 8601 format, when the job was
     * created.</p>
     */
    inline void SetCreatedAt(const Aws::Utils::DateTime& value) { m_createdAt = value; }

    /**
     * <p>The date and time, in UTC and extended ISO 8601 format, when the job was
     * created.</p>
     */
    inline void SetCreatedAt(Aws::Utils::DateTime&& value) { m_createdAt = std::move(value); }

    /**
     * <p>The date and time, in UTC and extended ISO 8601 format, when the job was
     * created.</p>
     */
    inline DescribeClassificationJobResult& WithCreatedAt(const Aws::Utils::DateTime& value) { SetCreatedAt(value); return *this;}

    /**
     * <p>The date and time, in UTC and extended ISO 8601 format, when the job was
     * created.</p>
     */
    inline DescribeClassificationJobResult& WithCreatedAt(Aws::Utils::DateTime&& value) { SetCreatedAt(std::move(value)); return *this;}


    /**
     * <p>The custom data identifiers that the job uses to analyze data.</p>
     */
    inline const Aws::Vector<Aws::String>& GetCustomDataIdentifierIds() const{ return m_customDataIdentifierIds; }

    /**
     * <p>The custom data identifiers that the job uses to analyze data.</p>
     */
    inline void SetCustomDataIdentifierIds(const Aws::Vector<Aws::String>& value) { m_customDataIdentifierIds = value; }

    /**
     * <p>The custom data identifiers that the job uses to analyze data.</p>
     */
    inline void SetCustomDataIdentifierIds(Aws::Vector<Aws::String>&& value) { m_customDataIdentifierIds = std::move(value); }

    /**
     * <p>The custom data identifiers that the job uses to analyze data.</p>
     */
    inline DescribeClassificationJobResult& WithCustomDataIdentifierIds(const Aws::Vector<Aws::String>& value) { SetCustomDataIdentifierIds(value); return *this;}

    /**
     * <p>The custom data identifiers that the job uses to analyze data.</p>
     */
    inline DescribeClassificationJobResult& WithCustomDataIdentifierIds(Aws::Vector<Aws::String>&& value) { SetCustomDataIdentifierIds(std::move(value)); return *this;}

    /**
     * <p>The custom data identifiers that the job uses to analyze data.</p>
     */
    inline DescribeClassificationJobResult& AddCustomDataIdentifierIds(const Aws::String& value) { m_customDataIdentifierIds.push_back(value); return *this; }

    /**
     * <p>The custom data identifiers that the job uses to analyze data.</p>
     */
    inline DescribeClassificationJobResult& AddCustomDataIdentifierIds(Aws::String&& value) { m_customDataIdentifierIds.push_back(std::move(value)); return *this; }

    /**
     * <p>The custom data identifiers that the job uses to analyze data.</p>
     */
    inline DescribeClassificationJobResult& AddCustomDataIdentifierIds(const char* value) { m_customDataIdentifierIds.push_back(value); return *this; }


    /**
     * <p>The custom description of the job.</p>
     */
    inline const Aws::String& GetDescription() const{ return m_description; }

    /**
     * <p>The custom description of the job.</p>
     */
    inline void SetDescription(const Aws::String& value) { m_description = value; }

    /**
     * <p>The custom description of the job.</p>
     */
    inline void SetDescription(Aws::String&& value) { m_description = std::move(value); }

    /**
     * <p>The custom description of the job.</p>
     */
    inline void SetDescription(const char* value) { m_description.assign(value); }

    /**
     * <p>The custom description of the job.</p>
     */
    inline DescribeClassificationJobResult& WithDescription(const Aws::String& value) { SetDescription(value); return *this;}

    /**
     * <p>The custom description of the job.</p>
     */
    inline DescribeClassificationJobResult& WithDescription(Aws::String&& value) { SetDescription(std::move(value)); return *this;}

    /**
     * <p>The custom description of the job.</p>
     */
    inline DescribeClassificationJobResult& WithDescription(const char* value) { SetDescription(value); return *this;}


    /**
     * <p>Specifies whether the job is configured to analyze all existing, eligible
     * objects immediately after it's created.</p>
     */
    inline bool GetInitialRun() const{ return m_initialRun; }

    /**
     * <p>Specifies whether the job is configured to analyze all existing, eligible
     * objects immediately after it's created.</p>
     */
    inline void SetInitialRun(bool value) { m_initialRun = value; }

    /**
     * <p>Specifies whether the job is configured to analyze all existing, eligible
     * objects immediately after it's created.</p>
     */
    inline DescribeClassificationJobResult& WithInitialRun(bool value) { SetInitialRun(value); return *this;}


    /**
     * <p>The Amazon Resource Name (ARN) of the job.</p>
     */
    inline const Aws::String& GetJobArn() const{ return m_jobArn; }

    /**
     * <p>The Amazon Resource Name (ARN) of the job.</p>
     */
    inline void SetJobArn(const Aws::String& value) { m_jobArn = value; }

    /**
     * <p>The Amazon Resource Name (ARN) of the job.</p>
     */
    inline void SetJobArn(Aws::String&& value) { m_jobArn = std::move(value); }

    /**
     * <p>The Amazon Resource Name (ARN) of the job.</p>
     */
    inline void SetJobArn(const char* value) { m_jobArn.assign(value); }

    /**
     * <p>The Amazon Resource Name (ARN) of the job.</p>
     */
    inline DescribeClassificationJobResult& WithJobArn(const Aws::String& value) { SetJobArn(value); return *this;}

    /**
     * <p>The Amazon Resource Name (ARN) of the job.</p>
     */
    inline DescribeClassificationJobResult& WithJobArn(Aws::String&& value) { SetJobArn(std::move(value)); return *this;}

    /**
     * <p>The Amazon Resource Name (ARN) of the job.</p>
     */
    inline DescribeClassificationJobResult& WithJobArn(const char* value) { SetJobArn(value); return *this;}


    /**
     * <p>The unique identifier for the job.</p>
     */
    inline const Aws::String& GetJobId() const{ return m_jobId; }

    /**
     * <p>The unique identifier for the job.</p>
     */
    inline void SetJobId(const Aws::String& value) { m_jobId = value; }

    /**
     * <p>The unique identifier for the job.</p>
     */
    inline void SetJobId(Aws::String&& value) { m_jobId = std::move(value); }

    /**
     * <p>The unique identifier for the job.</p>
     */
    inline void SetJobId(const char* value) { m_jobId.assign(value); }

    /**
     * <p>The unique identifier for the job.</p>
     */
    inline DescribeClassificationJobResult& WithJobId(const Aws::String& value) { SetJobId(value); return *this;}

    /**
     * <p>The unique identifier for the job.</p>
     */
    inline DescribeClassificationJobResult& WithJobId(Aws::String&& value) { SetJobId(std::move(value)); return *this;}

    /**
     * <p>The unique identifier for the job.</p>
     */
    inline DescribeClassificationJobResult& WithJobId(const char* value) { SetJobId(value); return *this;}


    /**
     * <p>The current status of the job. Possible values are:</p> <ul><li><p>CANCELLED
     * - You cancelled the job, or you paused the job and didn't resume it within 30
     * days of pausing it.</p></li> <li><p>COMPLETE - For a one-time job, Amazon Macie
     * finished processing all the data specified for the job. This value doesn't apply
     * to recurring jobs.</p></li> <li><p>IDLE - For a recurring job, the previous
     * scheduled run is complete and the next scheduled run is pending. This value
     * doesn't apply to one-time jobs.</p></li> <li><p>PAUSED - Amazon Macie started
     * running the job but completion of the job would exceed one or more quotas for
     * your account.</p></li> <li><p>RUNNING - For a one-time job, the job is in
     * progress. For a recurring job, a scheduled run is in progress.</p></li>
     * <li><p>USER_PAUSED - You paused the job. If you don't resume the job within 30
     * days of pausing it, the job will expire and be cancelled.</p></li></ul>
     */
    inline const JobStatus& GetJobStatus() const{ return m_jobStatus; }

    /**
     * <p>The current status of the job. Possible values are:</p> <ul><li><p>CANCELLED
     * - You cancelled the job, or you paused the job and didn't resume it within 30
     * days of pausing it.</p></li> <li><p>COMPLETE - For a one-time job, Amazon Macie
     * finished processing all the data specified for the job. This value doesn't apply
     * to recurring jobs.</p></li> <li><p>IDLE - For a recurring job, the previous
     * scheduled run is complete and the next scheduled run is pending. This value
     * doesn't apply to one-time jobs.</p></li> <li><p>PAUSED - Amazon Macie started
     * running the job but completion of the job would exceed one or more quotas for
     * your account.</p></li> <li><p>RUNNING - For a one-time job, the job is in
     * progress. For a recurring job, a scheduled run is in progress.</p></li>
     * <li><p>USER_PAUSED - You paused the job. If you don't resume the job within 30
     * days of pausing it, the job will expire and be cancelled.</p></li></ul>
     */
    inline void SetJobStatus(const JobStatus& value) { m_jobStatus = value; }

    /**
     * <p>The current status of the job. Possible values are:</p> <ul><li><p>CANCELLED
     * - You cancelled the job, or you paused the job and didn't resume it within 30
     * days of pausing it.</p></li> <li><p>COMPLETE - For a one-time job, Amazon Macie
     * finished processing all the data specified for the job. This value doesn't apply
     * to recurring jobs.</p></li> <li><p>IDLE - For a recurring job, the previous
     * scheduled run is complete and the next scheduled run is pending. This value
     * doesn't apply to one-time jobs.</p></li> <li><p>PAUSED - Amazon Macie started
     * running the job but completion of the job would exceed one or more quotas for
     * your account.</p></li> <li><p>RUNNING - For a one-time job, the job is in
     * progress. For a recurring job, a scheduled run is in progress.</p></li>
     * <li><p>USER_PAUSED - You paused the job. If you don't resume the job within 30
     * days of pausing it, the job will expire and be cancelled.</p></li></ul>
     */
    inline void SetJobStatus(JobStatus&& value) { m_jobStatus = std::move(value); }

    /**
     * <p>The current status of the job. Possible values are:</p> <ul><li><p>CANCELLED
     * - You cancelled the job, or you paused the job and didn't resume it within 30
     * days of pausing it.</p></li> <li><p>COMPLETE - For a one-time job, Amazon Macie
     * finished processing all the data specified for the job. This value doesn't apply
     * to recurring jobs.</p></li> <li><p>IDLE - For a recurring job, the previous
     * scheduled run is complete and the next scheduled run is pending. This value
     * doesn't apply to one-time jobs.</p></li> <li><p>PAUSED - Amazon Macie started
     * running the job but completion of the job would exceed one or more quotas for
     * your account.</p></li> <li><p>RUNNING - For a one-time job, the job is in
     * progress. For a recurring job, a scheduled run is in progress.</p></li>
     * <li><p>USER_PAUSED - You paused the job. If you don't resume the job within 30
     * days of pausing it, the job will expire and be cancelled.</p></li></ul>
     */
    inline DescribeClassificationJobResult& WithJobStatus(const JobStatus& value) { SetJobStatus(value); return *this;}

    /**
     * <p>The current status of the job. Possible values are:</p> <ul><li><p>CANCELLED
     * - You cancelled the job, or you paused the job and didn't resume it within 30
     * days of pausing it.</p></li> <li><p>COMPLETE - For a one-time job, Amazon Macie
     * finished processing all the data specified for the job. This value doesn't apply
     * to recurring jobs.</p></li> <li><p>IDLE - For a recurring job, the previous
     * scheduled run is complete and the next scheduled run is pending. This value
     * doesn't apply to one-time jobs.</p></li> <li><p>PAUSED - Amazon Macie started
     * running the job but completion of the job would exceed one or more quotas for
     * your account.</p></li> <li><p>RUNNING - For a one-time job, the job is in
     * progress. For a recurring job, a scheduled run is in progress.</p></li>
     * <li><p>USER_PAUSED - You paused the job. If you don't resume the job within 30
     * days of pausing it, the job will expire and be cancelled.</p></li></ul>
     */
    inline DescribeClassificationJobResult& WithJobStatus(JobStatus&& value) { SetJobStatus(std::move(value)); return *this;}


    /**
     * <p>The schedule for running the job. Possible values are:</p>
     * <ul><li><p>ONE_TIME - The job runs only once.</p></li> <li><p>SCHEDULED - The
     * job runs on a daily, weekly, or monthly basis. The scheduleFrequency property
     * indicates the recurrence pattern for the job.</p></li></ul>
     */
    inline const JobType& GetJobType() const{ return m_jobType; }

    /**
     * <p>The schedule for running the job. Possible values are:</p>
     * <ul><li><p>ONE_TIME - The job runs only once.</p></li> <li><p>SCHEDULED - The
     * job runs on a daily, weekly, or monthly basis. The scheduleFrequency property
     * indicates the recurrence pattern for the job.</p></li></ul>
     */
    inline void SetJobType(const JobType& value) { m_jobType = value; }

    /**
     * <p>The schedule for running the job. Possible values are:</p>
     * <ul><li><p>ONE_TIME - The job runs only once.</p></li> <li><p>SCHEDULED - The
     * job runs on a daily, weekly, or monthly basis. The scheduleFrequency property
     * indicates the recurrence pattern for the job.</p></li></ul>
     */
    inline void SetJobType(JobType&& value) { m_jobType = std::move(value); }

    /**
     * <p>The schedule for running the job. Possible values are:</p>
     * <ul><li><p>ONE_TIME - The job runs only once.</p></li> <li><p>SCHEDULED - The
     * job runs on a daily, weekly, or monthly basis. The scheduleFrequency property
     * indicates the recurrence pattern for the job.</p></li></ul>
     */
    inline DescribeClassificationJobResult& WithJobType(const JobType& value) { SetJobType(value); return *this;}

    /**
     * <p>The schedule for running the job. Possible values are:</p>
     * <ul><li><p>ONE_TIME - The job runs only once.</p></li> <li><p>SCHEDULED - The
     * job runs on a daily, weekly, or monthly basis. The scheduleFrequency property
     * indicates the recurrence pattern for the job.</p></li></ul>
     */
    inline DescribeClassificationJobResult& WithJobType(JobType&& value) { SetJobType(std::move(value)); return *this;}


    /**
     * <p>The date and time, in UTC and extended ISO 8601 format, when the job last
     * ran.</p>
     */
    inline const Aws::Utils::DateTime& GetLastRunTime() const{ return m_lastRunTime; }

    /**
     * <p>The date and time, in UTC and extended ISO 8601 format, when the job last
     * ran.</p>
     */
    inline void SetLastRunTime(const Aws::Utils::DateTime& value) { m_lastRunTime = value; }

    /**
     * <p>The date and time, in UTC and extended ISO 8601 format, when the job last
     * ran.</p>
     */
    inline void SetLastRunTime(Aws::Utils::DateTime&& value) { m_lastRunTime = std::move(value); }

    /**
     * <p>The date and time, in UTC and extended ISO 8601 format, when the job last
     * ran.</p>
     */
    inline DescribeClassificationJobResult& WithLastRunTime(const Aws::Utils::DateTime& value) { SetLastRunTime(value); return *this;}

    /**
     * <p>The date and time, in UTC and extended ISO 8601 format, when the job last
     * ran.</p>
     */
    inline DescribeClassificationJobResult& WithLastRunTime(Aws::Utils::DateTime&& value) { SetLastRunTime(std::move(value)); return *this;}


    /**
     * <p>The custom name of the job.</p>
     */
    inline const Aws::String& GetName() const{ return m_name; }

    /**
     * <p>The custom name of the job.</p>
     */
    inline void SetName(const Aws::String& value) { m_name = value; }

    /**
     * <p>The custom name of the job.</p>
     */
    inline void SetName(Aws::String&& value) { m_name = std::move(value); }

    /**
     * <p>The custom name of the job.</p>
     */
    inline void SetName(const char* value) { m_name.assign(value); }

    /**
     * <p>The custom name of the job.</p>
     */
    inline DescribeClassificationJobResult& WithName(const Aws::String& value) { SetName(value); return *this;}

    /**
     * <p>The custom name of the job.</p>
     */
    inline DescribeClassificationJobResult& WithName(Aws::String&& value) { SetName(std::move(value)); return *this;}

    /**
     * <p>The custom name of the job.</p>
     */
    inline DescribeClassificationJobResult& WithName(const char* value) { SetName(value); return *this;}


    /**
     * <p>The S3 buckets that the job is configured to analyze, and the scope of that
     * analysis.</p>
     */
    inline const S3JobDefinition& GetS3JobDefinition() const{ return m_s3JobDefinition; }

    /**
     * <p>The S3 buckets that the job is configured to analyze, and the scope of that
     * analysis.</p>
     */
    inline void SetS3JobDefinition(const S3JobDefinition& value) { m_s3JobDefinition = value; }

    /**
     * <p>The S3 buckets that the job is configured to analyze, and the scope of that
     * analysis.</p>
     */
    inline void SetS3JobDefinition(S3JobDefinition&& value) { m_s3JobDefinition = std::move(value); }

    /**
     * <p>The S3 buckets that the job is configured to analyze, and the scope of that
     * analysis.</p>
     */
    inline DescribeClassificationJobResult& WithS3JobDefinition(const S3JobDefinition& value) { SetS3JobDefinition(value); return *this;}

    /**
     * <p>The S3 buckets that the job is configured to analyze, and the scope of that
     * analysis.</p>
     */
    inline DescribeClassificationJobResult& WithS3JobDefinition(S3JobDefinition&& value) { SetS3JobDefinition(std::move(value)); return *this;}


    /**
     * <p>The sampling depth, as a percentage, that determines the percentage of
     * eligible objects that the job analyzes.</p>
     */
    inline int GetSamplingPercentage() const{ return m_samplingPercentage; }

    /**
     * <p>The sampling depth, as a percentage, that determines the percentage of
     * eligible objects that the job analyzes.</p>
     */
    inline void SetSamplingPercentage(int value) { m_samplingPercentage = value; }

    /**
     * <p>The sampling depth, as a percentage, that determines the percentage of
     * eligible objects that the job analyzes.</p>
     */
    inline DescribeClassificationJobResult& WithSamplingPercentage(int value) { SetSamplingPercentage(value); return *this;}


    /**
     * <p>The recurrence pattern for running the job. If the job is configured to run
     * only once, this value is null.</p>
     */
    inline const JobScheduleFrequency& GetScheduleFrequency() const{ return m_scheduleFrequency; }

    /**
     * <p>The recurrence pattern for running the job. If the job is configured to run
     * only once, this value is null.</p>
     */
    inline void SetScheduleFrequency(const JobScheduleFrequency& value) { m_scheduleFrequency = value; }

    /**
     * <p>The recurrence pattern for running the job. If the job is configured to run
     * only once, this value is null.</p>
     */
    inline void SetScheduleFrequency(JobScheduleFrequency&& value) { m_scheduleFrequency = std::move(value); }

    /**
     * <p>The recurrence pattern for running the job. If the job is configured to run
     * only once, this value is null.</p>
     */
    inline DescribeClassificationJobResult& WithScheduleFrequency(const JobScheduleFrequency& value) { SetScheduleFrequency(value); return *this;}

    /**
     * <p>The recurrence pattern for running the job. If the job is configured to run
     * only once, this value is null.</p>
     */
    inline DescribeClassificationJobResult& WithScheduleFrequency(JobScheduleFrequency&& value) { SetScheduleFrequency(std::move(value)); return *this;}


    /**
     * <p>The number of times that the job has run and processing statistics for the
     * job's current run.</p>
     */
    inline const Statistics& GetStatistics() const{ return m_statistics; }

    /**
     * <p>The number of times that the job has run and processing statistics for the
     * job's current run.</p>
     */
    inline void SetStatistics(const Statistics& value) { m_statistics = value; }

    /**
     * <p>The number of times that the job has run and processing statistics for the
     * job's current run.</p>
     */
    inline void SetStatistics(Statistics&& value) { m_statistics = std::move(value); }

    /**
     * <p>The number of times that the job has run and processing statistics for the
     * job's current run.</p>
     */
    inline DescribeClassificationJobResult& WithStatistics(const Statistics& value) { SetStatistics(value); return *this;}

    /**
     * <p>The number of times that the job has run and processing statistics for the
     * job's current run.</p>
     */
    inline DescribeClassificationJobResult& WithStatistics(Statistics&& value) { SetStatistics(std::move(value)); return *this;}


    /**
     * <p>A map of key-value pairs that specifies which tags (keys and values) are
     * associated with the classification job.</p>
     */
    inline const Aws::Map<Aws::String, Aws::String>& GetTags() const{ return m_tags; }

    /**
     * <p>A map of key-value pairs that specifies which tags (keys and values) are
     * associated with the classification job.</p>
     */
    inline void SetTags(const Aws::Map<Aws::String, Aws::String>& value) { m_tags = value; }

    /**
     * <p>A map of key-value pairs that specifies which tags (keys and values) are
     * associated with the classification job.</p>
     */
    inline void SetTags(Aws::Map<Aws::String, Aws::String>&& value) { m_tags = std::move(value); }

    /**
     * <p>A map of key-value pairs that specifies which tags (keys and values) are
     * associated with the classification job.</p>
     */
    inline DescribeClassificationJobResult& WithTags(const Aws::Map<Aws::String, Aws::String>& value) { SetTags(value); return *this;}

    /**
     * <p>A map of key-value pairs that specifies which tags (keys and values) are
     * associated with the classification job.</p>
     */
    inline DescribeClassificationJobResult& WithTags(Aws::Map<Aws::String, Aws::String>&& value) { SetTags(std::move(value)); return *this;}

    /**
     * <p>A map of key-value pairs that specifies which tags (keys and values) are
     * associated with the classification job.</p>
     */
    inline DescribeClassificationJobResult& AddTags(const Aws::String& key, const Aws::String& value) { m_tags.emplace(key, value); return *this; }

    /**
     * <p>A map of key-value pairs that specifies which tags (keys and values) are
     * associated with the classification job.</p>
     */
    inline DescribeClassificationJobResult& AddTags(Aws::String&& key, const Aws::String& value) { m_tags.emplace(std::move(key), value); return *this; }

    /**
     * <p>A map of key-value pairs that specifies which tags (keys and values) are
     * associated with the classification job.</p>
     */
    inline DescribeClassificationJobResult& AddTags(const Aws::String& key, Aws::String&& value) { m_tags.emplace(key, std::move(value)); return *this; }

    /**
     * <p>A map of key-value pairs that specifies which tags (keys and values) are
     * associated with the classification job.</p>
     */
    inline DescribeClassificationJobResult& AddTags(Aws::String&& key, Aws::String&& value) { m_tags.emplace(std::move(key), std::move(value)); return *this; }

    /**
     * <p>A map of key-value pairs that specifies which tags (keys and values) are
     * associated with the classification job.</p>
     */
    inline DescribeClassificationJobResult& AddTags(const char* key, Aws::String&& value) { m_tags.emplace(key, std::move(value)); return *this; }

    /**
     * <p>A map of key-value pairs that specifies which tags (keys and values) are
     * associated with the classification job.</p>
     */
    inline DescribeClassificationJobResult& AddTags(Aws::String&& key, const char* value) { m_tags.emplace(std::move(key), value); return *this; }

    /**
     * <p>A map of key-value pairs that specifies which tags (keys and values) are
     * associated with the classification job.</p>
     */
    inline DescribeClassificationJobResult& AddTags(const char* key, const char* value) { m_tags.emplace(key, value); return *this; }


    /**
     * <p>If the current status of the job is USER_PAUSED, specifies when the job was
     * paused and when the job will expire and be cancelled if it isn't resumed. This
     * value is present only if the value for jobStatus is USER_PAUSED.</p>
     */
    inline const UserPausedDetails& GetUserPausedDetails() const{ return m_userPausedDetails; }

    /**
     * <p>If the current status of the job is USER_PAUSED, specifies when the job was
     * paused and when the job will expire and be cancelled if it isn't resumed. This
     * value is present only if the value for jobStatus is USER_PAUSED.</p>
     */
    inline void SetUserPausedDetails(const UserPausedDetails& value) { m_userPausedDetails = value; }

    /**
     * <p>If the current status of the job is USER_PAUSED, specifies when the job was
     * paused and when the job will expire and be cancelled if it isn't resumed. This
     * value is present only if the value for jobStatus is USER_PAUSED.</p>
     */
    inline void SetUserPausedDetails(UserPausedDetails&& value) { m_userPausedDetails = std::move(value); }

    /**
     * <p>If the current status of the job is USER_PAUSED, specifies when the job was
     * paused and when the job will expire and be cancelled if it isn't resumed. This
     * value is present only if the value for jobStatus is USER_PAUSED.</p>
     */
    inline DescribeClassificationJobResult& WithUserPausedDetails(const UserPausedDetails& value) { SetUserPausedDetails(value); return *this;}

    /**
     * <p>If the current status of the job is USER_PAUSED, specifies when the job was
     * paused and when the job will expire and be cancelled if it isn't resumed. This
     * value is present only if the value for jobStatus is USER_PAUSED.</p>
     */
    inline DescribeClassificationJobResult& WithUserPausedDetails(UserPausedDetails&& value) { SetUserPausedDetails(std::move(value)); return *this;}

  private:

    Aws::String m_clientToken;

    Aws::Utils::DateTime m_createdAt;

    Aws::Vector<Aws::String> m_customDataIdentifierIds;

    Aws::String m_description;

    bool m_initialRun;

    Aws::String m_jobArn;

    Aws::String m_jobId;

    JobStatus m_jobStatus;

    JobType m_jobType;

    Aws::Utils::DateTime m_lastRunTime;

    Aws::String m_name;

    S3JobDefinition m_s3JobDefinition;

    int m_samplingPercentage;

    JobScheduleFrequency m_scheduleFrequency;

    Statistics m_statistics;

    Aws::Map<Aws::String, Aws::String> m_tags;

    UserPausedDetails m_userPausedDetails;
  };

} // namespace Model
} // namespace Macie2
} // namespace Aws
