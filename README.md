# DemoLib2
C++ Re-implementation of the complete demo and network parsing codeset for Team Fortress 2.

This codebase needs ~~some love~~ significant work, but it should at least provide something to get you going if you're interested in building something that needs to parse demo files for Team Fortress 2.

## Building
This project uses the [snappy](https://github.com/google/snappy) library to decode parts of the network stream. Compiling snappy requires [CMake](https://cmake.org/download/).

Assuming you're on Windows, make sure to run `build_snappy.bat` to set up snappy. Then you should be able to open up DemoLib2.sln and hack away.

The TF2PubStats and DemoLib2_Test projects are bascially examples, but they're not documented. Sorry.