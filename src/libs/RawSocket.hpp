#ifndef RAWSOCKET_HPP_
#define RAWSOCKET_HPP_

#include <arpa/inet.h>
extern "C" {
#include <net/ethernet.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <stdlib.h>
}

namespace CustomSocket {

  // Given in miliseconds
  const unsigned long TIMEOUT_LEN = 100;
  
  class RawSocket {
    private:
      int socketFd;
      size_t fixedBufLen;
      unsigned char *fixedBuf;
      
      int CreateSocket(const char *netIntName);
      void SetRecvTimeout();
    public:
      RawSocket(const char *netIntName);
      ~RawSocket();
      /* Set fixed number of bytes to be sent everytime Send is called. This
       * implies that Recv also reads fixedBufLen bytes. This method also 
       * allocates fixexBuf */
      void SetFixedBufLen(size_t len);
      int Send(void *ptr, size_t len);
      int Recv(void *ptr, size_t len);
  };

}

#endif