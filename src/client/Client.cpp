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

const char *FormatFileName(char *dest, const char *path, const char *name) {

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
  ret = netHandler.RetrieveData(NULL, 0);
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
    this->netHandler.RetrieveData((void**)&this->treasureFileName, NULL);
  }

  this->netHandler.InvertToReceiver();

  return TREASURE_FOUND;
}

int TreasureHunt::Client::GetServerTreasure() {
  MsgType msgRet;
  int intRet;
  size_t *fileSize;
  fs::space_info spaceInfo;
  unsigned char *data;
  size_t dataLen;
  static bool firstCall = true;

  /* get file size and test if there is enough disk space */
  while (1) {
    intRet = this->netHandler.RecvGenericData(CustomProtocol::WAIT_FOR_VALID_MESSAGE);
    if (intRet != CustomProtocol::REPEATED_MSG) break;
    if (firstCall) {
      /* last message was INVERT_REMINDER */
      this->netHandler.SendGenericData(CustomProtocol::INVERT, NULL, 0);
      firstCall = false;
    } else {
      /* last message was FILE_SIZE */
      this->netHandler.SendGenericData(CustomProtocol::INVERT, NULL, 0);
    }
  }
  msgRet = this->netHandler.RetrieveData((void**)&fileSize, NULL);
  if (msgRet != CustomProtocol::FILE_SIZE) {
    PrintErrorMsgType(msgRet, "GetServerTreasure/2nd test");
    exit(1);
  }

  spaceInfo = fs::space(TREASURES_DIR);
  if (spaceInfo.available < *fileSize) {
    PrintErrorNoSpace(spaceInfo.available, *fileSize);
    delete fileSize;
    delete treasureFileName;
    /* warn server that client does not have enough disk space */
    this->netHandler.SendGenericData(CustomProtocol::ERROR,
                                     (void*)CustomProtocol::NO_SPACE_ERROR,
                                     sizeof(CustomProtocol::NO_SPACE_ERROR));
    return 1;
  }
  this->netHandler.SendAcknowledgement(CustomProtocol::ACK);

  /* considering that treasureFileName is already complete path */
  this->buffer.OpenFileForWrite(treasureFileName);
  /* loop until END_OF_FILE message */
  while (1) {
    this->netHandler.RecvGenericData(CustomProtocol::WAIT_FOR_VALID_MESSAGE);
    intRet = this->netHandler.RetrieveData((void**)&data, &dataLen);
    /* continue if repeated */
    if (intRet == CustomProtocol::REPEATED_MSG) {
      this->netHandler.SendAcknowledgement(CustomProtocol::ACK);
      continue;
    }
    if (intRet == CustomProtocol::END_OF_FILE) {
      break;
    }

    /* append data to buffer */
    intRet = this->buffer.AppendToBuffer(data, dataLen);
    if (intRet == Data::APPEND_IMPOSSIBLE) {
      this->buffer.FlushBuffer();
    }
  }

  this->buffer.FlushBuffer();
  this->buffer.CloseFile();
  delete fileSize;

  return 0;
}