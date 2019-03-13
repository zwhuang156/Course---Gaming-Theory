g++ judge.cpp -std=c++11 -O2 -Wall -o judge -lws2_32
g++ search.cpp -std=c++11 -O2 -Wall -o search -lws2_32
START /B judge.exe 7122 > log_ju.txt
START /B search.exe 127.0.0.1 7122 > log_p1.txt
START /B search.exe 127.0.0.1 7122 > log_p2.txt