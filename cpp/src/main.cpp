#include "crow.h"
#include "netcdf"

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

  NcFile a(file_name, NcFile::FileMode::read);
  
  CROW_ROUTE(app, "/get-info")([&a](){
    return "Group count: " + std::to_string(a.getGroupCount());
  });

  app
    .port(8080)
    .multithreaded()  // Based on some experimentation, crow always uses a 
                      // background thread to service requests, but when you
                      // specify `multithreaded()` it will use *all* the 
                      // threads to service requests.  I'm leaving this
                      // specified here, not because this is a high-throughput
                      // application, but because it's an important indicator
                      // that we have to deal with multithreading concerns.
    .run();

  return EXIT_SUCCESS;
}