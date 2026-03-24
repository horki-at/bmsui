#ifndef INCLUDED_SIMULATOR_
#define INCLUDED_SIMULATOR_

#include "utils.hh"

#include <memory>
#include <string>

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
