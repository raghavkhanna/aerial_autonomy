#pragma once
#include <aerial_autonomy/actions_guards/base_functors.h>
#include <aerial_autonomy/basic_events.h>
#include <aerial_autonomy/logic_states/base_state.h>
#include <aerial_autonomy/robot_systems/uav_system.h>
#include <aerial_autonomy/types/completed_event.h>
#include <glog/logging.h>
#include <parsernode/common.h>

/**
* @brief namespace for basic events such as takeoff, land etc
*/
using namespace basic_events;

/**
* @brief action to take when starting takeoff
*
* @tparam LogicStateMachineT Logic state machine used to process events
*/
template <class LogicStateMachineT>
struct TakeoffTransitionActionFunctor_
    : EventAgnosticActionFunctor<UAVSystem, LogicStateMachineT> {
  void run(UAVSystem &robot_system, LogicStateMachineT &) {
    robot_system.takeOff();
  }
};

/**
* @brief action to perform when aborting takeoff
*
* @tparam LogicStateMachineT Logic state machine used to process events
*/
template <class LogicStateMachineT>
struct TakeoffAbortActionFunctor_
    : EventAgnosticActionFunctor<UAVSystem, LogicStateMachineT> {
  void run(UAVSystem &robot_system, LogicStateMachineT &) {
    robot_system.land();
  }
};

/**
* @brief Guard to avoid accidental takeoff
*
* @tparam LogicStateMachineT Logic state machine used to process events
*/
template <class LogicStateMachineT>
struct TakeoffTransitionGuardFunctor_
    : EventAgnosticGuardFunctor<UAVSystem, LogicStateMachineT> {
  bool guard(UAVSystem &robot_system, LogicStateMachineT &) {
    parsernode::common::quaddata data = robot_system.getUAVData();
    bool result = false;
    // Check for voltage
    if (data.batterypercent >
        robot_system.getConfiguration().minimum_battery_percent()) {
      result = true;
    } else {
      LOG(INFO) << "Battery too low! " << data.batterypercent
                << "% Cannot takeoff";
    }
    return result;
  }
};

/**
* @brief Check when takeoff is complete, and ensure enough battery voltage
*
* @tparam LogicStateMachineT Logic state machine used to process events
*/
template <class LogicStateMachineT>
struct TakeoffInternalActionFunctor_
    : EventAgnosticActionFunctor<UAVSystem, LogicStateMachineT> {
  /**
  * @brief Function to check when takeoff is complete.
  * If battery is low while takeoff, trigger Land event
  *
  *  \todo add a parameter for height when to transition from takeoff to
  * hovering
  *
  * @param robot_system robot system to get sensor data
  * @param logic_state_machine logic state machine to trigger events
  */
  void run(UAVSystem &robot_system, LogicStateMachineT &logic_state_machine) {
    parsernode::common::quaddata data = robot_system.getUAVData();
    // If battery too low abort and goto landed mode
    if (data.batterypercent <
        robot_system.getConfiguration().minimum_battery_percent()) {
      logic_state_machine.process_event(Land());
    } else if (data.altitude >=
               // Transition to hovering state once reached high altitude
               robot_system.getConfiguration().minimum_takeoff_height()) {
      logic_state_machine.process_event(Completed());
    }
  }
};

/**
* @brief Taking off state that uses the internal action functor
*
* @tparam LogicStateMachineT Logic state machine used to process events
*/
template <class LogicStateMachineT>
using TakingOff_ = BaseState<UAVSystem, LogicStateMachineT,
                             TakeoffInternalActionFunctor_<LogicStateMachineT>>;
