#include "read_netcdf.hpp"

#include <crow.h>

read_netcdf& get_nc_reader_for_thread(const char* file_name);

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

  CROW_ROUTE(app, "/get-info")([=](){
    read_netcdf& r = get_nc_reader_for_thread(file_name);
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
    .run();

  return EXIT_SUCCESS;
}

// NOTE: due to the concerns mentioned here...
//   https://github.com/Unidata/netcdf-c/issues/1373#issuecomment-637794942
//  ...we need to make sure that each NcFile instance is managed
//  by no more than one thread.  To accomplish this we are using
//  thread_local storage so that each thread has its own instance.
//  It is accepted that this will cause a slowdown when each thread
//  is invoked for the first time.
thread_local std::unique_ptr<read_netcdf> nc_reader = nullptr;

// Return the nc_reader instance unique to the current thread.
read_netcdf& get_nc_reader_for_thread(const char* file_name) {
  if (nc_reader == nullptr) {
    std::cout 
      << "Creating new read_netcdf for thread " 
      << std::this_thread::get_id() << std::endl;
    nc_reader = std::make_unique<read_netcdf>(file_name);
  } else {
    std::cout
        << "Re-using read_netcdf already created for thread "
        << std::this_thread::get_id() << std::endl;
  }
  return *(nc_reader.get());
}