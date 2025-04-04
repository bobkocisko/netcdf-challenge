### Overview
This repository is my attempt at solving a programming challenge related to the netCDF file format.
It contains two sections as follows:

#### cpp
A c++ REST API that serves information about the specified netCDF file.  Currently this is configured to use a netCDF file present in the source, but since it uses the command line to specify the desired netCDF file, this could be rather easily reconfigured to work for other netCDF files as well.

#### python
This python project is simply here to track my usage of python's netcdf4 plugin to examine the netCDF file provided and print general information.  I decided to use python initially for this because I figured it would be a quick path to iterate around browsing the data without waiting to setup the dependencies and compilation environment of c++, and without depending on the compile/run cycle.
