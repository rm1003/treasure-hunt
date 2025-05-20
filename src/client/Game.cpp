/** main function definition for client side of Treasure Hunt game **/

#include "Client.hpp"
#include <cstdio>
#include <unistd.h>
#include <termios.h>

bool IsNotMovementKey(char key) {
  return (key != 'w' && key != 'a' && key != 's' && key != 'd');
}

int main() {
  struct termios newConfT;
  struct termios oldConfT;
  TreasureHunt::Client client;
  char key;

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
        client.currentPosition.MoveLeft();
        break;
      case 'w':
        client.currentPosition.MoveUp();
        break;
      case 's':
        client.currentPosition.MoveDown();
        break;
      case 'd':
        client.currentPosition.MoveRight();
        break;
    }

    client.PrintEmptySpace();
    client.PrintGrid();
  }

  tcsetattr(STDIN_FILENO, TCSANOW, &oldConfT);
}