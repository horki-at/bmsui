#ifdef _WIN32

#include "simulator_impl.hh"
#include "utils.hh"

#include <filesystem>
#include <utility>
#include <stdexcept>
#include <string>
#include <format>
using namespace std;
namespace fs = std::filesystem;

Simulator::Impl::Impl(fs::path bmsDeviceFile, fs::path appDeviceFile)
  :
  d_running(false),
  d_populateDataStartupInfo{ sizeof(STARTUPINFO) },
  d_populateDataProcessInfo{},
  d_commStartupInfo{ sizeof(STARTUPINFO) },
  d_commProcessInfo{},
  d_bms(bmsDeviceFile.string()),
  d_app(appDeviceFile.string())
{
}

Simulator::Impl::~Impl()
{
  kill();
}

void Simulator::Impl::start_comm()
{
  const string windowsBrokerScript = format("{} {} {}", WINDOWS_BROKER, TTYVBMS, TTYVAPP);

  if (not CreateProcess(PYTHON3_BIN, (char*)windowsBrokerScript.c_str(),
    NULL, NULL, TRUE, 0, NULL, NULL,
    &d_commStartupInfo, &d_commProcessInfo))
    throw runtime_error("Simulator: couldn't launch communication script.");
}

void Simulator::Impl::populate_data()
{
  if (not CreateProcess(PYTHON3_BIN, (char*)POPULATE_BMS_SCRIPT,
    NULL, NULL, TRUE, 0, NULL, NULL,
    &d_populateDataStartupInfo, &d_populateDataProcessInfo))
    throw runtime_error("Simulator: couldn't launch data population (Python) script.");
}

void Simulator::Impl::start()
{
  if (d_running) throw runtime_error("Simulator already running.");

  d_running = true;
  start_comm();
  populate_data();
}

void Simulator::Impl::kill()
{
  if (not d_running) throw runtime_error("Simulator not running.");

  d_running = false;
  // TODO: @error-handling
  TerminateProcess(d_commProcessInfo.hProcess, 0);
  TerminateProcess(d_populateDataProcessInfo.hProcess, 0);
}

bool Simulator::Impl::running() const
{
  return d_running;
}

string Simulator::Impl::bms() const
{
  return d_bms;
}

string Simulator::Impl::app() const
{
  return d_app;
}

void Simulator::Impl::send_cmd(Command cmd, float arg) const
{
  // TODO: implement this function.
  throw runtime_error("send_cmd is not implemented in Windows.");
}

#endif