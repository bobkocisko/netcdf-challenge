#include "read_netcdf.hpp"

#include <crow.h>


class rest_server {
  crow::SimpleApp app;
  const char* file_name;

public:
  rest_server(int port, const char* file_name):
    file_name(file_name)
  {
    CROW_ROUTE(app, "/get-info")([=](){
      read_netcdf& r = get_read_netcdf_for_thread();
      return r.get_info();
    });

    CROW_ROUTE(app, "/get-data")([=](const crow::request& req){
      read_netcdf& r = get_read_netcdf_for_thread();
      uint64_t time_index, z_index;

      // 1. Check that the request is valid, and if not return BAD_REQUEST
      try
      {
        time_index = get_url_param_as_uint64(req, "time_index");
        z_index = get_url_param_as_uint64(req, "z_index");

        // Before continuing, make sure the dimensions are valid
        // so that if they are invalid, we will return BAD_RESPONSE
        r.validate_dimension_index("time", time_index);
        r.validate_dimension_index("z", z_index);
      }
      catch (std::exception &e)
      {
        json::wvalue rsp = json::wvalue::object();
        rsp["error"] = e.what();
        return crow::response(crow::status::BAD_REQUEST, rsp);
      }

      // 2. Return the data
      return crow::response(crow::status::OK, 
        r.get_data("concentration", 
          std::vector<uint64_t>({time_index, z_index})));
    });

    app
      .port(port)
      .multithreaded(); // Based on some experimentation, crow always uses a 
                        // background thread to service requests, but when you
                        // specify `multithreaded()` it will use *all* the 
                        // cpus to service requests.  I'm leaving this
                        // specified here, not because this is a high-throughput
                        // application, but because it's an important indicator
                        // that we have to deal with multithreading concerns.
  }

  /**
   * Runs the server and does not return unless the app
   * receives a signal which crow will handle and gracefully
   * shutdown, returning control to the callee.
   */
  void run_and_wait() {
    app.run();
  }

private:

  static std::uint64_t get_url_param_as_uint64(
      const crow::request& req, 
      const char* name) {
    char *val = req.url_params.get(name);
    if (nullptr == val) {
      throw std::invalid_argument(
        std::string("Missing required argument ") + name);
    }
    try {
      int64_t result = std::stoll(val);
      if (result < 0) {
        throw std::invalid_argument("Negative values not allowed");
      }
      return result;
    } catch (std::exception& e) {
      throw std::invalid_argument(
        std::string("Invalid argument ") + name + 
        ": " + e.what());
    }
  }

  /**
   * Return the read_netcdf instance unique to the current thread.
   * NOTE: due to the concerns mentioned here...
   *   https://github.com/Unidata/netcdf-c/issues/1373#issuecomment-637794942
   *  ...we need to make sure that each NcFile instance is managed
   *  by no more than one thread.  To accomplish this we are using
   *  thread_local storage so that each thread has its own instance.
   *  It is accepted that this will cause a slowdown when each thread
   *  is invoked for the first time.
   */
  read_netcdf& get_read_netcdf_for_thread() {
    thread_local std::unique_ptr<read_netcdf> reader = nullptr;    
    if (reader == nullptr) {
      std::cout 
        << "Creating new read_netcdf for thread " 
        << std::this_thread::get_id() << std::endl;
      reader = std::make_unique<read_netcdf>(file_name);
    } else {
      std::cout
          << "Re-using read_netcdf already created for thread "
          << std::this_thread::get_id() << std::endl;
    }
    return *(reader.get());
  }  
};
