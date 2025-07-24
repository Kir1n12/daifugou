// インデント修正版
#include <iostream>
#include <string>
#include <cstdlib> //わからんとき rand 関数使おうかな
#include "card.h"
#include "cardset.h"
#include "player.h"
#include "gamestatus.h"
#include "group2.h"

int Group2::check(int num) { //自身の手持ちを除く任意の数字の残り枚数検索
    int count = 0;
    for (int i = 0; i < memory.size(); i++) {
        Card c = memory.at(i);
        if (c.rank() == num && !hand.includes(c)) {
            count++;
        }
    }
    return count;
}

int Group2::numStrength(const Card &target) { //自身の手持ちを除く任意のカードより強い残りカード枚数検索
    int rank, count = 0;
    for (int i = target.strength() + 1; i <= 13; i++) {
        rank = (i - 1 - 10 + 13) % 13;
        if (rank == 0) {
            rank = 13;
        }
        count += check(rank);
    }
    count += check(15);
    return count;
}

int Group2::mycheck(int num) { //自身の任意の数字枚数検索
    int count = 0;
    for (int i = 0; i < hand.size(); i++) {
        Card c = hand.at(i);
        if (c.rank() == num) {
            count++;
        }
    }
    return count;
}

int Group2::minplayer(const GameStatus &gstat) { //カード最小枚数のプレイヤーのカード枚数検索
    int ans = 53;
    for (int i = 0; i < gstat.numPlayers; i++) {
        ans = std::min(ans, gstat.numCards[i]);
    }
    return ans;
}

int Group2::numhave(void) { //自身の持ち数字種類数検索
    int count = 0;
    for (int i = 1; i < 14; i++) {
        if (mycheck(i) != 0) {
            count++;
        }
    }
    if (mycheck(15) != 0) {
        count++;
    }
    return count;
}

int Group2::lowhave(int num) { //任意の数字以下の種類数の検索
    int rerank, strength, count = 0;
    strength = (num + 10) % 13 + 1;
    for (int i = 1; i < strength + 1; i++) {
        rerank = (i - 1 - 10 + 13) % 13;
        if (rerank == 0) {
            rerank = 13;
        }
        if (mycheck(rerank) != 0) {
            count++;
        }
    }
    //JOKER はカウントしない
    return count;
}

int Group2::highnum(int num) { //任意の数字以上の枚数の検索
    int rerank, strength, count = 0;
    strength = (num + 10) % 13 + 1;
    for (int i = strength; i < 14; i++) {
        rerank = (i - 1 - 10 + 13) % 13;
        if (rerank == 0) {
            rerank = 13;
        }
        count += mycheck(rerank);
    }
    count += mycheck(15);
    return count;
}

int Group2::single(void){ //任意の数字以下で単体所持の枚数検索
    int rerank, count = 0;
    for(int i=1; i < 14; i++){
        rerank = (i-1-10+13)%13;
        if(rerank == 0){
            rerank = 13;
        }
        if(mycheck(rerank) == 1){
            count++;
        }
    }
    //JOKER はカウントしない
    return count;
}

// return count;
// }


void Group2::ready() {
    memory.setupDeck();
    memory.sort(myCardCmp);
    hand.sort(myCardCmp);
    pattern = 1; //基本は慎重に
    befp = 1; //直前プレイヤーのパスは 0, それ以外が 1
}

