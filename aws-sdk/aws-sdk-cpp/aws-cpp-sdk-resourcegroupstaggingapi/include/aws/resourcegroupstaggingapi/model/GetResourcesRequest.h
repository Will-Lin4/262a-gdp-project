﻿/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#pragma once
#include <aws/resourcegroupstaggingapi/ResourceGroupsTaggingAPI_EXPORTS.h>
#include <aws/resourcegroupstaggingapi/ResourceGroupsTaggingAPIRequest.h>
#include <aws/core/utils/memory/stl/AWSString.h>
#include <aws/core/utils/memory/stl/AWSVector.h>
#include <aws/resourcegroupstaggingapi/model/TagFilter.h>
#include <utility>

namespace Aws
{
namespace ResourceGroupsTaggingAPI
{
namespace Model
{

  /**
   */
  class AWS_RESOURCEGROUPSTAGGINGAPI_API GetResourcesRequest : public ResourceGroupsTaggingAPIRequest
  {
  public:
    GetResourcesRequest();

    // Service request name is the Operation name which will send this request out,
    // each operation should has unique request name, so that we can get operation's name from this request.
    // Note: this is not true for response, multiple operations may have the same response name,
    // so we can not get operation's name from response.
    inline virtual const char* GetServiceRequestName() const override { return "GetResources"; }

    Aws::String SerializePayload() const override;

    Aws::Http::HeaderValueCollection GetRequestSpecificHeaders() const override;


    /**
     * <p>A string that indicates that additional data is available. Leave this value
     * empty for your initial request. If the response includes a
     * <code>PaginationToken</code>, use that string for this value to request an
     * additional page of data.</p>
     */
    inline const Aws::String& GetPaginationToken() const{ return m_paginationToken; }

    /**
     * <p>A string that indicates that additional data is available. Leave this value
     * empty for your initial request. If the response includes a
     * <code>PaginationToken</code>, use that string for this value to request an
     * additional page of data.</p>
     */
    inline bool PaginationTokenHasBeenSet() const { return m_paginationTokenHasBeenSet; }

    /**
     * <p>A string that indicates that additional data is available. Leave this value
     * empty for your initial request. If the response includes a
     * <code>PaginationToken</code>, use that string for this value to request an
     * additional page of data.</p>
     */
    inline void SetPaginationToken(const Aws::String& value) { m_paginationTokenHasBeenSet = true; m_paginationToken = value; }

    /**
     * <p>A string that indicates that additional data is available. Leave this value
     * empty for your initial request. If the response includes a
     * <code>PaginationToken</code>, use that string for this value to request an
     * additional page of data.</p>
     */
    inline void SetPaginationToken(Aws::String&& value) { m_paginationTokenHasBeenSet = true; m_paginationToken = std::move(value); }

    /**
     * <p>A string that indicates that additional data is available. Leave this value
     * empty for your initial request. If the response includes a
     * <code>PaginationToken</code>, use that string for this value to request an
     * additional page of data.</p>
     */
    inline void SetPaginationToken(const char* value) { m_paginationTokenHasBeenSet = true; m_paginationToken.assign(value); }

    /**
     * <p>A string that indicates that additional data is available. Leave this value
     * empty for your initial request. If the response includes a
     * <code>PaginationToken</code>, use that string for this value to request an
     * additional page of data.</p>
     */
    inline GetResourcesRequest& WithPaginationToken(const Aws::String& value) { SetPaginationToken(value); return *this;}

    /**
     * <p>A string that indicates that additional data is available. Leave this value
     * empty for your initial request. If the response includes a
     * <code>PaginationToken</code>, use that string for this value to request an
     * additional page of data.</p>
     */
    inline GetResourcesRequest& WithPaginationToken(Aws::String&& value) { SetPaginationToken(std::move(value)); return *this;}

    /**
     * <p>A string that indicates that additional data is available. Leave this value
     * empty for your initial request. If the response includes a
     * <code>PaginationToken</code>, use that string for this value to request an
     * additional page of data.</p>
     */
    inline GetResourcesRequest& WithPaginationToken(const char* value) { SetPaginationToken(value); return *this;}


    /**
     * <p>A list of TagFilters (keys and values). Each TagFilter specified must contain
     * a key with values as optional. A request can include up to 50 keys, and each key
     * can include up to 20 values. </p> <p>Note the following when deciding how to use
     * TagFilters:</p> <ul> <li> <p>If you <i>do</i> specify a TagFilter, the response
     * returns only those resources that are currently associated with the specified
     * tag. </p> </li> <li> <p>If you <i>don't</i> specify a TagFilter, the response
     * includes all resources that were ever associated with tags. Resources that
     * currently don't have associated tags are shown with an empty tag set, like this:
     * <code>"Tags": []</code>.</p> </li> <li> <p>If you specify more than one filter
     * in a single request, the response returns only those resources that satisfy all
     * specified filters.</p> </li> <li> <p>If you specify a filter that contains more
     * than one value for a key, the response returns resources that match any of the
     * specified values for that key.</p> </li> <li> <p>If you don't specify any values
     * for a key, the response returns resources that are tagged with that key
     * irrespective of the value.</p> <p>For example, for filters: filter1 = {key1,
     * {value1}}, filter2 = {key2, {value2,value3,value4}} , filter3 = {key3}:</p> <ul>
     * <li> <p>GetResources( {filter1} ) returns resources tagged with key1=value1</p>
     * </li> <li> <p>GetResources( {filter2} ) returns resources tagged with
     * key2=value2 or key2=value3 or key2=value4</p> </li> <li> <p>GetResources(
     * {filter3} ) returns resources tagged with any tag containing key3 as its tag
     * key, irrespective of its value</p> </li> <li> <p>GetResources(
     * {filter1,filter2,filter3} ) returns resources tagged with ( key1=value1) and (
     * key2=value2 or key2=value3 or key2=value4) and (key3, irrespective of the
     * value)</p> </li> </ul> </li> </ul>
     */
    inline const Aws::Vector<TagFilter>& GetTagFilters() const{ return m_tagFilters; }

