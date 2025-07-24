#include "group3.h"
#include "card.h"
#include "cardset.h"
#include "gamestatus.h"
#include <algorithm>

void Group3::ready() {
    // 初期化
    possibleCards.clear();
    possibleCards.setupDeck();
    possibleCards.remove(hand);
    memory.clear();
    memory.setupDeck();
    memory.sort();
    hand.sort();
    strategy = 1;
    lastPlayerAction = 1;
    gamePhase = 0;
    
    printf("[DEBUG] Group3 ready - possibleCards: ");
    possibleCards.print();
    printf("\n");
}

// Group2から採用したヘルパー関数群
int Group3::countRemainingCards(int rank) {
    int count = 0;
    for (int i = 0; i < memory.size(); i++) {
        Card c = memory.at(i);
        if (c.rank() == rank && !hand.includes(c)) {
            count++;
        }
    }
    return count;
}

int Group3::countStrongerCards(const Card &target) {
    int count = 0;
    for (int i = target.strength() + 1; i <= 13; i++) {
        int rank = (i - 1 - 10 + 13) % 13;
        if (rank == 0) rank = 13;
        count += countRemainingCards(rank);
    }
    count += countRemainingCards(15); // JOKER
    return count;
}

int Group3::countMyCards(int rank) {
    int count = 0;
    for (int i = 0; i < hand.size(); i++) {
        if (hand.at(i).rank() == rank) {
            count++;
        }
    }
    return count;
}

int Group3::getMinPlayerCards(const GameStatus &gstat) {
    int minCards = 53;
    for (int i = 0; i < gstat.numPlayers; i++) {
        minCards = std::min(minCards, gstat.numCards[i]);
    }
    return minCards;
}

int Group3::countMyRanks() {
    int count = 0;
    for (int rank = 1; rank <= 13; rank++) {
        if (countMyCards(rank) > 0) count++;
    }
    if (countMyCards(15) > 0) count++; // JOKER
    return count;
}

int Group3::countLowRanks(int threshold) {
    int count = 0;
    int strength = (threshold + 10) % 13 + 1;
    for (int i = 1; i <= strength; i++) {
        int rank = (i - 1 - 10 + 13) % 13;
        if (rank == 0) rank = 13;
        if (countMyCards(rank) > 0) count++;
    }
    return count;
}

int Group3::countHighCards(int threshold) {
    int count = 0;
    int strength = (threshold + 10) % 13 + 1;
    for (int i = strength; i <= 13; i++) {
        int rank = (i - 1 - 10 + 13) % 13;
        if (rank == 0) rank = 13;
        count += countMyCards(rank);
    }
    count += countMyCards(15); // JOKER
    return count;
}

bool Group3::isCardSafe(const Card &card, const GameStatus &gstat) {
    // Group2の安全性判定ロジックを簡略化
    if (card.rank() == 15) { // JOKER
        return countRemainingCards(2) == 0; // 2が残っていない場合のみ安全
    }
    if (card.rank() == 2) {
        return countRemainingCards(15) == 0; // JOKERが残っていない場合のみ安全
    }
    return countStrongerCards(card) == 0; // より強いカードが残っていない
}

void Group3::updateStrategy(const GameStatus &gstat) {
    // group2流: 状況ごとに戦略を柔軟に切り替え
    int minCards = getMinPlayerCards(gstat);
    if (hand.size() <= 3 || minCards <= 2 || gamePhase == 2) {
        strategy = 0; // ラッシュ・攻撃
    } else if (minCards == 1) {
        strategy = 2; // 守備
    } else {
        strategy = 1; // 通常
    }
}

void Group3::updateGamePhase(const GameStatus &gstat) {
    int totalRemainingCards = 0;
    for (int i = 0; i < gstat.numPlayers; i++) {
        totalRemainingCards += gstat.numCards[i];
    }
    
    if (totalRemainingCards > 40) {
        gamePhase = 0; // 序盤
    } else if (totalRemainingCards > 20) {
        gamePhase = 1; // 中盤
    } else {
        gamePhase = 2; // 終盤
    }
}

bool Group3::shouldPlayAggressive(const GameStatus &gstat) {
    return strategy == 0 || 
           (hand.size() <= 3) || 
           (getMinPlayerCards(gstat) == 1 && hand.size() > 1);
}

bool Group3::shouldSaveStrongCards(const Card &card, const GameStatus &gstat) {
    // group2流: ラッシュ・終盤・手札少ない時は温存しない
    if (strategy == 0 || hand.size() <= 3 || gamePhase == 2 || getMinPlayerCards(gstat) <= 2) return false;
    // 通常は強いカードを温存
    return card.strength() > 10 && countStrongerCards(card) > 0;
}

