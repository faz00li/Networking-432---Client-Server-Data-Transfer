#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/uio.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <pthread.h>
#include <chrono>

using namespace std;

const int BUFFSIZE = 1500;
const int NUM_CONNECTIONS = 5;

/*
 * Structure for passing socket and CLI param to thread
 */
struct thread_args
{
  int incomingSD;
  int repetitions;
};

void* server_thread(void* incomingSD);

int main(int argc, char *argv[])
{
  int portNumber;
  int repetitions;
  
  /*
   * Validate number of parameters
   */
  if (argc != 3)
    cout << "Usage: " << argv[0] << " [port] " << " [repetitions]" << endl;
  
  /*
   * Assign variables from CLI
   */
  portNumber = atoi(argv[1]);
  repetitions = atoi(argv[2]);

  /*
   * Build Address
   */
  sockaddr_in acceptSocketAddress;
  bzero((char *)&acceptSocketAddress, sizeof(acceptSocketAddress));
  acceptSocketAddress.sin_family = AF_INET;
  acceptSocketAddress.sin_addr.s_addr = htonl(INADDR_ANY);
  acceptSocketAddress.sin_port = htons(portNumber);
  
  /*
   * Create socket w/ quick release
   */
  int serverSD = socket(AF_INET,SOCK_STREAM, 0);
  
  const int on = 1;
  setsockopt(serverSD, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(int));
  
  /*
   * Bind
   */
  int rc = ::bind(serverSD, (sockaddr*) &acceptSocketAddress, sizeof(acceptSocketAddress));
  
  if (rc < 0)
  {
    cerr << "Bind failed" << endl;
    exit(-1);
  }
  
  /*
   * Listen
   */
  rc = listen(serverSD, NUM_CONNECTIONS);
  
  if (rc < 0)
  {
    cerr << "Listen failed" << endl;
    exit(-1);
  }
  
  while(true)
  {
    /*
     * Accept new connections and create worker thread
     */
    sockaddr_in newSockAddr;
    socklen_t newSocketAddrSize = sizeof(newSockAddr);
    
    int newSD = accept(serverSD,(sockaddr *) &newSockAddr, &newSocketAddrSize);
    
    struct thread_args params;
    params.incomingSD = newSD;
    params.repetitions = repetitions;
    pthread_t tid;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_create(&tid, &attr, server_thread, (void *) &params);
  }
  
  close(serverSD);
  
  return 0;
}

/*
 * Worker thread, accepts thread_args structure
 */
void* server_thread(void *params)
{
  unsigned long long totalBytesRead = 0;
  
  int bytesRead = 0;
  int readCount = 0;
  
  struct thread_args newParams = *(thread_args*) params;
  int newSD = newParams.incomingSD;
  
  char databuf[BUFFSIZE];
  bzero(databuf, BUFFSIZE);
  
  /*
   * Time how long it takes to read all data sent across network
   */
  auto startRTT = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < newParams.repetitions; i++)
  {
    while (bytesRead < BUFFSIZE)
    {
      bytesRead += read(newSD, &databuf, BUFFSIZE - bytesRead);
      readCount++;
    }
    totalBytesRead += bytesRead;
    bytesRead = 0;
  }
  auto endRTT = std::chrono::high_resolution_clock::now();
 
  /*
   * Report number of reads
   */
  write(newSD, &readCount, sizeof(readCount));
  
  /*
   * calculate
   */
  float elapsed_time = double(std::chrono::duration_cast<std::chrono::microseconds> (endRTT - startRTT).count());
  cout << "data-receiving time = " << elapsed_time << " usec" << endl;
  
  close(newSD);
  
  return (void *)0;
}

