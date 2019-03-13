#ifndef __board__
#define __board__
#include<cctype>
#include<cstdio>
#include<algorithm>
#include<iostream>
#include<vector>

inline bool is_on_board(int x,int y){
    return x >= 0 && x <= 7 && y >= 0 && y <= 7;
}
//directions
const std::pair<int,int>dir[]{
    std::pair<int,int>(-1,+0),
    std::pair<int,int>(-1,-1),
    std::pair<int,int>(+0,-1),
    std::pair<int,int>(+1,-1),
    std::pair<int,int>(+1,+0),
    std::pair<int,int>(+1,+1),
    std::pair<int,int>(+0,+1),
    std::pair<int,int>(-1,+1),
};
class board{
    //0=empty, 1=black, 2=white
    unsigned char a[8][8];
    int my_tile,op_tile,pass;
public:
    // Construct board.
    constexpr board():a{
        {0,0,0,0,0,0,0,0,},
        {0,0,0,0,0,0,0,0,},
        {0,0,0,0,0,0,0,0,},
        {0,0,0,2,1,0,0,0,},
        {0,0,0,1,2,0,0,0,},
        {0,0,0,0,0,0,0,0,},
        {0,0,0,0,0,0,0,0,},
        {0,0,0,0,0,0,0,0,},
    },my_tile(1),op_tile(2),pass(0){}
    // Construct board from code.
    board(const char*st,const char*ed){
        if(ed-st==66&&std::all_of(st,ed,isdigit)){
            for(int i=0;i<8;i++){
                for(int j=0;j<8;j++){
                    a[i][j] = (*(st++)-'0')%3;
                }
            }
            my_tile = 2-(*(st++)-'0')%2;
            op_tile = 3-my_tile;
            pass = (*(st++)-'0')%3;
        }else{
            *this = board();
        }
    }
    // Updates board with move (x,y) and copies flipped locations to an array
    // start from oit.
    // Parameters
    // 1. x,y the move.
    // 2. oit the beginning of the destination array.
    // Return value
    // Pointer to the flipped locations, one past the last element copied.
    int*update(int x,int y,int*oit){
        if(is_on_board(x,y)){
            a[x][y] = my_tile;
            for(auto d:dir){
                auto nx = x, ny = y;
                do{
                    nx+= d.first, ny+= d.second;
                }while(is_on_board(nx,ny) && a[nx][ny] == op_tile);
                if(is_on_board(nx,ny) && a[nx][ny] == my_tile){
                    do{
                        nx-= d.first, ny-= d.second;
                        a[nx][ny] = my_tile;
                        *(oit++) = nx<<3^ny;
                    }while(nx!=x||ny!=y);
                }
            }
            pass = 0;
        }else{
            ++pass;
        }
        std::swap(my_tile,op_tile);
        return oit;
    }
    // Undo board with move (x,y).
    // Parameters
    // 1. x,y the move.
    // 2. pass_ pass counter before the move.
    // 3. st,ed Pointers of locations which were flipped by the move.
    void undo(int x,int y,int pass_,const int*st,const int*ed){
        std::swap(my_tile,op_tile);
        pass = pass_;
        for(;st!=ed;++st){
            a[*st/8][*st%8] = 3-a[*st/8][*st%8];
        }
        if(is_on_board(x,y)){
            a[x][y] = 0;
        }
    }
    // Check whether move (x,y) is legal.
    // Parameters
    // 1. x,y the move.
    // Return value
    // true if the move is legal
    bool is_valid_move(int x,int y)const{
        if(x==8&&y==0){
            int b[64];
			int c;
            return b==get_valid_move(b,c);
        }else if(is_on_board(x,y)==0 || a[x][y]!=0){
            return false;
        }
        for(auto d:dir){
            auto nx = x+d.first, ny = y+d.second;
            if(is_on_board(nx,ny) && a[nx][ny] == op_tile){
                do{
                    nx+= d.first, ny+= d.second;
                }while(is_on_board(nx,ny) && a[nx][ny] == op_tile);
                if(is_on_board(nx,ny) && a[nx][ny] == my_tile){
                    return true;
                }
            }
        }
        return false;
    }
    // Generate all legal non-pass moves and stores the result in an array,
    // beginning at oit.
    // Parameters
    // 1. oit the beginning of the destination array.
    // Return value
    // Pointer to the legal moves, one past the last move generated.
    int*get_valid_move(int*oit , int &valid_num)const{
		valid_num = 0;
        for(int i=0;i<8;++i){
            for(int j=0;j<8;++j){
                *oit = i<<3^j;
                oit+= a[i][j]==0 && is_valid_move(i,j);
				valid_num += a[i][j]==0 && is_valid_move(i,j); 
            }
        }
        return oit;
    }
    void show_board(FILE*fp)const{
        static constexpr char c[]{'.','X','O'};
        fprintf(fp,"%c's turn\n",c[my_tile]);
        {
            fprintf(fp,".|");
            for(int j=0;j!=8;++j){
                fprintf(fp,"%c|",'0'+j);
            }
            fputs("\n------------------\n",fp);
        }
        for(int i=0;i!=8;++i){
            fprintf(fp,"%c|",'0'+i);
            for(int j=0;j!=8;++j){
                fprintf(fp,"%c|",c[int(a[i][j])]);
            }
            fputs("\n------------------\n",fp);
        }
        fflush(fp);
    }
    std::pair<int,int> get_count()const{
        int cnt[3]{};
        for(int i=0;i<8;++i){
            for(int j=0;j<8;++j){
                ++cnt[int(a[i][j])];
            }
        }
        return std::pair<int,int>(cnt[1],cnt[2]);
    }
    int get_score()const{
        return get_count().first-get_count().second;
    }
    const unsigned char*operator[](int x)const{
        return a[x];
    }
    int get_pass()const{
        return pass;
    }
    int get_my_tile()const{
        return my_tile;
    }
    bool is_game_over()const{
        return pass==2;
    }
};
#endif
