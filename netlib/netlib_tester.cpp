#include "hstp.hpp"
#include <iostream>

int main(int argc, char **argv)
{
  std::cout << "CLI executable for testing the networking library.\n";

  HstpHandler handlr;
  HstpHandlerTester handlr_tester;

  handlr_tester.test_serialize(handlr);

  return 0;
}
