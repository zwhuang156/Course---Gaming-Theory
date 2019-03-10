#ifndef ANQI
#define ANQI

// (color)
//  0 = 紅方 (大寫字母)
//  1 = 黑方 (小寫字母)
// -1 = 都不是
typedef int CLR;

// (level)
enum LVL {
	LVL_K=0, // 帥將 King
	LVL_G=1, // 仕士 Guard
	LVL_M=2, // 相象 Minister
	LVL_R=3, // 硨車 Rook     // BIG5 沒有人部的車
	LVL_N=4, // 傌馬 kNight
	LVL_C=5, // 炮砲 Cannon
	LVL_P=6  // 兵卒 Pawn
};

enum FIN {
	FIN_K= 0 /* 'K' 帥 */ , FIN_k= 7 /* 'k' 將 */ , FIN_X=14 /* 'X' 未翻 */ ,
	FIN_G= 1 /* 'G' 仕 */ , FIN_g= 8 /* 'g' 士 */ , FIN_E=15 /* '-' 空格 */ ,
	FIN_M= 2 /* 'M' 相 */ , FIN_m= 9 /* 'm' 象 */ ,
	FIN_R= 3 /* 'R' 硨 */ , FIN_r=10 /* 'r' 車 */ ,
	FIN_N= 4 /* 'N' 傌 */ , FIN_n=11 /* 'n' 馬 */ ,
	FIN_C= 5 /* 'C' 炮 */ , FIN_c=12 /* 'c' 砲 */ ,
	FIN_P= 6 /* 'P' 兵 */ , FIN_p=13 /* 'p' 卒 */
};

// (position)
//  0  1  2  3
//  4  5  6  7
//  8  9 10 11
// 12 13 14 15
// 16 17 18 19
// 20 21 22 23
// 24 25 26 27
// 28 29 30 31
typedef int POS;

struct MOV {
	POS st; // 起點
	POS ed; // 終點 // 若 ed==st 表示是翻子

	MOV() {}
	MOV(POS s,POS e):st(s),ed(e) {}

	bool operator==(const MOV &x) const {return st==x.st&&ed==x.ed;}
	MOV operator=(const MOV &x) {st=x.st;ed=x.ed;return MOV(x.st, x.ed);}
};

struct MOVLST {
	int num;     // 走法數(移動+吃子,不包括翻子)
	MOV mov[68];
};

struct BOARD {
	CLR who;     // 現在輪到那一方下
	FIN fin[32]; // 各個位置上面擺了啥
	int cnt[14]; // 各種棋子的未翻開數量

	void NewGame();              // 開新遊戲
	int  LoadGame(const char*);  // 載入遊戲並傳回時限(單位:秒)
	void Display() const;        // 顯示到 stderr 上
	int  MoveGen(MOVLST&) const; // 列出所有走法(走子+吃子,不包括翻子)
	                             // 回傳走法數量
	bool ChkLose() const;        // 檢查當前玩家(who)是否輸了
	bool ChkValid(MOV) const;    // 檢查是否為合法走法
	void Flip(POS,FIN=FIN_X);    // 翻子
	void Move(MOV);              // 移動 or 吃子
	void DoMove(MOV m, FIN f) ;
	//void Init(int Board[32], int Piece[14], int Color);
	void Init(char Board[32], int Piece[14], int Color);
};

CLR  GetColor(FIN);    // 算出棋子的顏色
LVL  GetLevel(FIN);    // 算出棋子的階級
bool ChkEats(FIN,FIN); // 判斷第一個棋子能否吃第二個棋子
void Output (MOV);     // 將答案傳給 GUI

#endif
