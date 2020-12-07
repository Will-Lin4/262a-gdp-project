﻿/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#pragma once
#include <aws/ssm/SSM_EXPORTS.h>
#include <aws/ssm/SSMRequest.h>
#include <aws/ssm/model/ResourceTypeForTagging.h>
#include <aws/core/utils/memory/stl/AWSString.h>
#include <aws/core/utils/memory/stl/AWSVector.h>
#include <utility>

namespace Aws
{
namespace SSM
{
namespace Model
{

  /**
   */
  class AWS_SSM_API RemoveTagsFromResourceRequest : public SSMRequest
  {
  public:
    RemoveTagsFromResourceRequest();

    // Service request name is the Operation name which will send this request out,
    // each operation should has unique request name, so that we can get operation's name from this request.
    // Note: this is not true for response, multiple operations may have the same response name,
    // so we can not get operation's name from response.
    inline virtual const char* GetServiceRequestName() const override { return "RemoveTagsFromResource"; }

    Aws::String SerializePayload() const override;

    Aws::Http::HeaderValueCollection GetRequestSpecificHeaders() const override;


    /**
     * <p>The type of resource from which you want to remove a tag.</p>  <p>The
     * ManagedInstance type for this API action is only for on-premises managed
     * instances. Specify the name of the managed instance in the following format:
     * mi-ID_number. For example, mi-1a2b3c4d5e6f.</p> 
     */
    inline const ResourceTypeForTagging& GetResourceType() const{ return m_resourceType; }

    /**
     * <p>The type of resource from which you want to remove a tag.</p>  <p>The
     * ManagedInstance type for this API action is only for on-premises managed
     * instances. Specify the name of the managed instance in the following format:
     * mi-ID_number. For example, mi-1a2b3c4d5e6f.</p> 
     */
    inline bool ResourceTypeHasBeenSet() const { return m_resourceTypeHasBeenSet; }

    /**
     * <p>The type of resource from which you want to remove a tag.</p>  <p>The
     * ManagedInstance type for this API action is only for on-premises managed
     * instances. Specify the name of the managed instance in the following format:
     * mi-ID_number. For example, mi-1a2b3c4d5e6f.</p> 
     */
    inline void SetResourceType(const ResourceTypeForTagging& value) { m_resourceTypeHasBeenSet = true; m_resourceType = value; }

    /**
     * <p>The type of resource from which you want to remove a tag.</p>  <p>The
     * ManagedInstance type for this API action is only for on-premises managed
     * instances. Specify the name of the managed instance in the following format:
     * mi-ID_number. For example, mi-1a2b3c4d5e6f.</p> 
     */
    inline void SetResourceType(ResourceTypeForTagging&& value) { m_resourceTypeHasBeenSet = true; m_resourceType = std::move(value); }

    /**
     * <p>The type of resource from which you want to remove a tag.</p>  <p>The
     * ManagedInstance type for this API action is only for on-premises managed
     * instances. Specify the name of the managed instance in the following format:
     * mi-ID_number. For example, mi-1a2b3c4d5e6f.</p> 
     */
    inline RemoveTagsFromResourceRequest& WithResourceType(const ResourceTypeForTagging& value) { SetResourceType(value); return *this;}

    /**
     * <p>The type of resource from which you want to remove a tag.</p>  <p>The
     * ManagedInstance type for this API action is only for on-premises managed
     * instances. Specify the name of the managed instance in the following format:
     * mi-ID_number. For example, mi-1a2b3c4d5e6f.</p> 
     */
    inline RemoveTagsFromResourceRequest& WithResourceType(ResourceTypeForTagging&& value) { SetResourceType(std::move(value)); return *this;}


