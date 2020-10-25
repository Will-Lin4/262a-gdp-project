﻿/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#pragma once
#include <aws/mediaconvert/MediaConvert_EXPORTS.h>
#include <aws/mediaconvert/model/Mp4CslgAtom.h>
#include <aws/mediaconvert/model/Mp4FreeSpaceBox.h>
#include <aws/mediaconvert/model/Mp4MoovPlacement.h>
#include <aws/core/utils/memory/stl/AWSString.h>
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
namespace MediaConvert
{
namespace Model
{

  /**
   * Settings for MP4 container. You can create audio-only AAC outputs with this
   * container.<p><h3>See Also:</h3>   <a
   * href="http://docs.aws.amazon.com/goto/WebAPI/mediaconvert-2017-08-29/Mp4Settings">AWS
   * API Reference</a></p>
   */
  class AWS_MEDIACONVERT_API Mp4Settings
  {
  public:
    Mp4Settings();
    Mp4Settings(Aws::Utils::Json::JsonView jsonValue);
    Mp4Settings& operator=(Aws::Utils::Json::JsonView jsonValue);
    Aws::Utils::Json::JsonValue Jsonize() const;


    /**
     * When enabled, file composition times will start at zero, composition times in
     * the 'ctts' (composition time to sample) box for B-frames will be negative, and a
     * 'cslg' (composition shift least greatest) box will be included per 14496-1
     * amendment 1. This improves compatibility with Apple players and tools.
     */
    inline const Mp4CslgAtom& GetCslgAtom() const{ return m_cslgAtom; }

    /**
     * When enabled, file composition times will start at zero, composition times in
     * the 'ctts' (composition time to sample) box for B-frames will be negative, and a
     * 'cslg' (composition shift least greatest) box will be included per 14496-1
     * amendment 1. This improves compatibility with Apple players and tools.
     */
    inline bool CslgAtomHasBeenSet() const { return m_cslgAtomHasBeenSet; }

    /**
     * When enabled, file composition times will start at zero, composition times in
     * the 'ctts' (composition time to sample) box for B-frames will be negative, and a
     * 'cslg' (composition shift least greatest) box will be included per 14496-1
     * amendment 1. This improves compatibility with Apple players and tools.
     */
    inline void SetCslgAtom(const Mp4CslgAtom& value) { m_cslgAtomHasBeenSet = true; m_cslgAtom = value; }

    /**
     * When enabled, file composition times will start at zero, composition times in
     * the 'ctts' (composition time to sample) box for B-frames will be negative, and a
     * 'cslg' (composition shift least greatest) box will be included per 14496-1
     * amendment 1. This improves compatibility with Apple players and tools.
     */
    inline void SetCslgAtom(Mp4CslgAtom&& value) { m_cslgAtomHasBeenSet = true; m_cslgAtom = std::move(value); }

    /**
     * When enabled, file composition times will start at zero, composition times in
     * the 'ctts' (composition time to sample) box for B-frames will be negative, and a
     * 'cslg' (composition shift least greatest) box will be included per 14496-1
     * amendment 1. This improves compatibility with Apple players and tools.
     */
    inline Mp4Settings& WithCslgAtom(const Mp4CslgAtom& value) { SetCslgAtom(value); return *this;}

    /**
     * When enabled, file composition times will start at zero, composition times in
     * the 'ctts' (composition time to sample) box for B-frames will be negative, and a
     * 'cslg' (composition shift least greatest) box will be included per 14496-1
     * amendment 1. This improves compatibility with Apple players and tools.
     */
    inline Mp4Settings& WithCslgAtom(Mp4CslgAtom&& value) { SetCslgAtom(std::move(value)); return *this;}


