#include "crow.h"

int main()
{
  crow::SimpleApp app;

  CROW_ROUTE(app, "/get-info")([](){
    return "Hello you";
  });

  app.port(8080).multithreaded().run();  

  return 0;
}