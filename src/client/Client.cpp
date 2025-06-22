#include "Client.hpp"
#include "../libs/Logging.hpp"
#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <filesystem>

namespace fs = std::filesystem;

static void PrintErrorMsgType(CustomProtocol::MsgType msg, const char *location) {
  ERROR_PRINT("Unexpected message type [%d] in [%s]\n", msg, location);
}

TreasureHunt::Client::Client() {
  this->user = std::getenv("SUDO_USER");
  this->currentPosition.SetPosition(INI_X, INI_Y);
  this->numberOfFoundTreasures = 0;
  memset(this->wasReached, 0, sizeof(this->wasReached));
  memset(this->hasTreasure, 0, sizeof(this->hasTreasure));
  this->wasReached[INI_X][INI_Y] = true;
  this->hasTreasure[INI_X][INI_Y] = false;
  this->filePath[0] = '\0';
  strcpy(filePath, TREASURES_DIR);

  printf("\n\nWelcome to TreasureHunt game %s!\n", this->user.c_str());
  printf("Use keyboard (a, w, s, d) keys to navigate through board\n");
  printf("Some positions have treasures. You must find all of them!\n");
}

TreasureHunt::Client::~Client() {
  this->netHandler.ClientEndGame();
  printf("\n\nCongratulations! You found all [%d] treasures!\n", TOTAL_TREASURES);
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
  char *treasureName;

  treasureName = &this->filePath[sizeof(TREASURES_DIR) - sizeof('\0')];
  do {
    netHandler.SendGenericData(mov, NULL, 0);
    ret = netHandler.RecvResponse((void*)treasureName, NULL);
  } while (ret == CustomProtocol::INVERT);

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
        PrintErrorMsgType(ret, "InformServerMovement");
        exit(1);
    }
  }
  this->netHandler.InvertToReceiver();

  return TREASURE_FOUND;
}

void TreasureHunt::Client::GetServerTreasure() {
  MsgType msgRet;
  fs::space_info spaceInfo;
  size_t fileSize;
  size_t availableSize;
  size_t dataLen;

  /* considering that player already moved to new location */
  this->hasTreasure[this->currentPosition.x][this->currentPosition.y] = true;
  this->numberOfFoundTreasures++;

  /* get file size and test if there is enough disk space */
  msgRet = this->netHandler.RecvGenericData(this->data, NULL);
  assert(msgRet == CustomProtocol::FILE_SIZE);
  fileSize = *(size_t *)(this->data);
  printf("Treasure size[%lu]\n", fileSize);

  spaceInfo = fs::space(TREASURES_DIR);
  availableSize = spaceInfo.available;
  if (fileSize > availableSize) {
    ERROR_PRINT("Available [%lu]; Requested [%lu]\n", availableSize, fileSize);
    this->netHandler.SendResponse(CustomProtocol::ERROR, NULL, 0);
    exit(1);
  }
  this->netHandler.SendResponse(CustomProtocol::ACK, NULL, 0);

  this->buffer.OpenFileForWrite(filePath);
  do {
    msgRet = netHandler.RecvGenericData(this->data, &dataLen);
    netHandler.SendResponse(CustomProtocol::ACK, NULL, 0);
    if (msgRet == CustomProtocol::DATA) {
      if (!buffer.AppendToBuffer(data, dataLen)) {
        buffer.FlushBuffer();
        buffer.AppendToBuffer(data, dataLen);
      }
    }
  } while(msgRet != CustomProtocol::END_OF_FILE);
  buffer.FlushBuffer();
  buffer.CloseFile();

  if (numberOfFoundTreasures != TOTAL_TREASURES) {
    this->netHandler.InvertToSender();
  }
}

int TreasureHunt::Client::ShowTreasure() {
  int ret;
  std::string command;

  printf("Setting up to show treasure...\n");
  switch (this->treasureType) {
    case MP4:
      command = SUDO_OPT + this->user + MP4_PLAYER + this->filePath + STDERR_OPT;
      ret = std::system(command.c_str());
      break;
    case JPG:
      command = SUDO_OPT + this->user +JPG_PLAYER + this->filePath + STDERR_OPT;
      ret = std::system(command.c_str());
      break;
    case TXT:
      command = SUDO_OPT + this->user +TXT_PLAYER + this->filePath + STDERR_OPT;
      ret = std::system(command.c_str());
      break;
    default:
      ERROR_PRINT("Unknown file type. Exiting.\n");
      exit(1);
  }
  printf("Done!\n");

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
      ERROR_PRINT("Invalid movement in [Move]. Exiting.\n");
      exit(1);
  }
  this->wasReached[this->currentPosition.x][this->currentPosition.y] = true;
}

bool TreasureHunt::Client::GameEnded() {
  return (this->numberOfFoundTreasures == TOTAL_TREASURES);
}