﻿/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#pragma once
#include <aws/alexaforbusiness/AlexaForBusiness_EXPORTS.h>
#include <aws/alexaforbusiness/AlexaForBusinessRequest.h>
#include <aws/core/utils/memory/stl/AWSString.h>
#include <aws/core/utils/memory/stl/AWSVector.h>
#include <aws/alexaforbusiness/model/Filter.h>
#include <aws/alexaforbusiness/model/Sort.h>
#include <utility>

namespace Aws
{
namespace AlexaForBusiness
{
namespace Model
{

  /**
   */
  class AWS_ALEXAFORBUSINESS_API SearchSkillGroupsRequest : public AlexaForBusinessRequest
  {
  public:
    SearchSkillGroupsRequest();

    // Service request name is the Operation name which will send this request out,
    // each operation should has unique request name, so that we can get operation's name from this request.
    // Note: this is not true for response, multiple operations may have the same response name,
    // so we can not get operation's name from response.
    inline virtual const char* GetServiceRequestName() const override { return "SearchSkillGroups"; }

    Aws::String SerializePayload() const override;

    Aws::Http::HeaderValueCollection GetRequestSpecificHeaders() const override;


    /**
     * <p>An optional token returned from a prior request. Use this token for
     * pagination of results from this action. If this parameter is specified, the
     * response includes only results beyond the token, up to the value specified by
     * <code>MaxResults</code>. Required.</p>
     */
    inline const Aws::String& GetNextToken() const{ return m_nextToken; }

    /**
     * <p>An optional token returned from a prior request. Use this token for
     * pagination of results from this action. If this parameter is specified, the
     * response includes only results beyond the token, up to the value specified by
     * <code>MaxResults</code>. Required.</p>
     */
    inline bool NextTokenHasBeenSet() const { return m_nextTokenHasBeenSet; }

    /**
     * <p>An optional token returned from a prior request. Use this token for
     * pagination of results from this action. If this parameter is specified, the
     * response includes only results beyond the token, up to the value specified by
     * <code>MaxResults</code>. Required.</p>
     */
    inline void SetNextToken(const Aws::String& value) { m_nextTokenHasBeenSet = true; m_nextToken = value; }

    /**
     * <p>An optional token returned from a prior request. Use this token for
     * pagination of results from this action. If this parameter is specified, the
     * response includes only results beyond the token, up to the value specified by
     * <code>MaxResults</code>. Required.</p>
     */
    inline void SetNextToken(Aws::String&& value) { m_nextTokenHasBeenSet = true; m_nextToken = std::move(value); }

    /**
     * <p>An optional token returned from a prior request. Use this token for
     * pagination of results from this action. If this parameter is specified, the
     * response includes only results beyond the token, up to the value specified by
     * <code>MaxResults</code>. Required.</p>
     */
    inline void SetNextToken(const char* value) { m_nextTokenHasBeenSet = true; m_nextToken.assign(value); }

    /**
     * <p>An optional token returned from a prior request. Use this token for
     * pagination of results from this action. If this parameter is specified, the
     * response includes only results beyond the token, up to the value specified by
     * <code>MaxResults</code>. Required.</p>
     */
    inline SearchSkillGroupsRequest& WithNextToken(const Aws::String& value) { SetNextToken(value); return *this;}

    /**
     * <p>An optional token returned from a prior request. Use this token for
     * pagination of results from this action. If this parameter is specified, the
     * response includes only results beyond the token, up to the value specified by
     * <code>MaxResults</code>. Required.</p>
     */
    inline SearchSkillGroupsRequest& WithNextToken(Aws::String&& value) { SetNextToken(std::move(value)); return *this;}

    /**
     * <p>An optional token returned from a prior request. Use this token for
     * pagination of results from this action. If this parameter is specified, the
     * response includes only results beyond the token, up to the value specified by
     * <code>MaxResults</code>. Required.</p>
     */
    inline SearchSkillGroupsRequest& WithNextToken(const char* value) { SetNextToken(value); return *this;}


    /**
     * <p>The maximum number of results to include in the response. If more results
     * exist than the specified <code>MaxResults</code> value, a token is included in
     * the response so that the remaining results can be retrieved. </p>
     */
    inline int GetMaxResults() const{ return m_maxResults; }

    /**
     * <p>The maximum number of results to include in the response. If more results
     * exist than the specified <code>MaxResults</code> value, a token is included in
     * the response so that the remaining results can be retrieved. </p>
     */
    inline bool MaxResultsHasBeenSet() const { return m_maxResultsHasBeenSet; }

    /**
     * <p>The maximum number of results to include in the response. If more results
     * exist than the specified <code>MaxResults</code> value, a token is included in
     * the response so that the remaining results can be retrieved. </p>
     */
    inline void SetMaxResults(int value) { m_maxResultsHasBeenSet = true; m_maxResults = value; }

