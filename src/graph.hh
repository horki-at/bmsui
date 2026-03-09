#ifndef INCLUDED_GRAPH_
#define INCLUDED_GRAPH_

#include <algorithm>
#include <array>
#include <tuple>
#include <limits>

template <size_t Size,
          float ExpectedMin = std::numeric_limits<float>::max(),
          float ExpectedMax = std::numeric_limits<float>::min()>
class Graph
{
  std::array<float, Size> d_x, d_y;

  float d_min = ExpectedMin;
  float d_max= ExpectedMax;

public:
  Graph() = default;

  void add(float xval, float yval); // Add a new (x;y) datapoint.

  void clear();                 // Clear the data in each graph.

  float min() const;
  float max() const;

  float const *x() const;
  float const *y() const;
};

template <size_t Size, float ExpectedMin, float ExpectedMax>
void Graph<Size, ExpectedMin, ExpectedMax>::clear()
{
  std::fill(d_x.begin(), d_x.begin(), 0.0f);
  std::fill(d_y.begin(), d_y.begin(), 0.0f);
  d_min = ExpectedMin;
  d_max = ExpectedMax;
}

template <size_t Size, float ExpectedMin, float ExpectedMax>
void Graph<Size, ExpectedMin, ExpectedMax>::add(float xval, float yval)
{
  for (size_t i = 1; i != Size; ++i)
  {
    d_x[i - 1] = d_x[i];
    d_y[i - 1] = d_y[i];
  }
  d_x[Size - 1] = xval;
  d_y[Size - 1] = yval;

  std::tie(d_min, d_max) = std::minmax({d_min, d_max, yval});
}

template <size_t Size, float ExpectedMin, float ExpectedMax>
inline float const *Graph<Size, ExpectedMin, ExpectedMax>::x() const
{
	return d_x.data();
}

template <size_t Size, float ExpectedMin, float ExpectedMax>
inline float const *Graph<Size, ExpectedMin, ExpectedMax>::y() const
{
  return d_y.data();	
}

template <size_t Size, float ExpectedMin, float ExpectedMax>
inline float Graph<Size, ExpectedMin, ExpectedMax>::min() const
{
	return d_min;
}

template <size_t Size, float ExpectedMin, float ExpectedMax>
inline float Graph<Size, ExpectedMin, ExpectedMax>::max() const
{
	return d_max;
}

#endif // INCLUDED_GRAPH_
