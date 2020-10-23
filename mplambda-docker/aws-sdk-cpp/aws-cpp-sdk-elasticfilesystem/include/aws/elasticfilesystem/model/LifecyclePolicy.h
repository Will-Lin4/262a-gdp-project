﻿/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#pragma once
#include <aws/elasticfilesystem/EFS_EXPORTS.h>
#include <aws/elasticfilesystem/model/TransitionToIARules.h>
#include <utility>

namespace Aws
{
namespace Utils
{
namespace Json
{
  class JsonValue;
  class JsonView;
} // namespace Json
} // namespace Utils
namespace EFS
{
namespace Model
{

  /**
   * <p>Describes a policy used by EFS lifecycle management to transition files to
   * the Infrequent Access (IA) storage class.</p><p><h3>See Also:</h3>   <a
   * href="http://docs.aws.amazon.com/goto/WebAPI/elasticfilesystem-2015-02-01/LifecyclePolicy">AWS
   * API Reference</a></p>
   */
  class AWS_EFS_API LifecyclePolicy
  {
  public:
    LifecyclePolicy();
    LifecyclePolicy(Aws::Utils::Json::JsonView jsonValue);
    LifecyclePolicy& operator=(Aws::Utils::Json::JsonView jsonValue);
    Aws::Utils::Json::JsonValue Jsonize() const;


    /**
     * <p> A value that describes the period of time that a file is not accessed, after
     * which it transitions to the IA storage class. Metadata operations such as
     * listing the contents of a directory don't count as file access events.</p>
     */
    inline const TransitionToIARules& GetTransitionToIA() const{ return m_transitionToIA; }

    /**
     * <p> A value that describes the period of time that a file is not accessed, after
     * which it transitions to the IA storage class. Metadata operations such as
     * listing the contents of a directory don't count as file access events.</p>
     */
    inline bool TransitionToIAHasBeenSet() const { return m_transitionToIAHasBeenSet; }

    /**
     * <p> A value that describes the period of time that a file is not accessed, after
     * which it transitions to the IA storage class. Metadata operations such as
     * listing the contents of a directory don't count as file access events.</p>
     */
    inline void SetTransitionToIA(const TransitionToIARules& value) { m_transitionToIAHasBeenSet = true; m_transitionToIA = value; }

    /**
     * <p> A value that describes the period of time that a file is not accessed, after
     * which it transitions to the IA storage class. Metadata operations such as
     * listing the contents of a directory don't count as file access events.</p>
     */
    inline void SetTransitionToIA(TransitionToIARules&& value) { m_transitionToIAHasBeenSet = true; m_transitionToIA = std::move(value); }

    /**
     * <p> A value that describes the period of time that a file is not accessed, after
     * which it transitions to the IA storage class. Metadata operations such as
     * listing the contents of a directory don't count as file access events.</p>
     */
    inline LifecyclePolicy& WithTransitionToIA(const TransitionToIARules& value) { SetTransitionToIA(value); return *this;}

    /**
     * <p> A value that describes the period of time that a file is not accessed, after
     * which it transitions to the IA storage class. Metadata operations such as
     * listing the contents of a directory don't count as file access events.</p>
     */
    inline LifecyclePolicy& WithTransitionToIA(TransitionToIARules&& value) { SetTransitionToIA(std::move(value)); return *this;}

  private:

    TransitionToIARules m_transitionToIA;
    bool m_transitionToIAHasBeenSet;
  };

} // namespace Model
} // namespace EFS
} // namespace Aws