    /**
     * <p>A list of TagFilters (keys and values). Each TagFilter specified must contain
     * a key with values as optional. A request can include up to 50 keys, and each key
     * can include up to 20 values. </p> <p>Note the following when deciding how to use
     * TagFilters:</p> <ul> <li> <p>If you <i>do</i> specify a TagFilter, the response
     * returns only those resources that are currently associated with the specified
     * tag. </p> </li> <li> <p>If you <i>don't</i> specify a TagFilter, the response
     * includes all resources that were ever associated with tags. Resources that
     * currently don't have associated tags are shown with an empty tag set, like this:
     * <code>"Tags": []</code>.</p> </li> <li> <p>If you specify more than one filter
     * in a single request, the response returns only those resources that satisfy all
     * specified filters.</p> </li> <li> <p>If you specify a filter that contains more
     * than one value for a key, the response returns resources that match any of the
     * specified values for that key.</p> </li> <li> <p>If you don't specify any values
     * for a key, the response returns resources that are tagged with that key
     * irrespective of the value.</p> <p>For example, for filters: filter1 = {key1,
     * {value1}}, filter2 = {key2, {value2,value3,value4}} , filter3 = {key3}:</p> <ul>
     * <li> <p>GetResources( {filter1} ) returns resources tagged with key1=value1</p>
     * </li> <li> <p>GetResources( {filter2} ) returns resources tagged with
     * key2=value2 or key2=value3 or key2=value4</p> </li> <li> <p>GetResources(
     * {filter3} ) returns resources tagged with any tag containing key3 as its tag
     * key, irrespective of its value</p> </li> <li> <p>GetResources(
     * {filter1,filter2,filter3} ) returns resources tagged with ( key1=value1) and (
     * key2=value2 or key2=value3 or key2=value4) and (key3, irrespective of the
     * value)</p> </li> </ul> </li> </ul>
     */
    inline bool TagFiltersHasBeenSet() const { return m_tagFiltersHasBeenSet; }

    /**
     * <p>A list of TagFilters (keys and values). Each TagFilter specified must contain
     * a key with values as optional. A request can include up to 50 keys, and each key
     * can include up to 20 values. </p> <p>Note the following when deciding how to use
     * TagFilters:</p> <ul> <li> <p>If you <i>do</i> specify a TagFilter, the response
     * returns only those resources that are currently associated with the specified
     * tag. </p> </li> <li> <p>If you <i>don't</i> specify a TagFilter, the response
     * includes all resources that were ever associated with tags. Resources that
     * currently don't have associated tags are shown with an empty tag set, like this:
     * <code>"Tags": []</code>.</p> </li> <li> <p>If you specify more than one filter
     * in a single request, the response returns only those resources that satisfy all
     * specified filters.</p> </li> <li> <p>If you specify a filter that contains more
     * than one value for a key, the response returns resources that match any of the
     * specified values for that key.</p> </li> <li> <p>If you don't specify any values
     * for a key, the response returns resources that are tagged with that key
     * irrespective of the value.</p> <p>For example, for filters: filter1 = {key1,
     * {value1}}, filter2 = {key2, {value2,value3,value4}} , filter3 = {key3}:</p> <ul>
     * <li> <p>GetResources( {filter1} ) returns resources tagged with key1=value1</p>
     * </li> <li> <p>GetResources( {filter2} ) returns resources tagged with
     * key2=value2 or key2=value3 or key2=value4</p> </li> <li> <p>GetResources(
     * {filter3} ) returns resources tagged with any tag containing key3 as its tag
     * key, irrespective of its value</p> </li> <li> <p>GetResources(
     * {filter1,filter2,filter3} ) returns resources tagged with ( key1=value1) and (
     * key2=value2 or key2=value3 or key2=value4) and (key3, irrespective of the
     * value)</p> </li> </ul> </li> </ul>
     */
    inline void SetTagFilters(const Aws::Vector<TagFilter>& value) { m_tagFiltersHasBeenSet = true; m_tagFilters = value; }

    /**
     * <p>A list of TagFilters (keys and values). Each TagFilter specified must contain
     * a key with values as optional. A request can include up to 50 keys, and each key
     * can include up to 20 values. </p> <p>Note the following when deciding how to use
     * TagFilters:</p> <ul> <li> <p>If you <i>do</i> specify a TagFilter, the response
     * returns only those resources that are currently associated with the specified
     * tag. </p> </li> <li> <p>If you <i>don't</i> specify a TagFilter, the response
     * includes all resources that were ever associated with tags. Resources that
     * currently don't have associated tags are shown with an empty tag set, like this:
     * <code>"Tags": []</code>.</p> </li> <li> <p>If you specify more than one filter
     * in a single request, the response returns only those resources that satisfy all
     * specified filters.</p> </li> <li> <p>If you specify a filter that contains more
     * than one value for a key, the response returns resources that match any of the
     * specified values for that key.</p> </li> <li> <p>If you don't specify any values
     * for a key, the response returns resources that are tagged with that key
     * irrespective of the value.</p> <p>For example, for filters: filter1 = {key1,
     * {value1}}, filter2 = {key2, {value2,value3,value4}} , filter3 = {key3}:</p> <ul>
     * <li> <p>GetResources( {filter1} ) returns resources tagged with key1=value1</p>
     * </li> <li> <p>GetResources( {filter2} ) returns resources tagged with
     * key2=value2 or key2=value3 or key2=value4</p> </li> <li> <p>GetResources(
     * {filter3} ) returns resources tagged with any tag containing key3 as its tag
     * key, irrespective of its value</p> </li> <li> <p>GetResources(
     * {filter1,filter2,filter3} ) returns resources tagged with ( key1=value1) and (
     * key2=value2 or key2=value3 or key2=value4) and (key3, irrespective of the
     * value)</p> </li> </ul> </li> </ul>
     */
    inline void SetTagFilters(Aws::Vector<TagFilter>&& value) { m_tagFiltersHasBeenSet = true; m_tagFilters = std::move(value); }

