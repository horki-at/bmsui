#include "bms.hh"
using namespace std;

istream &operator>>(istream &in, BMS::Data &data)
{
  auto nextValue = [&in]() -> float
    {
      if (string line; getline(in, line, BMS::sep))
        return stof(line);
      return 0.0f;
    };

  // Read BMS data in the right order.
  for (float &val : data.cellVolts) val = nextValue();
  for (float &val : data.cellTemps) val = nextValue();
  data.batAvgTemp = nextValue();
  for (float &val : data.pcbTemps) val = nextValue();
  data.humidity = nextValue();
  data.voltLow = nextValue();
  data.voltPack = nextValue();
  data.currLow = nextValue();
  data.currPack = nextValue();
  data.maxTemp = nextValue();
  data.minTemp = nextValue();
  data.maxBMSTemp = nextValue();
  data.minBMSTemp = nextValue();
  data.avgTemp = nextValue();
  data.maxCellVolt = nextValue();
  data.minCellVolt = nextValue();
  data.soc = nextValue();

  return in;
}
