#include "../TreasureHunt.hpp"
#include "../libs/KermitProtocol.hpp"

namespace TreasureHunt {

  class Server {
    private:
      CustomProtocol::NetworkHandler netHandler;
      Position *positionsList; // Maybe not necessary IDK
      bool foundTreasures[NUM_TREASURES];

    public:
      Server();
      ~Server();
      void InitClientPosition();
      void InformClientOkMove();
      void InformClientNotOkMove();
      void InformClientTreasureFound();
      void InformClientEndGame();
      void SendClientTreasure();
      void WaitForClientMsg();
      void InterpretClientMsg();
  };

}