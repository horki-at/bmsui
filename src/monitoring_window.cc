#include "monitoring_window.hh"
#include "graph.hh"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#define GLFW_INCLUDE_NONE
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "implot.h"

#include <format>
#include <optional>
#include <iostream>
#include <algorithm>
#include <string>
using namespace std;

#define PLOT_SIZE 256
static Graph<PLOT_SIZE, 0.0f, 140.f> s_high_voltage;
static Graph<PLOT_SIZE, 0.0f, 13.f> s_low_voltage;
static Graph<PLOT_SIZE> s_high_current;
static Graph<PLOT_SIZE> s_low_current;

static void clear_graphs()
{
  s_high_voltage.clear();
  s_low_voltage.clear();
  s_high_current.clear();
  s_low_current.clear();
}

//static
void (*const MonitoringWindow::s_modules[])(BMS::Data const &) = {
  &MonitoringWindow::render_demo_module,
  &MonitoringWindow::render_cell_view_module,
  &MonitoringWindow::render_general_stats_module,
  &MonitoringWindow::render_soc_module,
  &MonitoringWindow::render_sys_volt_graph_module,
  &MonitoringWindow::render_sys_curr_graph_module,
  &MonitoringWindow::render_console_module,
  &MonitoringWindow::render_simulator_module
};

//static
ImGuiWindowFlags MonitoringWindow::s_windowFlags = ImGuiWindowFlags_NoCollapse;
bool MonitoringWindow::s_openRealDeviceFlag = false;
bool MonitoringWindow::s_simulateVirtualDeviceFlag = false;
bool MonitoringWindow::s_closeSimulationFlag = false;
bool MonitoringWindow::s_clearConsoleFlag = false;
vector<string> MonitoringWindow::s_consoleBuffer{};
Simulator MonitoringWindow::s_simulator{};

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
MonitoringWindow::MonitoringWindow(std::string title,
                                   size_t width, size_t height)
:
  d_width(width),
  d_height(height),
  d_window(glfwCreateWindow(width, height, title.c_str(), NULL, NULL)),
  d_bms(),
  d_enabledModules(0),
  d_datacpy()
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
  if (s_simulator.running()) s_simulator.kill();
  
  ImGui::PopStyleColor();
  
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
  ImPlot::DestroyContext();
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
  ImPlot::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; 
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

  // Load the font
  ImFont *font = io.Fonts->AddFontFromFileTTF("./assets/iosevka-regular.ttf", 28.0f);
  ImGui::PushFont(font);

  // Disable TitleBgActive
  ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImGui::GetStyleColorVec4(ImGuiCol_TitleBg));

  // Setup GLFW3 and OpenGL backend
  ImGui_ImplGlfw_InitForOpenGL(d_window, true);
  ImGui_ImplOpenGL3_Init();
}

void MonitoringWindow::openRealDevice()
{
  consoleWrite("[ERROR]: openRealDevice is not implemented.");
  s_openRealDeviceFlag = false; // reset the flag
}

// TODO: @platform-dependent this function is platform dependent. For now, it
// works by assuming the linux environment and calling the
// ../util/start_comm.sh bash utility.
void MonitoringWindow::simulateVirtualDevice() try
{
  clear_graphs();
  s_simulator.start();

  if (not d_bms.open_device(s_simulator.app()))
    consoleWrite("[ERROR]: couldn't open "s + s_simulator.app());

  // start the module
  enable(Module::SIMULATOR);
}
catch (exception const &exc)
{
  consoleWrite("[ERROR]: "s + exc.what());
}

void MonitoringWindow::closeSimulation() try
{
  clear_graphs();
  d_datacpy = {};
  s_simulator.kill();
  disable(Module::SIMULATOR);
}
catch (exception const &exc)
{
  consoleWrite("[ERROR]: "s + exc.what());
}

