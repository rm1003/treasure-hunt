#include "Server.hpp"
#include <cstdlib>
#include <iostream>
#include "../libs/Logging.hpp"

TreasureHunt::Server::Server() {
  memset(this->hasTreasure, 0, sizeof(this->hasTreasure));
  this->treasureIt = 0;
  this->clientPos.SetPosition(0, 0);
}

TreasureHunt::Server::~Server() {
}

void TreasureHunt::Server::ReadTreasures() {
  for (int i = 0; i < TOTAL_TREASURES; i++) {
    printf("Treasure number [%d]: ", i+1);
    std::cin >> this->treasures[i];
    DEBUG_PRINT("[%s] read\n", this->treasures[i].c_str());

  }
}

void TreasureHunt::Server::FillHasTreasureArray() {
  int x;
  int y;
  bool newPositionFound;

  for (int i = 0; i < TOTAL_TREASURES; i++) {
    newPositionFound = false;
    while (!newPositionFound) {
      x = rand() % GRID_SIZE;
      y = rand() % GRID_SIZE;
      if (x == 0 && y == 0) {
        continue;
      }
      if (this->hasTreasure[x][y] == false) {
        this->hasTreasure[x][y] = true;
        newPositionFound = true; /* achou nova posicao nao ocupada */
        DEBUG_PRINT("x [%d] y [%d] selected\n", x, y);
      }
    }
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

    this->netHandler.SendResponse();
    this->clientPos = oldPos;
    return INVALID_MOVE;
  }

  if (this->hasTreasure[this->clientPos.x][this->clientPos.y] == true) {
    this->netHandler.SendResponse();
    this->hasTreasure[this->clientPos.x][this->clientPos.y] = false;

    this->netHandler.InvertToSender();
    return TREASURE_FOUND;
  }

  return VALID_MOVE;
}

void TreasureHunt::Server::PrintClientPosition() {

}

void TreasureHunt::Server::SendTreasure() {


  this->treasureIt++;
}

bool TreasureHunt::Server::GameEnded() {
  return (this->treasureIt == TOTAL_TREASURES);
}