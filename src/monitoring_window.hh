#ifndef INCLUDED_MONITORING_WINDOW_
#define INCLUDED_MONITORING_WINDOW_

#include "bms.hh"
#include "simulator.hh"
#include "utils.hh"

#include "imgui.h"

#define GLFW_INCLUDE_NONE
#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include <vector>
#include <memory>
#include <string>
#include <bitset>
#include <limits>

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
  static inline bool s_initialized = false; // you must call init() 

  static void (*const s_modules[])(BMS::Data const &data);
  std::bitset<static_cast<size_t>(Module::N_MODULE)> d_enabledModules;
  
  size_t d_width = 0, d_height = 0;
  GLFWwindow *d_window = nullptr;

  BMS d_bms;
  Simulator d_simulator;

  ImGuiWindowFlags d_windowFlags   = ImGuiWindowFlags_NoCollapse;
  bool d_openRealDeviceFlag        = false;
  bool d_simulateVirtualDeviceFlag = false;
  bool d_closeSimulationFlag       = false;
  bool d_clearConsoleFlag          = false;

  std::vector<std::string> d_consoleBuffer;

  BMS::Data d_datacpy = {0};    // the last copy of Data, used when there is no
                                // input from the circular Data buffer.
public:
  DISABLE_CPY_MV(MonitoringWindow);
  ~MonitoringWindow();

  // Initialize rendering backends. This function must be manually called before
  // constructing an instance of MonitorWindow.
  static void init(std::string title, size_t width = 2000, size_t height = 1400);

  // Obtain a singleton instance of the MonitoringWindow.
  static MonitoringWindow &instance();

  void enable(Module module); 
  void disable(Module module);
  void render();

private:
  MonitoringWindow() = default;

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
  void clearConsoleBuffer();

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
  d_consoleBuffer.push_back(message);
}

//static
inline void MonitoringWindow::clearConsoleBuffer()
{
	d_consoleBuffer.clear();
}

#endif //INCLUDED_MONITORING_WINDOW_
