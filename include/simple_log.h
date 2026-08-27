#ifndef SIMPLE_LOG_H
#define SIMPLE_LOG_H

#include <ostream>
#include <sstream>
#include <string>
#include <utility>

// Kept as an unscoped enum for source compatibility with INFO, DEBUG, etc.
enum LogLevel { INFO, DEBUG, WARNING, ERROR };

class Logger;

// A move-only, RAII log entry. It writes its accumulated message when destroyed.
class LogEntry {
public:
  LogEntry(const LogEntry &) = delete;
  LogEntry &operator=(const LogEntry &) = delete;
  LogEntry(LogEntry &&other) noexcept;
  LogEntry &operator=(LogEntry &&) = delete;
  ~LogEntry() noexcept;

  template <typename T> LogEntry &operator<<(const T &value) {
    message_ << value;
    return *this;
  }

  using Manip = std::ostream &(*)(std::ostream &);
  LogEntry &operator<<(Manip manip);

private:
  friend class Logger;

  LogEntry(std::string prefix, LogLevel level, std::ostream &output);

  std::string prefix_;
  LogLevel level_;
  std::ostream *output_;
  std::ostringstream message_;
  bool active_{true};
};

// A lightweight value type that holds immutable logging configuration.
class Logger {
public:
  explicit Logger(std::string prefix = {});

  // The caller must keep output alive while this Logger and any entries it
  // creates are alive. This overload is primarily useful for tests or files.
  Logger(std::string prefix, std::ostream &output);

  [[nodiscard]] LogEntry log(LogLevel level) const;
  [[nodiscard]] LogEntry operator()(LogLevel level) const;

  template <typename T> LogEntry operator<<(const T &value) const {
    auto entry = log(INFO);
    entry << value;
    return entry;
  }

  const std::string &prefix() const noexcept { return prefix_; }

private:
  std::string prefix_;
  std::ostream *output_;
};

[[nodiscard]] Logger getLogger();
[[nodiscard]] Logger getLogger(std::string prefix);

#endif // SIMPLE_LOG_H
