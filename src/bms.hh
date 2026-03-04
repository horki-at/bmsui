#ifndef INCLUDED_BMS_
#define INCLUDED_BMS_

#include <fstream>
#include <array>

class BMS
{
public:
  static constexpr size_t modules = 4;
  static constexpr size_t cells = 40;
  static constexpr size_t temps = 12;
  static constexpr size_t pcbs = 4;
  static constexpr char sep = ',';
  
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

  bool next();
  Data const &data() const;
};

inline BMS::BMS(char const *device)
:
  d_in(device),
  d_currentData()
{
}

inline bool BMS::next()
{
  d_in >> d_currentData;
  return static_cast<bool>(d_in);
}

inline BMS::Data const &BMS::data() const
{
	return d_currentData;
}

#endif // INCLUDED_BMS_
