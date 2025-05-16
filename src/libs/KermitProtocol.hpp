#ifndef KERMITPROTOCOL_HPP_
#define KERMITPROTOCOL_HPP_

#include "RawSocket.hpp"
#include <cstddef>

#define NEXT_IDX(idx) (idx + 1) & ((1 << 5) - 1)
#define PREV_IDX(idx) (idx - 1)
#define INC_MOD_K(idx, k) (idx + 1) % k

namespace CustomProtocol {
  
  const unsigned long DATA_SIZE = 128;
  const unsigned char INIT_MARK = 0x7e;
  const unsigned long DATA_BUFFER_SIZE = 1 << 20;
  const unsigned long SPLIT_BUFFER_SIZE = DATA_BUFFER_SIZE / (1 << 7);
  
  // Given in miliseconds
  const unsigned long TIMEOUT_LEN = 100;

  const int REPEATED_MSG = 1;
  const int TIMEOUT_REACHED = 2;
  const int VALID_NEW_MSG = 3;
  const int INVALID_NEW_MSG = 4;

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
    unsigned short size    : 7;
    unsigned short idx     : 5;
    unsigned short type    : 4;
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
      size_t GetPkgSize(struct KermitPackage *pkg);
      /* Verify if bytes pointed by currentPkg represent a KermitPackage */
      bool IsMsgKermitPackage();
      /* Send current package. Make sure to initialize it with InitPackage.
       * It returns whatever RecvPackage returns after Send is called */
      int SendPackage(struct KermitPackage *pkg);
    public:
      PackageHandler(const char *netIntName);
      ~PackageHandler();
      /* Initialize current package with type (message type), data (pointer 
       * to data) and number of data bytes (<= 128). If there is no data to be
       * sent fill data with NULL and len with 0 */
      int InitPackage(unsigned char type, void *data, size_t len);
      /* Send package pointed by currentPkg */
      int SendCurrentPkg();
      /* Send package pointed by prevPkg */
      int SendPreviousPkg();
      /* Receive package in currentPkg. This method implements timeout. It may
       * return REPEATED_MSG, TIMEOUT_REACHED, VALID_NEW_MSG or INVALID_NEW_MSG. */
      int RecvPackage();
      /* If one dont want to overwrite currentPkg with Recv/InitPackage, call
       * this method: currentPkg will be then pointed by prevPkg */
      void SwapPkg();
      /* It returns const pointer to last received/initiated package */
      const struct KermitPackage *GetCurrentPkg();
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
      void RecvFile(char *filePath);
      /* Send data in file and END_OF_FILE message when done */
      void SendFile(char *filePath);
      /* Send len bytes pointer by ptr void pointer */
      void SendGenericData(MsgType msg, void *ptr, size_t len);
      /* Recei */
      MsgType RecvGenericData(void **ptr, size_t *len);
      /* Return pointer to read data. Make sure to duplicate this data */
      const unsigned char *ReadIncomingData();
  };
  
}

#endif