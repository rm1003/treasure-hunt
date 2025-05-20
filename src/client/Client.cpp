#include "Client.hpp"
#include <cstdio>
#include <cstring>

TreasureHunt::Client::Client() {
  this->currentPosition.SetPosition(0, 0);
  this->numberOfFoundTreasures = 0;
  memset(this->WasReached, 0, sizeof(this->WasReached));
}

void TreasureHunt::Client::PrintGrid() {
  for (int i = GRID_SIZE - 1; i >= 0; i--) {
    for (int j = 0; j < GRID_SIZE; j++) {
      if (this->currentPosition.Equal(i, j)) {
        printf("X");
        printf(SPACE);
        continue;
      }

      if (this->WasReached[i][j]) {
        if (this->HasTreasure[i][j]) {
          printf("3");
          printf(SPACE);
        } else {
          printf("2");
          printf(SPACE);
        }
      } else {
        printf("1");
        printf(SPACE);
      }
    }
    printf("\n\n");
  }

  fflush(stdout);
}

void TreasureHunt::Client::PrintEmptySpace() {
  for (int i = 0; i < NUMBER_OF_NEW_LINES; i++) {
    printf("\n");
  }

  fflush(stdout);
}