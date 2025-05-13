#ifndef KERMITPROTOCOL_HPP_
#define KERMITPROTOCOL_HPP_

#include "RawSocket.hpp"

const unsigned long DATA_SIZE = 128;

const unsigned char INIT_MARK = 0x7e;

namespace CustomProtocol {

  enum MsgTypes {
    ACK = 0,
    NACK,
    OK_AND_ACK,
    A_DEFINIR_1,
    FILE_SIZE,
    DATA,
    TXT_FILE_NAME,
    VIDEO_FILE_NAME,
    IMG_FILE_NAME,
    END_OF_FILE,
    MOVE_RIGHT,
    MOVE_UP,
    MOVE_DOWN,
    MOVE_LEFT,
    A_DEFINIR_2,
    ERROR
  };

  struct KermitPackage {
    unsigned char initMark;
    unsigned char size    : 7;
    unsigned char idx     : 5;
    unsigned char type    : 4;
    unsigned char checkSum;
    unsigned char data[DATA_SIZE];
  }__attribute__((packed));

  class PackageHandler {
    private:
      struct KermitPackage currentPkg;
      CustomSocket::RawSocket *sokt;

      void SetInitMarkPkg();
    public:
      PackageHandler(char *netIntName);
      ~PackageHandler();
      int InitPackage(unsigned char idx, unsigned char type, 
                      unsigned char *dataPtr, unsigned long len);
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