    /**
     * <p>A list of TagFilters (keys and values). Each TagFilter specified must contain
     * a key with values as optional. A request can include up to 50 keys, and each key
     * can include up to 20 values. </p> <p>Note the following when deciding how to use
     * TagFilters:</p> <ul> <li> <p>If you <i>do</i> specify a TagFilter, the response
     * returns only those resources that are currently associated with the specified
     * tag. </p> </li> <li> <p>If you <i>don't</i> specify a TagFilter, the response
     * includes all resources that were ever associated with tags. Resources that
     * currently don't have associated tags are shown with an empty tag set, like this:
     * <code>"Tags": []</code>.</p> </li> <li> <p>If you specify more than one filter
     * in a single request, the response returns only those resources that satisfy all
     * specified filters.</p> </li> <li> <p>If you specify a filter that contains more
     * than one value for a key, the response returns resources that match any of the
     * specified values for that key.</p> </li> <li> <p>If you don't specify any values
     * for a key, the response returns resources that are tagged with that key
     * irrespective of the value.</p> <p>For example, for filters: filter1 = {key1,
     * {value1}}, filter2 = {key2, {value2,value3,value4}} , filter3 = {key3}:</p> <ul>
     * <li> <p>GetResources( {filter1} ) returns resources tagged with key1=value1</p>
     * </li> <li> <p>GetResources( {filter2} ) returns resources tagged with
     * key2=value2 or key2=value3 or key2=value4</p> </li> <li> <p>GetResources(
     * {filter3} ) returns resources tagged with any tag containing key3 as its tag
     * key, irrespective of its value</p> </li> <li> <p>GetResources(
     * {filter1,filter2,filter3} ) returns resources tagged with ( key1=value1) and (
     * key2=value2 or key2=value3 or key2=value4) and (key3, irrespective of the
     * value)</p> </li> </ul> </li> </ul>
     */
    inline GetResourcesRequest& WithTagFilters(const Aws::Vector<TagFilter>& value) { SetTagFilters(value); return *this;}

    /**
     * <p>A list of TagFilters (keys and values). Each TagFilter specified must contain
     * a key with values as optional. A request can include up to 50 keys, and each key
     * can include up to 20 values. </p> <p>Note the following when deciding how to use
     * TagFilters:</p> <ul> <li> <p>If you <i>do</i> specify a TagFilter, the response
     * returns only those resources that are currently associated with the specified
     * tag. </p> </li> <li> <p>If you <i>don't</i> specify a TagFilter, the response
     * includes all resources that were ever associated with tags. Resources that
     * currently don't have associated tags are shown with an empty tag set, like this:
     * <code>"Tags": []</code>.</p> </li> <li> <p>If you specify more than one filter
     * in a single request, the response returns only those resources that satisfy all
     * specified filters.</p> </li> <li> <p>If you specify a filter that contains more
     * than one value for a key, the response returns resources that match any of the
     * specified values for that key.</p> </li> <li> <p>If you don't specify any values
     * for a key, the response returns resources that are tagged with that key
     * irrespective of the value.</p> <p>For example, for filters: filter1 = {key1,
     * {value1}}, filter2 = {key2, {value2,value3,value4}} , filter3 = {key3}:</p> <ul>
     * <li> <p>GetResources( {filter1} ) returns resources tagged with key1=value1</p>
     * </li> <li> <p>GetResources( {filter2} ) returns resources tagged with
     * key2=value2 or key2=value3 or key2=value4</p> </li> <li> <p>GetResources(
     * {filter3} ) returns resources tagged with any tag containing key3 as its tag
     * key, irrespective of its value</p> </li> <li> <p>GetResources(
     * {filter1,filter2,filter3} ) returns resources tagged with ( key1=value1) and (
     * key2=value2 or key2=value3 or key2=value4) and (key3, irrespective of the
     * value)</p> </li> </ul> </li> </ul>
     */
    inline GetResourcesRequest& WithTagFilters(Aws::Vector<TagFilter>&& value) { SetTagFilters(std::move(value)); return *this;}

