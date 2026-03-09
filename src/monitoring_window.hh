#ifndef INCLUDED_MONITORING_WINDOW_
#define INCLUDED_MONITORING_WINDOW_

#include "bms.hh"
#include "simulator.hh"

#include "imgui.h"

#include <vector>
#include <string>
#include <bitset>
#include <limits>

// forward declarations
class GLFWwindow;

#define UNPACK(varname) (data.varname)
#define DECLARE_RENDER_MODULE(name)                                            \
static void render_##name##_module(BMS::Data const &data);
#define DEFINE_RENDER_MODULE(name)                                             \
void MonitoringWindow::render_##name##_module(BMS::Data const &data)

class MonitoringWindow
{
public:
  enum class Module
  {                             // NOTE: Keep this order.
    DEMO            = 0,        // render_demo_module
    CELL_VIEW       = 1,        // render_cell_module
    GENERAL_STATS   = 2,        // render_general_stats_module
    SOC             = 3,        // render_soc_module
    SYS_VOLT_GRAPH  = 4,        // render_sys_volt_graph_module
    SYS_CURR_GRAPH  = 5,        // render_sys_curr_graph_module
    CONSOLE         = 6,        // render_console_module
    SIMULATOR       = 7,        // render_simulator_module
    N_MODULE,
  };

private:
  size_t d_width, d_height;
  GLFWwindow *d_window;
  BMS d_bms;

  static Simulator s_simulator;

  std::bitset<static_cast<size_t>(Module::N_MODULE)> d_enabledModules;
  static void (*const s_modules[])(BMS::Data const & data);

  static ImGuiWindowFlags s_windowFlags;
  static bool s_openRealDeviceFlag;
  static bool s_simulateVirtualDeviceFlag;
  static bool s_closeSimulationFlag;
  static bool s_clearConsoleFlag;

  static std::vector<std::string> s_consoleBuffer;

  BMS::Data d_datacpy;          // the last copy of Data, used when there is no
                                // input from the circular Data buffer.
  
public:
  explicit MonitoringWindow(std::string title,
                            size_t width = 2000,
                            size_t height = 1400);
  DISABLE_CPY_MV(MonitoringWindow);
  ~MonitoringWindow();

  void enable(Module module); 
  void disable(Module module);

  void render();

  // Initialize rendering backends. This function must be manually called before
  // constructing an instance of MonitorWindow.
  static void init();

private:
  void setup_glfw_context() const;
  void setup_imgui_context() const;

  static void key_callback(GLFWwindow *window, int key, int scancode,
                           int action, int mods);

  // Renders the main menu
  void menu();

  // Menu option functions
  void openRealDevice();
  void simulateVirtualDevice();
  void closeSimulation();

  void consoleWrite(std::string const &message);
  static void clearConsoleBuffer();

  // Available modules that can be enabled using the appropriate Module flag.
  DECLARE_RENDER_MODULE(demo);
  DECLARE_RENDER_MODULE(cell_view);
  DECLARE_RENDER_MODULE(general_stats);
  DECLARE_RENDER_MODULE(soc);
  DECLARE_RENDER_MODULE(sys_volt_graph);
  DECLARE_RENDER_MODULE(sys_curr_graph);
  DECLARE_RENDER_MODULE(console);
  DECLARE_RENDER_MODULE(simulator);
};

inline void MonitoringWindow::enable(Module module)
{
  d_enabledModules.set(static_cast<size_t>(module));
}

inline void MonitoringWindow::disable(Module module)
{
  d_enabledModules.reset(static_cast<size_t>(module));
}

inline void MonitoringWindow::consoleWrite(std::string const &message)
{
  s_consoleBuffer.push_back(message);
}

//static
inline void MonitoringWindow::clearConsoleBuffer()
{
	s_consoleBuffer.clear();
}

#endif //INCLUDED_MONITORING_WINDOW_
