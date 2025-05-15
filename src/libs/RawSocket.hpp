#ifndef RAWSOCKET_HPP_
#define RAWSOCKET_HPP_

#include <arpa/inet.h>
#include <cstddef>
extern "C" {
#include <net/ethernet.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <stdlib.h>
#include <stdio.h> 
}

namespace CustomSocket {

  // Given in miliseconds
  const unsigned long TIMEOUT_LEN = 100; 
  
  class RawSocket {
    private:
      int socketFd;
      
      int CreateSocket(char *netIntName);
      void SetRecvTimeout();
    public:
      RawSocket(char *netIntName);
      ~RawSocket();
      int Send(char *buf, size_t len);
      int Recv(char *buf, size_t len);
  };

}

#endif