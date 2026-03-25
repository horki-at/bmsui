#ifndef INCLUDED_SIMULATOR_
#define INCLUDED_SIMULATOR_

#include "utils.hh"

#include <memory>
#include <string>

/* 
 * Because CMakeLang doesn't provide raw strings, and Windows Named
 * Pipes require backslashes, we need to use C++-style raw string literals,
 * and C-style platform detection.
*/
#ifdef _WIN32
#define TTYVBMS R"(\\.\pipe\ttyVBMS)"
#define TTYVAPP R"(\\.\pipe\ttyVAPP)"
// this is used for communication with populate_data.py
#define CHANNEL R"(\\.\pipe\populateDataChannel)"
#elifdef __linux__
#define TTYVBMS "/tmp/ttyVBMS"
#define TTYVAPP "/tmp/ttyVAPP"
// Placeholder for Windows compatibility
#define CHANNEL
#else
#define TTYVBMS
#define TTYVAPP
// Placeholder for Windows compatibility
#define CHANNEL
#endif

enum class Command
{
  IDLE,
  __ARGUMENTED,                 // do not use this enum entry
  CHARGE,
  DISCHARGE,
  N_COMMAND
};

class Simulator
{
public:
  class Impl;

private:
  std::unique_ptr<Impl> d_impl;

public:
  Simulator();
  Simulator(std::unique_ptr<Impl> impl);
  DISABLE_CPY_MV(Simulator);
  ~Simulator();

  void set_impl(std::unique_ptr<Impl> impl);

  void start();
  void kill();
  bool running() const;

  std::string bms() const;
  std::string app() const;

  void send_cmd(Command cmd, float arg = 0.0f) const;
};

#endif // INCLUDED_SIMULATOR_
