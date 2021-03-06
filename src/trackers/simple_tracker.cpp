#include "aerial_autonomy/trackers/simple_tracker.h"
#include "aerial_autonomy/trackers/simple_tracking_strategy.h"
#include <glog/logging.h>

SimpleTracker::SimpleTracker(parsernode::Parser &drone_hardware,
                             tf::Transform camera_transform)
    : BaseTracker(new SimpleTrackingStrategy()),
      drone_hardware_(drone_hardware), tracking_valid_(true),
      target_pose_(tf::Quaternion(0, 0, 0, 1), tf::Vector3(0, 0, 0)),
      camera_transform_(camera_transform) {}

bool SimpleTracker::getTrackingVectors(
    std::unordered_map<uint32_t, tf::Transform> &p) {
  CHECK(p.empty()) << "Tracking vector map not empty";
  if (!trackingIsValid()) {
    return false;
  }
  parsernode::common::quaddata uav_data;
  drone_hardware_.getquaddata(uav_data);
  tf::Transform quad_tf_global(
      tf::createQuaternionFromRPY(uav_data.rpydata.x, uav_data.rpydata.y,
                                  uav_data.rpydata.z),
      tf::Vector3(uav_data.localpos.x, uav_data.localpos.y,
                  uav_data.localpos.z));
  tf::Transform target_pose_camera =
      camera_transform_.inverse() * quad_tf_global.inverse() * target_pose_;
  p[0] = target_pose_camera;
  return true;
}

void SimpleTracker::setTargetPoseGlobalFrame(tf::Transform p) {
  target_pose_ = p;
}

void SimpleTracker::setTargetPositionGlobalFrame(Position p) {
  target_pose_.setOrigin(tf::Vector3(p.x, p.y, p.z));
  target_pose_.setRotation(tf::Quaternion(0, 0, 0, 1));
}

bool SimpleTracker::trackingIsValid() { return tracking_valid_; }

void SimpleTracker::setTrackingIsValid(bool is_valid) {
  tracking_valid_ = is_valid;
}

tf::Transform SimpleTracker::cameraTransform() { return camera_transform_; }
