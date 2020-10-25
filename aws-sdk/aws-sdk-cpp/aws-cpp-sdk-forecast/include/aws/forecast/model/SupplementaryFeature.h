﻿/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#pragma once
#include <aws/forecast/ForecastService_EXPORTS.h>
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
namespace ForecastService
{
namespace Model
{

  /**
   * <p>Describes a supplementary feature of a dataset group. This object is part of
   * the <a>InputDataConfig</a> object.</p> <p>The only supported feature is a
   * holiday calendar. If you use the calendar, all data in the datasets should
   * belong to the same country as the calendar. For the holiday calendar data, see
   * the <a href="http://jollyday.sourceforge.net/data.html">Jollyday</a> web
   * site.</p> <p>India and Korea's holidays are not included in the Jollyday
   * library, but both are supported by Amazon Forecast. Their holidays are:</p> <p>
   * <b>"IN" - INDIA</b> </p> <ul> <li> <p> <code>JANUARY 26 - REPUBLIC DAY</code>
   * </p> </li> <li> <p> <code>AUGUST 15 - INDEPENDENCE DAY</code> </p> </li> <li>
   * <p> <code>OCTOBER 2 GANDHI'S BIRTHDAY</code> </p> </li> </ul> <p> <b>"KR" -
   * KOREA</b> </p> <ul> <li> <p> <code>JANUARY 1 - NEW YEAR</code> </p> </li> <li>
   * <p> <code>MARCH 1 - INDEPENDENCE MOVEMENT DAY</code> </p> </li> <li> <p>
   * <code>MAY 5 - CHILDREN'S DAY</code> </p> </li> <li> <p> <code>JUNE 6 - MEMORIAL
   * DAY</code> </p> </li> <li> <p> <code>AUGUST 15 - LIBERATION DAY</code> </p>
   * </li> <li> <p> <code>OCTOBER 3 - NATIONAL FOUNDATION DAY</code> </p> </li> <li>
   * <p> <code>OCTOBER 9 - HANGEUL DAY</code> </p> </li> <li> <p> <code>DECEMBER 25 -
   * CHRISTMAS DAY</code> </p> </li> </ul><p><h3>See Also:</h3>   <a
   * href="http://docs.aws.amazon.com/goto/WebAPI/forecast-2018-06-26/SupplementaryFeature">AWS
   * API Reference</a></p>
   */
  class AWS_FORECASTSERVICE_API SupplementaryFeature
  {
  public:
    SupplementaryFeature();
    SupplementaryFeature(Aws::Utils::Json::JsonView jsonValue);
    SupplementaryFeature& operator=(Aws::Utils::Json::JsonView jsonValue);
    Aws::Utils::Json::JsonValue Jsonize() const;


    /**
     * <p>The name of the feature. This must be "holiday".</p>
     */
    inline const Aws::String& GetName() const{ return m_name; }

    /**
     * <p>The name of the feature. This must be "holiday".</p>
     */
    inline bool NameHasBeenSet() const { return m_nameHasBeenSet; }

    /**
     * <p>The name of the feature. This must be "holiday".</p>
     */
    inline void SetName(const Aws::String& value) { m_nameHasBeenSet = true; m_name = value; }

    /**
     * <p>The name of the feature. This must be "holiday".</p>
     */
    inline void SetName(Aws::String&& value) { m_nameHasBeenSet = true; m_name = std::move(value); }

    /**
     * <p>The name of the feature. This must be "holiday".</p>
     */
    inline void SetName(const char* value) { m_nameHasBeenSet = true; m_name.assign(value); }

    /**
     * <p>The name of the feature. This must be "holiday".</p>
     */
    inline SupplementaryFeature& WithName(const Aws::String& value) { SetName(value); return *this;}

    /**
     * <p>The name of the feature. This must be "holiday".</p>
     */
    inline SupplementaryFeature& WithName(Aws::String&& value) { SetName(std::move(value)); return *this;}

    /**
     * <p>The name of the feature. This must be "holiday".</p>
     */
    inline SupplementaryFeature& WithName(const char* value) { SetName(value); return *this;}


