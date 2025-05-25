#include "../TreasureHunt.hpp"
#include "../libs/KermitProtocol.hpp"
#include "../libs/Buffer.hpp"
#include <string>

using CustomProtocol::MsgType;

namespace TreasureHunt {

const int TREASURE_FOUND = 1;
const int VALID_MOVE = 2;
const int INVALID_MOVE = 3;
const char SPACE[] = "   ";
const int NUMBER_OF_NEW_LINES = 80;
const std::string MP4_PLAYER  = "vlc";
const std::string MP4_OPTIONS = "--play-and-exit";
const std::string JPG_PLAYER  = "eog";
const std::string TXT_PLAYER  = "xed";

class Client {
  private:
    CustomProtocol::NetworkHandler netHandler;
    Data::Buffer buffer;
    char treasureFileName[CustomProtocol::DATA_SIZE];
    FileType treasureType;
    Position currentPosition;
    int numberOfFoundTreasures;
    bool hasTreasure[GRID_SIZE][GRID_SIZE];
    bool wasReached[GRID_SIZE][GRID_SIZE];

  public:
    Client();
    ~Client() {};
    void PrintGrid();
    void PrintEmptySpace();
    /* */
    void Move(MsgType mov);
    /*  */
    int InformServerMovement(MsgType mov);
    /* Return 0 on success. Return 1 if failed */
    int GetServerTreasure();
    /* */
    int ShowTreasure();
};

} // namespace TreasureHunt