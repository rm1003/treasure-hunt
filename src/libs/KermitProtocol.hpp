#ifndef KERMITPROTOCOL_HPP_
#define KERMITPROTOCOL_HPP_

#include "RawSocket.hpp"
#include <cstddef>

#define NEXT_IDX(idx) (idx + 1) & ((1 << 5) - 1)
#define PREV_IDX(idx) (idx - 1)
#define INC_MOD_K(idx, k) (++idx) % k

namespace CustomProtocol {
  
  const unsigned long DATA_SIZE = 128;
  const unsigned char INIT_MARK = 0x7e;
  const unsigned long DATA_BUFFER_SIZE = 1 << 20;
  const unsigned long SPLIT_BUFFER_SIZE = DATA_BUFFER_SIZE / (1 << 7);
  // Given in miliseconds
  const unsigned long TIMEOUT_LEN = 100;

  enum MsgType {
    ACK = 0,          // reserved
    NACK,             // reserved
    OK_AND_ACK,       // reserved
    A_DEFINIR_1,      
    FILE_SIZE,      
    DATA,             // reserved
    TXT_FILE_NAME,  
    VIDEO_FILE_NAME,
    IMG_FILE_NAME,
    END_OF_FILE,      // reserved
    MOVE_RIGHT,     
    MOVE_UP,
    MOVE_DOWN,
    MOVE_LEFT,
    A_DEFINIR_2,
    ERROR             // reserved
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
      CustomSocket::RawSocket *sokt;
      struct KermitPackage pkgs[2];
      struct KermitPackage *prevPkg;
      struct KermitPackage *currentPkg;
      unsigned char currentPkgIdx;
      unsigned char lastRecvIdx;
      unsigned char lastUsedIdx;

      /* Fill currentPkg.initMark with 01111110 binary sequence */
      void SetInitMarkPkg();
      /* Append bytes that may be discard signal to network chip with 0xff */
      void Append0xffToPkg();
      /* Remove 0xff sequence that was inserted before package was sent */
      void Remove0xffInsertedInPkg();
      /* Fill checksum field of currentPkg. Make sure to call this after all 
       * other fields were loaded */
      void ChecksumResolver();
      /* size of KermitPackage - DATA_SIZE + currentPkg.size */
      size_t GetCurrentPkgSize();
      /* Verify if bytes pointed by currentPkg represent a KermitPackage */
      bool IsMsgKermitPackage();
    public:
      PackageHandler(char *netIntName);
      ~PackageHandler();
      /* Initialize current package with type (message type), data (pointer 
       * to data) and number of data bytes (<= 128) */
      int InitPackage(unsigned char type, unsigned char *data, size_t len);
      /* Send current package. Make sure to initialize it with InitPackage.
       * if NACK/ERROR is received, send previous package again */
      void SendPackage();
      /* Receive package in currentPkg. Send ACK if everything is right and
       * return; NACK if checksum does not match or ERROR if index is wrong and 
       * and wait. This method implements timeout */
      int RecvPackage();
      /* Get currentPkg.type */
      MsgType GetMsgTypeOfCurrentPkg();
      /* Copy currentPkg.data to ptr using memcpy and currentPkg.size to len */
      void GetDataInCurrentPkg(unsigned char *ptr, size_t *len);
      /* Verify checksum field of currentPkg */
      bool VerifyChecksum();
  };

  class NetworkHandler {
    private:
      PackageHandler *pkgHandler;
      unsigned char **splitDataArr;
      unsigned char *buffer;
      size_t bufferOffset;

      /* Return name of network interface. Make sure to free this pointer */
      const char *GetEthIntName();
      /* Split data pointed by ptr in splitDataArr respecting DATA_SIZE */
      void SplitRawData(unsigned char *ptr, size_t len);
      /* Append received data to buffer. If buffer does not have enough space,
       * return 1, otherwise return 0 */
      int AppendtoBuffer(unsigned char *ptr, size_t len);
      /* Flush number of buffer offset bytes to file */
      void FlushBuffer();
    public:
      NetworkHandler();
      ~NetworkHandler();
      /* Write incoming data to file until END_OF_FILE message is read */
      void WriteDataInFile(char *filePath);
      /* Send data in file and END_OF_FILE message when done */
      void SendDataInFile(char *filePath);
      /* Send len bytes pointer by ptr void pointer */
      void SendDataInPtr(void *ptr, size_t len);
      /* Send message indicated by msg. Do not use to send reserved message
       * types */
      void SendMsg(MsgType msg);
      /* Return pointer to read data. Make sure to duplicate this data */
      const unsigned char *ReadIncomingData();
  };
  
}

#endif