    /**
     * <p>The ID of the resource from which you want to remove tags. For example:</p>
     * <p>ManagedInstance: mi-012345abcde</p> <p>MaintenanceWindow: mw-012345abcde</p>
     * <p>PatchBaseline: pb-012345abcde</p> <p>For the Document and Parameter values,
     * use the name of the resource.</p>  <p>The ManagedInstance type for this
     * API action is only for on-premises managed instances. Specify the name of the
     * managed instance in the following format: mi-ID_number. For example,
     * mi-1a2b3c4d5e6f.</p> 
     */
    inline const Aws::String& GetResourceId() const{ return m_resourceId; }

    /**
     * <p>The ID of the resource from which you want to remove tags. For example:</p>
     * <p>ManagedInstance: mi-012345abcde</p> <p>MaintenanceWindow: mw-012345abcde</p>
     * <p>PatchBaseline: pb-012345abcde</p> <p>For the Document and Parameter values,
     * use the name of the resource.</p>  <p>The ManagedInstance type for this
     * API action is only for on-premises managed instances. Specify the name of the
     * managed instance in the following format: mi-ID_number. For example,
     * mi-1a2b3c4d5e6f.</p> 
     */
    inline bool ResourceIdHasBeenSet() const { return m_resourceIdHasBeenSet; }

    /**
     * <p>The ID of the resource from which you want to remove tags. For example:</p>
     * <p>ManagedInstance: mi-012345abcde</p> <p>MaintenanceWindow: mw-012345abcde</p>
     * <p>PatchBaseline: pb-012345abcde</p> <p>For the Document and Parameter values,
     * use the name of the resource.</p>  <p>The ManagedInstance type for this
     * API action is only for on-premises managed instances. Specify the name of the
     * managed instance in the following format: mi-ID_number. For example,
     * mi-1a2b3c4d5e6f.</p> 
     */
    inline void SetResourceId(const Aws::String& value) { m_resourceIdHasBeenSet = true; m_resourceId = value; }

    /**
     * <p>The ID of the resource from which you want to remove tags. For example:</p>
     * <p>ManagedInstance: mi-012345abcde</p> <p>MaintenanceWindow: mw-012345abcde</p>
     * <p>PatchBaseline: pb-012345abcde</p> <p>For the Document and Parameter values,
     * use the name of the resource.</p>  <p>The ManagedInstance type for this
     * API action is only for on-premises managed instances. Specify the name of the
     * managed instance in the following format: mi-ID_number. For example,
     * mi-1a2b3c4d5e6f.</p> 
     */
    inline void SetResourceId(Aws::String&& value) { m_resourceIdHasBeenSet = true; m_resourceId = std::move(value); }

    /**
     * <p>The ID of the resource from which you want to remove tags. For example:</p>
     * <p>ManagedInstance: mi-012345abcde</p> <p>MaintenanceWindow: mw-012345abcde</p>
     * <p>PatchBaseline: pb-012345abcde</p> <p>For the Document and Parameter values,
     * use the name of the resource.</p>  <p>The ManagedInstance type for this
     * API action is only for on-premises managed instances. Specify the name of the
     * managed instance in the following format: mi-ID_number. For example,
     * mi-1a2b3c4d5e6f.</p> 
     */
    inline void SetResourceId(const char* value) { m_resourceIdHasBeenSet = true; m_resourceId.assign(value); }

    /**
     * <p>The ID of the resource from which you want to remove tags. For example:</p>
     * <p>ManagedInstance: mi-012345abcde</p> <p>MaintenanceWindow: mw-012345abcde</p>
     * <p>PatchBaseline: pb-012345abcde</p> <p>For the Document and Parameter values,
     * use the name of the resource.</p>  <p>The ManagedInstance type for this
     * API action is only for on-premises managed instances. Specify the name of the
     * managed instance in the following format: mi-ID_number. For example,
     * mi-1a2b3c4d5e6f.</p> 
     */
    inline RemoveTagsFromResourceRequest& WithResourceId(const Aws::String& value) { SetResourceId(value); return *this;}

