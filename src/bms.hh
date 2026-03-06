#ifndef INCLUDED_BMS_
#define INCLUDED_BMS_

#include <fstream>
#include <array>

class BMS
{
public:
  enum Mode { CHARGING, DISCHARGING, IDLE, N_MODES };
  enum TempType { TOO_COLD, OPTIMAL, WARNING, TOO_HOT, N_TEMP_TYPES };
  
  // Battery Pack and Battery Management System Schematics/Datasheet Information
  static constexpr size_t modules = 4;
  static constexpr size_t cells = 40;
  static constexpr size_t parallel = 4; // how many cells (per module) are in parallel
  static constexpr size_t series = 10; // how many cells (per module) are in series
  static_assert(parallel * series == cells, "Wrong combination of parallel/series battery cells.");
  static constexpr size_t temps = 12;
  static constexpr size_t pcbs = 4;
  static constexpr char sep = ',';
  static constexpr float cellTempInfo[N_MODES][N_TEMP_TYPES] = {
    {0.0, 25.0f, 40.0f, 55.0f},
    {-20.0f, 25.0f, 40.0f, 60.0f},
    {-20.0f, 22.0f, 35.0f, 55.0f}
  };
  
  struct Data
  {
    friend std::istream &operator>>(std::istream &in, Data &data);
    
    std::array<float, modules * cells> cellVolts;
    std::array<float, modules * temps> cellTemps;
    std::array<float, pcbs> pcbTemps;
    float humidity, soc;
    float voltLow, voltPack, currLow, currPack;
    float batAvgTemp, maxTemp, minTemp, maxBMSTemp, minBMSTemp, avgTemp;
    float maxCellVolt, minCellVolt;
  };
  
private:
  std::ifstream d_in;           // input stream for the BMS device (eg via USB)
  Data d_currentData;
  
public:
  BMS(char const *device);
  BMS(BMS const &) = delete;
  BMS(BMS &&) = delete;
  BMS &operator=(BMS const &) = delete;
  BMS &operator=(BMS &&) = delete;
  ~BMS() = default;

  bool next();
  Data data() const;

  // Get linear battery cell index.
  static size_t cell_id(size_t module_idx, size_t parallel_idx, size_t series_idx);

  // Get linear temperature sensor index. It corresponds to all the cells that
  // are close-by.
  static size_t temp_id(size_t module_idx, size_t parallel_idx, size_t series_idx);
};

inline BMS::BMS(char const *device)
:
  d_in(device),
  d_currentData()
{
  // TODO: what if device was not found?
}

inline bool BMS::next()
{
  d_in >> d_currentData;
  return static_cast<bool>(d_in);
}

inline BMS::Data BMS::data() const
{
	return std::move(d_currentData);
}

#endif // INCLUDED_BMS_
