#include "monitoring_window.hh"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#define GLFW_INCLUDE_NONE
#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include <optional>
#include <iostream>
#include <string>
using namespace std;

void (*const MonitoringWindow::s_modules[])(optional<BMS::Data> const &) = {
  &MonitoringWindow::render_demo_module,
  &MonitoringWindow::render_cell_view_module,
  &MonitoringWindow::render_general_stats_module,
  &MonitoringWindow::render_pack_vi_module,
  &MonitoringWindow::render_bms_vi_module,
  &MonitoringWindow::render_bms_temp_module,
};

//static
void MonitoringWindow::init()
{
	if (not glfwInit()) throw "Couldn't initialize glfw3."s;

  // OpenGL Core Version 4.3
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
}

//explicit
MonitoringWindow::MonitoringWindow(std::string title, size_t width, size_t height)
:
  d_width(width),
  d_height(height),
  d_window(glfwCreateWindow(width, height, title.c_str(), NULL, NULL)),
  d_enabledModules(0)
{
  if (not d_window)
  {
    glfwTerminate();
    throw "Couldn't create a glfw3 window."s;
  }

  setup_glfw_context();
  setup_imgui_context();

  // set appropriate callbacks
  glfwSetKeyCallback(d_window, key_callback);
}

MonitoringWindow::~MonitoringWindow()
{
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
  glfwDestroyWindow(d_window);
  glfwTerminate();
}

void MonitoringWindow::setup_glfw_context() const
{
  glfwMakeContextCurrent(d_window);
  gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
  glfwSwapInterval(1);
}

void MonitoringWindow::setup_imgui_context() const
{
  // This code was copy pasted from ImGui setup documentation (for GLFW/OpenGl)
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; 
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

  ImGui_ImplGlfw_InitForOpenGL(d_window, true);
  ImGui_ImplOpenGL3_Init();
}

void MonitoringWindow::render(Ring<BMS::Data, 64> &buffer) const
{
	while (not glfwWindowShouldClose(d_window))
  {
    glfwPollEvents();

    // Clear background
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    // Obtain the last available data from the ring buffer.
    optional<BMS::Data> const data = buffer.try_pop();

    // Render enabled modules
    for (size_t i = 0; i != static_cast<size_t>(Module::N_MODULE); ++i)
      if (d_enabledModules.test(i))
        (*s_modules[i])(data);

    // Render ImGui frames (if any)
    if (ImGui::GetFrameCount())
    {
      ImGui::Render();
      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    glfwSwapBuffers(d_window);
  }
}

// Creates a new ImGui frame.
static void create_new_frame()
{
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
}

//static
void MonitoringWindow::key_callback(GLFWwindow *window, int key, int scancode,
                                    int action, int mods)
{
  if (key == GLFW_KEY_ESCAPE and action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, 1); // close the window 
  if (key == GLFW_KEY_E and action == GLFW_PRESS)
    cout << "you pressed E\n";
}

//static
DEFINE_RENDER_MODULE(cell_view)
{
  // TODO: implement this
}

//static
DEFINE_RENDER_MODULE(general_stats)
{
  // TODO: implement this
}

//static
DEFINE_RENDER_MODULE(pack_vi)
{
  // TODO: implement this
}

//static
DEFINE_RENDER_MODULE(bms_vi)
{
  // TODO: implement this
}

//static
DEFINE_RENDER_MODULE(bms_temp)
{
  // TODO: implement this
}

//static
DEFINE_RENDER_MODULE(demo)
{
  create_new_frame();
  ImGui::ShowDemoWindow();
}
