#include "ring.hh"
#include "bms.hh"
#include "monitoring_window.hh"

#include <iostream> // Remove this
#include <thread>
#include <string>
using namespace std;

int main()
{
  // Initialize all the subsystems.
  MonitoringWindow::init();
  
  // Open the stream to the BMS UART input (via UBS adaptor).
  char const *bmsdevice = "./ttyVAPP";
  BMS bms(bmsdevice);

  // Initialize the monitoring window.
  MonitoringWindow window("BMSUI - "s + bmsdevice);

  // Enalbe wanted views of data
  window.enable(MonitoringWindow::Module::DEMO);

  // Render the monitoring window
  Ring<BMS::Data, 64> ring;
  jthread producer{[&]()
  {
    bms.next();                 // TODO: what if this is EOF?
    ring.push(bms.data());
  }};

  // Start rendering the monitoring window
  window.render(ring);
}
