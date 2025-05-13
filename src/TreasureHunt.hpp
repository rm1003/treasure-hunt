#ifndef TREASUREHUNT_HPP_
#define TREASUREHUNT_HPP_

namespace TreasureHunt {

  const unsigned long NUM_TREASURES = 8;
  const char TREASURES_DIR[] = "./objects";

  struct Position {
    int x;
    int y;
  };

}

#endif