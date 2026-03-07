#ifndef INCLUDED_MONITORING_WINDOW_
#define INCLUDED_MONITORING_WINDOW_

#include "imgui.h"

#include "bms.hh"

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

// TODO: @priority1 @bug exiting window is lagging.
class MonitoringWindow
{
public:
  enum class Module
  {                             // NOTE: Keep this order.
    DEMO          = 0,          // render_demo_module
    CELL_VIEW     = 1,          // render_cell_module
    GENERAL_STATS = 2,          // render_general_stats_module
    SOC           = 3,          // render_soc_module
    N_MODULE,
  };

private:
  size_t d_width, d_height;
  GLFWwindow *d_window;
  BMS d_bms;

  std::bitset<static_cast<size_t>(Module::N_MODULE)> d_enabledModules;
  static void (*const s_modules[])(BMS::Data const & data);

  static ImGuiWindowFlags d_windowFlags;

  BMS::Data d_datacpy;          // the last copy of Data, used when there is no
                                // input from the circular Data buffer.
  
public:
  explicit MonitoringWindow(std::string title,
                            size_t width = 800,
                            size_t height = 800);
  MonitoringWindow(MonitoringWindow const &) = delete;
  MonitoringWindow(MonitoringWindow &&) = delete;
  MonitoringWindow &operator=(MonitoringWindow const &) = delete;
  MonitoringWindow &operator=(MonitoringWindow &&) = delete;

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
  void menu() const;

  // Available modules that can be enabled using the appropriate Module flag.
  DECLARE_RENDER_MODULE(demo);
  DECLARE_RENDER_MODULE(cell_view);
  DECLARE_RENDER_MODULE(general_stats);
  DECLARE_RENDER_MODULE(soc);
};

inline void MonitoringWindow::enable(Module module)
{
  d_enabledModules.set(static_cast<size_t>(module));
}

inline void MonitoringWindow::disable(Module module)
{
  d_enabledModules.reset(static_cast<size_t>(module));
}

#endif //INCLUDED_MONITORING_WINDOW_
