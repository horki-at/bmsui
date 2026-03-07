# BMSUI

Battery management system UI. This program reads input from the BMS via an
USB-adaptor and displays the data on a graphical user interface. This project is
my attempt at assignment for the Software Programmer position at
[TDSR](www.solarracing.nl).

Because the original assignment didn't provide any datasets or access to the
physical BMS, a simple data simulator was created in util. Please see
[usage](#Usage) for choosing either the simulator or the actual device.

Note that at the moment of making this assignment, it is tested on linux
only. Besides tools and requirements are available on linux, but might require
some tweakering or WSL on windows.

# Requirements

# Installation

# Usage

# Further TODOs

This is a list of things that bmsui might benefit from, or those that I haven't
achieved due to the time constraints.

- [ ] A better simulation model for the battery pack.
- [ ] A database collection of some battery cells and battery pack configurations.
- [ ] Ability to store/load the records in/from a local database for future investigation.
- [ ] Ability to dynamically change font size (e.g., with the mouse wheel).
- [ ] Ability to understand when the charging occurs, and with which current.

Expected issues
- [ ] Not entirely accurate model of BMS data (e.g., there is high dependence on
      the load resistance and there might be python bugs that I didn't catch,
      neighbour cell's temperature don't affect the other cells)
- [ ] Changing the order/contents of data received from BMS requires
      modifications in the entire project. I made it simpler due to time
      constraints.

# References

- The Art of Electronics by Horowitz et al.
- Aaron Danner youtube channel for electronics
- Sherman-Morrison formula
- Millman's theorem
