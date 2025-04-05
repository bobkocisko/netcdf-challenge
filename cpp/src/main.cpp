#include "crow.h"
#include "netcdf"

#include "read_netcdf.hpp"

int main(int argc, char *argv[])
{
  // Ensure we have one argument
  const int argument_count = argc - 1;
  if (argument_count < 1) {
    printf("Usage: %s <netCDF file>\n", argv[0]);
    return EXIT_FAILURE;
  }

  const char* file_name = argv[1];

  printf("Running netcdf-api with netCDF file %s\n", file_name);
  fflush(stdout);

  crow::SimpleApp app;

  using namespace netCDF;

  NcFile f(file_name, NcFile::FileMode::read);
  read_netcdf r(f);
  
  CROW_ROUTE(app, "/get-info")([&](){
    return r.get_info();
  });

  app
    .port(8080)
    .multithreaded()  // Based on some experimentation, crow always uses a 
                      // background thread to service requests, but when you
                      // specify `multithreaded()` it will use *all* the 
                      // cpus to service requests.  I'm leaving this
                      // specified here, not because this is a high-throughput
                      // application, but because it's an important indicator
                      // that we have to deal with multithreading concerns.
                      // However! Since we're dealing with a read-only file
                      // we should not have any issues sharing the single
                      // file across threads.
    .run();

  return EXIT_SUCCESS;
}