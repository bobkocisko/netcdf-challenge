## Introduction
This repository is my attempt at solving a programming challenge related to the netCDF file format.
It contains two sections as follows:

## cpp
A c++ REST API that serves information about the specified netCDF file.  Currently this is configured to use a netCDF file present in the source, but since it uses the command line to specify the desired netCDF file, this could be rather easily reconfigured to work for other netCDF files as well.

## python
This python project is simply here to track my usage of python's netcdf4 plugin to examine the netCDF file provided and print general information.  I decided to use python initially for this because I figured it would be a quick path to iterate around browsing the data without waiting to setup the dependencies and compilation environment of c++, and without depending on the compile/run cycle.

## Setup
This repository uses a combination of git submodules and docker to manage dependencies
and to provide the dev container for the VS Code ide.

1. `git clone --recurse-submodules git@github.com:bobkocisko/netcdf-challenge.git`

2. `cd cpp`

2. `docker-compose up --watch`

3. Test the endpoints available:
   - http://localhost:8080/get-info
   - http://localhost:8080/get-data?time=1&z=1
   - http://localhost:8080/get-image?time=1&z=1

## Reference

### netCDF file format

[Fundamentals of netCDF data storage](https://pro.arcgis.com/en/pro-app/latest/help/data/imagery/fundamentals-of-netcdf.htm)

[NetCDF: Introduction and Overview](https://docs.unidata.ucar.edu/netcdf-c/current/index.html)

[The NetCDF User's Guide](https://docs.unidata.ucar.edu/nug/current/index.html)

### netCDF library usage

[netCDF C++ Interface Guide](https://docs.unidata.ucar.edu/netcdf-cxx/current/index.html)

[NetCDF: The NetCDF-C Tutorial](https://docs.unidata.ucar.edu/netcdf-c/current/tutorial_8dox.html)

[NetCDF: Reading NetCDF/HDF5 Format NetCDF Files of Unknown Structure](https://docs.unidata.ucar.edu/netcdf-c/current/reading_unknown_nc4.html)

### Crow REST library

[Crow REST library docs](https://crowcpp.org/master/getting_started/setup/linux/)

### Docker reference

[PID 1 Signal Handling in Docker](https://petermalmgren.com/signal-handling-docker/)