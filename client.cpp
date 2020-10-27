#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <sys/uio.h>
#include <stdio.h>
#include <stdlib.h>
#include <chrono>
#include <iostream>
#include <iomanip>

using namespace std;

int main(int argc, char *argv[])
{
  char *serverIP;
  char *port;
  int repetition;
  int nbufs;
  int bufsize;
  int type;
  
  /*
   * Argument validation
   */
  if (argc != 7)
  {
    cerr << "Usage: " << "[port] " << "[repetition] "
    << "[nbufs] " << "[bufsize] " << "[serverIp] " << "[type] " << endl;
    return -1;
  }
  
  /*
   * Assign variables from CLI
   */
  port = argv[1];
  repetition = atoi(argv[2]);
  nbufs = atoi(argv[3]);
  bufsize = atoi(argv[4]);
  serverIP = argv[5];
  type = atoi(argv[6]);
  
  /*
   * Describe server address
   */
  int clientSD = -1;
  struct addrinfo hints;
  struct addrinfo *result, *rp;
  
  memset(&hints,0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = 0;
  hints.ai_protocol = 0;
  
  /*
   * Generate list of possible server addresses
   */
  int rc = getaddrinfo(serverIP, port, &hints, &result);
  
  if (rc != 0)
  {
    cerr << "ERROR: " << gai_strerror(rc) << endl;
    exit(EXIT_FAILURE);
  }
  
  for (rp = result; rp != NULL; rp = rp->ai_next)
  {
    clientSD = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (clientSD == -1)
    {
      continue;
    }
    
    //connect to addres using
    rc = connect(clientSD, rp->ai_addr, rp->ai_addrlen);
    if (rc < 0)
    {
      cerr << "Connection Failed" << endl;
      close(clientSD);
      return -1;
    }
    else//success
    {
      break;
    }
  }
  
  if (rp == NULL)
  {
    cerr << "No valid address" << endl;
    exit(EXIT_FAILURE);
  }
  
  freeaddrinfo(result);
  
  /*
   * Client Work
   */
  int bytesWritten = 0;
  int bytesRead = 0;
  long readCount = 0;
  
  /*
   * Start timer for round trip - full transmission
   */
  double elapsed_time = 0;
  
  auto startRTT = std::chrono::high_resolution_clock::now();

  switch(type) {
    case 1:
    {
      //case1 - Multiple Writes
      char databuf[nbufs][bufsize];//create double array
      
      for (int i = 0; i < repetition; i++)
      {
        auto startLap1 = std::chrono::high_resolution_clock::now();// Time and add lap times for total
        for (int j = 0; j < nbufs; j++)
        {
          write(clientSD, &databuf[j], bufsize);
        }
        auto endLap1 = std::chrono::high_resolution_clock::now();
        elapsed_time = elapsed_time + double(std::chrono::duration_cast<std::chrono::microseconds> (endLap1 - startLap1).count());
      }
    }
      break;
      
    case 2:
      //case 2 - writev()
    {
      char databuf[nbufs][bufsize];//create double array
      struct iovec vector[nbufs];//io vector
      
      for (int i = 0; i < repetition; i++)
      {
        auto startLap2 = std::chrono::high_resolution_clock::now();
        for (int j = 0; j < nbufs; j++)
        {
          vector[j].iov_base = databuf[j];
          vector[j].iov_len = bufsize;
        }
        writev(clientSD, vector, nbufs);
        auto endLap2 = std::chrono::high_resolution_clock::now();
        elapsed_time += double(std::chrono::duration_cast<std::chrono::microseconds> (endLap2 - startLap2).count());
      }
    }
      break;
      
    case 3:
    {
      //case3 - Single Write
      char databuf[bufsize * nbufs];//create array
      
      for (int i = 0; i < repetition; i++)//single write * num reps
      {
        auto startLap = std::chrono::high_resolution_clock::now();
        write(clientSD, &databuf, bufsize * nbufs);
        auto endLap = std::chrono::high_resolution_clock::now();
        elapsed_time += double(std::chrono::duration_cast<std::chrono::microseconds> (endLap - startLap).count());
      }
    }
      break;
  }
  
  //end RTT timer
  auto endRTT = std::chrono::high_resolution_clock::now();
  float duration = double(std::chrono::duration_cast<std::chrono::microseconds> (endRTT - startRTT).count());
  
  read(clientSD, &readCount, sizeof(readCount));//collect number of reads from server
  
  cout << "Test " << type << " data-sending time  = " << elapsed_time << " usec, round-trip time = "
  << duration <<  " usec, #reads = " << readCount  << endl;
  close(clientSD);
  
  return 0;
}
