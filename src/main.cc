#include "monitoring_window.hh"
#include <iostream>
using namespace std;

int processException(exception_ptr eptr) try
{
  if (eptr)
    rethrow_exception(eptr);
  return 0;                     // no exception here.
}
catch (exception const &exc)
{
  cerr << exc.what() << "\n";
  return 1;
}

int main() try
{
  // Initialize all the subsystems.
  MonitoringWindow::init();
  MonitoringWindow window("BMSUI - Battery Management System User Interface");

  // Enable wanted views of data
  window.enable(MonitoringWindow::Module::SYS_VOLT_GRAPH);
  window.enable(MonitoringWindow::Module::SYS_CURR_GRAPH);
  window.enable(MonitoringWindow::Module::GENERAL_STATS);
  window.enable(MonitoringWindow::Module::CONSOLE);
  window.enable(MonitoringWindow::Module::CELL_VIEW);
  window.enable(MonitoringWindow::Module::SOC);

  window.render();
}
catch (...)
{
  return processException(current_exception());
}
