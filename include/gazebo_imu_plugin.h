/*
 * Copyright 2015 Fadri Furrer, ASL, ETH Zurich, Switzerland
 * Copyright 2015 Michael Burri, ASL, ETH Zurich, Switzerland
 * Copyright 2015 Mina Kamel, ASL, ETH Zurich, Switzerland
 * Copyright 2015 Janosch Nikolic, ASL, ETH Zurich, Switzerland
 * Copyright 2015 Markus Achtelik, ASL, ETH Zurich, Switzerland
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0

 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <random>

#include <Eigen/Core>
#include "Imu.pb.h"
#include <gazebo/common/common.hh>
#include <gazebo/common/Plugin.hh>
#include <gazebo/gazebo.hh>
#include <gazebo/physics/physics.hh>
#include <ignition/math.hh>
#include "gazebo/transport/transport.hh"
#include "gazebo/msgs/msgs.hh"

#include "common.h"

namespace gazebo {
//typedef const boost::shared_ptr<const sensor_msgs::msgs::Imu> ImuPtr;

// Default values for use with IIM42653 IMU
static constexpr double kDefaultGyroscopeNoiseDensity =
    0.0008726646;  // [rad/s/sqrt(Hz)] (0.05 deg/s converted to rad/s)
static constexpr double kDefaultGyroscopeRandomWalk =
    0.0;  // [rad/s/s/sqrt(Hz)]
static constexpr double kDefaultGyroscopeBiasCorrelationTime =
    1000.0;  // [s]
static constexpr double kDefaultGyroscopeTurnOnBiasSigma =
    0.0;  // [rad/s]
static constexpr double kDefaultAccelerometerNoiseDensity =
    0.00637;  // [m/s^2/sqrt(Hz)] (0.65 mg-rms converted to m/s^2)
static constexpr double kDefaultAccelerometerRandomWalk =
    0.0;  // [m/s^2/s/sqrt(Hz)]
static constexpr double kDefaultAccelerometerBiasCorrelationTime =
    300.0;  // [s]
static constexpr double kDefaultAccelerometerTurnOnBiasSigma =
    0.0;  // [m/s^2]
// Earth's gravity in Zurich (lat=+47.3667degN, lon=+8.5500degE, h=+500m, WGS84)
static constexpr double kDefaultGravityMagnitude = 9.8068;

static const std::string kDefaultImuTopic = "imu";

// A description of the parameters:
// https://github.com/ethz-asl/kalibr/wiki/IMU-Noise-Model-and-Intrinsics
// TODO(burrimi): Should I have a minimalistic description of the params here?
struct ImuParameters {
  /// Gyroscope noise density (two-sided spectrum) [rad/s/sqrt(Hz)]
  double gyroscope_noise_density;
  /// Gyroscope bias random walk [rad/s/s/sqrt(Hz)]
  double gyroscope_random_walk;
  /// Gyroscope bias correlation time constant [s]
  double gyroscope_bias_correlation_time;
  /// Gyroscope turn on bias standard deviation [rad/s]
  double gyroscope_turn_on_bias_sigma;
  /// Accelerometer noise density (two-sided spectrum) [m/s^2/sqrt(Hz)]
  double accelerometer_noise_density;
  /// Accelerometer bias random walk. [m/s^2/s/sqrt(Hz)]
  double accelerometer_random_walk;
  /// Accelerometer bias correlation time constant [s]
  double accelerometer_bias_correlation_time;
  /// Accelerometer turn on bias standard deviation [m/s^2]
  double accelerometer_turn_on_bias_sigma;
  /// Norm of the gravitational acceleration [m/s^2]
  double gravity_magnitude;

  ImuParameters()
      : gyroscope_noise_density(kDefaultGyroscopeNoiseDensity),
        gyroscope_random_walk(kDefaultGyroscopeRandomWalk),
        gyroscope_bias_correlation_time(
            kDefaultGyroscopeBiasCorrelationTime),
        gyroscope_turn_on_bias_sigma(kDefaultGyroscopeTurnOnBiasSigma),
        accelerometer_noise_density(kDefaultAccelerometerNoiseDensity),
        accelerometer_random_walk(kDefaultAccelerometerRandomWalk),
        accelerometer_bias_correlation_time(
            kDefaultAccelerometerBiasCorrelationTime),
        accelerometer_turn_on_bias_sigma(
            kDefaultAccelerometerTurnOnBiasSigma),
        gravity_magnitude(kDefaultGravityMagnitude) {}
};

class GazeboImuPlugin : public ModelPlugin {
 public:

  GazeboImuPlugin();
  ~GazeboImuPlugin();

  void InitializeParams();
  void Publish();

 protected:
  void Load(physics::ModelPtr _model, sdf::ElementPtr _sdf);

  void addNoise(
      Eigen::Vector3d* linear_acceleration,
      Eigen::Vector3d* angular_velocity,
      const double dt);

  void OnUpdate(const common::UpdateInfo&);

 private:
  std::string namespace_;
  std::string imu_topic_;
  transport::NodePtr node_handle_;
  transport::PublisherPtr imu_pub_;
  std::string frame_id_;
  std::string link_name_;

  std::default_random_engine random_generator_;
  std::normal_distribution<double> standard_normal_distribution_;

  // Pointer to the world
  physics::WorldPtr world_;
  // Pointer to the model
  physics::ModelPtr model_;
  // Pointer to the link
  physics::LinkPtr link_;
  // Pointer to the update event connection
  event::ConnectionPtr updateConnection_;

  common::Time last_time_;

  sensor_msgs::msgs::Imu imu_message_;

  ignition::math::Vector3d gravity_W_;
  ignition::math::Vector3d velocity_prev_W_;

  Eigen::Vector3d gyroscope_bias_;
  Eigen::Vector3d accelerometer_bias_;

  ImuParameters imu_parameters_;

  uint64_t seq_ = 0;
};
}
