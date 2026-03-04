#include "bms.hh"
#include <iostream>
using namespace std;

int main()
{
  BMS bms("./ttyVAPP");
  while (bms.next()) cout << "The SoC is " << bms.data().soc << "\n";
}
