#include "aerial_autonomy/log/log.h"

#include <boost/filesystem.hpp>
#include <glog/logging.h>

#include <ctime>
#include <exception>

Log::~Log() {
  log_timer_.stop();
  writeStreams(); // Make sure all data is out of the stream buffers
}

void Log::configure(LogConfig config) {
  config_ = config;

  std::time_t t = std::time(nullptr);
  char time_str[100];
  if (std::strftime(time_str, sizeof(time_str), "_%y_%m_%d_%H_%M_%S",
                    std::localtime(&t))) {
    directory_ = config_.directory() + std::string(time_str);
    if (!boost::filesystem::exists(directory_)) {
      if (!boost::filesystem::create_directory(directory_)) {
        throw std::runtime_error("Could not create Log directory: " +
                                 directory_.string());
      }
    }
  } else {
    throw std::runtime_error("Log timestamp exceeds string size");
  }
  configureStreams(config_);
}

DataStream &Log::operator[](std::string id) {
  boost::recursive_mutex::scoped_lock(streams_mutex_);
  auto stream = streams_.find(id);
  if (stream == streams_.end()) {
    // \todo Matt Find a better way to deal with this...
    // Return a disabled stream if stream does not exist
    LOG(WARNING) << "DataStream with id \"" << id
                 << "\" does not exist! Returning disabled stream";
    std::string empty_name("empty");
    auto empty_stream = streams_.find(empty_name);
    if (empty_stream == streams_.end()) {
      DataStreamConfig config;
      config.set_stream_id(empty_name);
      config.set_log_data(false);
      addDataStream(config);
      empty_stream = streams_.find(empty_name);
    }
    return empty_stream->second;
  }
  return stream->second;
}

void Log::addDataStream(DataStreamConfig stream_config) {
  boost::recursive_mutex::scoped_lock(streams_mutex_);
  if (streams_.find(stream_config.stream_id()) != streams_.end()) {
    throw std::runtime_error("Stream ID not unique: " +
                             stream_config.stream_id());
  }
  streams_.emplace(
      stream_config.stream_id(),
      DataStream(directory_ / stream_config.stream_id(), stream_config));
}

void Log::configureStreams(LogConfig config) {
  boost::recursive_mutex::scoped_lock(streams_mutex_);
  log_timer_.stop(); // \todo Matt With locking, we should not have to stop the
                     // timer... but for some reason it does not write otherwise
  streams_.clear();  // streams are closed in destructor
  for (auto stream_config : config_.data_stream_configs()) {
    addDataStream(stream_config);
  }
  log_timer_.setDuration(std::chrono::milliseconds(config_.write_duration()));
  log_timer_.start();
}

void Log::writeStreams() {
  boost::recursive_mutex::scoped_lock(streams_mutex_);
  for (auto &stream : instance().streams_) {
    stream.second.write();
  }
}
