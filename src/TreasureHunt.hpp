#ifndef TREASUREHUNT_HPP_
#define TREASUREHUNT_HPP_

namespace TreasureHunt {

  const unsigned long NUM_TREASURES = 8;
  const char TREASURES_DIR[] = "./objects";

  struct Position {
    int x;
    int y;
  };

  enum MsgTypes {
    ACK = 0,
    NACK,
    OK_AND_ACK,
    A_DEFINIR_1,
    FILE_SIZE,
    DATA,
    TXT_FILE_NAME,
    VIDEO_FILE_NAME,
    IMG_FILE_NAME,
    END_OF_FILE,
    MOVE_RIGHT,
    MOVE_UP,
    MOVE_DOWN,
    MOVE_LEFT,
    A_DEFINIR_2,
    ERROR
  };

}

#endif