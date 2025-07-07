
#include "group3.h"
#include "card.h"
#include "cardset.h"
#include "gamestatus.h"

// 推定カードリストの更新
void Group3::updatePossibleCards(const GameStatus &gstat, const CardSet &played) {
    // 前ターンのpossibleCardsから場に出たカードと自分が出したカードを除外
    possibleCards.remove(gstat.pile);
    possibleCards.remove(played);
    // デバッグ表示
    printf("[DEBUG] possibleCards: ");
    possibleCards.print();
    printf("\n");
}

void Group3::ready() {
    // possibleCardsの初期値は「全部のカード - 自分の手札」
    possibleCards.setupDeck();
    possibleCards.remove(hand);
    // デバッグ表示
    printf("[DEBUG] possibleCards (ready): ");
    possibleCards.print();
    printf("\n");
}

// 他プレイヤーの行動後（自分のターンが来る前）に推定カードリストを更新・表示
bool Group3::approve(const GameStatus &gstat) {
    // 直前の場に出たカードでpossibleCardsを更新
    updatePossibleCards(gstat);
    return true;
}

// 貪欲に出せるカードを出す
bool Group3::follow(const GameStatus &gstat, CardSet &cs) {
    CardSet pile(gstat.pile);
    Card tmp;
    int leadSize = pile.size();
    hand.sort(); // 弱い順にソート（昇順）

    cs.clear();
    if (leadSize == 0) { // リーダーなら一番弱いカードを1枚出す
        tmp = hand.at(0);
        cs.insert(tmp);
        hand.remove(tmp);
        updatePossibleCards(gstat, cs);
        return true;
    }
    else if (leadSize == 1) { // 場に1枚
        for (int i = 0; i < hand.size(); i++) {
            tmp = hand.at(i);
            if (tmp.isGreaterThan(pile.at(0))) {
                cs.insert(tmp);
                hand.remove(tmp);
                updatePossibleCards(gstat, cs);
                return true;
            }
        }
    }
    else { // 2枚以上のセット
        for (int i = 0; i < hand.size(); i++) {
            tmp = hand.at(i);
            if (tmp.isGreaterThan(pile.at(0))) {
                cs.clear();
                cs.insert(tmp);
                for (int j = 1; j < leadSize && i + j < hand.size(); j++) {
                    if (hand.at(i).rank() == hand.at(i + j).rank())
                        cs.insert(hand.at(i + j));
                }
                if (cs.size() == leadSize) {
                    hand.remove(cs);
                    updatePossibleCards(gstat, cs);
                    return true;
                }
            }
        }
    }
    // 出せるカードがないのでパス
    cs.clear();
    updatePossibleCards(gstat, cs);
    return true;
}
