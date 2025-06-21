#include "../TreasureHunt.hpp"
#include "../libs/KermitProtocol.hpp"
#include "../libs/Buffer.hpp"

#include <string>

using CustomProtocol::MsgType;

namespace TreasureHunt {

  class Server {
    private:
      CustomProtocol::NetworkHandler netHandler;
      Data::Buffer buffer;
      std::string treasures[TOTAL_TREASURES];
      Position clientPos;
      int treasureIt;
      bool hasTreasure[GRID_SIZE][GRID_SIZE];

    public:
      Server();
      ~Server();
      void ReadTreasures();
      void FillHasTreasureArray();
      int GetClientMovement();
      void PrintClientPosition();
      void SendTreasure();
      bool GameEnded();
  };

}