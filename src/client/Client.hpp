#include "../TreasureHunt.hpp"
#include "../libs/KermitProtocol.hpp"

namespace TreasureHunt {

const int TREASURE_FOUND = 1;
const int TREASURE_NOT_FOUND = 2;
const int INVALID_MOVE = 3;
const char SPACE[] = "   ";

class Client {
  private:
    // CustomProtocol::NetworkHandler netHandler;
    Position currentPosition;
    char *treasureFileName;
    int numberOfFoundTreasures;
    bool HasTreasure[GRID_SIZE][GRID_SIZE];
    bool WasReached[GRID_SIZE][GRID_SIZE];
  public:
    Client();
    ~Client() {};
    void PrintGrid();
    void PrintEmptySpace();
    int InformServerMoveUp();
    int InformServerMoveDown();
    int InformServerMoveLeft();
    int InformServerMoveRight();
    void GetServerTreasure();
};

}