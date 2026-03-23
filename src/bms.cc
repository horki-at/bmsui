#include "bms.hh"
#include <sstream>
using namespace std;

istream &operator>>(istream &in, BMS::Data &data)
{
  string line; if (not getline(in, line))
    return in;

  stringstream ss(line);
  string token;
  
  auto nextValue = [&]() -> float {
    if (getline(ss, token, BMS::sep))
    {
      if (token.empty()) return 0.0f;
      else               return stof(token);
    }
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

bool BMS::open_device(std::string const &device)
{
  d_in.close();                 // close previously opened streams
  d_in.open(device);            // try opening the device

  if (not d_in.is_open())
    return false;

  // start the producing of data
  d_producer = jthread{[this](){
    while (next())
      d_buffer.push(move(d_currentData));
  }};
  
  return true;
}

//static
size_t BMS::cell_id(size_t module_idx, size_t parallel_idx, size_t series_idx)
{
  size_t const inner_idx = BMS::series * parallel_idx + series_idx;
  size_t const outer_idx = BMS::cells * module_idx + inner_idx;
  return outer_idx;
}

//static
size_t BMS::temp_id(size_t module_idx, size_t parallel_idx, size_t series_idx)
{
  // NOTE: @important This function must be implemented in accordance with
  // the way the temperature sensor is actually connected in the battery
  // pack. Right now, I map the linear index of each cell to consecutive
  // temperature sensor. In practice, this probably won't be the case.
  size_t const cell_idx = BMS::cell_id(module_idx, parallel_idx, series_idx);
  size_t const cell_idx_in_module = cell_idx % BMS::cells;
  size_t const sensor_in_module = (cell_idx_in_module * temps) / cells;
  return module_idx * BMS::temps + sensor_in_module;
}
