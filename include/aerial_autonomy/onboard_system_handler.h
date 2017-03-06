#pragma once

#include <ros/ros.h>

#include <parsernode/parser.h>
#include <pluginlib/class_loader.h>

#include "onboard_system_handler_config.pb.h"
#include <aerial_autonomy/actions_guards/base_functors.h>
#include <aerial_autonomy/common/async_timer.h>
#include <aerial_autonomy/robot_systems/uav_system.h>
#include <aerial_autonomy/state_machines/state_machine_gui_connector.h>

/**
 * @brief Owns all of the autonomous system components and is responsible for
 * thread management.
 * @tparam LogicStateMachineT Logic state machine to use
 * @tparam EventManagerT Event manager to use
 */
template <class LogicStateMachineT, class EventManagerT>
class OnboardSystemHandler {
public:
  /**
   * @brief Temporary constructor which takes a parser directly
   * @param nh NodeHandle to use for event and command subscription
   * @param uav_hardware Hardware instance to use
   * TODO(matt): Remove this once we have a suitable test parser that can be
   * loaded as a plugin
   */
  OnboardSystemHandler(ros::NodeHandle &nh, parsernode::Parser *uav_hardware)
      : OnboardSystemHandler(nh) {
    uav_hardware_.reset(uav_hardware);
    initialize();
  }

  /**
   * @brief Constructor
   * @param nh NodeHandle to use for event and command subscription
   * @param config Proto configuration parameters
   */
  OnboardSystemHandler(ros::NodeHandle &nh, OnboardSystemHandlerConfig &config)
      : OnboardSystemHandler(nh) {
    // Load configured uav parser
    config_ = config;
    loadUAVPlugin(nh_, config_.uav_parser_type());
    initialize();
  }

  OnboardSystemHandler(const OnboardSystemHandler &) = delete;

  /**
   * @brief Checks if internal ROS topics are connected
   * @return Returns true if connected and false otherwise
   */
  bool isConnected() {
    if (state_machine_gui_connector_) {
      return state_machine_gui_connector_->isEventManagerConnected() &&
             state_machine_gui_connector_->isPoseCommandConnected();
    } else {
      return false;
    }
  }

private:
  /**
   * @brief Base constructor to be used in constructor delegation only
   * @param nh NodeHandle to use for event and command subscription
   */
  OnboardSystemHandler(ros::NodeHandle &nh)
      : nh_(nh), logic_state_machine_timer_(
                     std::bind(&OnboardSystemHandler::stateMachineThread, this),
                     std::chrono::milliseconds(20)),
        uav_controller_timer_(
            std::bind(&OnboardSystemHandler::uavControllerThread, this),
            std::chrono::milliseconds(20)) {}
  /**
   * @brief Initializes all member variables and starts threads
   */
  void initialize() {
    // Instantiate members
    uav_system_.reset(new UAVSystem(*uav_hardware_));
    logic_state_machine_.reset(
        new LogicStateMachineT(boost::ref(*uav_system_)));
    event_manager_.reset(new EventManagerT());
    state_machine_gui_connector_.reset(
        new StateMachineGUIConnector<EventManagerT, LogicStateMachineT>(
            nh_, boost::ref(*event_manager_),
            boost::ref(*logic_state_machine_)));

    // Get the party started
    logic_state_machine_->start();
    logic_state_machine_timer_.start();
    uav_controller_timer_.start();
  }
  /**
   * @brief Load the UAV plugin with the given name
   * @param nh NodeHandle for the plugin to use for ROS communication
   * @param plugin_name Name of plugin to load
   */
  bool loadUAVPlugin(ros::NodeHandle &nh, std::string plugin_name) {
    // TODO(matt): Maybe move this functionality to its own class
    if (!parser_loader)
      parser_loader.reset(new pluginlib::ClassLoader<parsernode::Parser>(
          "parsernode", "parsernode::Parser"));

    try {
      uav_hardware_.reset(parser_loader->createUnmanagedInstance(plugin_name));
    } catch (pluginlib::PluginlibException &ex) {
      std::cout << "The plugin failed to load: " << ex.what() << std::endl;
      return false;
    }
    // Wait till parser is initialized:
    uav_hardware_->initialize(nh);
    auto load_time = std::chrono::steady_clock::now();
    while (std::chrono::duration_cast<std::chrono::seconds>(
               std::chrono::steady_clock::now() - load_time)
               .count() < config_.uav_parser_load_timeout()) {
      if (uav_hardware_->initialized)
        break;
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return uav_hardware_->initialized;
  }
  /**
   * @brief State machine processing loop
   */
  void stateMachineThread() {
    logic_state_machine_->process_event(InternalTransitionEvent());
  }

  /**
   * @brief UAV control loop
   */
  void uavControllerThread() {
    uav_system_->runActiveController(HardwareType::UAV);
  }

  ros::NodeHandle nh_;
  std::unique_ptr<parsernode::Parser> uav_hardware_;
  std::unique_ptr<UAVSystem> uav_system_;
  std::unique_ptr<LogicStateMachineT> logic_state_machine_;
  std::unique_ptr<EventManagerT> event_manager_;
  std::unique_ptr<StateMachineGUIConnector<EventManagerT, LogicStateMachineT>>
      state_machine_gui_connector_;
  std::unique_ptr<pluginlib::ClassLoader<parsernode::Parser>> parser_loader;
  AsyncTimer logic_state_machine_timer_;
  AsyncTimer uav_controller_timer_;
  OnboardSystemHandlerConfig config_;
};
