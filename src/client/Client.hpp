#include "../TreasureHunt.hpp"
#include "../libs/KermitProtocol.hpp"

namespace TreasureHunt {

  class Client {
    private:
      CustomProtocol::NetworkHandler netHandler;
      Position currentPosition;
    public:
      Client();
      ~Client();
      void WaitForServer();

      void GetInitialPositial();
      void InformServerMoveUp();
      void InformServerMoveDown();
      void InformServerMoveLeft();
      void InformServerMoveRight();
      void WaitForServerResponse();
      void InterpretServerResponse();
      void GetServerTreasure();
  };

}