    /**
     * <p>The ID of the resource from which you want to remove tags. For example:</p>
     * <p>ManagedInstance: mi-012345abcde</p> <p>MaintenanceWindow: mw-012345abcde</p>
     * <p>PatchBaseline: pb-012345abcde</p> <p>For the Document and Parameter values,
     * use the name of the resource.</p>  <p>The ManagedInstance type for this
     * API action is only for on-premises managed instances. Specify the name of the
     * managed instance in the following format: mi-ID_number. For example,
     * mi-1a2b3c4d5e6f.</p> 
     */
    inline RemoveTagsFromResourceRequest& WithResourceId(Aws::String&& value) { SetResourceId(std::move(value)); return *this;}

    /**
     * <p>The ID of the resource from which you want to remove tags. For example:</p>
     * <p>ManagedInstance: mi-012345abcde</p> <p>MaintenanceWindow: mw-012345abcde</p>
     * <p>PatchBaseline: pb-012345abcde</p> <p>For the Document and Parameter values,
     * use the name of the resource.</p>  <p>The ManagedInstance type for this
     * API action is only for on-premises managed instances. Specify the name of the
     * managed instance in the following format: mi-ID_number. For example,
     * mi-1a2b3c4d5e6f.</p> 
     */
    inline RemoveTagsFromResourceRequest& WithResourceId(const char* value) { SetResourceId(value); return *this;}


    /**
     * <p>Tag keys that you want to remove from the specified resource.</p>
     */
    inline const Aws::Vector<Aws::String>& GetTagKeys() const{ return m_tagKeys; }

    /**
     * <p>Tag keys that you want to remove from the specified resource.</p>
     */
    inline bool TagKeysHasBeenSet() const { return m_tagKeysHasBeenSet; }

    /**
     * <p>Tag keys that you want to remove from the specified resource.</p>
     */
    inline void SetTagKeys(const Aws::Vector<Aws::String>& value) { m_tagKeysHasBeenSet = true; m_tagKeys = value; }

    /**
     * <p>Tag keys that you want to remove from the specified resource.</p>
     */
    inline void SetTagKeys(Aws::Vector<Aws::String>&& value) { m_tagKeysHasBeenSet = true; m_tagKeys = std::move(value); }

    /**
     * <p>Tag keys that you want to remove from the specified resource.</p>
     */
    inline RemoveTagsFromResourceRequest& WithTagKeys(const Aws::Vector<Aws::String>& value) { SetTagKeys(value); return *this;}

    /**
     * <p>Tag keys that you want to remove from the specified resource.</p>
     */
    inline RemoveTagsFromResourceRequest& WithTagKeys(Aws::Vector<Aws::String>&& value) { SetTagKeys(std::move(value)); return *this;}

    /**
     * <p>Tag keys that you want to remove from the specified resource.</p>
     */
    inline RemoveTagsFromResourceRequest& AddTagKeys(const Aws::String& value) { m_tagKeysHasBeenSet = true; m_tagKeys.push_back(value); return *this; }

    /**
     * <p>Tag keys that you want to remove from the specified resource.</p>
     */
    inline RemoveTagsFromResourceRequest& AddTagKeys(Aws::String&& value) { m_tagKeysHasBeenSet = true; m_tagKeys.push_back(std::move(value)); return *this; }

    /**
     * <p>Tag keys that you want to remove from the specified resource.</p>
     */
    inline RemoveTagsFromResourceRequest& AddTagKeys(const char* value) { m_tagKeysHasBeenSet = true; m_tagKeys.push_back(value); return *this; }

  private:

    ResourceTypeForTagging m_resourceType;
    bool m_resourceTypeHasBeenSet;

    Aws::String m_resourceId;
    bool m_resourceIdHasBeenSet;

    Aws::Vector<Aws::String> m_tagKeys;
    bool m_tagKeysHasBeenSet;
  };

} // namespace Model
} // namespace SSM
} // namespace Aws