void MonitoringWindow::menu()
{
  // Show menu options
  if (ImGui::BeginMainMenuBar())
  {
    if (ImGui::BeginMenu("Device"))
    {
      ImGui::MenuItem("Open real device", NULL, &s_openRealDeviceFlag);
      ImGui::MenuItem("Simulate virtual device", NULL, &s_simulateVirtualDeviceFlag);
      ImGui::MenuItem("Close simulation", NULL, &s_closeSimulationFlag);
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Extra"))
    {
      ImGui::MenuItem("Clear console", "C", &s_clearConsoleFlag);
      ImGui::EndMenu();
    }
    
    ImGui::EndMainMenuBar();
  }

  // Process menu flags
  if (s_clearConsoleFlag)
  {
    clearConsoleBuffer();
    s_clearConsoleFlag = false;
  }
  if (s_openRealDeviceFlag)
  {
    openRealDevice();
    s_openRealDeviceFlag = false;
  }
  if (s_simulateVirtualDeviceFlag)
  {
    simulateVirtualDevice();
    s_simulateVirtualDeviceFlag = false;
  }
  if (s_closeSimulationFlag)
  {
    closeSimulation();
    s_closeSimulationFlag = false;
  }
}

void MonitoringWindow::render()
{
	while (not glfwWindowShouldClose(d_window))
  {
    glfwPollEvents();

    // Clear background
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    // Obtain the last available data from the ring buffer.
    optional<BMS::Data> const data = d_bms.buffer().try_pop();
    if (data)                   // the ring had up-to-date Data
      d_datacpy = *data;        // copy it to the Data copy

    // Start a new ImGui frame
    if (d_enabledModules != 0) { // if some module is enabled, create the frame
      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplGlfw_NewFrame();
      ImGui::NewFrame();
    }

    ImGui::DockSpaceOverViewport();

    // Render the main menu bar
    menu();

    // Render enabled modules
    for (size_t i = 0; i != static_cast<size_t>(Module::N_MODULE); ++i)
      if (d_enabledModules.test(i))
        (*s_modules[i])(d_datacpy);

    // Render ImGui frames (if any)
    if (ImGui::GetFrameCount())
    {
      ImGui::Render();
      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    glfwSwapBuffers(d_window);
  }
}

//static
void MonitoringWindow::key_callback(GLFWwindow *window, int key, int scancode,
                                    int action, int mods)
{
  if (key == GLFW_KEY_ESCAPE and action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, 1); // close the window 
  if (key == GLFW_KEY_C and action == GLFW_PRESS)
    clearConsoleBuffer();
}

//static
DEFINE_RENDER_MODULE(demo)
{
  ImGui::ShowDemoWindow();
}

#define TEMP_TOO_COLD_COLOR ImVec4(0.0, 0.5, 1.0, 1.0)
#define TEMP_OPTIMAL_COLOR ImVec4(0.0, 1.0, 0.0, 1.0)
#define TEMP_WARNING_COLOR ImVec4(1.0, 0.8, 0.0, 1.0)
#define TEMP_TOO_HOT_COLOR ImVec4(1.0, 0.0, 0.0, 1.0)

// Get the color to a button box that represents the cell with the given temp.
static ImVec4 get_cell_color(float temp)
{
  float const too_cold = BMS::cellTempInfo[BMS::TOO_COLD];
  float const optimal = BMS::cellTempInfo[BMS::OPTIMAL];
  float const warning = BMS::cellTempInfo[BMS::WARNING];
  float const too_hot = BMS::cellTempInfo[BMS::TOO_HOT];
  
  static auto lerp = [](ImVec4 from, ImVec4 to, float t) -> ImVec4 {
    return ImVec4(from.x + (to.x - from.x) * t, // Red
                  from.y + (to.y - from.y) * t, // Green
                  from.z + (to.z - from.z) * t, // Blue
                  1.0);                         // Alpha
  };

  if (temp <= too_cold)
    return TEMP_TOO_COLD_COLOR;
  else if (temp >= too_hot)
    return TEMP_TOO_HOT_COLOR;
  else if (temp < optimal)      // <too_COLD, OPTIMAL>
    return lerp(TEMP_TOO_COLD_COLOR,
                TEMP_OPTIMAL_COLOR,
                (temp - too_cold) / (optimal - too_cold));
  else if (temp < warning)      // <OPTIMAL, WARNING> 
    return lerp(TEMP_OPTIMAL_COLOR,
                TEMP_WARNING_COLOR,
                (temp - optimal) / (warning - optimal));
  else                          // <WARNING, HOT>
    return lerp(TEMP_WARNING_COLOR,
                TEMP_TOO_HOT_COLOR,
                (temp - warning) / (too_hot - warning));
}

//static
DEFINE_RENDER_MODULE(cell_view)
{
  ImGui::Begin("CELL VIEW", NULL, s_windowFlags);

  // For each battery module, display a group of individual cells
  for (size_t module_idx = 0; module_idx != BMS::modules; ++module_idx)
  {
    ImGui::Text("Module %zu", module_idx + 1);

    // Prepare the grid style
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(1, 1)); // grid spacing 
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f); // strict corners

    for (size_t parallel_idx = 0; parallel_idx != BMS::parallel; ++parallel_idx)
    {
      for (size_t series_idx = 0; series_idx != BMS::series; ++series_idx)
      {
        size_t const cell_index = BMS::cell_id(module_idx, parallel_idx, series_idx);
        size_t const temp_index = BMS::temp_id(module_idx, parallel_idx, series_idx);
        ImVec4 const color = get_cell_color(UNPACK(cellTemps)[temp_index]);

        ImGui::PushID(cell_index);
        ImGui::PushStyleColor(ImGuiCol_Button, color);
        ImGui::Button("##cellbtn", ImVec2(25, 25));
        if (ImGui::IsItemHovered())
          ImGui::SetTooltip("Cell [M:%zu, P:%zu, S:%zu]\nVoltage %.2f\nTemperature %.2f",
                            module_idx + 1, parallel_idx, series_idx,
                            UNPACK(cellVolts)[cell_index],
                            UNPACK(cellTemps)[temp_index]);
        ImGui::PopStyleColor();
        ImGui::PopID();

        if (series_idx != BMS::series - 1)
          ImGui::SameLine();
      }
    }

    ImGui::PopStyleVar(2);      // 2: grid spacing, strict corners
  }

  ImGui::End();
}

