#include "aerial_autonomy/controllers/manual_velocity_controller.h"
#include "aerial_autonomy/common/math.h"

bool ManualVelocityController::runImplementation(std::tuple<JoysticksYaw, VelocityYaw> sensor_data,
  EmptyGoal goal,
  RollPitchYawThrust &control){

  VelocityYaw vel_goal;
  JoysticksYaw joy_sensor_data = std::get<0>(sensor_data);

  vel_goal.x = map(joy_sensor_data.channel1, -10000, 10000, -1, 1);
  vel_goal.y = map(joy_sensor_data.channel2, -10000, 10000, -1, 1);
  vel_goal.z = map(joy_sensor_data.channel3, -10000, 10000, -1, 1);

  double yaw_rate = map(joy_sensor_data.channel4, -10000, 10000, -M_PI, M_PI);

  // \todo soham get mappings and time step from config
  vel_goal.yaw = math::angleWrap(joy_sensor_data.yaw - yaw_rate * 0.03);
  
  VelocityYaw vel_sensor_data = std::get<1>(sensor_data);

  rpyt_vel_ctlr.setGoal(vel_goal);
  rpyt_vel_ctlr.run(vel_sensor_data, control);
  
  return true;
}

double ManualVelocityController::map(double input, double input_min,
 double input_max, double output_min,
 double output_max) {
  if (input > input_max)
    return output_max;
  else if (input < input_min)
    return output_min;
  return output_min +
  ((input - input_min) * (output_max - output_min)) /
  (input_max - input_min);
}