bool Group2::follow(const GameStatus &gstat, CardSet &cs) {
    CardSet pile(gstat.pile);
    Card tmp;
    int leadSize;
    hand.sort(myCardCmp); // 手札をソート(弱い方から順に)
    leadSize = pile.size();

    Card most = hand.at(hand.size() - 1); //最も強いカード
    //###########################################################################
    if (leadSize == 0) { // 自分がリーダなら
        if (numhave() > 1 && numStrength(hand.at(mycheck(hand.at(0).rank()))) == 0) { //もし自分の二種類目に弱いカードが必ず勝てるなら
            tmp = most; //最強カードから出す
            cs.insert(tmp);
            hand.remove(tmp);
            return true;
        }
        if (numhave() == 2) { //持ってるカードの種類が 2 種類のとき
            if (numStrength(most) == 0) { //最強カードは出す
                tmp = most;
                cs.insert(tmp);
                hand.remove(tmp);
                return true;
            }
            if (mycheck(hand.at(0).rank()) < mycheck(most.rank())) { //もし弱いほうより, 強いほうの枚数が多かったら
                for (int i = 0; i < hand.size(); i++) { //そっちから出す
                    tmp = hand.at(i);
                    if (tmp.rank() != most.rank()) {
                        continue;
                    }
                    cs.insert(tmp);
                    for (int j = 1; i + j < hand.size(); j++) {
                        for (int j = 1; i + j < hand.size(); j++) {
                            if (hand.at(i).rank() == hand.at(i + j).rank())
                                cs.insert(hand.at(i + j));
                        }
                    }
                    hand.remove(cs);
                    return true;
                }
            }
        }
        if (minplayer(gstat) == 1) { //残りカード枚数 1 枚のやつがいたら
            if ((lowhave(7) >= 4 || gstat.numPlayers >= 4) && hand.size() > 5) {
                // 何もしない
            } else {
                for (int i = 0; i < hand.size(); i++) { //2 枚を優先して出す
                    tmp = hand.at(i);
                    if (mycheck(hand.at(i).rank()) < 2) {
                        continue;
                    }
                    cs.insert(tmp);
                    for (int j = 1; i + j < hand.size(); j++) {
                        if (hand.at(i).rank() == hand.at(i + j).rank())
                            cs.insert(hand.at(i + j));
                    }
                    hand.remove(cs);
                    return true;
                }
            }
        }
    }
    for (int i = 0; i < hand.size(); i++) { //11 以下の 3 枚持ちは優先して出す(改善の余地ありあり)
        tmp = hand.at(i);
        if (mycheck(hand.at(i).rank()) < 3 || tmp.strength() > 9) {
            continue;
        }
        if (tmp.rank() == 15) { //Joker の分岐
            if (check(2) == 0 && numStrength(pile.at(0)) == 0) { //出されたカードが最後の 2 もしくは最強のカードなら
                cs.insert(tmp);
                for (int j = 1; i + j < hand.size(); j++) {
                    if (hand.at(i).rank() == hand.at(i + j).rank()) {
                        cs.insert(hand.at(i + j));
                    }
                }
                hand.remove(cs);
                return true;
            }
        }
        tmp = hand.at(0); //一番弱いカードから出す
        cs.insert(tmp);
        for (int i = 1; i < hand.size(); i++) {
            if (hand.at(0).rank() == hand.at(i).rank())
                cs.insert(hand.at(i));
            else
                break;
        }
        hand.remove(cs);
        return true;
    }

    //#######################################################################
    if (leadSize == 1) { // リードが 1 枚の場合
        if ((numhave() > 1 && numStrength(hand.at(mycheck(hand.at(0).rank()))) == 0) || hand.size() == 1) { //自分の二番目に弱いカードが必ず勝てる, もしくは自身のカードが 1 枚のとき
            tmp = most; //最強カードから出す
            cs.insert(tmp);
            hand.remove(tmp);
            return true;
        }
        if (numhave() == 2) { //持ってるカードの種類が 2 種類のとき
            if (numStrength(most) == 0) { //最強カードは出す
                tmp = most;
                cs.insert(tmp);
                hand.remove(tmp);
                return true;
            }
        }
        for (int i = 0; i < hand.size(); i++) {
            tmp = hand.at(i);
            if (tmp.isGreaterThan(pile.at(0))) {
                if (gstat.numPlayers >= 4 && befp == 1 && gstat.pile.at(0).strength() >= 11 && (numStrength(gstat.pile.at(0)) != 0 || numStrength(tmp) != 0) && pattern != 0) {
                    cs.clear();
                    return true;
                }
                if (gstat.numPlayers >= 4 && befp == 1 && gstat.pile.at(0).strength() >= 13) {
                    cs.clear();
                    return true;
                }
                if (pattern == 2) {
                    if ((tmp.strength() > 10 || tmp.rank() == most.rank()) && numStrength(tmp) > 0) {
                        continue;
                    }
                }
                if (pattern != 2) { //勝てるカード→最弱カードのループの実現のために JOKERを温存する理由がないので,pattern2 は除く
                    if (tmp.rank() == 15) { //Joker の分岐
                        if (check(2) == 0 && numStrength(pile.at(0)) == 0) { //出されたカードが最後の 2 もしくは最強のカードなら
                            cs.insert(tmp);
                            hand.remove(tmp);
                            return true;
                        } else {
                            cs.clear();
                            return true;
                        }
                    }
                }
                if (pattern == 1) { //慎重モード(このモード時点で既に JOKER が出ている)のプレー
                    if (tmp.rank() == 2) { //2 の分岐
                        if (mycheck(2) <= 1 && numStrength(tmp) > 0) { //自分の唯一の 2が Joker に負けるなら出さない
                            cs.clear();
                            return true;
                        }
                    }
                    if (tmp.rank() == 1) { //1 の分岐
                        if ((check(15) == 0 && check(2) == 0)) {
                            cs.insert(tmp); //1 が必ず勝てるなら(JOKER が出てるので 1 があったら被されちゃう)
                            hand.remove(tmp);
                            return true;
                        } else {
                            continue;
                        }
                    }
                }
                if (single() != 0 && gstat.numPlayers >= 3 && (tmp.strength() < 10 || tmp.rank() != most.rank()) && mycheck(tmp.rank()) > 1) {
                    continue; //プレイヤーが 4 人以上のとき, 出せるカード(手持ち最強カード以外)がペアをつくれるなら, それを出すのを避ける.(序盤はペアを温存したい,逆に後半は 1 枚でも消費したい)
                }
                cs.insert(tmp);
                hand.remove(tmp);
                return true;
            }
        }
    }

    //#######################################################################
    else { // 2 枚以上の場合
        for (int i = 0; i < hand.size(); i++) {
            tmp = hand.at(i);
            if (tmp.isGreaterThan(pile.at(0)) || tmp.isGreaterThan(pile.at(1))) { // by Teigo Nakamura 2020/07/04
                cs.clear();
                cs.insert(tmp);
                for (int j = 1; j < leadSize && i + j < hand.size(); j++) {
                    if (hand.at(i).rank() == hand.at(i + j).rank())
                        cs.insert(hand.at(i + j));
                }
                if (cs.size() == leadSize) { // 出せるカードが揃った
                    if (gstat.numPlayers == 2 && numStrength(tmp) == 0) {
                        hand.remove(cs);
                        return true;
                    }
                    if (gstat.numPlayers != 2 && pattern == 2 && tmp.strength() > 10 && (highnum(12) - leadSize) <= (lowhave(8) - mycheck(hand.at(0).rank()))) {
                        cs.clear();
                        return true;
                    }
                    if (numhave() < 3) { //カード種類数が 2 以下なら
                        if (tmp.rank() != most.rank() && mycheck(most.rank()) == leadSize) { //強いほうのペアが出せるなら強いほうから出す
                            continue;
                        } else {
                            hand.remove(cs);
                            return true;
                        }
                    }
                    if (mycheck(tmp.rank()) > leadSize && tmp.strength() < 10) { //11 以下のカードは, 手持ち以下の数で出さない
                        continue;
                    }
                    hand.remove(cs);
                    return true;
                }
            }
        }
    }

    // 出せるカードがないのでパス
    cs.clear();
    return true;
}

