/*
 * ThresholdFilter.cpp
 *
 *  Created on: Mar 18, 2015
 *      Author: Martin Wermelinger
 *   Institute: ETH Zurich, Autonomous Systems Lab
 */

#include "filters/ThresholdFilter.hpp"
#include <pluginlib/class_list_macros.h>

// Grid Map
#include <grid_map/GridMap.hpp>

// Grid Map lib
#include <grid_map_lib/GridMap.hpp>
#include <grid_map_lib/iterators/GridMapIterator.hpp>

// Eigen
#include <Eigen/Core>
#include <Eigen/Dense>

namespace filters {

template<typename T>
ThresholdFilter<T>::ThresholdFilter()
  : useLowerThreshold_(false),
    useUpperThreshold_(false)
{

}

template<typename T>
ThresholdFilter<T>::~ThresholdFilter()
{

}

template<typename T>
bool ThresholdFilter<T>::configure()
{
  // Load Parameters
  if (FilterBase<T>::getParam(std::string("lower_threshold"), lowerThreshold_)) {
    useLowerThreshold_ = true;
    ROS_INFO("lower threshold = %f", lowerThreshold_);
  }

  if (FilterBase<T>::getParam(std::string("upper_threshold"), upperThreshold_)) {
    useUpperThreshold_ = true;
    ROS_INFO("upper threshold = %f", upperThreshold_);
  }

  if (!useLowerThreshold_ && !useUpperThreshold_) {
    ROS_ERROR("ThresholdFilter did not find param lower_threshold or upper_threshold");
    return false;
  }

  if (useLowerThreshold_ && useUpperThreshold_) {
    ROS_ERROR("Set either lower_threshold or upper_threshold! Only one threshold can be used!");
    return false;
  }

  if (!FilterBase<T>::getParam(std::string("set_to"), setTo_)) {
    ROS_ERROR("ThresholdFilter did not find param set_to");
    return false;
  }

  if (!FilterBase<T>::getParam(std::string("threshold_types"), thresholdTypes_)) {
    ROS_ERROR("ThresholdFilter did not find param threshold_types");
    return false;
  }

  return true;
}

template<typename T>
bool ThresholdFilter<T>::update(const T& mapIn, T& mapOut)
{
  mapOut = mapIn;

  for (int i = 0; i < thresholdTypes_.size(); i++) {
    // Check if layer exists.
    if (!mapOut.exists(thresholdTypes_.at(i))) {
      ROS_ERROR("Check your threshold types! Type %s does not exist",
                thresholdTypes_.at(i).c_str());
      continue;
    }

    std::vector<std::string> validTypes;
    validTypes.push_back(thresholdTypes_.at(i));

    for (grid_map_lib::GridMapIterator iterator(mapOut);
        !iterator.isPassedEnd(); ++iterator) {
      if (!mapOut.isValid(*iterator, validTypes))
        continue;

      double value = mapOut.at(thresholdTypes_.at(i), *iterator);
      if (useLowerThreshold_) {
        if (value < lowerThreshold_) value = setTo_;
      }
      if (useUpperThreshold_) {
        if (value > upperThreshold_) value = setTo_;
      }
      mapOut.at(thresholdTypes_.at(i), *iterator) = value;
    }

  }

  return true;
}

} /* namespace */

PLUGINLIB_REGISTER_CLASS(ThresholdFilter, filters::ThresholdFilter<grid_map::GridMap>, filters::FilterBase<grid_map::GridMap>)
