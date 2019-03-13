#include"board.h"
#include<random>
#ifdef _WIN32
#include<chrono>
#endif
#include<cstring>
#include<string>
#include<iostream>
#include<fstream>
#include<vector>
#include<stdlib.h>
#include <math.h>
#include <time.h>

using namespace std;


constexpr char m_tolower(char c){
    return c+('A'<=c&&c<='Z')*('a'-'A');
}
constexpr unsigned my_hash(const char*s,unsigned long long int hv=0){
    return *s&&*s!=' '?my_hash(s+1,(hv*('a'+1)+m_tolower(*s))%0X3FFFFFFFU):hv;
}
struct history{
    int x,y,pass,tiles_to_flip[27],*ed;
};
template<class RIT>RIT random_choice(RIT st,RIT ed){
#ifdef _WIN32
    //std::random_device is deterministic with MinGW gcc 4.9.2 on Windows
    static std::mt19937 local_rand(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
#else
    static std::mt19937 local_rand(std::random_device{}());
#endif
    return st+std::uniform_int_distribution<int>(0,ed-st-1)(local_rand);
}

class node{
    public:	
		board b;
		bool is_real_root;
		int win_count , total_count;
		double ucb , win_rate;
		int move;
		vector<node*> child;
		node *parent;
		
		node(){is_real_root = 0;}
};

class OTP{
    node *root;
    board B;
    history H[128],*HED;
    //initialize in do_init
    void do_init(){
        B = board();
        HED = H;
    }

//=================   use Monte-Carlo to choose move   ============================    
    //choose the best move in do_genmove
    int do_genmove(){
		clock_t start , finish;
		double time_spend;
		start = clock();
		
		vector<node*> leaf;
    	board B_temp , B_leaf;
		root = new node;
		root->is_real_root = 1;
		root->total_count = 0;
		node *temp_node;
		int chosen_child , chosen_to_expand , valid_num;
		int ML[64],*MLED(B.get_valid_move(ML,valid_num));
		int x,y,xy;
		int temp_my_tile = B.get_my_tile();
		bool is_valid = 1;
	
		cout << "1 ==================================="<<endl;
		cout << "My Tile: "<<temp_my_tile<<endl;
		if(valid_num==0){
			valid_num++;
			is_valid = 0;
		}
		for(int i = 0 ; i < valid_num ; i++){
			// choose a leaf and simulate
			temp_node = new node;
			temp_node->win_count = 0;
			temp_node->total_count = 0;
			B_leaf = B;
			if(is_valid == 0){
				x = 8;
				y = 0;
			}
			else{
				x = ML[i]/8;
				y = ML[i]%8;				
			}
			HED->ed = B_leaf.update(x,y,HED->tiles_to_flip);
			
			int scout;
			int simu[64],*simuED(B_leaf.get_valid_move(simu,scout));
			if(B_leaf.get_pass()>=1 && scout==0){
cout << "return 64"<<endl;
				delete temp_node;
				return 64;
			}
			else{
				temp_node->b = B_leaf;
				//start simulate
				for(int simulate = 0 ; simulate < 50 ; simulate++){
					B_temp = B_leaf;
					for(int j = 0 ; B_temp.get_pass() < 2 ; j++){
//					B_temp.show_board(stdout);
//					cout << "pass: "<<B_temp.get_pass()<<endl;
						int useless;
						int simu[64],*simuED(B_temp.get_valid_move(simu,useless));
						xy = simuED==simu?64:*random_choice(simu,simuED);
						x = xy/8;
						y = xy%8;
						HED->ed = B_temp.update(x,y,HED->tiles_to_flip);
					}
					if(temp_my_tile==1 && B_temp.get_score()>0){
						temp_node->win_count++;
					}
					else if(temp_my_tile==2 && B_temp.get_score()<0){
						temp_node->win_count++;
					}
					temp_node->total_count++;
				}
				root->total_count += temp_node->total_count;
				if(is_valid == 0){
					temp_node->move = 64;
				}
				else{
					temp_node->move = ML[i];				
				}				
				temp_node->parent = root;			
				root->child.push_back(temp_node);
				leaf.push_back(temp_node);
			}
		}
		
		double temp_rate=0 , temp_ucb=0;
		for(int i = 0 ; i < root->child.size() ; i++){
			UCB(root->child[i]->win_count , root->child[i]->total_count , root->total_count , root->child[i]->win_rate , root->child[i]->ucb ,2);			
//			cout <<"node "<<i<<": "<<endl<<"Win Rate: "<<root->child[i]->win_rate<<endl<<"UCB: "<<root->child[i]->ucb<<endl;
			if(root->child[i]->win_rate >= temp_rate){
				temp_rate = root->child[i]->win_rate;
				chosen_child = i;
			}
		}
		for(int i = 0 ; i < leaf.size() ; i++){
			if(leaf[i]->ucb > temp_ucb){
				temp_ucb = leaf[i]->ucb;
				chosen_to_expand = i;
			}
		}

		while(1){
			finish = clock();
			time_spend = (double)(finish - start) / CLOCKS_PER_SEC;
			if(time_spend < 9.95 && leaf.size()>0){
				MCS(leaf[chosen_to_expand] , chosen_to_expand , temp_my_tile , leaf , root , chosen_child );
			}
			else{
				break;
			}
			
		}
				
cout <<"return a child"<<endl;
		cout << "2 ==================================="<<endl;
				
		return root->child[chosen_child]->move;   
    }

	/* calcu_type = 1 means only calculate win rate */
	/* calcu_type = 2 means calculate win rate and UCB */
	void UCB(int win , int total , int parent_total , double &win_rate , double &ucb, int calcu_type){
		double Ni , N ;
		N = parent_total;
		Ni = total;
		win_rate = (double)win/total;
		if(calcu_type==2)
		{ucb = win_rate + 1.18*sqrt(log(N)/Ni);}
	}
		
		
	void MCS(node* root , int &chosen_to_expand , int temp_my_tile , vector<node*> &leaf , node *real_root , int &chosen_child){
		
		leaf.erase(leaf.begin()+chosen_to_expand);
		board B_temp ;
		node *temp_node;
		int valid_num;
		int ML[64],*MLED(root->b.get_valid_move(ML,valid_num));
		int x,y,xy;
		bool is_valid = 1;
			
		if(valid_num==0){
			valid_num++;
			is_valid = 0;
		}
		for(int i = 0 ; i < valid_num ; i++){
			// choose a leaf and simulate
			temp_node = new node;
			temp_node->win_count = 0;
			temp_node->total_count = 0;
			B_temp = root->b;
			if(is_valid == 0){
				x = 8;
				y = 0;
			}
			else{
				x = ML[i]/8;
				y = ML[i]%8;				
			}

			HED->ed = B_temp.update(x,y,HED->tiles_to_flip);
			int scout;
			int simu[64],*simuED(B_temp.get_valid_move(simu,scout));
			if(B_temp.get_pass()>=1 && scout==0){
				delete temp_node;
			}
			else{
				temp_node->b = B_temp;
	
				//start simulate
				for(int simulate = 0 ; simulate < 50 ; simulate++){
					for(int j = 0 ; B_temp.get_pass() < 2 ; j++){
//					B_temp.show_board(stdout);
						int useless;
						int simu[64],*simuED(B_temp.get_valid_move(simu,useless));
						xy = simuED==simu?64:*random_choice(simu,simuED);
						x = xy/8;
						y = xy%8;
						HED->ed = B_temp.update(x,y,HED->tiles_to_flip);
					}
					if(temp_my_tile==1 && B_temp.get_score()>0){
						temp_node->win_count++;
					}
					else if(temp_my_tile==2 && B_temp.get_score()<0){
						temp_node->win_count++;
					}
					temp_node->total_count++;
				}

				root->total_count += temp_node->total_count;
				root->win_count += temp_node->win_count;
				if(is_valid == 0){
					temp_node->move = 64;
				}
				else{
					temp_node->move = ML[i];				
				}
				temp_node->parent = root;			
				root->child.push_back(temp_node);
				leaf.push_back(temp_node);
			}
		}
		
		back_propagation(root);
		
		ProgressivePruning(root);
		
		
		double temp_rate=0 , temp_ucb=0;
		// choose the highest win rate move
		for(int i = 0 ; i < real_root->child.size() ; i++){
			UCB(real_root->child[i]->win_count , real_root->child[i]->total_count , real_root->total_count , real_root->child[i]->win_rate , real_root->child[i]->ucb ,1);			
			if(real_root->child[i]->win_rate > temp_rate){
				temp_rate = real_root->child[i]->win_rate;
				chosen_child = i;
			}
		}
			
//cout <<	"leaf size: "<< leaf.size()<<" chosen_to_expand: "<<chosen_to_expand<<endl;	
		for(int i = 0 ; i < leaf.size() ; i++){
			UCB(leaf[i]->win_count , leaf[i]->total_count , leaf[i]->parent->total_count , leaf[i]->win_rate , leaf[i]->ucb ,2);
			

			if(leaf[i]->ucb > temp_ucb){
				temp_ucb = leaf[i]->ucb;
				chosen_to_expand = i;
			}
		}
	
	}

	void back_propagation(node *root){
		node *temp_root ;
		temp_root = root;
		int temp_total=0 , temp_win=0;
		
		for(int i = 0 ; i < root->child.size() ; i++){
			temp_total += root->child[i]->total_count;
			temp_win += root->child[i]->win_count;
		}		
		while(1){
			if(temp_root->is_real_root == 0){
				temp_root->total_count += temp_total;
				temp_root->win_count += temp_win;
				temp_root = temp_root->parent;
			}
			else{
				temp_root += temp_total;
				break;
			}
		}
	}
	
	void ProgressivePruning(node *root){
		double mean=0 , deviation;
		double win_rate;
		int count=0;
	
		for(int i = 0 ; i < root->child.size() ; i++){
			if(root->child[i]->total_count > 1000){
				count++;
			}
		}
		if(count == root->child.size()){
			// calculate mean
			for(int i = 0 ; i < root->child.size() ; i++){
				win_rate = (double) root->child[i]->win_count / root->child[i]->total_count;
				root->child[i]->win_rate = win_rate;
				mean += win_rate ;			
			}
			mean = mean/root->child.size();
			//calculate deviation
			double temp = 0;
			for(int i = 0 ; i < root->child.size() ; i++){			
				temp += (root->child[i]->win_rate - mean) * (root->child[i]->win_rate - mean);			
			}
			deviation = sqrt(temp / root->child.size());	
		}
		
	}

//=======================================================



    //update board and history in do_play
    void do_play(int x,int y){
        if(HED!=std::end(H)&&B.is_game_over()==0&&B.is_valid_move(x,y)){
            HED->x = x;
            HED->y = y;
            HED->pass = B.get_pass();
            HED->ed = B.update(x,y,HED->tiles_to_flip);
            ++HED;
        }else{
            fputs("wrong play.\n",stderr);
        }
    }
    //undo board and history in do_undo
    void do_undo(){
        if(HED!=H){
            --HED;
            B.undo(HED->x,HED->y,HED->pass,HED->tiles_to_flip,HED->ed);
        }else{
            fputs("wrong undo.\n",stderr);
        }
    }
public:
    OTP():B(),HED(H){
        do_init();
    }
    bool do_op(const char*cmd,char*out,FILE*myerr){
        switch(my_hash(cmd)){
            case my_hash("name"):
                sprintf(out,"name template7122");
                return true;
            case my_hash("clear_board"):
                do_init();
                B.show_board(myerr);
                sprintf(out,"clear_board");
                return true;
            case my_hash("showboard"):
                B.show_board(myerr);
                sprintf(out,"showboard");
                return true;
            case my_hash("play"):{
                int x,y;
                sscanf(cmd,"%*s %d %d",&x,&y);
                do_play(x,y);
                B.show_board(myerr);
                sprintf(out,"play by p1");
                return true;
            }
            case my_hash("genmove"):{
                int xy = do_genmove();
cout << "xy: "<<xy<<endl;
                int x = xy/8, y = xy%8;
                do_play(x,y);
                B.show_board(myerr);
                sprintf(out,"genmove %d %d by p1",x,y);
                return true;
            }
            case my_hash("undo"):
                do_undo();
                sprintf(out,"undo");
                return true;
            case my_hash("final_score"):
                sprintf(out,"final_score %d",B.get_score());
                return true;
            case my_hash("quit"):
                sprintf(out,"quit");
                return false;
            //commmands used in simple_http_UI.cpp
            case my_hash("playgen"):{
                int x,y;
                sscanf(cmd,"%*s %d %d",&x,&y);
                do_play(x,y);
                if(B.is_game_over()==0){
                    int xy = do_genmove();
                    x = xy/8, y = xy%8;
                    do_play(x,y);
                }
                B.show_board(myerr);
                sprintf(out,"playgen %d %d",x,y);
                return true;
            }
            case my_hash("undoundo"):{
                do_undo();
                do_undo();
                sprintf(out,"undoundo");
                return true;
            }
            case my_hash("code"):
                do_init();
                B = board(cmd+5,cmd+strlen(cmd));
                B.show_board(myerr);
                sprintf(out,"code");
                return true;
            default:
                sprintf(out,"unknown command");
                return true;
        }
    }
    std::string get_html(unsigned,unsigned)const;
};
