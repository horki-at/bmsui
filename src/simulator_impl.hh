#ifndef INCLUDED_SIMULATOR_IMPL_
#define INCLUDED_SIMULATOR_IMPL_

#ifdef __linux__

#include "simulator.hh"
#include "double_fork.hh"
#include "pipe.hh"

#include <string>
#include <filesystem>

#include <unistd.h>

class Simulator::Impl
{
  DoubleFork d_forker;       // utility to easily fork the children processes.
  Pipe d_cmdPipe;            // for communicating with the populate_data script.
  Pipe d_rdyPipe;            // for communicating when populate_data is ready.

  bool d_running;
  std::filesystem::path d_bmsDeviceFile; // virtual BMS device.
  std::filesystem::path d_appDeviceFile; // BMS sends data to this APP device.

public:
  Impl(std::filesystem::path bmsDeviceFile = TTYVBMS,
       std::filesystem::path appDeviceFile = TTYVAPP);
  ~Impl();

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

#elifdef _WIN32

#include "simulator.hh"

#include <string>
#include <filesystem>

#include <Windows.h>

class Simulator::Impl
{
  bool d_running;

  // Information about child processes.
  STARTUPINFO d_populateDataStartupInfo;
  PROCESS_INFORMATION d_populateDataProcessInfo;

  STARTUPINFO d_commStartupInfo;
  PROCESS_INFORMATION d_commProcessInfo;

  // Populate data script communication channel
  HANDLE d_channelHandler;

  std::string d_bms;
  std::string d_app;

public:
  Impl(std::filesystem::path bmsDeviceFile = TTYVBMS, 
       std::filesystem::path appDeviceFile = TTYVAPP);
  ~Impl();

  void start();
  void kill();
  bool running() const;

  std::string bms() const;
  std::string app() const;

  void send_cmd(Command cmd, float arg = 0.0f) const;

private:
  void start_comm();                 // Starts the communication process ttyVBMS :: ttyVAPP
  void populate_data();              // Starts the data population script (ie the Python simulator)

  void open_popdata_channel(); // Creates a named-pipe channel to communicate with populate data script.
};

#else
#error "Simulator::Impl is not implemented on your system."
#endif

#endif // INCLUDED_SIMULATOR_IMPL_