void Group3::updatePossibleCards(const GameStatus &gstat, const CardSet &played) {
    // group2流: 場・自分が出したカードを記憶・可能カードから一括除外
    if (!gstat.pile.isEmpty()) {
        possibleCards.remove(gstat.pile);
        memory.remove(gstat.pile);
    }
    if (!played.isEmpty()) {
        possibleCards.remove(played);
        memory.remove(played);
    }
    memory.sort();
}

bool Group3::approve(const GameStatus &gstat) {
    // group2流: 安全チェックと記憶・戦略更新
    if (gstat.numPlayers <= 0 || gstat.numPlayers > 8) {
        return false;
    }
    updateStrategy(gstat);
    updateGamePhase(gstat);
    updatePossibleCards(gstat, CardSet());
    return true;
}

bool Group3::follow(const GameStatus &gstat, CardSet &cs) {
    if (gstat.numPlayers <= 0 || gstat.numPlayers > 8) {
        cs.clear();
        return false;
    }
    
    CardSet pile(gstat.pile);
    int leadSize = pile.size();
    cs.clear();
    hand.sort();
    
    if (hand.size() == 0) return true;
    
    Card strongest = hand.at(hand.size() - 1);
    Card weakest = hand.at(0);
    int minCards = getMinPlayerCards(gstat);
    bool rush = (hand.size() <= 4 || minCards <= 2 || gamePhase == 2);
    
    bool isRevolution = false;
    #ifdef HAS_REVOLUTION
    isRevolution = gstat.hasRevolution;
    #endif

    auto canBeat = [&](const Card &a, const Card &b) {
        if (isRevolution) return a.strength() < b.strength();
        return a.strength() > b.strength();
    };

    bool mustNotPass = (hand.size() == 1 || minCards == 1);

    // 確殺処理（既存のまま）
    if (minCards == 1) {
        for (int n = 4; n >= 2; n--) {
            for (int r = 1; r <= 15; r++) {
                if (countMyCards(r) >= n && n == leadSize) {
                    CardSet trySet;
                    for (int i = 0; i < hand.size() && trySet.size() < n; i++) {
                        if (hand.at(i).rank() == r) trySet.insert(hand.at(i));
                    }
                    if (trySet.size() == n && (leadSize == 0 || canBeat(trySet.at(0), pile.at(0)))) {
                        cs = trySet;
                        hand.remove(cs);
                        updatePossibleCards(gstat, cs);
                        return true;
                    }
                }
            }
        }
        
        for (int i = hand.size() - 1; i >= 0; i--) {
            if (leadSize == 0 || (leadSize == 1 && canBeat(hand.at(i), pile.at(0)))) {
                cs.insert(hand.at(i));
                hand.remove(hand.at(i));
                updatePossibleCards(gstat, cs);
                return true;
            }
        }
    }

    // ここから新しいロジック：shouldUseStrongCardを使用
    if (leadSize == 0) {
        // 親の場合：ペア/セット優先
        for (int n = 4; n >= 2; n--) {
            for (int r = 1; r <= 15; r++) {
                if (countMyCards(r) >= n) {
                    // 新しい判定ロジック
                    Card testCard(r, 0);  // ランクだけの仮カード
                    if (shouldUseStrongCard(testCard, gstat, 2)) {  // playType=2はペア/セット
                        CardSet trySet;
                        for (int i = 0; i < hand.size() && trySet.size() < n; i++) {
                            if (hand.at(i).rank() == r) trySet.insert(hand.at(i));
                        }
                        if (trySet.size() == n) {
                            cs = trySet;
                            hand.remove(cs);
                            updatePossibleCards(gstat, cs);
                            return true;
                        }
                    }
                }
            }
        }
        
        // 単体出し：新しい判定
        if (rush) {
            if (shouldUseStrongCard(strongest, gstat, 0)) {  // playType=0は親
                cs.insert(strongest);
                hand.remove(strongest);
                updatePossibleCards(gstat, cs);
                return true;
            }
        }
        
        // 通常は最弱カード（JOKER/2でなければ）
        if (shouldUseStrongCard(weakest, gstat, 0)) {
            cs.insert(weakest);
            hand.remove(weakest);
            updatePossibleCards(gstat, cs);
            return true;
        }
        
        // JOKER/2が温存対象でも、他に出せるカードを探す
        for (int i = 0; i < hand.size(); i++) {
            if (shouldUseStrongCard(hand.at(i), gstat, 0)) {
                cs.insert(hand.at(i));
                hand.remove(hand.at(i));
                updatePossibleCards(gstat, cs);
                return true;
            }
        }
        
        return true;
    }
    else if (leadSize == 1) {
        // フォロー1枚
        if (rush) {
            for (int i = hand.size() - 1; i >= 0; i--) {
                if (canBeat(hand.at(i), pile.at(0)) && shouldUseStrongCard(hand.at(i), gstat, 1)) {  // playType=1はフォロー
                    cs.insert(hand.at(i));
                    hand.remove(hand.at(i));
                    updatePossibleCards(gstat, cs);
                    return true;
                }
            }
        }
        
        // 通常フォロー：最弱で出せるカード
        for (int i = 0; i < hand.size(); i++) {
            if (canBeat(hand.at(i), pile.at(0)) && shouldUseStrongCard(hand.at(i), gstat, 1)) {
                cs.insert(hand.at(i));
                hand.remove(hand.at(i));
                updatePossibleCards(gstat, cs);
                return true;
            }
        }
    }
    else {
        // ペア/セットフォロー
        for (int n = 4; n >= 2; n--) {
            for (int i = 0; i < hand.size(); i++) {
                int r = hand.at(i).rank();
                if (countMyCards(r) >= n && n == leadSize) {
                    Card testCard(r, 0);
                    if (shouldUseStrongCard(testCard, gstat, 2)) {
                        CardSet trySet;
                        for (int j = 0; j < hand.size() && trySet.size() < n; j++) {
                            if (hand.at(j).rank() == r) trySet.insert(hand.at(j));
                        }
                        if (trySet.size() == n && canBeat(trySet.at(0), pile.at(0))) {
                            cs = trySet;
                            hand.remove(cs);
                            updatePossibleCards(gstat, cs);
                            return true;
                        }
                    }
                }
            }
        }
    }

    // パス処理
    if (mustNotPass) {
        // 最後の手段：どれか出せるカードを無理やり出す
        for (int i = 0; i < hand.size(); i++) {
            cs.insert(hand.at(i));
            hand.remove(hand.at(i));
            updatePossibleCards(gstat, cs);
            return true;
        }
    }
    
    cs.clear();
    updatePossibleCards(gstat, cs);
    return true;
}

