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

3. `docker compose up -d` *(start containers in background)*

4. `docker compose watch netcdf-api` *(automatically rebuild and restart container on any code changes, and display build output)*

4. Test the endpoints available:
   - http://localhost:8080/get-info
   - http://localhost:8080/get-data?time=1&z=1
   - http://localhost:8080/get-image?time=1&z=1

5. To get full intellisense support in VSCode:

   1. Choose **Attach to running container...**
   2. Then select **/netcdf-api-ide** from the options

## Reference

### netCDF file format

[Fundamentals of netCDF data storage](https://pro.arcgis.com/en/pro-app/latest/help/data/imagery/fundamentals-of-netcdf.htm)

[NetCDF: Introduction and Overview](https://docs.unidata.ucar.edu/netcdf-c/current/index.html)

[The NetCDF User's Guide](https://docs.unidata.ucar.edu/nug/current/index.html)

### netCDF library usage

[netCDF C++ Interface Guide](https://docs.unidata.ucar.edu/netcdf-cxx/current/index.html)

[NetCDF: The NetCDF-C Tutorial](https://docs.unidata.ucar.edu/netcdf-c/current/tutorial_8dox.html)

[NetCDF: Reading NetCDF/HDF5 Format NetCDF Files of Unknown Structure](https://docs.unidata.ucar.edu/netcdf-c/current/reading_unknown_nc4.html) *(groups/types don't appear to be present for the sample file we are working with)*

[ncdump.c: handling NC_CHAR data type](https://github.com/Unidata/netcdf-c/blob/main/ncdump/ncdump.c#L414) *(source code for ncdump utility, this link is to a particular function that parses NC_CHAR data types)*

[Memory management for NC_STRING types](https://docs.unidata.ucar.edu/netcdf-c/4.9.3/group__attributes.html#ga19cae92a58e1bf7f999c3eeab5404189)

### Crow REST library

[Crow REST library docs](https://crowcpp.org/master/getting_started/setup/linux/)

### Docker reference

[Setting up a dev environment with Docker/Compose](https://youtu.be/0H2miBK_gAk?si=0iP6uQveDUoeohpw)

[Understanding multi-stage builds](https://stackoverflow.com/questions/69011431/building-and-deploying-c-through-docker-multistage-build-vs-mount)

[PID 1 Signal Handling in Docker](https://petermalmgren.com/signal-handling-docker/)