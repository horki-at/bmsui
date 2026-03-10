#ifndef INCLUDED_SIMULATOR_
#define INCLUDED_SIMULATOR_

#include "utils.hh"
#include "paths.hh"
#include "pipe.hh"
#include "double_fork.hh"

#include <utility>
#include <filesystem>

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
  DoubleFork d_forker;       // utility to easily fork the children processes.
  Pipe d_cmdPipe;            // for communicating with the populate_data script.
  Pipe d_rdyPipe;            // for communicating when populate_data is ready.
  
  bool d_running;
  std::filesystem::path d_bmsDeviceFile; // virtual BMS device.
  std::filesystem::path d_appDeviceFile; // BMS sends data to this APP device.

public:
  Simulator(std::filesystem::path bmsDeviceFile = TTYVBMS,
            std::filesystem::path appDeviceFile = TTYVAPP);
  DISABLE_CPY_MV(Simulator);
  ~Simulator();

  void start();
  void kill();
  bool running() const;

  std::string bms() const;
  std::string app() const;

  void send_cmd(Command cmd, float arg = 0.0f) const;

private:
  [[noreturn]] void start_comm();    // Runs start_comm to simulate the device.
  [[noreturn]] void populate_data(); // Runs the script to generate fake data.
  void manage_children();            // Controls both the children.

  void remove_files() const;    // Remove generated virtual files.
};

inline Simulator::Simulator(std::filesystem::path bmsDeviceFile,
                            std::filesystem::path appDeviceFile)
:
  d_forker(),
  d_cmdPipe(),
  d_running(false),
  d_bmsDeviceFile(bmsDeviceFile),
  d_appDeviceFile(appDeviceFile)
{
}

inline bool Simulator::running() const
{
	return d_running;
}

inline std::string Simulator::bms() const
{
	return d_bmsDeviceFile.string();
}

inline std::string Simulator::app() const
{
	return d_appDeviceFile.string();
}
#endif // INCLUDED_SIMULATOR_