    /**
     * <p>One of the following 2 letter country codes:</p> <ul> <li> <p>"AR" -
     * ARGENTINA</p> </li> <li> <p>"AT" - AUSTRIA</p> </li> <li> <p>"AU" -
     * AUSTRALIA</p> </li> <li> <p>"BE" - BELGIUM</p> </li> <li> <p>"BR" - BRAZIL</p>
     * </li> <li> <p>"CA" - CANADA</p> </li> <li> <p>"CN" - CHINA</p> </li> <li>
     * <p>"CZ" - CZECH REPUBLIC</p> </li> <li> <p>"DK" - DENMARK</p> </li> <li> <p>"EC"
     * - ECUADOR</p> </li> <li> <p>"FI" - FINLAND</p> </li> <li> <p>"FR" - FRANCE</p>
     * </li> <li> <p>"DE" - GERMANY</p> </li> <li> <p>"HU" - HUNGARY</p> </li> <li>
     * <p>"IE" - IRELAND</p> </li> <li> <p>"IN" - INDIA</p> </li> <li> <p>"IT" -
     * ITALY</p> </li> <li> <p>"JP" - JAPAN</p> </li> <li> <p>"KR" - KOREA</p> </li>
     * <li> <p>"LU" - LUXEMBOURG</p> </li> <li> <p>"MX" - MEXICO</p> </li> <li> <p>"NL"
     * - NETHERLANDS</p> </li> <li> <p>"NO" - NORWAY</p> </li> <li> <p>"PL" -
     * POLAND</p> </li> <li> <p>"PT" - PORTUGAL</p> </li> <li> <p>"RU" - RUSSIA</p>
     * </li> <li> <p>"ZA" - SOUTH AFRICA</p> </li> <li> <p>"ES" - SPAIN</p> </li> <li>
     * <p>"SE" - SWEDEN</p> </li> <li> <p>"CH" - SWITZERLAND</p> </li> <li> <p>"US" -
     * UNITED STATES</p> </li> <li> <p>"UK" - UNITED KINGDOM</p> </li> </ul>
     */
    inline const Aws::String& GetValue() const{ return m_value; }

    /**
     * <p>One of the following 2 letter country codes:</p> <ul> <li> <p>"AR" -
     * ARGENTINA</p> </li> <li> <p>"AT" - AUSTRIA</p> </li> <li> <p>"AU" -
     * AUSTRALIA</p> </li> <li> <p>"BE" - BELGIUM</p> </li> <li> <p>"BR" - BRAZIL</p>
     * </li> <li> <p>"CA" - CANADA</p> </li> <li> <p>"CN" - CHINA</p> </li> <li>
     * <p>"CZ" - CZECH REPUBLIC</p> </li> <li> <p>"DK" - DENMARK</p> </li> <li> <p>"EC"
     * - ECUADOR</p> </li> <li> <p>"FI" - FINLAND</p> </li> <li> <p>"FR" - FRANCE</p>
     * </li> <li> <p>"DE" - GERMANY</p> </li> <li> <p>"HU" - HUNGARY</p> </li> <li>
     * <p>"IE" - IRELAND</p> </li> <li> <p>"IN" - INDIA</p> </li> <li> <p>"IT" -
     * ITALY</p> </li> <li> <p>"JP" - JAPAN</p> </li> <li> <p>"KR" - KOREA</p> </li>
     * <li> <p>"LU" - LUXEMBOURG</p> </li> <li> <p>"MX" - MEXICO</p> </li> <li> <p>"NL"
     * - NETHERLANDS</p> </li> <li> <p>"NO" - NORWAY</p> </li> <li> <p>"PL" -
     * POLAND</p> </li> <li> <p>"PT" - PORTUGAL</p> </li> <li> <p>"RU" - RUSSIA</p>
     * </li> <li> <p>"ZA" - SOUTH AFRICA</p> </li> <li> <p>"ES" - SPAIN</p> </li> <li>
     * <p>"SE" - SWEDEN</p> </li> <li> <p>"CH" - SWITZERLAND</p> </li> <li> <p>"US" -
     * UNITED STATES</p> </li> <li> <p>"UK" - UNITED KINGDOM</p> </li> </ul>
     */
    inline bool ValueHasBeenSet() const { return m_valueHasBeenSet; }