//static
DEFINE_RENDER_MODULE(general_stats)
{
  ImGui::Begin("GENERAL STATISTICS", NULL, s_windowFlags);

  // Display general statistics on the cell level
  ImGui::TextColored(ImVec4(1, 0, 0, 1), "Cell Level");
  ImGui::Text("Highest cell temperature: %.2f", UNPACK(maxTemp));
  ImGui::Text("Lowest cell temperature: %.2f", UNPACK(minTemp));
  ImGui::Text("Average cell temperature: %.2f", UNPACK(avgTemp));
  ImGui::Text("Highest cell voltage: %.2f", UNPACK(maxCellVolt));
  ImGui::Text("Lowest cell voltage: %.2f", UNPACK(minCellVolt));
  
  // Display general statistics on the pack level
  ImGui::TextColored(ImVec4(1, 0, 0, 1), "Pack Level");
  ImGui::Text("System high voltage: %.2f", UNPACK(voltPack));
  ImGui::Text("System high current: %.2f", UNPACK(currPack));
  ImGui::Text("System low voltage: %.2f", UNPACK(voltLow));
  ImGui::Text("System low current: %.2f", UNPACK(currLow));

  // Display general statistics on the BMS level
  ImGui::TextColored(ImVec4(1, 0, 0, 1), "BMS Level");
  ImGui::Text("BMS temperature: %.2f", UNPACK(avgTemp));

  ImGui::End();
}

//static
DEFINE_RENDER_MODULE(soc)
{
  ImGui::Begin("CHARGE PERCENTAGE", NULL, s_windowFlags);
  ImGui::ProgressBar(UNPACK(soc) / 100); // convert from 100% to fractions
  ImGui::End();
}

