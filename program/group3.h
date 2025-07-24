
#ifndef _GROUP3_H_
#define _GROUP3_H_

#include <random>

#include "player.h"
#include "gamestatus.h"

//
// すこしルールを覚えたプレイヤーのクラス
// Player クラスを継承して作成するクラスのサンプル
//
class Group3 : public Player {
    CardSet field;  // 色々覚えておくための作業用
    CardSet comp;
    CardSet player_noplay[10];   // 同上

public:
    Group3(const char *name = "group3_B") : Player(name) {
      // 必要ならば初期化を書く．
      // 基底クラス Player の初期化の継承部分（ : の後の Player(name)　の部分）
      // は変更しない．name = の後の部分は，デフォルト（引数を省略した場合の標準引数値）
      // として，他のグループと違う名前をつけるとよい．
    }
    ~Group3() { }

    // 思考処理の関数：このクラスで実装する
    int count_rank(int);
    int count_stronger(int);
    int count_type(void);
    bool priority(int, int);
    CardSet make_set(int, int);
    bool approve(const GameStatus &);
    void ready(void);
    bool follow(const GameStatus &, CardSet &);
    

    // 比較関数(ソート用)
    static bool myCardCmp(const Card &a, const Card &b) {
      return a.strength() <= b.strength();
    }

};

#endif