    /**
     * <p>One of the following 2 letter country codes:</p> <ul> <li> <p>"AR" -
     * ARGENTINA</p> </li> <li> <p>"AT" - AUSTRIA</p> </li> <li> <p>"AU" -
     * AUSTRALIA</p> </li> <li> <p>"BE" - BELGIUM</p> </li> <li> <p>"BR" - BRAZIL</p>
     * </li> <li> <p>"CA" - CANADA</p> </li> <li> <p>"CN" - CHINA</p> </li> <li>
     * <p>"CZ" - CZECH REPUBLIC</p> </li> <li> <p>"DK" - DENMARK</p> </li> <li> <p>"EC"
     * - ECUADOR</p> </li> <li> <p>"FI" - FINLAND</p> </li> <li> <p>"FR" - FRANCE</p>
     * </li> <li> <p>"DE" - GERMANY</p> </li> <li> <p>"HU" - HUNGARY</p> </li> <li>
     * <p>"IE" - IRELAND</p> </li> <li> <p>"IN" - INDIA</p> </li> <li> <p>"IT" -
     * ITALY</p> </li> <li> <p>"JP" - JAPAN</p> </li> <li> <p>"KR" - KOREA</p> </li>
     * <li> <p>"LU" - LUXEMBOURG</p> </li> <li> <p>"MX" - MEXICO</p> </li> <li> <p>"NL"
     * - NETHERLANDS</p> </li> <li> <p>"NO" - NORWAY</p> </li> <li> <p>"PL" -
     * POLAND</p> </li> <li> <p>"PT" - PORTUGAL</p> </li> <li> <p>"RU" - RUSSIA</p>
     * </li> <li> <p>"ZA" - SOUTH AFRICA</p> </li> <li> <p>"ES" - SPAIN</p> </li> <li>
     * <p>"SE" - SWEDEN</p> </li> <li> <p>"CH" - SWITZERLAND</p> </li> <li> <p>"US" -
     * UNITED STATES</p> </li> <li> <p>"UK" - UNITED KINGDOM</p> </li> </ul>
     */
    inline void SetValue(const Aws::String& value) { m_valueHasBeenSet = true; m_value = value; }

    /**
     * <p>One of the following 2 letter country codes:</p> <ul> <li> <p>"AR" -
     * ARGENTINA</p> </li> <li> <p>"AT" - AUSTRIA</p> </li> <li> <p>"AU" -
     * AUSTRALIA</p> </li> <li> <p>"BE" - BELGIUM</p> </li> <li> <p>"BR" - BRAZIL</p>
     * </li> <li> <p>"CA" - CANADA</p> </li> <li> <p>"CN" - CHINA</p> </li> <li>
     * <p>"CZ" - CZECH REPUBLIC</p> </li> <li> <p>"DK" - DENMARK</p> </li> <li> <p>"EC"
     * - ECUADOR</p> </li> <li> <p>"FI" - FINLAND</p> </li> <li> <p>"FR" - FRANCE</p>
     * </li> <li> <p>"DE" - GERMANY</p> </li> <li> <p>"HU" - HUNGARY</p> </li> <li>
     * <p>"IE" - IRELAND</p> </li> <li> <p>"IN" - INDIA</p> </li> <li> <p>"IT" -
     * ITALY</p> </li> <li> <p>"JP" - JAPAN</p> </li> <li> <p>"KR" - KOREA</p> </li>
     * <li> <p>"LU" - LUXEMBOURG</p> </li> <li> <p>"MX" - MEXICO</p> </li> <li> <p>"NL"
     * - NETHERLANDS</p> </li> <li> <p>"NO" - NORWAY</p> </li> <li> <p>"PL" -
     * POLAND</p> </li> <li> <p>"PT" - PORTUGAL</p> </li> <li> <p>"RU" - RUSSIA</p>
     * </li> <li> <p>"ZA" - SOUTH AFRICA</p> </li> <li> <p>"ES" - SPAIN</p> </li> <li>
     * <p>"SE" - SWEDEN</p> </li> <li> <p>"CH" - SWITZERLAND</p> </li> <li> <p>"US" -
     * UNITED STATES</p> </li> <li> <p>"UK" - UNITED KINGDOM</p> </li> </ul>
     */
    inline void SetValue(Aws::String&& value) { m_valueHasBeenSet = true; m_value = std::move(value); }

