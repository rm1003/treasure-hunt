#include "Server.hpp"
#include "../libs/Logging.hpp"
#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <filesystem>

namespace fs = std::filesystem;

TreasureHunt::Server::Server() {
  this->clientPos.SetPosition(0, 0);
  this->treasures.resize(TOTAL_TREASURES);
  this->foundTreasures = 0;
  this->filePath[0] = '\0';
  strcpy(filePath, TREASURES_DIR);
  printf("\n//=======================================================//\n");
  printf("Initiated Server side of TreasureHunt game\n");
  printf("//=======================================================//\n");
}

TreasureHunt::Server::~Server() {
  this->netHandler.ServerEndGame();
  printf("//=======================================================//\n");
  printf("Closing Server side of TreasureHunt game\n");
  printf("//=======================================================//\n\n");
}

TreasureHunt::Treasure *TreasureHunt::Server::LocateTreasure(Position pos) {
  for (int it = 0; it < TOTAL_TREASURES; it++) {
    if (this->treasures[it].pos.Equal(pos.x, pos.y)) {
      return &this->treasures[it];
    }
  }
  return NULL;
}

void TreasureHunt::Server::ReadTreasures() {
  std::string path = TREASURES_DIR;
  int it = 0;
  char *strIt;

  printf("\nReading treasures...\n");
  printf("Type [1] = TXT; Type[2] = MP4; Type[3] = JPG\n");

  for (const auto &entry : fs::directory_iterator(path)) {
    if (it == TOTAL_TREASURES) {
      ERROR_PRINT("Too many treasures! Exiting.\n");
      exit(1);
    }
    this->treasures[it].treasureName[0] = '\0';
    strcpy(this->treasures[it].treasureName, entry.path().filename().generic_string().c_str());
    strIt = strchr(this->treasures[it].treasureName, '.');
    if (!strcmp(strIt, ".mp4")) {
      this->treasures[it].type = MP4;
    } else if (!strcmp(strIt, ".jpg")) {
      this->treasures[it].type = JPG;
    } else if (!strcmp(strIt, ".txt")) {
      this->treasures[it].type = TXT;
    } else {
      ERROR_PRINT("Invalid [%s]. Exiting.\n", strIt);
      exit(1);
    }
    printf("Treasure [%s] registered ", this->treasures[it].treasureName);
    printf("Type [%d]\n", this->treasures[it].type);
    this->treasures[it].pos.SetPosition(0, 0); /* invalid position */
    it++;
  }

}

void TreasureHunt::Server::SetTreasurePositions() {
  bool newPositionFound;
  Position pos;

  printf("\nSetting random positions to treasures...\n");

  for (int i = 0; i < TOTAL_TREASURES; i++) {
    newPositionFound = false;
    while (!newPositionFound) {
      pos.SetPosition(rand() % GRID_SIZE, rand() % GRID_SIZE);
      if (pos.Equal(0, 0)) {
        continue;
      }
      if (this->LocateTreasure(pos) == NULL) {
        this->treasures[i].pos = pos;
        newPositionFound = true;
      }
    }
    printf("Set [%s] to [%d][%d]\n", this->treasures[i].treasureName,
                                     this->treasures[i].pos.x,
                                     this->treasures[i].pos.y);
  }

}

int TreasureHunt::Server::GetClientMovement() {
  MsgType msgRet;
  Position oldPos;

  msgRet = this->netHandler.RecvGenericData(NULL, NULL);
  oldPos = this->clientPos;

  switch (msgRet) {
    case CustomProtocol::MOVE_UP:
      this->clientPos.MoveUp();
      break;
    case CustomProtocol::MOVE_DOWN:
      this->clientPos.MoveDown();
      break;
    case CustomProtocol::MOVE_LEFT:
      this->clientPos.MoveLeft();
      break;
    case CustomProtocol::MOVE_RIGHT:
      this->clientPos.MoveRight();
      break;
    default:
      ERROR_PRINT("Not expected msgRet [%d]. Exiting\n", msgRet);
      exit(1);
  }

  if (this->clientPos.x >= GRID_SIZE || this->clientPos.x < 0 ||
      this->clientPos.y >= GRID_SIZE || this->clientPos.y < 0) {
    this->clientPos = oldPos;
    return INVALID_MOVE;
  }

  Treasure* treasurePtr = this->LocateTreasure(this->clientPos);
  if (treasurePtr) {
    this->foundTreasure = treasurePtr;
    this->foundTreasures++;
    treasurePtr->pos.SetPosition(-1, -1); /* invalidate treasure */
    return TREASURE_FOUND;
  }

  return VALID_MOVE;
}

