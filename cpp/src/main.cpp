#include "rest_server.hpp"

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

  rest_server server(8080, file_name);
  server.run_and_wait();

  return EXIT_SUCCESS;
}