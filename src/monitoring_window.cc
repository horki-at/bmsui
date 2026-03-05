#include "monitoring_window.hh"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#define GLFW_INCLUDE_NONE
#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include <string>
using namespace std;

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
  d_window(glfwCreateWindow(width, height, title.c_str(), NULL, NULL))
{
  if (not d_window) {
    glfwTerminate();
    throw "Couldn't create a glfw3 window."s;
  }

  setup_glfw_context();
  setup_imgui_context();
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

void MonitoringWindow::render() const
{
	while (not glfwWindowShouldClose(d_window)) {
    glfwPollEvents();

    // TODO: load MonitoringWindow modules
    // Start the Dear ImGui Frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::ShowDemoWindow();

    // Render ImGui frames
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(d_window);
  }
}