void TreasureHunt::Server::RespondToClient(int ret) {
  char *name;
  size_t len;

  switch (ret) {
    case INVALID_MOVE:
      this->netHandler.SendResponse(CustomProtocol::ACK, NULL, 0);
      break;
    case VALID_MOVE:
      this->netHandler.SendResponse(CustomProtocol::OK_AND_ACK, NULL, 0);
      break;
    case TREASURE_FOUND:
      name = this->foundTreasure->treasureName;
      len = strlen(this->foundTreasure->treasureName) + 1;
      switch (this->foundTreasure->type) {
        case MP4:
          this->netHandler.SendResponse(CustomProtocol::VIDEO_FILE_NAME_ACK, name, len);
          break;
        case JPG:
          this->netHandler.SendResponse(CustomProtocol::IMG_FILE_NAME_ACK, name, len);
          break;
        case TXT:
          this->netHandler.SendResponse(CustomProtocol::TXT_FILE_NAME_ACK, name, len);
          break;
      }
      break;
  }

  if (ret == TREASURE_FOUND) {
    this->netHandler.InvertToSender();
  }
}

void TreasureHunt::Server::PrintClientPosition() {
  printf("Client position [%d][%d]\n", this->clientPos.x, this->clientPos.y);
}

void TreasureHunt::Server::SendTreasure() {
  size_t fileSize;
  MsgType msgRet;
  void *ptr;
  size_t actualSize;

  /* format file path */
  strcat(this->filePath, this->foundTreasure->treasureName);

  fs::path arquivo(this->filePath);
  printf("File path [%s]; ", this->filePath);
  fileSize = fs::file_size(arquivo);
  printf("File size [%lu]\n", fileSize);

  /* check if client has enough to space */
  this->netHandler.SendGenericData(CustomProtocol::FILE_SIZE, &fileSize, sizeof(fileSize));
  msgRet = netHandler.RecvResponse(NULL, NULL);
  if (msgRet == CustomProtocol::ERROR) {
    printf("Client cant handle file of size [%lu]. Exiting.", fileSize);
    exit(1);
  }
  assert(msgRet == CustomProtocol::ACK);

  buffer.OpenFileForRead(this->filePath);
  this->buffer.RetrieveBuffer();
  ptr = this->buffer.GetData(CustomProtocol::DATA_SIZE, &actualSize);
  do {
    netHandler.SendGenericData(CustomProtocol::DATA, ptr, actualSize);
    msgRet = netHandler.RecvResponse(NULL, NULL);
    assert(msgRet == CustomProtocol::ACK);

    ptr = this->buffer.GetData(CustomProtocol::DATA_SIZE, &actualSize);
    if (ptr == NULL) {
      this->buffer.RetrieveBuffer();
      ptr = this->buffer.GetData(CustomProtocol::DATA_SIZE, &actualSize);
    }
  } while (ptr != NULL);
  netHandler.SendGenericData(CustomProtocol::END_OF_FILE, NULL, 0);
  msgRet = netHandler.RecvResponse(NULL, NULL);
  assert(msgRet == CustomProtocol::ACK);
  buffer.CloseFile();

  if (this->foundTreasures != TOTAL_TREASURES) {
    this->netHandler.InvertToReceiver();
  }

  this->filePath[sizeof(TREASURES_DIR)-1] = '\0';
}

bool TreasureHunt::Server::GameEnded() {
  return (this->foundTreasures == TOTAL_TREASURES);
}

int TreasureHunt::Server::GetTotalFound() {
  return this->foundTreasures;
}