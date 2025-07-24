#include <iostream>
#include <string>

#include "card.h"
#include "cardset.h"
#include "player.h"
#include "gamestatus.h"
#include "group3.h"

//指定カードが手札に何枚あるかをカウント
int Group3::count_rank(int rank) {
    int count = 0;
    for(int i = 0; i < hand.size(); i++){
        if(hand.at(i).strength() == rank){
            count++;
        }
    }

    return count;
}

//指定カードより強いカードをカウント
int Group3::count_stronger(int strength) {
    int count = 0;
    for(int i = field.size()-1; i >= 0; i--){
        if(field.at(i).strength() <= strength){
            break;
        }
        count++;
    }

    return count;
}

//手札のランクの種類
int Group3::count_type(){
    int count=1;
    for(int i = 1; i < hand.size(); i++){
        if(hand.at(i).strength() != hand.at(i-1).strength()){
            count++;
        }
    }

    return count;
}

//指定カードが優先権を持っているか
bool Group3::priority(int strength, int num){
    int count = 1;
    for(int i = field.size()-1; field.at(i).strength() > strength; i--){
        if(field.at(i).strength() == field.at(i-1).strength()){
            count++;
        }else {
            count = 1;
        }
        if(count >= num){
            return false;
        }
    }
    return true;
}

CardSet Group3::make_set(int point, int num){
    CardSet s;
    for(int i = 0; i < num; i++){
        s.insert(hand.at(point + i));
    }

    return s;
}

void Group3::ready() {
    for (int i = 0; i < 10; i++) {
        player_noplay[i].clear();
    }
    hand.sort(myCardCmp);
    /*フィールドに山札をセットし、自分の手札を抜く*/
    field.setupDeck();
    for(int i = 0; i < hand.size(); i++){
        field.remove(hand.at(i));
    }
}

bool Group3::approve(const GameStatus &gstat) {
    //出たカードをfieldから削除
    field.remove(gstat.pile);

    return true;
}

