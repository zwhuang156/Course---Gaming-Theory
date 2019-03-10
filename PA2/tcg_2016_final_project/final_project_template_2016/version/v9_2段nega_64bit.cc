/*****************************************************************************\
 * Theory of Computer Games: Fall 2012
 * Chinese Dark Chess Search Engine Template by You-cheng Syu
 *
 * This file may not be used out of the class unless asking
 * for permission first.
 *
 * Modify by Hung-Jui Chang, December 2013
\*****************************************************************************/
#include<cstdio>
#include<cstdlib>
#include<iostream>
#include <random>
#include<map>
#include"anqi.hh"
#include"Protocol.h"
#include"ClientSocket.h"

#ifdef _WINDOWS
#include<windows.h>
#else
#include<ctime>
#endif

const int DEFAULTTIME = 30;
typedef  int SCORE;
static const SCORE INF=1000001;
static const SCORE WIN=1000000;
unsigned long long int Zobrist[15][32];
unsigned long long int ZobristColor[2];
//SCORE SearchMax(const BOARD&,int,int);
//SCORE SearchMin(const BOARD&,int,int);
SCORE NegaScout(const BOARD&,int,int,SCORE,SCORE,bool,bool,unsigned int);

class hash_entry{
	public:
		unsigned int hash_val;
//		BOARD p;
		SCORE m;
		int dep;
		bool exact;
		int m_child; // which child m came from
};
std::map<unsigned int,hash_entry> trans_table;
std::map<unsigned int,hash_entry>::iterator it;
hash_entry e;

#ifdef _WINDOWS
DWORD Tick;     // 開始時刻
int   TimeOut;  // 時限
#else
clock_t Tick;     // 開始時刻
clock_t TimeOut;  // 時限
#endif
MOV   BestMove; // 搜出來的最佳著法

bool TimesUp() {
#ifdef _WINDOWS
	return GetTickCount()-Tick>=TimeOut;
#else
	return clock() - Tick > TimeOut;
#endif
}

//64bit-random
void random_64bit(){
	std::random_device rd;
	std::mt19937_64 e2(rd());
	std::uniform_int_distribution<unsigned long long int> dist(llround(std::pow(2, 61)), llround(std::pow(2, 62)));
	for (int i = 0; i < 15; i++){
		for (int j = 0; j < 32; j++)Zobrist[i][j] = dist(e2);
	}
	ZobristColor[0] = dist(e2);	ZobristColor[1] = dist(e2);
}

//翻棋策略
void safe_record(const BOARD &B,int safe_piece[][14],int p,int p_next){
	if(GetColor(B.fin[p_next])==B.who){
		switch(B.who){
			case 0:
				for(int i=7;i<=GetLevel(B.fin[p_next])+7;i++)safe_piece[p][i]=0;
				if(GetLevel(B.fin[p_next])==0)safe_piece[p][13]=0;
				break;
			case 1:
				for(int i=0;i<=GetLevel(B.fin[p_next]);i++)safe_piece[p][i]=0;
				if(GetLevel(B.fin[p_next])==0)safe_piece[p][6]=0;
				break;
		}
	}
	else if(GetColor(B.fin[p_next])!=B.who){
		switch(B.who){
			case 0:
				for(int i=GetLevel(B.fin[p_next]);i<7;i++){
					if(GetLevel(B.fin[p_next])==0&&i==6)break;
					safe_piece[p][i]=0;	
				}
				break;
			case 1:
				for(int i=GetLevel(B.fin[p_next])+7;i<14;i++){
					if(GetLevel(B.fin[p_next])==0&&i==13)break;
					safe_piece[p][i]=0;	
				}
				break;
		}	
	}
}

// 審局函數
SCORE Eval(const BOARD &B) {
	int total_power[2]={0,0};
	int chess_power;
	for(POS p=0;p<32;p++){
		const CLR c=GetColor(B.fin[p]);
		if(c!=-1){
			switch(GetLevel(B.fin[p])){
				case 0: chess_power=1000;break;
				case 1: chess_power=500;break;
				case 2: chess_power=180;break;
				case 3: chess_power=60;break;
				case 4: chess_power=20;break;
				case 5: chess_power=150;break;
				case 6:	chess_power=5;break;
			}
			total_power[c]=total_power[c] + chess_power;		
		}
	}
	return total_power[B.who]-total_power[B.who^1];
}

