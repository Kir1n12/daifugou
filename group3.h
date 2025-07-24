// group3.h
#ifndef GROUP3_H_
#define GROUP3_H_
#include "player.h"

struct HandComposition {
    int singles[16], pairs[16], triples[16], quadruples[16];
    int totalSets, totalSingles;
    HandComposition();
};

class Group3 : public Player {
private:
    CardSet possibleCards;  // 他プレイヤーが持っている可能性のあるカード
    CardSet memory;         // 全カードの記憶（Group2から採用）
    int strategy;          // 戦略パターン (0:攻撃的, 1:通常, 2:守備的)
    int lastPlayerAction;  // 直前プレイヤーの行動 (0:パス, 1:カード出し)
    int gamePhase;         // ゲーム段階 (0:序盤, 1:中盤, 2:終盤)
    
    // Group2から採用したヘルパー関数
    int countRemainingCards(int rank);           // 指定ランクの残りカード数
    int countStrongerCards(const Card &target);  // 指定カードより強い残りカード数
    int countMyCards(int rank);                  // 自分の指定ランクカード数
    int getMinPlayerCards(const GameStatus &gstat); // 最少手札プレイヤーの枚数
    int countMyRanks();                          // 自分の持つランク種類数
    int countLowRanks(int threshold);            // 指定値以下のランク種類数
    int countHighCards(int threshold);           // 指定値以上のカード枚数
    bool isCardSafe(const Card &card, const GameStatus &gstat); // カード出しの安全性判定
    
    // 戦略判定関数
    void updateStrategy(const GameStatus &gstat);
    void updateGamePhase(const GameStatus &gstat);
    bool shouldPlayAggressive(const GameStatus &gstat);
    bool shouldSaveStrongCards(const Card &card, const GameStatus &gstat);
    
    bool canWinWithoutStrongCards(const GameStatus &gstat);
    int evaluateJokerUrgency(const GameStatus &gstat);
    int evaluate2Urgency(const GameStatus &gstat);
    int evaluateCardStrategicValue(const Card &card, const GameStatus &gstat);
    bool shouldUseStrongCard(const Card &card, const GameStatus &gstat, int playType);
    Card selectBestSingleCard(const GameStatus &gstat, int playType);
    CardSet selectBestSet(int rank, int size, const GameStatus &gstat, int playType);
    int evaluateCardPlay(const Card &card, const GameStatus &gstat, int playType);
    int evaluateHandBalance(const Card &card);
    int calculateOpponentImpact(const Card &card, const GameStatus &gstat);

    HandComposition analyzeHandComposition();
    int evaluateSetStrategicValue(const CardSet &set, const GameStatus &gstat);
    CardSet selectOptimalSet(const GameStatus &gstat, int requiredSize);
    CardSet selectCounterSet(const GameStatus &gstat, int requiredSize, int targetStrength);
    void adjustSetStrategy(const GameStatus &gstat);
    
public:
    Group3(const char *s = "group3") : Player(s), strategy(1), lastPlayerAction(1), gamePhase(0) {}
    virtual void ready() override;
    virtual bool follow(const GameStatus &gstat, CardSet &cs) override;
    virtual bool approve(const GameStatus &gstat) override;
    void updatePossibleCards(const GameStatus &gstat, const CardSet &played = CardSet());
};
#endif