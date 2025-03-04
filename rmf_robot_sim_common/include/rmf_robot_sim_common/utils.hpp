/*
 * Copyright (C) 2020 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/

#ifndef SRC__RMF_PLUGINS__UTILS_HPP
#define SRC__RMF_PLUGINS__UTILS_HPP

#include <rclcpp/rclcpp.hpp>
#include <memory>
#include <Eigen/Geometry>

namespace rmf_plugins_utils {

/////////////////////////////////////////////////////////////////////////////////////////////
enum Simulator {Ignition, Gazebo};

// Holds an identifier referring to either an Ignition or Gazebo classic entity
// Contains either a uint64_t value or a std::string value depending on `sim_type`
// Enables functions to be written that generically operate on both Ignition or Gazebo entities
struct SimEntity
{
  Simulator sim_type;
  uint64_t entity; // If used for Ignition Gazebo
  std::string name; // If used for Gazebo classic

  SimEntity(uint64_t en)
  : sim_type(Ignition), entity(en)
  {
    name = "";
  }
  SimEntity(std::string nm)
  : sim_type(Gazebo), name(nm)
  {
    entity = 0;
  }

  const std::string& get_name() const
  {
    if (sim_type != Gazebo)
    {
      std::cerr << "SimEntity Ignition object does not hold a name."
                << std::endl;
    }
    return name;
  }

  uint64_t get_entity() const
  {
    if (sim_type != Ignition)
    {
      std::cerr << "SimEntity Gazebo object does not hold a uint64_t entity."
                << std::endl;
    }
    return entity;
  }
};

/////////////////////////////////////////////////////////////////////////////////////////////////

struct MotionParams
{
  double v_max = 0.2;
  double a_max = 0.1;
  double a_nom = 0.08;
  double dx_min = 0.01;
};

double compute_desired_rate_of_change(
  double _s_target,              // Displacement to destination
  double _v_actual,              // Current velocity
  double _speed_target_now,      // Target speed now while on route
  double _speed_target_dest,     // Target speed at destination
  const MotionParams& _motion_params,
  const double _dt);

rclcpp::Time simulation_now(double t);

void sanitize_node_name(std::string& node_name);

/////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename ResultMsgT>
std::shared_ptr<ResultMsgT> make_response(uint8_t status,
  const double sim_time,
  const std::string& request_guid,
  const std::string& guid)
{
  std::shared_ptr<ResultMsgT> response = std::make_shared<ResultMsgT>();
  response->time = simulation_now(sim_time);
  response->request_guid = request_guid;
  response->source_guid = guid;
  response->status = status;
  return response;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
// Version agnostic conversion functions between Ignition Math and Eigen. Removes need for Ignition
// Math dependencies in rmf_robot_sim_common

template<typename IgnQuatT>
inline void convert(const Eigen::Quaterniond& _q, IgnQuatT& quat)
{
  quat.W() = _q.w();
  quat.X() = _q.x();
  quat.Y() = _q.y();
  quat.Z() = _q.z();
}

template<typename IgnVec3T>
inline void convert(const Eigen::Vector3d& _v, IgnVec3T& vec)
{
  vec.X() = _v[0];
  vec.Y() = _v[1];
  vec.Z() = _v[2];
}

template<typename IgnVec3T>
inline Eigen::Vector3d convert_vec(const IgnVec3T& _v)
{
  return Eigen::Vector3d(_v[0], _v[1], _v[2]);
}

template<typename IgnQuatT>
inline Eigen::Quaterniond convert_quat(const IgnQuatT& _q)
{
  Eigen::Quaterniond quat;
  quat.w() = _q.W();
  quat.x() = _q.X();
  quat.y() = _q.Y();
  quat.z() = _q.Z();

  return quat;
}

template<typename IgnPoseT>
inline auto convert_to_pose(const Eigen::Isometry3d& _tf)
{
  IgnPoseT pose;
  convert(Eigen::Vector3d(_tf.translation()), pose.Pos());
  convert(Eigen::Quaterniond(_tf.linear()), pose.Rot());

  return pose;
}

template<typename IgnPoseT>
inline Eigen::Isometry3d convert_pose(const IgnPoseT& _pose)
{
  Eigen::Isometry3d tf = Eigen::Isometry3d::Identity();
  tf.translation() = convert_vec(_pose.Pos());
  tf.linear() = Eigen::Matrix3d(convert_quat(_pose.Rot()));

  return tf;
}

} // namespace rmf_plugins_utils

#endif // SRC__RMF_PLUGINS__UTILS_HPP