    /**
     * <p>One of the following 2 letter country codes:</p> <ul> <li> <p>"AR" -
     * ARGENTINA</p> </li> <li> <p>"AT" - AUSTRIA</p> </li> <li> <p>"AU" -
     * AUSTRALIA</p> </li> <li> <p>"BE" - BELGIUM</p> </li> <li> <p>"BR" - BRAZIL</p>
     * </li> <li> <p>"CA" - CANADA</p> </li> <li> <p>"CN" - CHINA</p> </li> <li>
     * <p>"CZ" - CZECH REPUBLIC</p> </li> <li> <p>"DK" - DENMARK</p> </li> <li> <p>"EC"
     * - ECUADOR</p> </li> <li> <p>"FI" - FINLAND</p> </li> <li> <p>"FR" - FRANCE</p>
     * </li> <li> <p>"DE" - GERMANY</p> </li> <li> <p>"HU" - HUNGARY</p> </li> <li>
     * <p>"IE" - IRELAND</p> </li> <li> <p>"IN" - INDIA</p> </li> <li> <p>"IT" -
     * ITALY</p> </li> <li> <p>"JP" - JAPAN</p> </li> <li> <p>"KR" - KOREA</p> </li>
     * <li> <p>"LU" - LUXEMBOURG</p> </li> <li> <p>"MX" - MEXICO</p> </li> <li> <p>"NL"
     * - NETHERLANDS</p> </li> <li> <p>"NO" - NORWAY</p> </li> <li> <p>"PL" -
     * POLAND</p> </li> <li> <p>"PT" - PORTUGAL</p> </li> <li> <p>"RU" - RUSSIA</p>
     * </li> <li> <p>"ZA" - SOUTH AFRICA</p> </li> <li> <p>"ES" - SPAIN</p> </li> <li>
     * <p>"SE" - SWEDEN</p> </li> <li> <p>"CH" - SWITZERLAND</p> </li> <li> <p>"US" -
     * UNITED STATES</p> </li> <li> <p>"UK" - UNITED KINGDOM</p> </li> </ul>
     */
    inline void SetValue(const char* value) { m_valueHasBeenSet = true; m_value.assign(value); }

    /**
     * <p>One of the following 2 letter country codes:</p> <ul> <li> <p>"AR" -
     * ARGENTINA</p> </li> <li> <p>"AT" - AUSTRIA</p> </li> <li> <p>"AU" -
     * AUSTRALIA</p> </li> <li> <p>"BE" - BELGIUM</p> </li> <li> <p>"BR" - BRAZIL</p>
     * </li> <li> <p>"CA" - CANADA</p> </li> <li> <p>"CN" - CHINA</p> </li> <li>
     * <p>"CZ" - CZECH REPUBLIC</p> </li> <li> <p>"DK" - DENMARK</p> </li> <li> <p>"EC"
     * - ECUADOR</p> </li> <li> <p>"FI" - FINLAND</p> </li> <li> <p>"FR" - FRANCE</p>
     * </li> <li> <p>"DE" - GERMANY</p> </li> <li> <p>"HU" - HUNGARY</p> </li> <li>
     * <p>"IE" - IRELAND</p> </li> <li> <p>"IN" - INDIA</p> </li> <li> <p>"IT" -
     * ITALY</p> </li> <li> <p>"JP" - JAPAN</p> </li> <li> <p>"KR" - KOREA</p> </li>
     * <li> <p>"LU" - LUXEMBOURG</p> </li> <li> <p>"MX" - MEXICO</p> </li> <li> <p>"NL"
     * - NETHERLANDS</p> </li> <li> <p>"NO" - NORWAY</p> </li> <li> <p>"PL" -
     * POLAND</p> </li> <li> <p>"PT" - PORTUGAL</p> </li> <li> <p>"RU" - RUSSIA</p>
     * </li> <li> <p>"ZA" - SOUTH AFRICA</p> </li> <li> <p>"ES" - SPAIN</p> </li> <li>
     * <p>"SE" - SWEDEN</p> </li> <li> <p>"CH" - SWITZERLAND</p> </li> <li> <p>"US" -
     * UNITED STATES</p> </li> <li> <p>"UK" - UNITED KINGDOM</p> </li> </ul>
     */
    inline SupplementaryFeature& WithValue(const Aws::String& value) { SetValue(value); return *this;}

