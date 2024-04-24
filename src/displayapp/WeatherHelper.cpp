/*  Copyright (C) 2024 Caleb Fontenot

    This file is part of InfiniTime.

    InfiniTime is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published
    by the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    InfiniTime is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "WeatherHelper.h"
#include <FreeRTOS.h>
#include <tuple>
#include <vector>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <nrfx_log.h>
using namespace Pinetime::Applications;
    //Linear gradient temperature color calculator :)

    int16_t WeatherHelper::RoundTemperature(int16_t temp) {
      return temp = temp / 100 + (temp % 100 >= 50 ? 1 : 0);
    }
  
    const char* floatToRgbHex(std::tuple<float, float, float> rgb) {
      char *rgbHex = new char[7];
      snprintf(rgbHex, 7, "%02X%02X%02X", static_cast<int>(std::get<0>(rgb)), static_cast<int>(std::get<1>(rgb)), static_cast<int>(std::get<2>(rgb)));
      return rgbHex;
    }
  
    std::tuple<float, float, float> hexToFloat(int rgb) {
      float r = ((rgb >> 16) & 0xFF);
      float g = ((rgb >> 8) & 0xFF);
      float b = (rgb & 0xFF);
      return std::tuple<float, float, float>(r, g, b);
    }
    
    float normalize(float value) {
    if (value < 0.0f) {
        return 0.0f;
    } else if (value > 1.0f) {
        return 1.0f;
    } else {
        return value;
    }
}
    
    // reference: https://dev.to/ndesmic/linear-color-gradients-from-scratch-1a0e
    std::tuple<float, float, float> lerp(std::tuple<float, float, float> pointA, std::tuple<float, float, float> pointB, float normalValue) {
      NRF_LOG_INFO("Normal value: %f", normalValue);
      auto lerpOutput = std::tuple<float, float, float>(
        get<0>(pointA) + (get<0>(pointB) - get<0>(pointA)) * normalValue,
        get<1>(pointA) + (get<1>(pointB) - get<1>(pointA)) * normalValue,
        get<2>(pointA) + (get<2>(pointB) - get<2>(pointA)) * normalValue
        //std::lerp(get<0>(pointA), get<0>(pointB), normalValue),
        //std::lerp(get<1>(pointA), get<1>(pointB), normalValue),
        //std::lerp(get<2>(pointA), get<2>(pointB), normalValue)
        );
      NRF_LOG_INFO("pointA: %f, %f, %f", get<0>(pointA), get<1>(pointA), get<2>(pointA));
      NRF_LOG_INFO("pointB: %f, %f, %f", get<0>(pointB), get<1>(pointB), get<2>(pointB));
      NRF_LOG_INFO("lerp: %f, %f, %f", get<0>(lerpOutput), get<1>(lerpOutput), get<2>(lerpOutput));
      return lerpOutput;
    }
    
    const char* WeatherHelper::TemperatureColor(int16_t temperature) {
      const std::vector<int> colors = {0x5555ff, 0x00c9ff, 0xff9b00, 0xff0000};
      std::vector<std::tuple<float, float, float>> stops;
      for (auto colorVal: colors) {
       stops.emplace_back(hexToFloat(colorVal));
      }
      int tempRounded = RoundTemperature(temperature);
      if (tempRounded < 0) {
         tempRounded = 1;
      }
      // convert temperature to range between newMin and newMax
      float oldMax = 50;
      float oldMin = 0;
      float newMax = 1;
      float newMin = 0;
      float oldRange = (oldMax - oldMin);
      float newRange = (newMax - newMin);
      float newValue = (((tempRounded - oldMin) * newRange) / oldRange) + newMin;
      newValue = normalize(newValue);
      if (newValue <= .33f) {
        return floatToRgbHex(lerp(stops[0], stops[1], newValue));
      } else if (newValue <= .66f) {
        return floatToRgbHex(lerp(stops[1], stops[2], newValue));
      } else {
        return floatToRgbHex(lerp(stops[2], stops[3], newValue));
      }
    }