    /**
     * Ignore this setting unless compliance to the CTTS box version specification
     * matters in your workflow. Specify a value of 1 to set your CTTS box version to 1
     * and make your output compliant with the specification. When you specify a value
     * of 1, you must also set CSLG atom (cslgAtom) to the value INCLUDE. Keep the
     * default value 0 to set your CTTS box version to 0. This can provide backward
     * compatibility for some players and packagers.
     */
    inline int GetCttsVersion() const{ return m_cttsVersion; }

    /**
     * Ignore this setting unless compliance to the CTTS box version specification
     * matters in your workflow. Specify a value of 1 to set your CTTS box version to 1
     * and make your output compliant with the specification. When you specify a value
     * of 1, you must also set CSLG atom (cslgAtom) to the value INCLUDE. Keep the
     * default value 0 to set your CTTS box version to 0. This can provide backward
     * compatibility for some players and packagers.
     */
    inline bool CttsVersionHasBeenSet() const { return m_cttsVersionHasBeenSet; }

    /**
     * Ignore this setting unless compliance to the CTTS box version specification
     * matters in your workflow. Specify a value of 1 to set your CTTS box version to 1
     * and make your output compliant with the specification. When you specify a value
     * of 1, you must also set CSLG atom (cslgAtom) to the value INCLUDE. Keep the
     * default value 0 to set your CTTS box version to 0. This can provide backward
     * compatibility for some players and packagers.
     */
    inline void SetCttsVersion(int value) { m_cttsVersionHasBeenSet = true; m_cttsVersion = value; }

    /**
     * Ignore this setting unless compliance to the CTTS box version specification
     * matters in your workflow. Specify a value of 1 to set your CTTS box version to 1
     * and make your output compliant with the specification. When you specify a value
     * of 1, you must also set CSLG atom (cslgAtom) to the value INCLUDE. Keep the
     * default value 0 to set your CTTS box version to 0. This can provide backward
     * compatibility for some players and packagers.
     */
    inline Mp4Settings& WithCttsVersion(int value) { SetCttsVersion(value); return *this;}


    /**
     * Inserts a free-space box immediately after the moov box.
     */
    inline const Mp4FreeSpaceBox& GetFreeSpaceBox() const{ return m_freeSpaceBox; }

    /**
     * Inserts a free-space box immediately after the moov box.
     */
    inline bool FreeSpaceBoxHasBeenSet() const { return m_freeSpaceBoxHasBeenSet; }

    /**
     * Inserts a free-space box immediately after the moov box.
     */
    inline void SetFreeSpaceBox(const Mp4FreeSpaceBox& value) { m_freeSpaceBoxHasBeenSet = true; m_freeSpaceBox = value; }

    /**
     * Inserts a free-space box immediately after the moov box.
     */
    inline void SetFreeSpaceBox(Mp4FreeSpaceBox&& value) { m_freeSpaceBoxHasBeenSet = true; m_freeSpaceBox = std::move(value); }

    /**
     * Inserts a free-space box immediately after the moov box.
     */
    inline Mp4Settings& WithFreeSpaceBox(const Mp4FreeSpaceBox& value) { SetFreeSpaceBox(value); return *this;}

    /**
     * Inserts a free-space box immediately after the moov box.
     */
    inline Mp4Settings& WithFreeSpaceBox(Mp4FreeSpaceBox&& value) { SetFreeSpaceBox(std::move(value)); return *this;}


    /**
     * If set to PROGRESSIVE_DOWNLOAD, the MOOV atom is relocated to the beginning of
     * the archive as required for progressive downloading. Otherwise it is placed
     * normally at the end.
     */
    inline const Mp4MoovPlacement& GetMoovPlacement() const{ return m_moovPlacement; }

    /**
     * If set to PROGRESSIVE_DOWNLOAD, the MOOV atom is relocated to the beginning of
     * the archive as required for progressive downloading. Otherwise it is placed
     * normally at the end.
     */
    inline bool MoovPlacementHasBeenSet() const { return m_moovPlacementHasBeenSet; }

