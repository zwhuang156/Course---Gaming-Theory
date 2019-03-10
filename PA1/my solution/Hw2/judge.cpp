#include"my_socket.h"
#include"board.h"
#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<chrono>
#include<algorithm>
constexpr char m_tolower(char c){
    return c+('A'<=c&&c<='Z')*('a'-'A');
}
constexpr unsigned my_hash(const char*s,unsigned long long int hv=0){
    return *s&&*s!=' '?my_hash(s+1,(hv*('a'+1)+m_tolower(*s))%0X3FFFFFFFU):hv;
}
constexpr timeval GENMOVE_TIME_LIMIT{10,0};
constexpr timeval OTHEROP_TIME_LIMIT{1,0};
constexpr int     INITIAL_TIME_LIMIT(10); //second
struct player{
    char name[1024];
    skt sk;
    int cn;
    bool ok;
    player(SOCKET fd):name{"unknown"},sk(fd),cn(),ok(fd!=INVALID_SOCKET){}
    player(player&&b):sk(std::move(b.sk)),cn(b.cn),ok(b.ok){
        std::copy(b.name,b.name+1024,name);
    }
    player&operator=(player&&b){
        std::copy(b.name,b.name+1024,name);
        sk = std::move(b.sk);
        cn = b.cn;
        ok = b.ok;
        return *this;
    }
    player(const player&b) = delete;
    player&operator=(const player&b) = delete;
};
int main(int argc, char*argv[]){
    int R = 2;
    switch(argc){
        case 3:
            R = atoi(argv[2]);
        case 2:
            break;
        default:
            puts("usage: judge port# [round]");
            return 1;
    }
    const get_sfd server(atoi(argv[1]));
    SOCKET cli[2]{INVALID_SOCKET,INVALID_SOCKET};
    typedef std::chrono::system_clock my_clock;
    typedef std::chrono::microseconds my_tunit;
    const std::chrono::time_point<my_clock> st = my_clock::now();
    constexpr int u_per = 1000000;
    for(int i=0, rest_time=INITIAL_TIME_LIMIT*u_per;i<2&&rest_time>0;){
        cli[i] = server.get_cfd_t({rest_time/u_per,rest_time%u_per});
        int wait_time = std::chrono::duration_cast<my_tunit>(my_clock::now() - st).count();
        if(cli[i]!=INVALID_SOCKET){
            printf("ply%d accepted after %d us\n",i,wait_time);
            ++i;
        }
        rest_time = INITIAL_TIME_LIMIT*u_per - wait_time;
    }
    char buf[1024];
    player ply[2]{cli[0],cli[1]};
    for(auto&p:ply)if(p.ok){
        p.sk.Send_r("name");
        p.sk.Recv_t(buf,1023,OTHEROP_TIME_LIMIT);
        p.ok = my_hash(buf)==my_hash("name");
        if(p.ok){
            sscanf(buf,"name %s",p.name);
        }
    }
    for(int r=R;r;--r){
        if(ply[0].ok&&ply[1].ok){
            for(auto&p:ply){
                p.sk.Send_r("clear_board");
                p.sk.Recv_t(buf,1023,OTHEROP_TIME_LIMIT);
                p.ok = my_hash(buf)==my_hash("clear_board");
            }
        }
        board B;
        for(;ply[0].ok&&ply[1].ok&&B.is_game_over()==0;){
            auto&np = ply[+B.get_my_tile()-1];
            auto&op = ply[-B.get_my_tile()+2];
            np.sk.Send_r("genmove");
            np.sk.Recv_t(buf,1023,GENMOVE_TIME_LIMIT);
            int x,y,t[64];
            if(my_hash(buf)==my_hash("genmove")&&sscanf(buf,"%*s %d %d",&x,&y)==2&&B.is_valid_move(x,y)){
                B.update(x,y,t);
                sprintf(buf,"play %d %d",x,y);
                op.sk.Send_r(buf);
                op.sk.Recv_t(buf,1023,OTHEROP_TIME_LIMIT);
                op.ok = my_hash(buf)==my_hash("play");
            }else{
                np.ok = false;
            }
        }
        const int scr = ply[0].ok&&ply[1].ok?B.get_score():ply[0].ok-ply[1].ok;
        ply[0].cn+= scr>0;
        ply[1].cn+= scr<0;
        std::swap(ply[0],ply[1]);
    }
    if(R&1){
        std::swap(ply[0],ply[1]);
    }
    for(auto&p:ply)if(p.ok){
        p.sk.Send_r("quit");
        p.sk.Recv_t(buf,1023,OTHEROP_TIME_LIMIT);
        p.ok = my_hash(buf)==my_hash("quit");
    }
    {
        auto&np = ply[0], &op = ply[1];
        for(int i=0;i<2;++i){
            printf("NAME:%s QUIT:%d WIN:%d LOSE:%d DRAW:%d\n",np.name,np.ok,np.cn,op.cn,R-np.cn-op.cn);
            std::swap(np,op);
        }
    }
    auto et = std::chrono::duration_cast<std::chrono::seconds>(my_clock::now().time_since_epoch()).count();
    unsigned et_h = et/60/60%24;
    unsigned et_m = et/60%60;
    unsigned et_s = et%60;
    printf("time stamp: %02u:%02u:%02u\n",et_h,et_m,et_s);
    return 0;
}