    /**
     * <p>The maximum number of results to include in the response. If more results
     * exist than the specified <code>MaxResults</code> value, a token is included in
     * the response so that the remaining results can be retrieved. </p>
     */
    inline SearchSkillGroupsRequest& WithMaxResults(int value) { SetMaxResults(value); return *this;}


    /**
     * <p>The filters to use to list a specified set of skill groups. The supported
     * filter key is SkillGroupName. </p>
     */
    inline const Aws::Vector<Filter>& GetFilters() const{ return m_filters; }

    /**
     * <p>The filters to use to list a specified set of skill groups. The supported
     * filter key is SkillGroupName. </p>
     */
    inline bool FiltersHasBeenSet() const { return m_filtersHasBeenSet; }

    /**
     * <p>The filters to use to list a specified set of skill groups. The supported
     * filter key is SkillGroupName. </p>
     */
    inline void SetFilters(const Aws::Vector<Filter>& value) { m_filtersHasBeenSet = true; m_filters = value; }

    /**
     * <p>The filters to use to list a specified set of skill groups. The supported
     * filter key is SkillGroupName. </p>
     */
    inline void SetFilters(Aws::Vector<Filter>&& value) { m_filtersHasBeenSet = true; m_filters = std::move(value); }

    /**
     * <p>The filters to use to list a specified set of skill groups. The supported
     * filter key is SkillGroupName. </p>
     */
    inline SearchSkillGroupsRequest& WithFilters(const Aws::Vector<Filter>& value) { SetFilters(value); return *this;}

    /**
     * <p>The filters to use to list a specified set of skill groups. The supported
     * filter key is SkillGroupName. </p>
     */
    inline SearchSkillGroupsRequest& WithFilters(Aws::Vector<Filter>&& value) { SetFilters(std::move(value)); return *this;}

    /**
     * <p>The filters to use to list a specified set of skill groups. The supported
     * filter key is SkillGroupName. </p>
     */
    inline SearchSkillGroupsRequest& AddFilters(const Filter& value) { m_filtersHasBeenSet = true; m_filters.push_back(value); return *this; }

    /**
     * <p>The filters to use to list a specified set of skill groups. The supported
     * filter key is SkillGroupName. </p>
     */
    inline SearchSkillGroupsRequest& AddFilters(Filter&& value) { m_filtersHasBeenSet = true; m_filters.push_back(std::move(value)); return *this; }


    /**
     * <p>The sort order to use in listing the specified set of skill groups. The
     * supported sort key is SkillGroupName. </p>
     */
    inline const Aws::Vector<Sort>& GetSortCriteria() const{ return m_sortCriteria; }

    /**
     * <p>The sort order to use in listing the specified set of skill groups. The
     * supported sort key is SkillGroupName. </p>
     */
    inline bool SortCriteriaHasBeenSet() const { return m_sortCriteriaHasBeenSet; }

    /**
     * <p>The sort order to use in listing the specified set of skill groups. The
     * supported sort key is SkillGroupName. </p>
     */
    inline void SetSortCriteria(const Aws::Vector<Sort>& value) { m_sortCriteriaHasBeenSet = true; m_sortCriteria = value; }

    /**
     * <p>The sort order to use in listing the specified set of skill groups. The
     * supported sort key is SkillGroupName. </p>
     */
    inline void SetSortCriteria(Aws::Vector<Sort>&& value) { m_sortCriteriaHasBeenSet = true; m_sortCriteria = std::move(value); }

    /**
     * <p>The sort order to use in listing the specified set of skill groups. The
     * supported sort key is SkillGroupName. </p>
     */
    inline SearchSkillGroupsRequest& WithSortCriteria(const Aws::Vector<Sort>& value) { SetSortCriteria(value); return *this;}

    /**
     * <p>The sort order to use in listing the specified set of skill groups. The
     * supported sort key is SkillGroupName. </p>
     */
    inline SearchSkillGroupsRequest& WithSortCriteria(Aws::Vector<Sort>&& value) { SetSortCriteria(std::move(value)); return *this;}

    /**
     * <p>The sort order to use in listing the specified set of skill groups. The
     * supported sort key is SkillGroupName. </p>
     */
    inline SearchSkillGroupsRequest& AddSortCriteria(const Sort& value) { m_sortCriteriaHasBeenSet = true; m_sortCriteria.push_back(value); return *this; }

    /**
     * <p>The sort order to use in listing the specified set of skill groups. The
     * supported sort key is SkillGroupName. </p>
     */
    inline SearchSkillGroupsRequest& AddSortCriteria(Sort&& value) { m_sortCriteriaHasBeenSet = true; m_sortCriteria.push_back(std::move(value)); return *this; }

  private:

    Aws::String m_nextToken;
    bool m_nextTokenHasBeenSet;

    int m_maxResults;
    bool m_maxResultsHasBeenSet;

    Aws::Vector<Filter> m_filters;
    bool m_filtersHasBeenSet;

    Aws::Vector<Sort> m_sortCriteria;
    bool m_sortCriteriaHasBeenSet;
  };

} // namespace Model
} // namespace AlexaForBusiness
} // namespace Aws
