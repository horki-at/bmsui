#ifdef _POSIX

#include "simulator_impl.hh"
#include "utils.hh"

#include <filesystem>
#include <utility>
#include <stdexcept>
#include <string>
#include <format>
using namespace std;
namespace fs = std::filesystem;

Simulator::Impl::Impl(fs::path bmsDeviceFile,
  fs::path appDeviceFile)
  :
  d_forker(),
  d_cmdPipe(),
  d_running(false),
  d_bmsDeviceFile(bmsDeviceFile),
  d_appDeviceFile(appDeviceFile)
{
}

Simulator::Impl::~Impl()
{
  d_cmdPipe.close_write();
  d_rdyPipe.close_read();
  kill();
}

void Simulator::Impl::start()
{
  if (d_running) throw runtime_error("Simulator already running.");

  d_running = true;
  d_cmdPipe.open();
  d_rdyPipe.open();
  d_forker.run([this]() { start_comm(); },
    [this]() { populate_data(); },
    [this]() { manage_children(); });
}

void Simulator::Impl::kill()
{
  if (d_running)
  {
    d_running = false;
    d_forker.kill();
    remove_files();
    return;
  }

  throw runtime_error("Simulator not running.");
}

bool Simulator::Impl::running() const
{
  return d_running;
}

std::string Simulator::Impl::bms() const
{
  return d_bmsDeviceFile.string();
}

std::string Simulator::Impl::app() const
{
  return d_appDeviceFile.string();
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
  string const msg = (cmd > Command::__ARGUMENTED)
    ? format("{} {}\n", s_command_type_str[(size_t)cmd], arg)
    : format("{}\n", s_command_type_str[(size_t)cmd]);
  write(d_cmdPipe.writeend(), msg.c_str(), msg.size());
}

void Simulator::Impl::start_comm()
{
  char* args[] = { (char*)START_COMM_SCRIPT,
                 (char*)d_bmsDeviceFile.string().c_str(),
                 (char*)d_appDeviceFile.string().c_str(),
                 NULL };
  execvp(START_COMM_SCRIPT, args);
  _exit(1);
}

void Simulator::Impl::populate_data()
{
  d_cmdPipe.close_write();
  dup2(d_cmdPipe.readend(), STDIN_FILENO);
  d_cmdPipe.close_read();

  d_rdyPipe.close_read();
  dup2(d_rdyPipe.writeend(), 3); // NOTE: fd 3 must be used in populate data
  d_rdyPipe.close_write();

  char* args[] = { (char*)PYTHON3_BIN,
                   (char*)POPULATE_BMS_SCRIPT,
                   (char*)d_bmsDeviceFile.string().c_str(),
                   NULL };
  execvp(PYTHON3_BIN, args);
  _exit(3);
}

void Simulator::Impl::manage_children()
{
  d_cmdPipe.close_read();
  d_rdyPipe.close_write();

  // populate_data script must send "ready" message when it is fully initialized
  char buf[16];                 // 16 bytes must be enough 
  if (read(d_rdyPipe.readend(), buf, sizeof(buf) / sizeof(buf[0])) == -1)
    throw runtime_error("Couldn't receive ready signal from the simulation script.");

  if (string{ buf } != "ready")
    throw runtime_error("The simulation script sent "s + string{ buf });
}

void Simulator::Impl::remove_files() const
{
  if (fs::exists(d_bmsDeviceFile))
    fs::remove(d_bmsDeviceFile);
  if (fs::exists(d_appDeviceFile))
    fs::remove(d_appDeviceFile);
}


#endif 