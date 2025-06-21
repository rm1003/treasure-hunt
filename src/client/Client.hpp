#include "../TreasureHunt.hpp"
#include "../libs/KermitProtocol.hpp"
#include "../libs/Buffer.hpp"
#include <string>

using CustomProtocol::MsgType;

namespace TreasureHunt {

const char SPACE[] = "   ";
const int NUMBER_OF_NEW_LINES = 80;

const std::string MP4_PLAYER  = "vlc ";
const std::string JPG_PLAYER  = "eog ";
const std::string TXT_PLAYER  = "xed ";
const std::string USER        = "fbc04 ";
const std::string SUDO_OPT    = "sudo -u " + USER;

class Client {
  private:
    CustomProtocol::NetworkHandler netHandler;
    Data::Buffer buffer;
    FileType treasureType;
    Position currentPosition;
    unsigned char data[CustomProtocol::DATA_SIZE];
    char filePath[256];
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
    void GetServerTreasure();
    /* */
    int ShowTreasure();
    /*  */
    bool GameEnded();
};

} // namespace TreasureHunt