    /**
     * <p>A list of TagFilters (keys and values). Each TagFilter specified must contain
     * a key with values as optional. A request can include up to 50 keys, and each key
     * can include up to 20 values. </p> <p>Note the following when deciding how to use
     * TagFilters:</p> <ul> <li> <p>If you <i>do</i> specify a TagFilter, the response
     * returns only those resources that are currently associated with the specified
     * tag. </p> </li> <li> <p>If you <i>don't</i> specify a TagFilter, the response
     * includes all resources that were ever associated with tags. Resources that
     * currently don't have associated tags are shown with an empty tag set, like this:
     * <code>"Tags": []</code>.</p> </li> <li> <p>If you specify more than one filter
     * in a single request, the response returns only those resources that satisfy all
     * specified filters.</p> </li> <li> <p>If you specify a filter that contains more
     * than one value for a key, the response returns resources that match any of the
     * specified values for that key.</p> </li> <li> <p>If you don't specify any values
     * for a key, the response returns resources that are tagged with that key
     * irrespective of the value.</p> <p>For example, for filters: filter1 = {key1,
     * {value1}}, filter2 = {key2, {value2,value3,value4}} , filter3 = {key3}:</p> <ul>
     * <li> <p>GetResources( {filter1} ) returns resources tagged with key1=value1</p>
     * </li> <li> <p>GetResources( {filter2} ) returns resources tagged with
     * key2=value2 or key2=value3 or key2=value4</p> </li> <li> <p>GetResources(
     * {filter3} ) returns resources tagged with any tag containing key3 as its tag
     * key, irrespective of its value</p> </li> <li> <p>GetResources(
     * {filter1,filter2,filter3} ) returns resources tagged with ( key1=value1) and (
     * key2=value2 or key2=value3 or key2=value4) and (key3, irrespective of the
     * value)</p> </li> </ul> </li> </ul>
     */
    inline GetResourcesRequest& AddTagFilters(const TagFilter& value) { m_tagFiltersHasBeenSet = true; m_tagFilters.push_back(value); return *this; }

    /**
     * <p>A list of TagFilters (keys and values). Each TagFilter specified must contain
     * a key with values as optional. A request can include up to 50 keys, and each key
     * can include up to 20 values. </p> <p>Note the following when deciding how to use
     * TagFilters:</p> <ul> <li> <p>If you <i>do</i> specify a TagFilter, the response
     * returns only those resources that are currently associated with the specified
     * tag. </p> </li> <li> <p>If you <i>don't</i> specify a TagFilter, the response
     * includes all resources that were ever associated with tags. Resources that
     * currently don't have associated tags are shown with an empty tag set, like this:
     * <code>"Tags": []</code>.</p> </li> <li> <p>If you specify more than one filter
     * in a single request, the response returns only those resources that satisfy all
     * specified filters.</p> </li> <li> <p>If you specify a filter that contains more
     * than one value for a key, the response returns resources that match any of the
     * specified values for that key.</p> </li> <li> <p>If you don't specify any values
     * for a key, the response returns resources that are tagged with that key
     * irrespective of the value.</p> <p>For example, for filters: filter1 = {key1,
     * {value1}}, filter2 = {key2, {value2,value3,value4}} , filter3 = {key3}:</p> <ul>
     * <li> <p>GetResources( {filter1} ) returns resources tagged with key1=value1</p>
     * </li> <li> <p>GetResources( {filter2} ) returns resources tagged with
     * key2=value2 or key2=value3 or key2=value4</p> </li> <li> <p>GetResources(
     * {filter3} ) returns resources tagged with any tag containing key3 as its tag
     * key, irrespective of its value</p> </li> <li> <p>GetResources(
     * {filter1,filter2,filter3} ) returns resources tagged with ( key1=value1) and (
     * key2=value2 or key2=value3 or key2=value4) and (key3, irrespective of the
     * value)</p> </li> </ul> </li> </ul>
     */
    inline GetResourcesRequest& AddTagFilters(TagFilter&& value) { m_tagFiltersHasBeenSet = true; m_tagFilters.push_back(std::move(value)); return *this; }


    /**
     * <p>A limit that restricts the number of resources returned by GetResources in
     * paginated output. You can set ResourcesPerPage to a minimum of 1 item and the
     * maximum of 100 items. </p>
     */
    inline int GetResourcesPerPage() const{ return m_resourcesPerPage; }

    /**
     * <p>A limit that restricts the number of resources returned by GetResources in
     * paginated output. You can set ResourcesPerPage to a minimum of 1 item and the
     * maximum of 100 items. </p>
     */
    inline bool ResourcesPerPageHasBeenSet() const { return m_resourcesPerPageHasBeenSet; }

    /**
     * <p>A limit that restricts the number of resources returned by GetResources in
     * paginated output. You can set ResourcesPerPage to a minimum of 1 item and the
     * maximum of 100 items. </p>
     */
    inline void SetResourcesPerPage(int value) { m_resourcesPerPageHasBeenSet = true; m_resourcesPerPage = value; }

    /**
     * <p>A limit that restricts the number of resources returned by GetResources in
     * paginated output. You can set ResourcesPerPage to a minimum of 1 item and the
     * maximum of 100 items. </p>
     */
    inline GetResourcesRequest& WithResourcesPerPage(int value) { SetResourcesPerPage(value); return *this;}


    /**
     * <p>AWS recommends using <code>ResourcesPerPage</code> instead of this
     * parameter.</p> <p>A limit that restricts the number of tags (key and value
     * pairs) returned by GetResources in paginated output. A resource with no tags is
     * counted as having one tag (one key and value pair).</p> <p>
     * <code>GetResources</code> does not split a resource and its associated tags
     * across pages. If the specified <code>TagsPerPage</code> would cause such a
     * break, a <code>PaginationToken</code> is returned in place of the affected
     * resource and its tags. Use that token in another request to get the remaining
     * data. For example, if you specify a <code>TagsPerPage</code> of <code>100</code>
     * and the account has 22 resources with 10 tags each (meaning that each resource
     * has 10 key and value pairs), the output will consist of three pages. The first
     * page displays the first 10 resources, each with its 10 tags. The second page
     * displays the next 10 resources, each with its 10 tags. The third page displays
     * the remaining 2 resources, each with its 10 tags.</p> <p>You can set
     * <code>TagsPerPage</code> to a minimum of 100 items and the maximum of 500
     * items.</p>
     */
    inline int GetTagsPerPage() const{ return m_tagsPerPage; }

