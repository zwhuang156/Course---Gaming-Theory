g++ judge.cpp -std=c++11 -O2 -Wall -o judge -lws2_32
g++ search.cpp -std=c++11 -O2 -Wall -o search -lws2_32
echo compile_finish
START /B judge.exe 7122 > log_ju.txt
START /B search_10.exe 127.0.0.1 7122 > 10.txt
START /B search_50.exe 127.0.0.1 7122 > 50.txt
echo total finish