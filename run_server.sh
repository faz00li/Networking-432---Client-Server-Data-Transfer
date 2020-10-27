# compile server, run on port 5555 w/ 20,000 reps
g++ -pthread server.cpp -o server

echo "ACTIVE SERVER..."
./server 5555 20000
