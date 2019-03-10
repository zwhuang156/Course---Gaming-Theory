g++ judge.cpp -std=c++11 -O2 -Wall -o judge
echo compile_judge_complete
g++ search.cpp -std=c++11 -O2 -Wall -o search
echo compile_search_complete
rm -rf log_ju.txt log_mcs.txt log_random.txt
./judge 5285 > log_ju.txt &
./search 127.0.0.1 5285 > log_mcs.txt &
./../random/search 127.0.0.1 5285 > log_random.txt &

