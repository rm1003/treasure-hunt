#include "../TreasureHunt.hpp"
#include "../libs/KermitProtocol.hpp"
#include "../libs/Buffer.hpp"

#include <vector>

using CustomProtocol::MsgType;

namespace TreasureHunt {

  struct Treasure {
    char treasureName[CustomProtocol::DATA_SIZE];
    Position pos;
    FileType type;
  };

  class Server {
    private:
      CustomProtocol::NetworkHandler netHandler;
      std::vector<Treasure> treasures;
      Treasure *foundTreasure;
      Data::Buffer buffer;
      Position clientPos;
      int foundTreasures;
      char filePath[256];

      /* return NULL if pos has no treasure */
      Treasure *LocateTreasure(Position pos);
    public:
      Server();
      ~Server();
      void ReadTreasures();
      void SetTreasurePositions();
      int GetTotalFound();
      int GetClientMovement();
      void RespondToClient(int ret);
      void PrintClientPosition();
      void SendTreasure();
      bool GameEnded();
  };

}