    /**
     * <p>AWS recommends using <code>ResourcesPerPage</code> instead of this
     * parameter.</p> <p>A limit that restricts the number of tags (key and value
     * pairs) returned by GetResources in paginated output. A resource with no tags is
     * counted as having one tag (one key and value pair).</p> <p>
     * <code>GetResources</code> does not split a resource and its associated tags
     * across pages. If the specified <code>TagsPerPage</code> would cause such a
     * break, a <code>PaginationToken</code> is returned in place of the affected
     * resource and its tags. Use that token in another request to get the remaining
     * data. For example, if you specify a <code>TagsPerPage</code> of <code>100</code>
     * and the account has 22 resources with 10 tags each (meaning that each resource
     * has 10 key and value pairs), the output will consist of three pages. The first
     * page displays the first 10 resources, each with its 10 tags. The second page
     * displays the next 10 resources, each with its 10 tags. The third page displays
     * the remaining 2 resources, each with its 10 tags.</p> <p>You can set
     * <code>TagsPerPage</code> to a minimum of 100 items and the maximum of 500
     * items.</p>
     */
    inline bool TagsPerPageHasBeenSet() const { return m_tagsPerPageHasBeenSet; }

    /**
     * <p>AWS recommends using <code>ResourcesPerPage</code> instead of this
     * parameter.</p> <p>A limit that restricts the number of tags (key and value
     * pairs) returned by GetResources in paginated output. A resource with no tags is
     * counted as having one tag (one key and value pair).</p> <p>
     * <code>GetResources</code> does not split a resource and its associated tags
     * across pages. If the specified <code>TagsPerPage</code> would cause such a
     * break, a <code>PaginationToken</code> is returned in place of the affected
     * resource and its tags. Use that token in another request to get the remaining
     * data. For example, if you specify a <code>TagsPerPage</code> of <code>100</code>
     * and the account has 22 resources with 10 tags each (meaning that each resource
     * has 10 key and value pairs), the output will consist of three pages. The first
     * page displays the first 10 resources, each with its 10 tags. The second page
     * displays the next 10 resources, each with its 10 tags. The third page displays
     * the remaining 2 resources, each with its 10 tags.</p> <p>You can set
     * <code>TagsPerPage</code> to a minimum of 100 items and the maximum of 500
     * items.</p>
     */
    inline void SetTagsPerPage(int value) { m_tagsPerPageHasBeenSet = true; m_tagsPerPage = value; }

    /**
     * <p>AWS recommends using <code>ResourcesPerPage</code> instead of this
     * parameter.</p> <p>A limit that restricts the number of tags (key and value
     * pairs) returned by GetResources in paginated output. A resource with no tags is
     * counted as having one tag (one key and value pair).</p> <p>
     * <code>GetResources</code> does not split a resource and its associated tags
     * across pages. If the specified <code>TagsPerPage</code> would cause such a
     * break, a <code>PaginationToken</code> is returned in place of the affected
     * resource and its tags. Use that token in another request to get the remaining
     * data. For example, if you specify a <code>TagsPerPage</code> of <code>100</code>
     * and the account has 22 resources with 10 tags each (meaning that each resource
     * has 10 key and value pairs), the output will consist of three pages. The first
     * page displays the first 10 resources, each with its 10 tags. The second page
     * displays the next 10 resources, each with its 10 tags. The third page displays
     * the remaining 2 resources, each with its 10 tags.</p> <p>You can set
     * <code>TagsPerPage</code> to a minimum of 100 items and the maximum of 500
     * items.</p>
     */
    inline GetResourcesRequest& WithTagsPerPage(int value) { SetTagsPerPage(value); return *this;}


    /**
     * <p>The constraints on the resources that you want returned. The format of each
     * resource type is <code>service[:resourceType]</code>. For example, specifying a
     * resource type of <code>ec2</code> returns all Amazon EC2 resources (which
     * includes EC2 instances). Specifying a resource type of <code>ec2:instance</code>
     * returns only EC2 instances. </p> <p>The string for each service name and
     * resource type is the same as that embedded in a resource's Amazon Resource Name
     * (ARN). Consult the <i>AWS General Reference</i> for the following:</p> <ul> <li>
     * <p>For a list of service name strings, see <a
     * href="http://docs.aws.amazon.com/general/latest/gr/aws-arns-and-namespaces.html#genref-aws-service-namespaces">AWS
     * Service Namespaces</a>.</p> </li> <li> <p>For resource type strings, see <a
     * href="http://docs.aws.amazon.com/general/latest/gr/aws-arns-and-namespaces.html#arns-syntax">Example
     * ARNs</a>.</p> </li> <li> <p>For more information about ARNs, see <a
     * href="http://docs.aws.amazon.com/general/latest/gr/aws-arns-and-namespaces.html">Amazon
     * Resource Names (ARNs) and AWS Service Namespaces</a>.</p> </li> </ul> <p>You can
     * specify multiple resource types by using an array. The array can include up to
     * 100 items. Note that the length constraint requirement applies to each resource
     * type filter. </p>
     */
    inline const Aws::Vector<Aws::String>& GetResourceTypeFilters() const{ return m_resourceTypeFilters; }

