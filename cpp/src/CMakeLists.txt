cmake_minimum_required(VERSION 3.10)
project(NetCDF_API)

find_package(Crow)

file(GLOB SOURCES "./*.cpp")

add_executable(netcdf_api ${SOURCES})

target_link_libraries(netcdf_api PUBLIC Crow::Crow)

install(TARGETS netcdf_api)