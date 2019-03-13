#include"OTP.h"
#include"my_socket.h"
#include<cstdio>
#include<string>
#include<vector>
#include<chrono>
#include<cctype>
std::string my_to_string(unsigned c,int n){
    std::string s(n,0);
    for(int i=n-1;i>=0;--i){
        s[i] = '0' + c%10;
        c/=10;
    }
    return s;
}
struct pic_f{
    std::vector<char>a;
    pic_f(const char*fn){
        auto fp(fopen(fn,"rb")); assert(fp!=nullptr);
        for(;;){
            auto N(a.size());
            a.resize(N+1024);
            auto r(fread(a.data()+N,1,1024,fp));
            a.erase(a.begin()+N+r,a.end());
            if(r==0)break;
        }
        fclose(fp);
        printf("load %s %zu bytes\n",fn,a.size());
    }
}pic[]{
    "./pic/e.png",
    "./pic/b.png",
    "./pic/w.png",
    "./pic/favicon.ico",
};
enum my_mode{VS_MODE=0,TEST_MODE=1};
std::string OTP::get_html(unsigned mode,unsigned time_used)const{
    static unsigned cnt = -1U; ++cnt;
    std::string op(
        "<html><head>"
        "<link rel=\"shortcut icon\" href=\"/p3\">"
        "<title>Othello</title>"
        "</head><body>"
    );
    static const std::string turn_str[]{
        "black's turn",
        "white's turn",
    };
    op+="<center>";
    op+=turn_str[B.get_my_tile()-1];
    op+=", passnum = ";
    op+=my_to_string(B.get_pass(),1);
    op+=", request counter = ";
    op+=my_to_string(cnt,10);
    const auto c(B.get_count());
    op+=", black: "+my_to_string(c.first ,2);
    op+=", white: "+my_to_string(c.second,2);
    op+="<br>";
    if(mode!=VS_MODE && B.is_game_over()==0){
        op+="<input type=\"button\" value=\"GENMOVE\" onclick=\"location.href='genmove'\">";
    }else{
        op+="<input type=\"button\" value=\"GENMOVE\">";
    }
    if(HED==H){
        op+="<input type=\"button\" value=\"UNDO\">";
    }else if(mode==VS_MODE){
        op+="<input type=\"button\" value=\"UNDO\" onclick=\"location.href='undoundo'\">";
    }else{
        op+="<input type=\"button\" value=\"UNDO\" onclick=\"location.href='undo'\">";
    }
    int ML[64],*st(ML),*const MLED(B.get_valid_move(ML));
    if(B.is_game_over() || MLED!=ML){
        op+="<input type=\"button\" value=\"PASS\">";
    }else if(mode==VS_MODE){
        op+="<input type=\"button\" value=\"PASS\" onclick=\"location.href='playgen+8+0'\">";
    }else{
        op+="<input type=\"button\" value=\"PASS\" onclick=\"location.href='play+8+0'\">";
    }
    op+="<input type=\"button\" value=\"CLEAR\" onclick=\"location.href='clear_board'\">";
    if(mode==VS_MODE){
        op+="<input type=\"button\" value=\"TEST MODE\" onclick=\"location.href='change_mode'\">";
    }else{
        op+="<input type=\"button\" value=\"VS MODE\" onclick=\"location.href='change_mode'\">";
    }
    op+="<br>";
    for(int i=0;i<8;++i){
        for(int j=0;j<8;j++){
            const bool is_move = st!=MLED&&*st==(i<<3^j);
            if(is_move){
                if(mode==VS_MODE){
                    op+="<a href=\"playgen+";
                }else{
                    op+="<a href=\"play+";
                }
                op+=char('0'+i);
                op+="+";
                op+=char('0'+j);
                op+="\">";
            }
            op+="<img src=\"/p";
            op+=char('0'+B[i][j]);
            op+="\">";
            if(is_move){
                op+="</a>";
                ++st;
            }
        }
        op+="<br>";
    }
    op+="code+";
    for(int i=0;i<8;++i){
        for(int j=0;j<8;j++){
            op+=char('0'+B[i][j]);
        }
    }
    op+=char('0'+B.get_my_tile());
    op+=char('0'+B.get_pass());
    op+="<br>";
    op+="use time: ";
    op+=my_to_string(time_used,10);
    op+="ms";
    op+="<br>";
    op+="</center>";
    op+="</body></html>";
    return op;
}
std::vector<std::string>my_token(const char*bst,const char*bed,int n){
    std::vector<std::string>token;
    for(;n&&bst!=bed;--n){
        auto nst(bst);
        for(;nst!=bed&&isspace(*nst)!=0;++nst);
        if(nst == bed)break;
        bst = nst;
        for(;bst!=bed&&isspace(*bst)==0;++bst);
        token.emplace_back(nst,bst);
    }
    return token;
}
int main(int argc, char*argv[]){
    if( argc != 2 ){
        printf("usage: %s port#",argv[0]);
        return 1;
    }
    const auto server(get_sfd(atoi(argv[1])));
    OTP EG;
    my_mode mode(TEST_MODE);
    for(;;){
        const skt cfd(server.get_cfd());
        char cmd[1024],*cmded(cmd);
        char buf[1024],*const bed(cfd.Recv_r<true>(buf,std::end(buf)-buf-1));
        const auto token(my_token(buf,bed,2));
        int type = 404;
        if(token.size() == 2){
            auto ted = token.cbegin();
            type = *ted == "GET"?200:404; ++ted;
            if(0<ted->size() && ted->size()<=size_t(std::end(cmd)-cmd)){
                cmded = std::copy(ted->begin()+1,ted->end(),cmd);
            }
        }
        *cmded = 0;
        std::replace(cmd,cmded,'+',' ');
        puts(cmd);
        printf("buf:%zu gap:%zu type:%3d\n",bed-buf,cmded-cmd,type);
        fflush(stdout);
        switch(type){
        case 200:{
            std::string header(
                "HTTP/1.1 200 OK\n"
                "Content-length: "
            ),content;
            if(cmded-cmd==2&&cmd[0]=='p'){
                int id((cmd[1]-'0')&3);
                content.append(pic[id].a.begin(),pic[id].a.end());
                header+= my_to_string(content.size(),10);
                header+= "\nLast-modified: Thu, 11 Oct 2012 10:11:00 GMT";
                header+= "\nContent-Type: image/png\n\n";
            }else if(my_hash(cmd)==my_hash("change_mode")){
                mode = my_mode(mode^1);
                content = EG.get_html(mode,0);
                header+= my_to_string(content.size(),10);
                header+= "\nContent-Type: text/html\n\n";
            }else{
                typedef std::chrono::system_clock clock;
                typedef std::chrono::milliseconds tunit;
                std::chrono::time_point<clock> st(clock::now());
                EG.do_op(cmd,buf,stdout);
                content = EG.get_html(mode,std::chrono::duration_cast<tunit>(clock::now() - st).count());
                header+= my_to_string(content.size(),10);
                header+= "\nContent-Type: text/html\n\n";
            }
            cfd.Send_r<true>(header.c_str(),header.size());
            cfd.Send_r<true>(content.c_str(),content.size());
            printf("header:%zu content:%zu\n",header.size(),content.size());
            break;
        }
        default:
            static const char s[]{"HTTP/1.1 404\n"};
            cfd.Send_r<true>(s,std::end(s)-s);
        }
    }
    return 0;
}
