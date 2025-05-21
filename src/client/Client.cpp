#include "Client.hpp"
#include "../libs/Logging.hpp"
#include <cstdio>
#include <cstring>

TreasureHunt::Client::Client() {
  this->currentPosition.SetPosition(INI_X, INI_Y);
  this->numberOfFoundTreasures = 0;
  memset(this->wasReached, 0, sizeof(this->wasReached));
  this->wasReached[INI_X][INI_Y] = true;
}

void TreasureHunt::Client::PrintGrid() {
  for (int i = GRID_SIZE - 1; i >= 0; i--) {
    for (int j = 0; j < GRID_SIZE; j++) {
      if (this->currentPosition.Equal(i, j)) {
        printf("X");
        printf(SPACE);
        continue;
      }

      if (this->wasReached[i][j]) {
        if (this->hasTreasure[i][j]) {
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

int TreasureHunt::Client::InformServerMovement(MsgType mov) {
  MsgType ret;
  netHandler.SendGenericData(mov, NULL, 0);
  ret = netHandler.RetrieveData(NULL, 0);
  switch (ret) {
    case CustomProtocol::OK_AND_ACK:
      return VALID_MOVE;
    case CustomProtocol::ACK:
      return INVALID_MOVE;
    case CustomProtocol::TXT_FILE_NAME_ACK:
      netHandler.RetrieveData((void**)&this->treasureFileName, NULL);
      return TREASURE_FOUND;
    case CustomProtocol::IMG_FILE_NAME_ACK:
      netHandler.RetrieveData((void**)&this->treasureFileName, NULL);
      return TREASURE_FOUND;
    case CustomProtocol::VIDEO_FILE_NAME_ACK:
      netHandler.RetrieveData((void**)&this->treasureFileName, NULL);
      return TREASURE_FOUND;
    default:
      ERROR_PRINT("Not expected return type. Exiting.\n");
      exit(1);
  }
}

void TreasureHunt::Client::GetServerTreasure() {
  MsgType ret;
  this->netHandler.RecvGenericData(CustomProtocol::WAIT_FOR_VALID_MESSAGE);
  this->netHandler.RetrieveData((void**)&this->fileSize, NULL);
  this->netHandler.SendAcknowledgement(CustomProtocol::ACK);
  this->netHandler.RecvFile(this->treasureFileName);

  delete this->treasureFileName;
}