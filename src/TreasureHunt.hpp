#ifndef TREASUREHUNT_HPP_
#define TREASUREHUNT_HPP_

namespace TreasureHunt {

  const int TOTAL_TREASURES = 8;
  const int GRID_SIZE = 8;
  const char TREASURES_DIR[] = "../objects";

  struct Position {
    int x;
    int y;
    inline void SetPosition(int newX, int newY) {
      this->x = newX;
      this->y = newY;
    }
    inline bool Equal(int x, int y) {
      return (this->x == x && this->y == y);
    }
  };

}

#endif