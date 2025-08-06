#ifndef MOVE_H
#define MOVE_H

enum class MoveType {
    Change,
    Swap,
    BlockSwap,
    RuinAndRecreate,
    FixShiftRotation
};

struct Move {
    MoveType type;
    int employee1;
    int day1;
    int shift1;
    int employee2;
    int day2;
    int shift2;
    int block_size;
};

#endif // MOVE_H
