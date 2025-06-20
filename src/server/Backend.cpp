/** main function definition for server side of Treasure Hunt game **/

#include "Server.hpp"

using namespace TreasureHunt;

int main(int argc, char **argv) {
  TreasureHunt::Server server;
  int ret;

  server.ReadTreasures();
  server.FillHasTreasureArray();
  do {
    ret = server.GetClientMovement(); /* obter movimento */
    if (ret == TREASURE_FOUND) {
      server.SendTreasure(); /* mandar tesouro */
    }
    server.PrintClientPosition(); /* imprimir posicao cliente */
  } while(!server.GameEnded());
}