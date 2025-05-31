/** main function definition for client side of Treasure Hunt game **/

#include "Client.hpp"
#include <cstdio>
#include <unistd.h>
#include <termios.h>

using namespace TreasureHunt;

bool IsNotMovementKey(char key) {
  return (key != 'w' && key != 'a' && key != 's' && key != 'd');
}

int main(int argc, char *argv[]) {
  struct termios newConfT;
  struct termios oldConfT;
  TreasureHunt::Client client;
  char key;
  int ret;

  tcgetattr(STDIN_FILENO, &oldConfT);
  newConfT = oldConfT;
  newConfT.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newConfT);

  client.PrintEmptySpace();
  client.PrintGrid();
  while (1) {
    do {
      key = getchar();
    } while (IsNotMovementKey(key));

    switch(key) {
      case 'a':
        ret = client.InformServerMovement(CustomProtocol::MOVE_LEFT);
        if (ret == INVALID_MOVE) continue;
        client.Move(CustomProtocol::MOVE_LEFT);
        break;
      case 'w':
        ret = client.InformServerMovement(CustomProtocol::MOVE_UP);
        if (ret == INVALID_MOVE) continue;
        client.Move(CustomProtocol::MOVE_UP);
        break;
      case 's':
        ret = client.InformServerMovement(CustomProtocol::MOVE_DOWN);
        if (ret == INVALID_MOVE) continue;
        client.Move(CustomProtocol::MOVE_DOWN);
        break;
      case 'd':
        ret = client.InformServerMovement(CustomProtocol::MOVE_RIGHT);
        if (ret == INVALID_MOVE) continue;
        client.Move(CustomProtocol::MOVE_RIGHT);
        break;
    }

    client.PrintEmptySpace();
    client.PrintGrid();

    if (ret == TREASURE_FOUND) {
      client.GetServerTreasure();
      client.ShowTreasure();
    }
  }

  tcsetattr(STDIN_FILENO, TCSANOW, &oldConfT);

  return 0;
}