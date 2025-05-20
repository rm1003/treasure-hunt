#include "../TreasureHunt.hpp"
#include "../libs/KermitProtocol.hpp"

using CustomProtocol::MsgType;

namespace TreasureHunt {

const int TREASURE_FOUND = 1;
const int TREASURE_NOT_FOUND = 2;
const int INVALID_MOVE = 3;
const char SPACE[] = "   ";
const int NUMBER_OF_NEW_LINES = 80;
const int INI_X = 0;
const int INI_Y = 0;

class Client {
  private:
    // CustomProtocol::NetworkHandler netHandler;
    char *treasureFileName;
    int numberOfFoundTreasures;
    bool HasTreasure[GRID_SIZE][GRID_SIZE];
    bool WasReached[GRID_SIZE][GRID_SIZE];
    public:
    Position currentPosition;
    Client();
    ~Client() {};
    void PrintGrid();
    void PrintEmptySpace();
    int InformServerMovement(MsgType mov);
    void GetServerTreasure();
};

}