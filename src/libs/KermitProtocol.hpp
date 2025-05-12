#ifndef KERMITPROTOCOL_HPP_
#define KERMITPROTOCOL_HPP_

#include "RawSocket.hpp"

const unsigned long DATA_SIZE = 128;

namespace CustomProtocol {

  struct KermitPackage {
    unsigned char data[DATA_SIZE];
    unsigned char initMark;
    unsigned char checkSum;
    unsigned char size    : 7;
    unsigned char idx     : 5;
    unsigned char type    : 4;
  }__attribute__((packed));

  class PackageHandler {
    private:
      struct KermitPackage currentPkg;
      CustomSocket::RawSocket sokt;

    public:
      PackageHandler();
      ~PackageHandler();
      int InitPackage(unsigned char idx, unsigned char type, unsigned char *dataPtr);
      void SendPackage();
      void RecvPackage();
  };

  class NetworkHandler {
    private:
      PackageHandler pkgHandler;
      unsigned char *rawData;
      unsigned char **splitDataArr;

      void SplitRawData();
      void GatherRawData();
    public:
      NetworkHandler();
      ~NetworkHandler();
      const struct KermitPackage GetCurrentPkg();
      void SendDataInFile(char *filePath);
      void SendDataInPtr(unsigned char *ptr);
  };
  
}

#endif