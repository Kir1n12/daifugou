#ifndef _Group2_H_
#define _Group2_H_
#include <random>
#include "player.h"
#include "gamestatus.h"
//
// すこしルールを覚えたプレイヤーのクラス
// Player クラスを継承して作成するクラスのサンプル
//
class Group2 : public Player {
 CardSet memory; // 色々覚えておくための作業用
 CardSet trump; // 同上
 int turnchange, beforesize, befp, pattern;
public:
 Group2(const char *name = "Simple") : Player(name) {
 // 必要ならば初期化を書く．
 // 基底クラス Player の初期化の継承部分（ : の後の Player(name) の部分）
 // は変更しない．name = の後の部分は，デフォルト（引数を省略した場合の標準引数値）
 // として，他のグループと違う名前をつけるとよい．
 }
 ~Group2() { }
 // 思考処理の関数：このクラスで実装する
 void ready(void);
 bool follow(const GameStatus &, CardSet &);
 bool approve(const GameStatus &);
 int check(int num); //自身の手持ちを除く任意の数字の残り枚数検索
 int numStrength(const Card &target); //自身の手持ちを除く任意のカードより強い残りカード枚数検索
 int mycheck(int num); //自身の任意の数字枚数検索
 int minplayer(const GameStatus &gstat); //カード最小枚数のプレイヤーのカード枚数検索
  int numhave(void); //自身の持ち数字種類数検索
 int lowhave(int num); //任意の数字以下の種類数の検索
 int highnum(int num); //任意の数字以上の枚数の検索
 int single(void);
 // 比較関数(ソート用)
 static bool myCardCmp(const Card &a, const Card &b) {
 return a.strength() <= b.strength();
 }
};
#endif
