/** main function definition for client side of Treasure Hunt game **/

#include "Client.hpp"
#include <unistd.h>

int main() {
  TreasureHunt::Client client;

  client.PrintEmptySpace();
  client.PrintGrid();
  sleep(1);
  client.PrintEmptySpace();
  client.PrintGrid();
}