    /**
     * <p>One of the following 2 letter country codes:</p> <ul> <li> <p>"AR" -
     * ARGENTINA</p> </li> <li> <p>"AT" - AUSTRIA</p> </li> <li> <p>"AU" -
     * AUSTRALIA</p> </li> <li> <p>"BE" - BELGIUM</p> </li> <li> <p>"BR" - BRAZIL</p>
     * </li> <li> <p>"CA" - CANADA</p> </li> <li> <p>"CN" - CHINA</p> </li> <li>
     * <p>"CZ" - CZECH REPUBLIC</p> </li> <li> <p>"DK" - DENMARK</p> </li> <li> <p>"EC"
     * - ECUADOR</p> </li> <li> <p>"FI" - FINLAND</p> </li> <li> <p>"FR" - FRANCE</p>
     * </li> <li> <p>"DE" - GERMANY</p> </li> <li> <p>"HU" - HUNGARY</p> </li> <li>
     * <p>"IE" - IRELAND</p> </li> <li> <p>"IN" - INDIA</p> </li> <li> <p>"IT" -
     * ITALY</p> </li> <li> <p>"JP" - JAPAN</p> </li> <li> <p>"KR" - KOREA</p> </li>
     * <li> <p>"LU" - LUXEMBOURG</p> </li> <li> <p>"MX" - MEXICO</p> </li> <li> <p>"NL"
     * - NETHERLANDS</p> </li> <li> <p>"NO" - NORWAY</p> </li> <li> <p>"PL" -
     * POLAND</p> </li> <li> <p>"PT" - PORTUGAL</p> </li> <li> <p>"RU" - RUSSIA</p>
     * </li> <li> <p>"ZA" - SOUTH AFRICA</p> </li> <li> <p>"ES" - SPAIN</p> </li> <li>
     * <p>"SE" - SWEDEN</p> </li> <li> <p>"CH" - SWITZERLAND</p> </li> <li> <p>"US" -
     * UNITED STATES</p> </li> <li> <p>"UK" - UNITED KINGDOM</p> </li> </ul>
     */
    inline SupplementaryFeature& WithValue(Aws::String&& value) { SetValue(std::move(value)); return *this;}

    /**
     * <p>One of the following 2 letter country codes:</p> <ul> <li> <p>"AR" -
     * ARGENTINA</p> </li> <li> <p>"AT" - AUSTRIA</p> </li> <li> <p>"AU" -
     * AUSTRALIA</p> </li> <li> <p>"BE" - BELGIUM</p> </li> <li> <p>"BR" - BRAZIL</p>
     * </li> <li> <p>"CA" - CANADA</p> </li> <li> <p>"CN" - CHINA</p> </li> <li>
     * <p>"CZ" - CZECH REPUBLIC</p> </li> <li> <p>"DK" - DENMARK</p> </li> <li> <p>"EC"
     * - ECUADOR</p> </li> <li> <p>"FI" - FINLAND</p> </li> <li> <p>"FR" - FRANCE</p>
     * </li> <li> <p>"DE" - GERMANY</p> </li> <li> <p>"HU" - HUNGARY</p> </li> <li>
     * <p>"IE" - IRELAND</p> </li> <li> <p>"IN" - INDIA</p> </li> <li> <p>"IT" -
     * ITALY</p> </li> <li> <p>"JP" - JAPAN</p> </li> <li> <p>"KR" - KOREA</p> </li>
     * <li> <p>"LU" - LUXEMBOURG</p> </li> <li> <p>"MX" - MEXICO</p> </li> <li> <p>"NL"
     * - NETHERLANDS</p> </li> <li> <p>"NO" - NORWAY</p> </li> <li> <p>"PL" -
     * POLAND</p> </li> <li> <p>"PT" - PORTUGAL</p> </li> <li> <p>"RU" - RUSSIA</p>
     * </li> <li> <p>"ZA" - SOUTH AFRICA</p> </li> <li> <p>"ES" - SPAIN</p> </li> <li>
     * <p>"SE" - SWEDEN</p> </li> <li> <p>"CH" - SWITZERLAND</p> </li> <li> <p>"US" -
     * UNITED STATES</p> </li> <li> <p>"UK" - UNITED KINGDOM</p> </li> </ul>
     */
    inline SupplementaryFeature& WithValue(const char* value) { SetValue(value); return *this;}

  private:

    Aws::String m_name;
    bool m_nameHasBeenSet;

    Aws::String m_value;
    bool m_valueHasBeenSet;
  };

} // namespace Model
} // namespace ForecastService
} // namespace Aws
