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
  d_channelHandler{},
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
  const string args = format("{} {} {} {}", PYTHON3_BIN, WINDOWS_BROKER, TTYVBMS, TTYVAPP);

  if (not CreateProcess(PYTHON3_BIN, (char*)args.c_str(),
    NULL, NULL, TRUE, 0, NULL, NULL,
    &d_commStartupInfo, &d_commProcessInfo))
    throw runtime_error("Simulator: couldn't launch communication script.");
}

void Simulator::Impl::populate_data()
{
  const string args = format("{} {} {} {}", PYTHON3_BIN, POPULATE_BMS_SCRIPT, TTYVBMS, CHANNEL);

  if (not CreateProcess(PYTHON3_BIN, (char*)args.c_str(),
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
  open_popdata_channel();

  // Populate data script must send "ready" message when it is fully initialized
  char buffer[64] = { 0 };
  DWORD bytesRead;
  if (not ReadFile(d_channelHandler, buffer, sizeof(buffer), &bytesRead, nullptr))
    throw runtime_error("Couldn't receive ready signal from the simulation script.");
  if (string{ buffer } != "ready")
    throw runtime_error("The simulation script sent "s + buffer);
}

void Simulator::Impl::kill()
{
  if (not d_running) throw runtime_error("Simulator not running.");

  d_running = false;
  CloseHandle(d_channelHandler);
  if (not TerminateProcess(d_populateDataProcessInfo.hProcess, 0))
    throw runtime_error("Couldn't terminate data population process.");
  if (not TerminateProcess(d_commProcessInfo.hProcess, 0))
    throw runtime_error("Couldn't terminate communication process.");
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

void Simulator::Impl::open_popdata_channel() 
{
  d_channelHandler = CreateNamedPipe(
    CHANNEL,
    PIPE_ACCESS_DUPLEX,
    PIPE_TYPE_BYTE,
    2,
    64, 64, 0, nullptr);

  if (d_channelHandler == INVALID_HANDLE_VALUE)
    throw runtime_error("Simulator: Couldn't create channel "s + CHANNEL);

  auto connected = ConnectNamedPipe(d_channelHandler, nullptr);

  if (!connected && GetLastError() != ERROR_PIPE_CONNECTED)
    throw runtime_error("Simulator: Connection failed on "s + CHANNEL);
}

namespace
{
  static string s_command_type_str[static_cast<size_t>(Command::N_COMMAND)] = {
    "idle",                     // Command::IDLE
    "__argumented__ERROR",      // Command::__ARGUMENTED
    "charge",                   // Command::CHARGE
    "discharge"                 // Command::DISCHARGE
  };
};

void Simulator::Impl::send_cmd(Command cmd, float arg) const
{
  if (cmd < Command::IDLE || cmd >= Command::N_COMMAND)
    throw std::invalid_argument("Invalid command value");

  string const msg = (cmd > Command::__ARGUMENTED)
    ? format("{} {}\n", s_command_type_str[(size_t)cmd], arg)
    : format("{}\n", s_command_type_str[(size_t)cmd]);

  if (not WriteFile(d_channelHandler, msg.c_str(), msg.size(), nullptr, nullptr))
    throw runtime_error("Couldn't send command to the simulation script.");
}

#endif