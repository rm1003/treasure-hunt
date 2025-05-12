#ifndef RAWSOCKET_HPP_
#define RAWSOCKET_HPP_

#include <arpa/inet.h>
#include <cstddef>
#include <net/ethernet.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <stdlib.h>
#include <stdio.h>

namespace CustomSocket {

  class RawSocket {
    private:
      int socketFd;
      
      int CreateSocket(char *netIntName);
    public:
      RawSocket(char *);
      ~RawSocket();
      int Send(char *buf, size_t len);
      int Recv(char *buf, size_t len);
  };

}

#endif