bool Group3::follow(const GameStatus &gstat, CardSet &cs) {
    CardSet pile(gstat.pile), s;
    Card tmp;
    int leadSize, weak, w_count, w_at, strong, s_count, s_at, s_type, tmp_num;

    hand.sort(myCardCmp);  // 手札をソート(弱い方から順に)
    field.sort(myCardCmp);

    leadSize = pile.size();

    weak = hand.at(0).strength();
    w_count = count_rank(weak);
    w_at = 0;
    for(int i = w_count; w_count > 1 || i < hand.size(); i += w_count){
        weak = hand.at(i).strength();
        w_at += w_count;
        w_count = count_rank(weak);
    }

    strong = hand.at(hand.size()-1).strength();
    s_count = count_rank(strong);
    s_at = hand.size()-s_count;
    s_type = 1;
    while(priority(strong, s_count) && s_at > 0){
        strong = hand.at(s_at-1).strength();
        s_count = count_rank(strong);
        s_at -= s_count;
        s_type++;
    }

    printf("count_type = %d\n s_at = %d\n", count_type(), s_at);

    if (leadSize == 0) { // 自分がリーダなら
        // 他プレイヤーの残り枚数が1枚かどうかチェック
        bool someone_one_left = false;
        for(int i = 0; i < gstat.numPlayers; i++) {
            // 自分自身は除外（playerID[自分] == myid などがあればそれで判定、なければ全員対象）
            if(gstat.numCards[i] == 1) {
                someone_one_left = true;
                break;
            }
        }
        int type_count = count_type();
        if(someone_one_left) {
            // 強いカード（最も強いランクのカード群）を惜しまず出す
        }
        if(type_count == 1){ //手札が一種類
            s = make_set(0, hand.size());
            cs.insert(s);
            hand.remove(s); 
            return true;
        } else if(type_count == 2) { //手札が2種類
            // 2種類の強さと枚数を調べる
            int first_rank = hand.at(0).strength();
            int first_count = count_rank(first_rank);
            int second_rank = hand.at(first_count).strength();
            int second_count = count_rank(second_rank);
            // 多い方から出す
            if(first_count >= second_count) {
                s = make_set(0, first_count);
            } else {
                s = make_set(first_count, second_count);
            }
            cs.insert(s);
            hand.remove(s);
            return true;
        } else if(s_at == 0){ //強いカードが優先権を持つ
            s = make_set(count_rank(strong), count_rank(hand.at(count_rank(strong)).strength()));
            cs.insert(s);
            hand.remove(s);
        } else if(type_count == s_type + 1){
            s = make_set(0, count_rank(hand.at(0).strength()));
            cs.insert(s);
            hand.remove(s);
        } else {
            for(tmp_num = 0; hand.at(tmp_num).strength() == weak; tmp_num++){ //最後に出す予定のカードは温存
            }
            s = make_set(tmp_num, count_rank(hand.at(tmp_num).strength()));
            cs.insert(s);
            hand.remove(s);
        }
        return true;
    }

    else { // リードではない場合
        // 5位阻止（最下位回避）戦略: 自分の手札が多く、他プレイヤーで残り2枚以下が2人以上いる場合は強いカードを惜しまず出す
        bool isPinch = false;
        int lowCount = 0;
        bool someone_one_left = false;
        for(int i = 0; i < gstat.numPlayers; i++) {
            if(gstat.numCards[i] <= 2) lowCount++;
            if(gstat.numCards[i] == 1) someone_one_left = true;
        }
        if(hand.size() >= 5 && lowCount >= 2) isPinch = true;
        if(isPinch || someone_one_left) {
            // 出せる最大枚数のセット（3→2→1枚）を優先して出す
            for(int n = std::min(leadSize, 3); n >= 1; n--) { // 3枚→2枚→1枚
                for(int i = 0; i <= hand.size() - n; i++) {
                    bool canPlay = true;
                    int baseStrength = hand.at(i).strength();
                    for(int j = 1; j < n; j++) {
                        if(hand.at(i+j).strength() != baseStrength) {
                            canPlay = false;
                            break;
                        }
                    }
                    if(canPlay && n == leadSize && hand.at(i).isGreaterThan(pile.at(0))) {
                        s = make_set(i, n);
                        cs.insert(s);
                        hand.remove(s);
                        return true;
                    }
                }
            }
            // セットで出せるものがなければ強いカード単体を惜しまず出す
            // unused variable strong_rank, strong_count 削除
            for(int i = hand.size()-1; i >= 0; i--) {
                if(hand.at(i).isGreaterThan(pile.at(0))) {
                    s = make_set(i, 1);
                    cs.insert(s);
                    hand.remove(s);
                    return true;
                }
            }
            // それ以外は通常通り
        }
        if(s_at == 0){ //強いカードが優先権を持つ
            tmp_num = hand.size()-1;
            for(int i = 0; i < s_type; i++){
                tmp = hand.at(tmp_num);
                if((tmp.strength() - leadSize > 0  && priority(tmp.strength(), count_rank(tmp.strength()) - leadSize))
                                                || leadSize == count_rank(tmp.strength())){ //優先権がある
                    int count = 0;
                    for(int i = 0; i < leadSize; i++){
                        tmp = hand.at(tmp_num-i);
                        if(tmp.isGreaterThan(pile.at(0))){
                            count++;
                        }
                    }
                    if(count == leadSize){
                        s = make_set(tmp_num-leadSize+1, leadSize);
                        cs.insert(s);
                        hand.remove(s);
                        return true;
                    }
                }
                tmp_num -= count_rank(hand.at(tmp_num).strength());
            }
            //優先権が無い
            cs.clear();
            return true;
        }else{
            for (int i = 0; i < hand.size(); i++) {
                tmp = hand.at(i);
                if (tmp.isGreaterThan(pile.at(0)) && count_rank(tmp.strength()) == leadSize) {
                    if(tmp.strength() < strong || s_type > 1){
                        s = make_set(i, leadSize);
                        cs.insert(s);
                        hand.remove(s);
                        return true;
                    }
                }
            }
        }
        cs.clear();
        return true;
    }

    // 出せるカードがないのでパス
    cs.clear();
    return true;
}