// dep=現在在第幾層
// cut=還要再走幾層
SCORE NegaScout(const BOARD &B,int dep,int cut,SCORE alpha,SCORE beta,bool hash_read,bool hash_write,unsigned int hash_value) {
	if(B.ChkLose())return -WIN;
//std::cout << "depth: "<<dep<<"\n";
	MOVLST lst;
	if(cut==0||TimesUp()||B.MoveGen(lst)==0){
		return +Eval(B);
	}
			
	SCORE m,n;
	m = -INF;
	n = beta;
	unsigned int index,hash_val;
	int start_child=0,temp_child=0;
	bool hash_hit=0;
	if(hash_read){
		if(dep==0){
			hash_val = ZobristColor[B.who];
			for(int i=0;i<32;i++){
				if(B.fin[i]!=15)hash_val^=Zobrist[B.fin[i]][i];
			}		
		}
		else if(dep!=0)hash_val=hash_value;		
		if(dep!=0){
			index = hash_val % 33554432;
			it = trans_table.find(index);
			if (it != trans_table.end()&&trans_table[index].hash_val==hash_val){
			// hash hit !!
				hash_hit=1;
//std::cout<<"exact: "<<trans_table[index].exact<<"\n";
				if(trans_table[index].exact==1){
//					if(dep==0)BestMove=lst.mov[trans_table[index].m_child];	
					if(dep<=trans_table[index].dep)return trans_table[index].m;
					else m=trans_table[index].m;
				}
				else{
//std::cout<<"child: "<<trans_table[index].m_child<<"\n";
//					if(dep==0)BestMove=lst.mov[trans_table[index].m_child];
					m=trans_table[index].m;
				} 
				start_child = trans_table[index].m_child;
			}
		}
	}
//std::cout << "hash_hit: "<<hash_hit<<"\n";
//if(dep==0){std::cout<<"lst.mum: "<<lst.num<<"\n";system("pause");}
	for(int i=start_child;i<lst.num;i++) {
//if(dep==0)std::cout<<"i: "<<i<<"\n";
//if(TimesUp())std::cout<<"times up!!"<<"\n";
		BOARD N(B);
		N.Move(lst.mov[i]);
		// 更新這一步走完後的hash_value(存到new_hash)
		unsigned int new_hash=0;
		if(hash_read){
			int moved_chess=B.fin[lst.mov[i].st];
			new_hash = hash_val^Zobrist[moved_chess][lst.mov[i].st]^Zobrist[moved_chess][lst.mov[i].ed];
			if(B.fin[lst.mov[i].ed]!=15)new_hash^=Zobrist[B.fin[lst.mov[i].ed]][lst.mov[i].ed];
		}
		
		const SCORE tmp=-NegaScout(N,dep+1,cut-1,-n,-std::max(alpha,m),hash_read,hash_write,new_hash);
		
		if(tmp>m){
			if(n==beta || cut<3 || tmp>=beta)m=tmp;
			else{m = -NegaScout(N,dep+1,cut-1,-beta,-tmp,hash_read,hash_write,new_hash);}			
			if(dep==0)BestMove=lst.mov[i];
			temp_child = i;
		}
		if(m >= beta){
			if(hash_write&&hash_hit==0){
				e.hash_val=hash_val; e.m=m; e.dep=dep; e.exact=0; e.m_child=i;
				trans_table[index] = e;
			}
			else if(hash_write&&hash_hit==1&&trans_table[index].dep<dep){
				e.hash_val=hash_val; e.m=m; e.dep=dep; e.exact=0; e.m_child=i;
				trans_table[index] = e;		
			}
//std::cout << "end_1 "<<"\n";
			return m;
		}
		n = std::max(alpha,m)+1;
	}
	if(hash_write&&hash_hit==0){
		e.hash_val=hash_val; e.m=m; e.dep=dep; e.exact=1; e.m_child=temp_child;
		trans_table[index] = e;
	}
	else if(hash_write&&hash_hit==1&&trans_table[index].dep<dep){
		e.hash_val=hash_val; e.m=m; e.dep=dep; e.exact=1; e.m_child=temp_child;
		trans_table[index] = e;
	}
//std::cout << "end_2 "<<"\n";
	return m;
}


