#pragma once
#include <aerial_autonomy/robot_systems/uav_system.h>
#include <aerial_autonomy/types/position_yaw.h>
#include <type_traits>

/**
* @brief Robot system that does not perform any actions
*/
struct EmptyRobotSystem {};

/**
* @brief Example Logic state machine that stores triggered event
*
* @tparam RobotSystemT The robot system that is used by states to perform
* actions
*/
template <class RobotSystemT> class SampleLogicStateMachine_ {
  /**
  * @brief Friend action class to guard robot system
  *
  * @tparam EventT  Event triggering action functor
  * @tparam RobotSystemT1 Robot system used by action functor
  * @tparam LogicStateMachineT Logic state Machine backend used by Guard functor
  */
  template <class EventT, class RobotSystemT1, class LogicStateMachineT>
  friend class ActionFunctor;
  /**
  * @brief Friend guard class to access robot system
  *
  * @tparam EventT  Event triggering action functor
  * @tparam RobotSystemT1 Robot system used by action functor
  * @tparam LogicStateMachineT Logic state Machine backend used by Guard functor
  */
  template <class EventT, class RobotSystemT1, class LogicStateMachineT>
  friend class GuardFunctor;
  /**
  * @brief Frient Event agnostic action class
  *
  * @tparam EventT  Event triggering action functor
  * @tparam RobotSystemT1 Robot system used by action functor
  * @tparam LogicStateMachineT Logic state Machine backend used by Guard functor
  */
  template <class RobotSystemT1, class LogicStateMachineT>
  friend class EventAgnosticActionFunctor;
  /**
  * @brief Frient Event agnostic guard class
  *
  * @tparam EventT  Event triggering action functor
  * @tparam RobotSystemT1 Robot system used by action functor
  * @tparam LogicStateMachineT Logic state Machine backend used by Guard functor
  */
  template <class RobotSystemT1, class LogicStateMachineT>
  friend class EventAgnosticGuardFunctor;
  /**
  * @brief type index of the event triggered by LSM
  */
  std::type_index type_index_event_ = typeid(NULL);
  /**
  * @brief Positionyaw message thats triggered
  */
  PositionYaw pose_event_;
  /**
  * @brief store the robot system passed in
  */
  RobotSystemT &robot_system_;

public:
  /**
  * @brief Constructor that takes robot system
  *
  * @param robot_system provides actions to select controllers, get/set Goals.
  */
  SampleLogicStateMachine_(RobotSystemT &robot_system)
      : robot_system_(robot_system) {}
  /**
  * @brief Generic process event that stores the event type
  *
  * @tparam Event Event type
  * @param event Event instance sent in to process
  */
  template <class Event> void process_event(const Event &event) {
    type_index_event_ = typeid(event);
  }

  /**
  * @brief process pose event message
  *
  * @param event message that is stored
  */
  void process_event(const PositionYaw &event) {
    type_index_event_ = typeid(event);
    pose_event_ = event;
  }
  /**
  * @brief retrieve the event type_index of last processed event
  *
  * @return  type_index of the last processed event
  */
  std::type_index getProcessEventTypeId() { return type_index_event_; }
  /**
  * @brief retrieve poseyaw event message
  *
  * @return poseyaw event message
  */
  PositionYaw getPoseEvent() { return pose_event_; }
};

/**
* @brief Sample logic state machine templated on empty robot system
*/
using SampleLogicStateMachine = SampleLogicStateMachine_<EmptyRobotSystem>;
/**
* @brief Sample logic state machine templated on UAV system
*/
using UAVLogicStateMachine = SampleLogicStateMachine_<UAVSystem>;