bool Group3::canWinWithoutStrongCards(const GameStatus &gstat) {
    CardSet nonStrongCards;
    for (int i = 0; i < hand.size(); i++) {
        Card c = hand.at(i);
        if (c.rank() != 15 && c.rank() != 2) {
            nonStrongCards.insert(c);
        }
    }
    
    if (nonStrongCards.size() == 0) return false;
    
    int minOpponentCards = getMinPlayerCards(gstat);
    if (minOpponentCards >= 4 && nonStrongCards.size() >= 3) {
        return true;
    }
    
    return false;
}

// evaluateJokerUrgenc:\Users\kiyoh\Downloads\group3.cpp c:\Users\kiyoh\Downloads\group3.hcy を追加
int Group3::evaluateJokerUrgency(const GameStatus &gstat) {
    int urgency = 0;
    int minOpponentCards = getMinPlayerCards(gstat);
    
    if (minOpponentCards == 1) {
        urgency += 80;
        if (hand.size() >= 2) urgency += 15;
    } else if (minOpponentCards == 2) {
        urgency += 50;
        if (hand.size() >= 4) urgency += 10;
    }
    
    if (hand.size() <= 2) urgency += 60;
    else if (hand.size() <= 3) urgency += 30;
    
    if (gamePhase == 2) urgency += 25;
    
    if (canWinWithoutStrongCards(gstat)) urgency -= 30;
    
    if (!gstat.pile.isEmpty()) {
        Card topCard = gstat.pile.at(0);
        if (topCard.strength() >= 11) urgency += 20;
    }
    
    return std::max(0, std::min(100, urgency));
}

// evaluate2Urgency を追加
int Group3::evaluate2Urgency(const GameStatus &gstat) {
    int urgency = 0;
    int minOpponentCards = getMinPlayerCards(gstat);
    
    if (!gstat.pile.isEmpty() && gstat.pile.at(0).rank() == 15) {
        return 95;
    }
    
    if (minOpponentCards == 1 && countRemainingCards(15) > 0) {
        urgency += 70;
    }
    
    if (hand.size() <= 2) urgency += 50;
    else if (hand.size() <= 3) urgency += 25;
    
    if (gamePhase == 2) urgency += 20;
    
    if (countRemainingCards(15) > 0) urgency -= 25;
    
    return std::max(0, std::min(100, urgency));
}

// shouldUseStrongCard を追加
bool Group3::shouldUseStrongCard(const Card &card, const GameStatus &gstat, int playType) {
    if (card.rank() != 15 && card.rank() != 2) return true;
    
    // 絶対使用条件
    if (hand.size() == 1) return true;
    if (getMinPlayerCards(gstat) == 1 && hand.size() >= 2) return true;
    
    int urgency = 0;
    if (card.rank() == 15) {
        urgency = evaluateJokerUrgency(gstat);
    } else {
        urgency = evaluate2Urgency(gstat);
    }
    
    if (urgency >= 80) return true;
    if (urgency >= 60 && playType != 0) return true;
    if (urgency >= 40 && hand.size() <= 3) return true;
    
    if (playType == 2) {  // ペア/セット使用
        if (urgency >= 30) return true;
        if (countMyCards(card.rank()) >= 4) return true;
    }
    
    return false;
}

