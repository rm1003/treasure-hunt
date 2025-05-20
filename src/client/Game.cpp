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
    printf("%c\n", key);
  }

  tcsetattr(STDIN_FILENO, TCSANOW, &oldConfT);
}