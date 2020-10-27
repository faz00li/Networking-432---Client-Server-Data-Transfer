# compile client, connect to server CSS432B-01.uwb.edu aka 10.158.82.113

g++ client.cpp -std=c++11 -o client

echo "CASE 3 -> 20,000"
./client 5555 20000 10 150 CSS432B-01.uwb.edu 3
./client 5555 20000 30 50 CSS432B-01.uwb.edu 3
./client 5555 20000 60 25 CSS432B-01.uwb.edu 3

echo "CASE 2 -> 20,000"
./client 5555 20000 10 150 CSS432B-01.uwb.edu 2
./client 5555 20000 30 50 CSS432B-01.uwb.edu 2
./client 5555 20000 60 25 CSS432B-01.uwb.edu 2

echo "CASE 1 -> 20,000"
./client 5555 20000 10 150 CSS432B-01.uwb.edu 1
./client 5555 20000 30 50 CSS432B-01.uwb.edu 1
./client 5555 20000 60 25 CSS432B-01.uwb.edu 1