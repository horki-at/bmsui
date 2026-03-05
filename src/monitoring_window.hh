#ifndef INCLUDED_MONITORING_WINDOW_
#define INCLUDED_MONITORING_WINDOW_

#include <string>

// forward declarations
class GLFWwindow;

class MonitoringWindow {
  size_t d_width, d_height;
  GLFWwindow *d_window;
  
public:
  explicit MonitoringWindow(std::string title,
                            size_t width = 800,
                            size_t height = 800);
  ~MonitoringWindow();

  void render() const;

  // Initialize rendering backends. This function must be manually called before
  // constructing an instance of MonitorWindow.
  static void init();

private:
  void setup_glfw_context() const;
  void setup_imgui_context() const;
};

#endif //INCLUDED_MONITORING_WINDOW_