bool Group2::approve(const GameStatus &gstat) {
    int j = gstat.pile.size();
    for (int i = 0; i < j; i++) {
        if (memory.remove(gstat.pile.at(i)) == false)
            befp = 0;
        else
            befp = 1;
        memory.sort(myCardCmp);
    }
    if (turnchange > gstat.pile.at(0).strength() && beforesize == 1) {
        if ((check(15) == 1 || mycheck(15) == 1)) { //一番最初の leadsize=1 のターン後に, JOKER が場になかったら
            pattern = 0; //全力で勝ちに行くモード(JOKER が誰かの手元にあるなら, 2 が出づらい事を考えて, 1 でも 2 でもそれより強いカードに被せられるのを恐れない)
        }
    }
    if (check(15) == 0 && mycheck(15) == 0) { //JOKER が場に出たら pattern1(慎重)に戻す想定
        pattern = 1; //慎重に出す(2 が最強になるから).
    }
    if (lowhave(8) >= highnum(12) || lowhave(9) >= 5 || (hand.size() <= 7 && lowhave(7) >= 3)) { //13 以上の枚数よりも 8 以下の種類数が多いなら
        pattern = 2; //5 位阻止モード(1 位を取れないことを前提にする), 上の if に上書きされる可能性あり
    }
    if (lowhave(8) >= 4 && gstat.pile.size() == 0) { //「0 ターン目に」8 以下の数字を単体が 4 枚以上持ってるなら
        pattern = 2; //5 位阻止モード(1 位を取れないことを前提にする)
    }
    if (gstat.pile.size() == 0) { //ゲーム開始直後
        turnchange = 0;
        beforesize = 0;
    } else {
        turnchange = gstat.pile.at(0).strength(); //カードの強さを示す, これによってターン変化を知らせる
        beforesize = gstat.pile.size(); //pile.size()を次ゲームに伝える
    }
    return true;
}