    /**
     * <p>The constraints on the resources that you want returned. The format of each
     * resource type is <code>service[:resourceType]</code>. For example, specifying a
     * resource type of <code>ec2</code> returns all Amazon EC2 resources (which
     * includes EC2 instances). Specifying a resource type of <code>ec2:instance</code>
     * returns only EC2 instances. </p> <p>The string for each service name and
     * resource type is the same as that embedded in a resource's Amazon Resource Name
     * (ARN). Consult the <i>AWS General Reference</i> for the following:</p> <ul> <li>
     * <p>For a list of service name strings, see <a
     * href="http://docs.aws.amazon.com/general/latest/gr/aws-arns-and-namespaces.html#genref-aws-service-namespaces">AWS
     * Service Namespaces</a>.</p> </li> <li> <p>For resource type strings, see <a
     * href="http://docs.aws.amazon.com/general/latest/gr/aws-arns-and-namespaces.html#arns-syntax">Example
     * ARNs</a>.</p> </li> <li> <p>For more information about ARNs, see <a
     * href="http://docs.aws.amazon.com/general/latest/gr/aws-arns-and-namespaces.html">Amazon
     * Resource Names (ARNs) and AWS Service Namespaces</a>.</p> </li> </ul> <p>You can
     * specify multiple resource types by using an array. The array can include up to
     * 100 items. Note that the length constraint requirement applies to each resource
     * type filter. </p>
     */
    inline bool ResourceTypeFiltersHasBeenSet() const { return m_resourceTypeFiltersHasBeenSet; }

    /**
     * <p>The constraints on the resources that you want returned. The format of each
     * resource type is <code>service[:resourceType]</code>. For example, specifying a
     * resource type of <code>ec2</code> returns all Amazon EC2 resources (which
     * includes EC2 instances). Specifying a resource type of <code>ec2:instance</code>
     * returns only EC2 instances. </p> <p>The string for each service name and
     * resource type is the same as that embedded in a resource's Amazon Resource Name
     * (ARN). Consult the <i>AWS General Reference</i> for the following:</p> <ul> <li>
     * <p>For a list of service name strings, see <a
     * href="http://docs.aws.amazon.com/general/latest/gr/aws-arns-and-namespaces.html#genref-aws-service-namespaces">AWS
     * Service Namespaces</a>.</p> </li> <li> <p>For resource type strings, see <a
     * href="http://docs.aws.amazon.com/general/latest/gr/aws-arns-and-namespaces.html#arns-syntax">Example
     * ARNs</a>.</p> </li> <li> <p>For more information about ARNs, see <a
     * href="http://docs.aws.amazon.com/general/latest/gr/aws-arns-and-namespaces.html">Amazon
     * Resource Names (ARNs) and AWS Service Namespaces</a>.</p> </li> </ul> <p>You can
     * specify multiple resource types by using an array. The array can include up to
     * 100 items. Note that the length constraint requirement applies to each resource
     * type filter. </p>
     */
    inline void SetResourceTypeFilters(const Aws::Vector<Aws::String>& value) { m_resourceTypeFiltersHasBeenSet = true; m_resourceTypeFilters = value; }

    /**
     * <p>The constraints on the resources that you want returned. The format of each
     * resource type is <code>service[:resourceType]</code>. For example, specifying a
     * resource type of <code>ec2</code> returns all Amazon EC2 resources (which
     * includes EC2 instances). Specifying a resource type of <code>ec2:instance</code>
     * returns only EC2 instances. </p> <p>The string for each service name and
     * resource type is the same as that embedded in a resource's Amazon Resource Name
     * (ARN). Consult the <i>AWS General Reference</i> for the following:</p> <ul> <li>
     * <p>For a list of service name strings, see <a
     * href="http://docs.aws.amazon.com/general/latest/gr/aws-arns-and-namespaces.html#genref-aws-service-namespaces">AWS
     * Service Namespaces</a>.</p> </li> <li> <p>For resource type strings, see <a
     * href="http://docs.aws.amazon.com/general/latest/gr/aws-arns-and-namespaces.html#arns-syntax">Example
     * ARNs</a>.</p> </li> <li> <p>For more information about ARNs, see <a
     * href="http://docs.aws.amazon.com/general/latest/gr/aws-arns-and-namespaces.html">Amazon
     * Resource Names (ARNs) and AWS Service Namespaces</a>.</p> </li> </ul> <p>You can
     * specify multiple resource types by using an array. The array can include up to
     * 100 items. Note that the length constraint requirement applies to each resource
     * type filter. </p>
     */
    inline void SetResourceTypeFilters(Aws::Vector<Aws::String>&& value) { m_resourceTypeFiltersHasBeenSet = true; m_resourceTypeFilters = std::move(value); }

    /**
     * <p>The constraints on the resources that you want returned. The format of each
     * resource type is <code>service[:resourceType]</code>. For example, specifying a
     * resource type of <code>ec2</code> returns all Amazon EC2 resources (which
     * includes EC2 instances). Specifying a resource type of <code>ec2:instance</code>
     * returns only EC2 instances. </p> <p>The string for each service name and
     * resource type is the same as that embedded in a resource's Amazon Resource Name
     * (ARN). Consult the <i>AWS General Reference</i> for the following:</p> <ul> <li>
     * <p>For a list of service name strings, see <a
     * href="http://docs.aws.amazon.com/general/latest/gr/aws-arns-and-namespaces.html#genref-aws-service-namespaces">AWS
     * Service Namespaces</a>.</p> </li> <li> <p>For resource type strings, see <a
     * href="http://docs.aws.amazon.com/general/latest/gr/aws-arns-and-namespaces.html#arns-syntax">Example
     * ARNs</a>.</p> </li> <li> <p>For more information about ARNs, see <a
     * href="http://docs.aws.amazon.com/general/latest/gr/aws-arns-and-namespaces.html">Amazon
     * Resource Names (ARNs) and AWS Service Namespaces</a>.</p> </li> </ul> <p>You can
     * specify multiple resource types by using an array. The array can include up to
     * 100 items. Note that the length constraint requirement applies to each resource
     * type filter. </p>
     */
    inline GetResourcesRequest& WithResourceTypeFilters(const Aws::Vector<Aws::String>& value) { SetResourceTypeFilters(value); return *this;}

