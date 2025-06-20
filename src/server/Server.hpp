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
      std::string treasures[TOTAL_TREASURES]; /* guarda nomes tesouros */
      Position currentClientPosition; /* posicao atual cliente */
      int currentTreasure; /* posicao no vetor 'treasures' */
      bool hasTreasure[GRID_SIZE][GRID_SIZE]; /* matriz booleana indica se tem tesouro na posicao (x,y) */

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