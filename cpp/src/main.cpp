#include "crow.h"

int main()
{
  crow::SimpleApp app;

  CROW_ROUTE(app, "/")([](){
    return "Hello you";
  });

  app.port(80).multithreaded().run();  

  return 0;
}