MOV Play(const BOARD &B) {
	
#ifdef _WINDOWS
	Tick=GetTickCount();
	TimeOut = (DEFAULTTIME-3)*1000;
#else
	Tick=clock();
	TimeOut = (DEFAULTTIME-3)*CLOCKS_PER_SEC;
#endif
	POS p; int c=0;

	// 新遊戲？隨機翻子
	if(B.who==-1){p=rand()%32;printf("%d\n",p);return MOV(p,p);}

	//若未翻棋子<20則開始加入hash table功能
	bool do_hash;
	int dark = 0;
	for(int i=0;i<14;i++)dark+=B.cnt[i];
	if(dark<20)do_hash=1;
	
	// 若搜出來的結果會>現在 就用搜出來的走法
	MOVLST list;
	int useless=B.MoveGen(list);
	BestMove=list.mov[0];//先隨便記錄一步 避免沒棋可翻又找不到更好的走步時會crash
	
	SCORE tmpScore=NegaScout(B,0,4,-INF,INF,do_hash,0,0);
	MOV tmpMove=BestMove;
	SCORE BestScore=NegaScout(B,0,18,-INF,INF,do_hash,do_hash,0);
	if(tmpScore>BestScore)BestMove=tmpMove;
	if(BestScore>Eval(B)||tmpScore>Eval(B))return BestMove;

	// 否則隨便翻一個地方 但小心可能已經沒地方翻了
	for(p=0;p<32;p++)if(B.fin[p]==FIN_X)c++;
	if(c==0)return BestMove;
	
	//翻棋分數
	int tmp_cnt,safe_cnt=0;
	double safe_rate[33]={0};	
	int safe_piece[32][14];
	for(int i=0;i<32;i++){
		for(int j=0;j<14;j++)safe_piece[i][j]=B.cnt[j];
	}
	for(p=0;p<32;p++){
		tmp_cnt=0;
		if(B.fin[p]==14){
			if(p-1>=0&&p-1<=31&&(p-1)%4!=3&&B.fin[p-1]!=14&&B.fin[p-1]!=15)
			{safe_record(B,safe_piece,p,p-1);}
			if(p+1>=0&&p+1<=31&&(p+1)%4!=0&&B.fin[p+1]!=14&&B.fin[p+1]!=15)
			{safe_record(B,safe_piece,p,p+1);}
			if(p-4>=0&&p-4<=31&&B.fin[p-4]!=14&&B.fin[p-4]!=15)
			{safe_record(B,safe_piece,p,p-4);}
			if(p+4>=0&&p+4<=31&&B.fin[p+4]!=14&&B.fin[p+4]!=15)
			{safe_record(B,safe_piece,p,p+4);}
		
			for(int i=0;i<14;i++)tmp_cnt+=safe_piece[p][i];
//std::cout<<"P: "<<p<<" tmp_count: "<<tmp_cnt<<"\n";
//system("pause");
			safe_rate[p]= (double)tmp_cnt/dark;			
		}
	}
	POS tmp_p=33;
	safe_rate[33]=-1;
	for(p=0;p<32;p++){
		if(safe_rate[p]!=1&&safe_rate[p]>safe_rate[tmp_p]&&safe_rate[p]>=0.5)tmp_p=p;
	}
	p=tmp_p;
//std::cout<<"safe rate: "<<safe_rate[tmp_p]<<" p:"<<tmp_p<<"\n";

	if(p==33){
		for(p=0;p<32;p++)if(safe_rate[p]==1)safe_cnt++;
		if(safe_cnt==0){
			c=rand()%c;
			for(p=0;p<32;p++)if(B.fin[p]==FIN_X&&--c<0)break;
		}
		else {
			safe_cnt=rand()%safe_cnt;
			for(p=0;p<32;p++)if(safe_rate[p]==1&&--safe_cnt<0)break;
		}		
	}
	return MOV(p,p);
}

