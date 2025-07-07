
#ifndef _GROUP3_H_
#define _GROUP3_H_

#include "player.h"


class Group3 : public Player {
public:
    CardSet possibleCards; // 他プレイヤーが持っている可能性のあるカード

    Group3(const char *s = "group3") : Player(s) {}
    virtual void ready() override;
    virtual bool follow(const GameStatus &gstat, CardSet &cs) override;
    virtual bool approve(const GameStatus &gstat) override;

    // 推定カードリストの更新
    void updatePossibleCards(const GameStatus &gstat, const CardSet &played = CardSet());
};

#endif