    /**
     * <p>The constraints on the resources that you want returned. The format of each
     * resource type is <code>service[:resourceType]</code>. For example, specifying a
     * resource type of <code>ec2</code> returns all Amazon EC2 resources (which
     * includes EC2 instances). Specifying a resource type of <code>ec2:instance</code>
     * returns only EC2 instances. </p> <p>The string for each service name and
     * resource type is the same as that embedded in a resource's Amazon Resource Name
     * (ARN). Consult the <i>AWS General Reference</i> for the following:</p> <ul> <li>
     * <p>For a list of service name strings, see <a
     * href="http://docs.aws.amazon.com/general/latest/gr/aws-arns-and-namespaces.html#genref-aws-service-namespaces">AWS
     * Service Namespaces</a>.</p> </li> <li> <p>For resource type strings, see <a
     * href="http://docs.aws.amazon.com/general/latest/gr/aws-arns-and-namespaces.html#arns-syntax">Example
     * ARNs</a>.</p> </li> <li> <p>For more information about ARNs, see <a
     * href="http://docs.aws.amazon.com/general/latest/gr/aws-arns-and-namespaces.html">Amazon
     * Resource Names (ARNs) and AWS Service Namespaces</a>.</p> </li> </ul> <p>You can
     * specify multiple resource types by using an array. The array can include up to
     * 100 items. Note that the length constraint requirement applies to each resource
     * type filter. </p>
     */
    inline GetResourcesRequest& WithResourceTypeFilters(Aws::Vector<Aws::String>&& value) { SetResourceTypeFilters(std::move(value)); return *this;}

    /**
     * <p>The constraints on the resources that you want returned. The format of each
     * resource type is <code>service[:resourceType]</code>. For example, specifying a
     * resource type of <code>ec2</code> returns all Amazon EC2 resources (which
     * includes EC2 instances). Specifying a resource type of <code>ec2:instance</code>
     * returns only EC2 instances. </p> <p>The string for each service name and
     * resource type is the same as that embedded in a resource's Amazon Resource Name
     * (ARN). Consult the <i>AWS General Reference</i> for the following:</p> <ul> <li>
     * <p>For a list of service name strings, see <a
     * href="http://docs.aws.amazon.com/general/latest/gr/aws-arns-and-namespaces.html#genref-aws-service-namespaces">AWS
     * Service Namespaces</a>.</p> </li> <li> <p>For resource type strings, see <a
     * href="http://docs.aws.amazon.com/general/latest/gr/aws-arns-and-namespaces.html#arns-syntax">Example
     * ARNs</a>.</p> </li> <li> <p>For more information about ARNs, see <a
     * href="http://docs.aws.amazon.com/general/latest/gr/aws-arns-and-namespaces.html">Amazon
     * Resource Names (ARNs) and AWS Service Namespaces</a>.</p> </li> </ul> <p>You can
     * specify multiple resource types by using an array. The array can include up to
     * 100 items. Note that the length constraint requirement applies to each resource
     * type filter. </p>
     */
    inline GetResourcesRequest& AddResourceTypeFilters(const Aws::String& value) { m_resourceTypeFiltersHasBeenSet = true; m_resourceTypeFilters.push_back(value); return *this; }

    /**
     * <p>The constraints on the resources that you want returned. The format of each
     * resource type is <code>service[:resourceType]</code>. For example, specifying a
     * resource type of <code>ec2</code> returns all Amazon EC2 resources (which
     * includes EC2 instances). Specifying a resource type of <code>ec2:instance</code>
     * returns only EC2 instances. </p> <p>The string for each service name and
     * resource type is the same as that embedded in a resource's Amazon Resource Name
     * (ARN). Consult the <i>AWS General Reference</i> for the following:</p> <ul> <li>
     * <p>For a list of service name strings, see <a
     * href="http://docs.aws.amazon.com/general/latest/gr/aws-arns-and-namespaces.html#genref-aws-service-namespaces">AWS
     * Service Namespaces</a>.</p> </li> <li> <p>For resource type strings, see <a
     * href="http://docs.aws.amazon.com/general/latest/gr/aws-arns-and-namespaces.html#arns-syntax">Example
     * ARNs</a>.</p> </li> <li> <p>For more information about ARNs, see <a
     * href="http://docs.aws.amazon.com/general/latest/gr/aws-arns-and-namespaces.html">Amazon
     * Resource Names (ARNs) and AWS Service Namespaces</a>.</p> </li> </ul> <p>You can
     * specify multiple resource types by using an array. The array can include up to
     * 100 items. Note that the length constraint requirement applies to each resource
     * type filter. </p>
     */
    inline GetResourcesRequest& AddResourceTypeFilters(Aws::String&& value) { m_resourceTypeFiltersHasBeenSet = true; m_resourceTypeFilters.push_back(std::move(value)); return *this; }

