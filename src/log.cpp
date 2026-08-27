#include "simple_log.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <thread>

namespace {

// Intentionally never destroyed: it remains usable by late static destructors.
// Logging after the standard streams themselves are destroyed is still unsupported.
std::mutex &output_mutex() {
  static auto *mutex = new std::mutex;
  return *mutex;
}

const char *level_to_string(LogLevel level) noexcept {
  switch (level) {
  case INFO:
    return "INFO";
  case DEBUG:
    return "DEBUG";
  case WARNING:
    return "WARNING";
  case ERROR:
    return "ERROR";
  }
  return "UNKNOWN";
}

std::string current_datetime_string() {
  const auto now = std::chrono::system_clock::now();
  const auto time = std::chrono::system_clock::to_time_t(now);
  std::tm local_time{};
#if defined(_MSC_VER)
  localtime_s(&local_time, &time);
#else
  localtime_r(&time, &local_time);
#endif

  const auto milliseconds =
      std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
  std::ostringstream result;
  result << std::put_time(&local_time, "%Y-%m-%d %H:%M:%S") << '.'
         << std::setw(3) << std::setfill('0') << milliseconds.count();
  return result.str();
}

} // namespace

Logger::Logger(std::string prefix) : prefix_(std::move(prefix)), output_(&std::cout) {}

Logger::Logger(std::string prefix, std::ostream &output)
    : prefix_(std::move(prefix)), output_(&output) {}

LogEntry Logger::log(LogLevel level) const { return LogEntry(prefix_, level, *output_); }

LogEntry Logger::operator()(LogLevel level) const { return log(level); }

Logger getLogger() { return Logger{}; }

Logger getLogger(std::string prefix) { return Logger{std::move(prefix)}; }

LogEntry::LogEntry(std::string prefix, LogLevel level, std::ostream &output)
    : prefix_(std::move(prefix)), level_(level), output_(&output) {}

LogEntry::LogEntry(LogEntry &&other) noexcept
    : prefix_(std::move(other.prefix_)), level_(other.level_), output_(other.output_),
      message_(std::move(other.message_)), active_(other.active_) {
  other.active_ = false;
}

LogEntry::~LogEntry() noexcept {
  if (!active_) {
    return;
  }

  // A destructor must not propagate an iostream/allocation failure during stack unwinding.
  try {
    std::ostringstream line;
    line << current_datetime_string() << "; " << level_to_string(level_) << "; ";
    if (!prefix_.empty()) {
      line << prefix_;
    }
    line << '(' << std::hash<std::thread::id>{}(std::this_thread::get_id()) << "): "
         << message_.str();

    std::lock_guard<std::mutex> lock(output_mutex());
    *output_ << line.str() << '\n';
  } catch (...) {
    // Logging must not terminate the application. There is no safe reporting channel here.
  }
}

LogEntry &LogEntry::operator<<(Manip manip) {
  manip(message_);
  return *this;
}
