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
      
      int CreateSocket(char *);
    public:
      RawSocket(char *);
      ~RawSocket();
      int Send(char *, size_t);
      int Recv(char *, size_t);
  };

}