    /**
     * If set to PROGRESSIVE_DOWNLOAD, the MOOV atom is relocated to the beginning of
     * the archive as required for progressive downloading. Otherwise it is placed
     * normally at the end.
     */
    inline void SetMoovPlacement(const Mp4MoovPlacement& value) { m_moovPlacementHasBeenSet = true; m_moovPlacement = value; }

    /**
     * If set to PROGRESSIVE_DOWNLOAD, the MOOV atom is relocated to the beginning of
     * the archive as required for progressive downloading. Otherwise it is placed
     * normally at the end.
     */
    inline void SetMoovPlacement(Mp4MoovPlacement&& value) { m_moovPlacementHasBeenSet = true; m_moovPlacement = std::move(value); }

    /**
     * If set to PROGRESSIVE_DOWNLOAD, the MOOV atom is relocated to the beginning of
     * the archive as required for progressive downloading. Otherwise it is placed
     * normally at the end.
     */
    inline Mp4Settings& WithMoovPlacement(const Mp4MoovPlacement& value) { SetMoovPlacement(value); return *this;}

    /**
     * If set to PROGRESSIVE_DOWNLOAD, the MOOV atom is relocated to the beginning of
     * the archive as required for progressive downloading. Otherwise it is placed
     * normally at the end.
     */
    inline Mp4Settings& WithMoovPlacement(Mp4MoovPlacement&& value) { SetMoovPlacement(std::move(value)); return *this;}


    /**
     * Overrides the "Major Brand" field in the output file. Usually not necessary to
     * specify.
     */
    inline const Aws::String& GetMp4MajorBrand() const{ return m_mp4MajorBrand; }

    /**
     * Overrides the "Major Brand" field in the output file. Usually not necessary to
     * specify.
     */
    inline bool Mp4MajorBrandHasBeenSet() const { return m_mp4MajorBrandHasBeenSet; }

    /**
     * Overrides the "Major Brand" field in the output file. Usually not necessary to
     * specify.
     */
    inline void SetMp4MajorBrand(const Aws::String& value) { m_mp4MajorBrandHasBeenSet = true; m_mp4MajorBrand = value; }

    /**
     * Overrides the "Major Brand" field in the output file. Usually not necessary to
     * specify.
     */
    inline void SetMp4MajorBrand(Aws::String&& value) { m_mp4MajorBrandHasBeenSet = true; m_mp4MajorBrand = std::move(value); }

    /**
     * Overrides the "Major Brand" field in the output file. Usually not necessary to
     * specify.
     */
    inline void SetMp4MajorBrand(const char* value) { m_mp4MajorBrandHasBeenSet = true; m_mp4MajorBrand.assign(value); }

    /**
     * Overrides the "Major Brand" field in the output file. Usually not necessary to
     * specify.
     */
    inline Mp4Settings& WithMp4MajorBrand(const Aws::String& value) { SetMp4MajorBrand(value); return *this;}

    /**
     * Overrides the "Major Brand" field in the output file. Usually not necessary to
     * specify.
     */
    inline Mp4Settings& WithMp4MajorBrand(Aws::String&& value) { SetMp4MajorBrand(std::move(value)); return *this;}

    /**
     * Overrides the "Major Brand" field in the output file. Usually not necessary to
     * specify.
     */
    inline Mp4Settings& WithMp4MajorBrand(const char* value) { SetMp4MajorBrand(value); return *this;}

  private:

    Mp4CslgAtom m_cslgAtom;
    bool m_cslgAtomHasBeenSet;

    int m_cttsVersion;
    bool m_cttsVersionHasBeenSet;

    Mp4FreeSpaceBox m_freeSpaceBox;
    bool m_freeSpaceBoxHasBeenSet;

    Mp4MoovPlacement m_moovPlacement;
    bool m_moovPlacementHasBeenSet;

    Aws::String m_mp4MajorBrand;
    bool m_mp4MajorBrandHasBeenSet;
  };

} // namespace Model
} // namespace MediaConvert
} // namespace Aws