static inline void setup_plot_style(char const *xlabel, char const *ylabel)
{
  ImPlot::SetupAxes(xlabel, ylabel,
                    ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_NoTickLabels |
                    ImPlotAxisFlags_NoGridLines | ImPlotAxisFlags_NoTickMarks,
                    ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_NoGridLines |
                    ImPlotAxisFlags_NoTickMarks | ImPlotAxisFlags_NoTickLabels);
}

DEFINE_RENDER_MODULE(sys_volt_graph)
{
  ImGui::Begin("SYSTEM VOLTAGE GPAPHS");

  // update the graphs with new incoming data.
  s_high_voltage.add(ImGui::GetTime(), UNPACK(voltPack));
  s_low_voltage.add(ImGui::GetTime(), UNPACK(voltLow));

  ImPlot::PushStyleVar(ImPlotStyleVar_FitPadding, ImVec2(0.0f, 10.f));
  // plot the graphs
  if (ImPlot::BeginPlot("##x"))
  {
    setup_plot_style("time [s]", "voltage [V]");
    ImPlot::PlotLine("System High Voltage",
                     s_high_voltage.x(),
                     s_high_voltage.y(),
                     PLOT_SIZE);
    ImPlot::EndPlot();
  }
  ImGui::Separator();
  if (ImPlot::BeginPlot("##xx"))
  {
    setup_plot_style("time [s]", "voltage [V]");
    ImPlot::PlotLine("System Low Voltage",
                     s_low_voltage.x(),
                     s_low_voltage.y(),
                     PLOT_SIZE);
    ImPlot::EndPlot();
  }
  ImPlot::PopStyleVar();
  
  ImGui::End();
}

DEFINE_RENDER_MODULE(sys_curr_graph)
{
  ImGui::Begin("SYSTEM CURRENT GPAPHS");

  // update the graphs with new incoming data.
  s_high_current.add(ImGui::GetTime(), UNPACK(currPack));
  s_low_current.add(ImGui::GetTime(), UNPACK(currLow));

  // plot the graphs
  ImPlot::PushStyleVar(ImPlotStyleVar_FitPadding, ImVec2(0.0f, 10.f));
  if (ImPlot::BeginPlot("##x"))
  {
    setup_plot_style("time [s]", "current [A]");
    ImPlot::PlotLine("System High Current",
                     s_high_current.x(),
                     s_high_current.y(),
                     PLOT_SIZE);
    ImPlot::EndPlot();
  }
  ImGui::Separator();
  if (ImPlot::BeginPlot("##xx"))
  {
    setup_plot_style("time [s]", "current [A]");
    ImPlot::PlotLine("System Low Current",
                     s_low_current.x(),
                     s_low_current.y(),
                     PLOT_SIZE);
    ImPlot::EndPlot();
  }
  ImPlot::PopStyleVar();
  
  ImGui::End();
}

//static
DEFINE_RENDER_MODULE(console)
{
  ImGui::Begin("CONSOLE", NULL, s_windowFlags);
  for_each(s_consoleBuffer.begin(), s_consoleBuffer.end(),
           [](string const &msg)
           {
             ImGui::Text("%s", msg.c_str());
           });
  ImGui::End();
}

//static
DEFINE_RENDER_MODULE(simulator)
{
  ImGui::Begin("SIMULATOR", NULL, s_windowFlags);
  // IDLE Mode
  if (ImGui::Button("Disconnect the battery"))
    s_simulator.send_cmd(Command::IDLE);

  // CHARGE Mode
  static float s_charge_current = 10.0;
  if (ImGui::Button("Charge the battery"))
    s_simulator.send_cmd(Command::CHARGE, s_charge_current);
  ImGui::SameLine();
  ImGui::InputFloat("Charging current [A]", &s_charge_current, 10.f, 0, "%.2f");

  // DISCHARGE Mode
  static float s_discharge_load = 300.0;
  if (ImGui::Button("Discharge the battery"))
    s_simulator.send_cmd(Command::DISCHARGE, s_discharge_load);
  ImGui::SameLine();
  ImGui::InputFloat("Load resistance [Ω]", &s_discharge_load, 300.f, 0, "%.2f");

  ImGui::End();
}
