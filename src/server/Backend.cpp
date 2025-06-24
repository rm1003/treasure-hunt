/** main function definition for server side of Treasure Hunt game **/

#include "Server.hpp"
#include "../libs/Logging.hpp"

using namespace TreasureHunt;

int main(int argc, char **argv) {
  TreasureHunt::Server server;
  int ret;

  server.ReadTreasures();
  server.SetTreasurePositions();
  do {
    printf("\nWaiting for next move...\n");
    ret = server.GetClientMovement();
    printf("Got it!: ");
    switch (ret) {
      case INVALID_MOVE:
        printf("Client tried to make invalid move!\n");
        break;
      case VALID_MOVE:
        printf("Valid move!\n");
        break;
      case TREASURE_FOUND:
        printf("Client found treasure! [%d] found.\n", server.GetTotalFound());
        break;
      default:
        ERROR_PRINT("Invalid ret. Exiting\n");
        exit(1);
    }
    server.PrintClientPosition();
    server.RespondToClient(ret);
    if (ret == TREASURE_FOUND) {
      server.SendTreasure();
    }
  } while(!server.GameEnded());
}