    /**
     * <p>The constraints on the resources that you want returned. The format of each
     * resource type is <code>service[:resourceType]</code>. For example, specifying a
     * resource type of <code>ec2</code> returns all Amazon EC2 resources (which
     * includes EC2 instances). Specifying a resource type of <code>ec2:instance</code>
     * returns only EC2 instances. </p> <p>The string for each service name and
     * resource type is the same as that embedded in a resource's Amazon Resource Name
     * (ARN). Consult the <i>AWS General Reference</i> for the following:</p> <ul> <li>
     * <p>For a list of service name strings, see <a
     * href="http://docs.aws.amazon.com/general/latest/gr/aws-arns-and-namespaces.html#genref-aws-service-namespaces">AWS
     * Service Namespaces</a>.</p> </li> <li> <p>For resource type strings, see <a
     * href="http://docs.aws.amazon.com/general/latest/gr/aws-arns-and-namespaces.html#arns-syntax">Example
     * ARNs</a>.</p> </li> <li> <p>For more information about ARNs, see <a
     * href="http://docs.aws.amazon.com/general/latest/gr/aws-arns-and-namespaces.html">Amazon
     * Resource Names (ARNs) and AWS Service Namespaces</a>.</p> </li> </ul> <p>You can
     * specify multiple resource types by using an array. The array can include up to
     * 100 items. Note that the length constraint requirement applies to each resource
     * type filter. </p>
     */
    inline GetResourcesRequest& AddResourceTypeFilters(const char* value) { m_resourceTypeFiltersHasBeenSet = true; m_resourceTypeFilters.push_back(value); return *this; }


    /**
     * <p>Specifies whether to include details regarding the compliance with the
     * effective tag policy. Set this to <code>true</code> to determine whether
     * resources are compliant with the tag policy and to get details.</p>
     */
    inline bool GetIncludeComplianceDetails() const{ return m_includeComplianceDetails; }

    /**
     * <p>Specifies whether to include details regarding the compliance with the
     * effective tag policy. Set this to <code>true</code> to determine whether
     * resources are compliant with the tag policy and to get details.</p>
     */
    inline bool IncludeComplianceDetailsHasBeenSet() const { return m_includeComplianceDetailsHasBeenSet; }

    /**
     * <p>Specifies whether to include details regarding the compliance with the
     * effective tag policy. Set this to <code>true</code> to determine whether
     * resources are compliant with the tag policy and to get details.</p>
     */
    inline void SetIncludeComplianceDetails(bool value) { m_includeComplianceDetailsHasBeenSet = true; m_includeComplianceDetails = value; }

    /**
     * <p>Specifies whether to include details regarding the compliance with the
     * effective tag policy. Set this to <code>true</code> to determine whether
     * resources are compliant with the tag policy and to get details.</p>
     */
    inline GetResourcesRequest& WithIncludeComplianceDetails(bool value) { SetIncludeComplianceDetails(value); return *this;}


    /**
     * <p>Specifies whether to exclude resources that are compliant with the tag
     * policy. Set this to <code>true</code> if you are interested in retrieving
     * information on noncompliant resources only.</p> <p>You can use this parameter
     * only if the <code>IncludeComplianceDetails</code> parameter is also set to
     * <code>true</code>.</p>
     */
    inline bool GetExcludeCompliantResources() const{ return m_excludeCompliantResources; }

    /**
     * <p>Specifies whether to exclude resources that are compliant with the tag
     * policy. Set this to <code>true</code> if you are interested in retrieving
     * information on noncompliant resources only.</p> <p>You can use this parameter
     * only if the <code>IncludeComplianceDetails</code> parameter is also set to
     * <code>true</code>.</p>
     */
    inline bool ExcludeCompliantResourcesHasBeenSet() const { return m_excludeCompliantResourcesHasBeenSet; }

    /**
     * <p>Specifies whether to exclude resources that are compliant with the tag
     * policy. Set this to <code>true</code> if you are interested in retrieving
     * information on noncompliant resources only.</p> <p>You can use this parameter
     * only if the <code>IncludeComplianceDetails</code> parameter is also set to
     * <code>true</code>.</p>
     */
    inline void SetExcludeCompliantResources(bool value) { m_excludeCompliantResourcesHasBeenSet = true; m_excludeCompliantResources = value; }

    /**
     * <p>Specifies whether to exclude resources that are compliant with the tag
     * policy. Set this to <code>true</code> if you are interested in retrieving
     * information on noncompliant resources only.</p> <p>You can use this parameter
     * only if the <code>IncludeComplianceDetails</code> parameter is also set to
     * <code>true</code>.</p>
     */
    inline GetResourcesRequest& WithExcludeCompliantResources(bool value) { SetExcludeCompliantResources(value); return *this;}

  private:

    Aws::String m_paginationToken;
    bool m_paginationTokenHasBeenSet;

    Aws::Vector<TagFilter> m_tagFilters;
    bool m_tagFiltersHasBeenSet;

    int m_resourcesPerPage;
    bool m_resourcesPerPageHasBeenSet;

    int m_tagsPerPage;
    bool m_tagsPerPageHasBeenSet;

    Aws::Vector<Aws::String> m_resourceTypeFilters;
    bool m_resourceTypeFiltersHasBeenSet;

    bool m_includeComplianceDetails;
    bool m_includeComplianceDetailsHasBeenSet;

    bool m_excludeCompliantResources;
    bool m_excludeCompliantResourcesHasBeenSet;
  };

} // namespace Model
} // namespace ResourceGroupsTaggingAPI
} // namespace Aws
