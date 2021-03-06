#pragma once
#include "aerial_autonomy/controller_hardware_connectors/base_controller_hardware_connector.h"
#include "aerial_autonomy/controllers/velocity_based_relative_pose_controller.h"
#include "aerial_autonomy/trackers/base_tracker.h"
#include "aerial_autonomy/types/velocity_yaw_rate.h"
#include "uav_vision_system_config.pb.h"

#include <parsernode/parser.h>

#include <tf/tf.h>

/**
 * @brief A visual servoing controller that uses a tracker output as feedback
 * and brings the quadrotor
 * to a goal pose expressed in the tracked object's coordinate frame
 */
class RelativePoseVisualServoingControllerDroneConnector
    : public ControllerHardwareConnector<
          std::tuple<tf::Transform, tf::Transform>, PositionYaw,
          VelocityYawRate> {
public:
  /**
   * @brief Constructor
   * @param tracker Tracker to connect to controller
   * @param drone_hardware UAV hardware to send controller commands t
   * @param controller Controller to connect to UAV hardware
   * @param camera_transform Camera transform in UAV frame
   * @param tracking_offset_transform Additional transform to apply to tracked
   * object before it is roll/pitch compensated
   */
  RelativePoseVisualServoingControllerDroneConnector(
      BaseTracker &tracker, parsernode::Parser &drone_hardware,
      VelocityBasedRelativePoseController &controller,
      tf::Transform camera_transform,
      tf::Transform tracking_offset_transform =
          tf::Transform(tf::Quaternion(0, 0, 0, 1), tf::Vector3(0, 0, 0)))
      : ControllerHardwareConnector(controller, HardwareType::UAV),
        drone_hardware_(drone_hardware), tracker_(tracker),
        camera_transform_(camera_transform),
        // \todo Matt This will become unwieldy when we are tracking multiple
        // objects, each with different offsets.  This assumes the offset is the
        // same for all tracked objects
        tracking_offset_transform_(tracking_offset_transform) {}
  /**
   * @brief Destructor
   */
  virtual ~RelativePoseVisualServoingControllerDroneConnector() {}

  /**
   * @brief Get the rotation-compensated tracking pose of the tracker in the
   * rotation-compensated
   * frame of the quadrotor
   * @param tracking_vector Returned tracking pose
   * @return True if successful and false otherwise
   */
  bool getTrackingTransformRotationCompensatedQuadFrame(
      tf::Transform &tracking_transform);

protected:
  /**
   * @brief Extracts pose data from tracker
   *
   * @param sensor_data Current transform of quadrotor in the
   * rotation-compensated frame of the quadrotor
   * and tracking transform in the rotation-compensated frame of the quadrotor
   *
   * @return true if able to compute transforms
   */
  virtual bool
  extractSensorData(std::tuple<tf::Transform, tf::Transform> &sensor_data);

  /**
   * @brief Send velocity commands to hardware
   *
   * @param controls velocity command to send to UAV
   */
  virtual void sendHardwareCommands(VelocityYawRate controls);

private:
  /**
   * @brief Get the rotation of the uav body frame
   * @return The rotation transform
   */
  tf::Transform getBodyFrameRotation();

  /**
  * @brief Quad hardware to send commands
  */
  parsernode::Parser &drone_hardware_;
  /**
  * @brief Tracks whatever we are servoing to
  */
  BaseTracker &tracker_;
  /**
  * @brief camera transform with respect to body
  */
  tf::Transform camera_transform_;
  /**
  * @brief transform to apply to tracked object in its own frame before
  * roll/pitch compensation
  */
  tf::Transform tracking_offset_transform_;
};