FIN type2fin(int type) {
    switch(type) {
	case  1: return FIN_K;
	case  2: return FIN_G;
	case  3: return FIN_M;
	case  4: return FIN_R;
	case  5: return FIN_N;
	case  6: return FIN_C;
	case  7: return FIN_P;
	case  9: return FIN_k;
	case 10: return FIN_g;
	case 11: return FIN_m;
	case 12: return FIN_r;
	case 13: return FIN_n;
	case 14: return FIN_c;
	case 15: return FIN_p;
	default: return FIN_E;
    }
}
FIN chess2fin(char chess) {
    switch (chess) {
	case 'K': return FIN_K;
	case 'G': return FIN_G;
	case 'M': return FIN_M;
	case 'R': return FIN_R;
	case 'N': return FIN_N;
	case 'C': return FIN_C;
	case 'P': return FIN_P;
	case 'k': return FIN_k;
	case 'g': return FIN_g;
	case 'm': return FIN_m;
	case 'r': return FIN_r;
	case 'n': return FIN_n;
	case 'c': return FIN_c;
	case 'p': return FIN_p;
	default: return FIN_E;
    }
}

int main(int argc, char* argv[]) {

#ifdef _WINDOWS
	srand(Tick=GetTickCount());
#else
	srand(Tick=time(NULL));
#endif
	
	random_64bit();

	BOARD B;
	if (argc!=2) {
	    TimeOut=(B.LoadGame("board.txt")-3)*1000;
	    if(!B.ChkLose())Output(Play(B));
	    return 0;
	}
	Protocol *protocol;
	protocol = new Protocol();
	protocol->init_protocol(argv[0],atoi(argv[1]));
	int iPieceCount[14];
	char iCurrentPosition[32];
	int type, remain_time;
	bool turn;
	PROTO_CLR color;

	char src[3], dst[3], mov[6];
	History moveRecord;
	protocol->init_board(iPieceCount, iCurrentPosition, moveRecord, remain_time);
	protocol->get_turn(turn,color);

	TimeOut = (DEFAULTTIME-3)*1000;

	B.Init(iCurrentPosition, iPieceCount, (color==2)?(-1):(int)color);

	MOV m;
	if(turn) // 我先
	{
	    m = Play(B);
	    sprintf(src, "%c%c",(m.st%4)+'a', m.st/4+'1');
	    sprintf(dst, "%c%c",(m.ed%4)+'a', m.ed/4+'1');
	    protocol->send(src, dst);
	    protocol->recv(mov, remain_time);
	    if( color == 2)
		color = protocol->get_color(mov);
	    B.who = color;
	    B.DoMove(m, chess2fin(mov[3]));
	    protocol->recv(mov, remain_time);
	    m.st = mov[0] - 'a' + (mov[1] - '1')*4;
	    m.ed = (mov[2]=='(')?m.st:(mov[3] - 'a' + (mov[4] - '1')*4);
	    B.DoMove(m, chess2fin(mov[3]));
	}
	else // 對方先
	{
	    protocol->recv(mov, remain_time);
	    if( color == 2)
	    {
		color = protocol->get_color(mov);
		B.who = color;
	    }
	    else {
		B.who = color;
		B.who^=1;
	    }
	    m.st = mov[0] - 'a' + (mov[1] - '1')*4;
	    m.ed = (mov[2]=='(')?m.st:(mov[3] - 'a' + (mov[4] - '1')*4);
	    B.DoMove(m, chess2fin(mov[3]));
	}
	B.Display();
	while(1)
	{
/*
std::cout << "remain time: "<<remain_time<<"\n";
system("pause");
*/
	    m = Play(B);
	    sprintf(src, "%c%c",(m.st%4)+'a', m.st/4+'1');
	    sprintf(dst, "%c%c",(m.ed%4)+'a', m.ed/4+'1');
	    protocol->send(src, dst);
	    protocol->recv(mov, remain_time);
	    m.st = mov[0] - 'a' + (mov[1] - '1')*4;
	    m.ed = (mov[2]=='(')?m.st:(mov[3] - 'a' + (mov[4] - '1')*4);
	    B.DoMove(m, chess2fin(mov[3]));
	    B.Display();

	    protocol->recv(mov, remain_time);
	    m.st = mov[0] - 'a' + (mov[1] - '1')*4;
	    m.ed = (mov[2]=='(')?m.st:(mov[3] - 'a' + (mov[4] - '1')*4);
	    B.DoMove(m, chess2fin(mov[3]));
	    B.Display();
	}

	return 0;
}
