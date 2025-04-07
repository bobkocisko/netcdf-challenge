cmake_minimum_required(VERSION 3.10)
project(NetCDF_API)

find_package(Crow REQUIRED)
find_package(Matplot++ REQUIRED)
find_package(nlohmann_json 3.11.3 REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(NETCDF REQUIRED netcdf-cxx4)


file(GLOB SOURCES "./*.cpp")

add_executable(netcdf_api ${SOURCES})

target_link_libraries(netcdf_api PUBLIC 
  Crow::Crow 
  Matplot++::matplot
  nlohmann_json::nlohmann_json
  ${NETCDF_LIBRARIES})

target_include_directories(netcdf_api PUBLIC 
  ${NETCDF_INCLUDE_DIRS})

install(TARGETS netcdf_api)