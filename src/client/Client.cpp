#include "Client.hpp"
#include "../libs/Logging.hpp"
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <filesystem>

namespace fs = std::filesystem;

void PrintErrorMsgType(CustomProtocol::MsgType msg, const char *location) {
  ERROR_PRINT("Unexpected message type [%d] in [%s]\n", msg, location);
}

TreasureHunt::Client::Client() {
  this->currentPosition.SetPosition(INI_X, INI_Y);
  this->numberOfFoundTreasures = 0;
  memset(this->wasReached, 0, sizeof(this->wasReached));
  this->wasReached[INI_X][INI_Y] = true;
  this->hasTreasure[INI_X][INI_Y] = false;
  this->filePath[0] = '\0';
  strcpy(filePath, TREASURES_DIR);
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
  char *treasureName = &this->filePath[sizeof(TREASURES_DIR) - 1];
  netHandler.SendGenericData(mov, NULL, 0);
  ret = netHandler.RecvGenericData((void*)treasureName, NULL);
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

  DEBUG_PRINT("Raw treasureFileName [%s]\n", this->treasureFileName);

  this->numberOfFoundTreasures++;

  return TREASURE_FOUND;
}

void TreasureHunt::Client::GetServerTreasure() {
  MsgType msgRet;
  fs::space_info spaceInfo;
  int intRet;
  size_t fileSize;
  size_t availableSize;
  size_t dataLen;
  static unsigned char data[CustomProtocol::DATA_SIZE];

  /* considering that player already moved to new location */
  this->hasTreasure[this->currentPosition.x][this->currentPosition.y] = true;

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
    ERROR_PRINT("Available [%lu]; Requested [%lu]\n", availableSize, fileSize);
    this->netHandler.SendResponse(CustomProtocol::ERROR);
    ERROR_PRINT("Warned server that file size is incompatible. Exiting.\n");
    exit(1);
  }
  this->netHandler.SendResponse(CustomProtocol::ACK);

  this->buffer.OpenFileForWrite(filePath);
  do {
    msgRet = this->netHandler.RecvGenericData((void*)data, &dataLen);
    if (msgRet == CustomProtocol::END_OF_FILE) {
      this->buffer.FlushBuffer();
      this->buffer.CloseFile();
      if (numberOfFoundTreasures == TOTAL_TREASURES) {

      }
    } else if (msgRet == CustomProtocol::DATA) {
      intRet = this->buffer.AppendToBuffer(data, dataLen);
      if (intRet == Data::APPEND_IMPOSSIBLE)
        this->buffer.FlushBuffer();
    } else {
      PrintErrorMsgType(msgRet, "GetServerTreasure");
      exit(1);
    }

    this->netHandler.SendResponse(CustomProtocol::ACK);
  } while(msgRet != CustomProtocol::END_OF_FILE);

  this->netHandler.InvertToSender();
}

int TreasureHunt::Client::ShowTreasure() {
  int ret;
  std::string command;

  switch (this->treasureType) {
    case MP4:
      command = MP4_PLAYER + this->filePath;
      ret = std::system(command.c_str());
    case JPG:
      command = JPG_PLAYER + this->filePath;
      ret = std::system(command.c_str());
    case TXT:
      command = TXT_PLAYER + this->filePath;
      ret = std::system(command.c_str());
    default:
      ERROR_PRINT("Unknown file type. Exiting.\n");
      exit(1);
  }
  return ret;
}

void TreasureHunt::Client::Move(MsgType mov) {
  switch (mov) {
    case CustomProtocol::MOVE_LEFT:
      this->currentPosition.MoveLeft();
      break;
    case CustomProtocol::MOVE_RIGHT:
      this->currentPosition.MoveRight();
      break;
    case CustomProtocol::MOVE_UP:
      this->currentPosition.MoveUp();
      break;
    case CustomProtocol::MOVE_DOWN:
      this->currentPosition.MoveDown();
      break;
    default:
      ERROR_PRINT("Invalid movement in [Client::Move]. Exiting.\n");
      exit(1);
  }
  this->wasReached[this->currentPosition.x][this->currentPosition.y] = true;
}