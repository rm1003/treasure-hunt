#include "Client.hpp"
#include "../libs/Logging.hpp"
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <filesystem>

namespace fs = std::filesystem;

void PrintErrorMsgType(CustomProtocol::MsgType msg, const char *location) {
  ERROR_PRINT("Unexpected message type [%d] in [%s]\n", msg, location);
}

void PrintErrorNoSpace(size_t free, size_t len) {
  ERROR_PRINT("Not enough space available [%lu] to store [%lu]\n", free, len);
}

TreasureHunt::Client::Client() {
  this->currentPosition.SetPosition(INI_X, INI_Y);
  this->numberOfFoundTreasures = 0;
  memset(this->wasReached, 0, sizeof(this->wasReached));
  this->wasReached[INI_X][INI_Y] = true;
  this->hasTreasure[INI_X][INI_Y] = false;
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
  ret = netHandler.RecvGenericData((void*)this->treasureFileName, NULL);
  if (ret == CustomProtocol::OK_AND_ACK) {
    return VALID_MOVE;
  } else if (ret == CustomProtocol::ACK) {
    return INVALID_MOVE;
  } else {
    switch (ret) {
      case CustomProtocol::TXT_FILE_NAME_ACK:
        this->treasureType = TXT;
        break;
      case CustomProtocol::IMG_FILE_NAME_ACK:
        this->treasureType = JPG;
        break;
      case CustomProtocol::VIDEO_FILE_NAME_ACK:
        this->treasureType = MP4;
        break;
      default:
        ERROR_PRINT("Not expected return type. Exiting.\n");
        exit(1);
    }
  }
  this->netHandler.InvertToReceiver();

  return TREASURE_FOUND;
}

int TreasureHunt::Client::GetServerTreasure() {
  MsgType msgRet;
  fs::space_info spaceInfo;
  int intRet;
  size_t fileSize;
  size_t availableSize;
  size_t dataLen;
  static unsigned char data[CustomProtocol::DATA_SIZE];
  static bool firstCall = true;

  /* get file size and test if there is enough disk space */
  msgRet = this->netHandler.RecvGenericData((void*)data, NULL);
  if (msgRet != CustomProtocol::FILE_SIZE) {
    PrintErrorMsgType(msgRet, "GetServerTreasure/1st test");
    exit(1);
  }
  fileSize = *(size_t *)(data);

  spaceInfo = fs::space(TREASURES_DIR);
  availableSize = spaceInfo.available;
  if (fileSize > availableSize) {
    this->netHandler.SendResponse(CustomProtocol::ERROR);
    ERROR_PRINT("Warned server that file size is incompatible. Exiting.");
    exit(1);
  }
  this->netHandler.SendResponse(CustomProtocol::ACK);

  /* considering that treasureFileName is already complete path */
  this->buffer.OpenFileForWrite(treasureFileName);
  while (1) {
    msgRet = this->netHandler.RecvGenericData((void*)data, &dataLen);
    if (msgRet == CustomProtocol::END_OF_FILE) {
      this->buffer.FlushBuffer();
      this->buffer.CloseFile();
      break;
    }
    /* append data to buffer */
    intRet = this->buffer.AppendToBuffer(data, dataLen);
    if (intRet == Data::APPEND_IMPOSSIBLE) {
      this->buffer.FlushBuffer();
    }
    this->netHandler.SendResponse(CustomProtocol::ACK);
  }

  return 0;
}

void TreasureHunt::Client::ShowTreasure() {

}