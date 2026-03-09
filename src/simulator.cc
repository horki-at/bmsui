#include "simulator.hh"
#include "paths.hh"

#include <iostream> // TODO: @trace remove this
#include <filesystem>
#include <stdexcept>
#include <string>
#include <format>
#include <unistd.h>
using namespace std;
namespace fs = std::filesystem;

void Simulator::start()
{
  if (d_running) throw runtime_error("Simulator already running.");

  d_running = true;
  d_cmdPipe.open();
  d_rdyPipe.open();
  d_forker.run([this]() { start_comm(); },
               [this]() { populate_data(); },
               [this]() { manage_children(); });
}

void Simulator::start_comm()
{
  char *args[] = { START_COMM_SCRIPT,
                   (char *)d_bmsDeviceFile.string().c_str(),
                   (char *)d_appDeviceFile.string().c_str(),
                   NULL };
  execvp(START_COMM_SCRIPT, args);
  _exit(1);
}

void Simulator::populate_data()
{
  d_cmdPipe.close_write();
  dup2(d_cmdPipe.readend(), STDIN_FILENO);
  d_cmdPipe.close_read();

  d_rdyPipe.close_read();
  dup2(d_rdyPipe.writeend(), 3); // NOTE: fd 3 must be used in populate data
  d_rdyPipe.close_write();
  
  char *args[] = { PYTHON3_BIN,
                   POPULATE_BMS_SCRIPT, NULL };
  execvp(PYTHON3_BIN, args);
  _exit(3);
}

void Simulator::manage_children()
{
  d_cmdPipe.close_read();
  d_rdyPipe.close_write();
  
  // populate_data script must send "ready" message when it is fully initialized
  char buf[16];                 // 16 bytes must be enough 
  if (read(d_rdyPipe.readend(), buf, sizeof(buf)/sizeof(buf[0])) == -1)
    throw runtime_error("Couldn't receive ready signal from the simulation script.");

  if (string{buf} != "ready")
    throw runtime_error("The simulation script sent "s + string{buf});
}

void Simulator::kill()
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

void Simulator::remove_files() const
{
	if (fs::exists(d_bmsDeviceFile))
    fs::remove(d_bmsDeviceFile);
  if (fs::exists(d_appDeviceFile))
    fs::remove(d_appDeviceFile);
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

void Simulator::send_cmd(Command cmd, float arg) const
{
  string const msg = (cmd > Command::__ARGUMENTED)
                     ? format("{} {}\n", s_command_type_str[(size_t)cmd], arg)
                     : format("{}\n", s_command_type_str[(size_t)cmd]);
  write(d_cmdPipe.writeend(), msg.c_str(), msg.size());
}

Simulator::~Simulator()
{
  d_cmdPipe.close_write();
  d_rdyPipe.close_read();
  kill();
}
