#include "bms.hh"
#include "monitoring_window.hh"

#include <string>
using namespace std;

int main() {
  // Initialize all the subsystems.
  MonitoringWindow::init();
  
  // Open the stream to the BMS UART input (via UBS adaptor).
  char const *bmsdevice = "./ttyVAPP";
  BMS bms(bmsdevice);

  // Initialize the monitoring window.
  MonitoringWindow window("BMSUI - "s + bmsdevice);

  // Enable wanted views of data
  // window.module_view();         // each cell's temperature & voltage
  // window.general_statistics();  // general statistics (min, max, avg)
  // window.pack_vi();             // display battery pack V and I
  // window.bms_vi();              // display BMS-level V and I
  // window.bms_temp();            // display the BMS PCB temperatures

  // Start rendering the monitoring window
  window.render();
}
