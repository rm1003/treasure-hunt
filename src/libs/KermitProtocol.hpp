#ifndef KERMITPROTOCOL_HPP_
#define KERMITPROTOCOL_HPP_

#include "RawSocket.hpp"
#include <cstddef>

#define NEXT_IDX(idx) (idx + 1) & ((1 << 5) - 1)
#define PREV_IDX(idx) (idx - 1)
#define INC_MOD_K(idx, k) (idx + 1) % k

namespace CustomProtocol {

const unsigned long DATA_SIZE = 0x7f;
const unsigned char INIT_MARK = 0x7e;
const unsigned long DATA_BUFFER_SIZE = 1 << 20;
const unsigned long SPLIT_BUFFER_SIZE = DATA_BUFFER_SIZE / (1 << 7);

/* Given in miliseconds */
const unsigned long TIMEOUT_LEN = 100;

const int REPEATED_MSG = 1;
const int TIMEOUT_REACHED = 2;
const int VALID_NEW_MSG = 3;
const int INVALID_NEW_MSG = 4;

enum MsgType {
  ACK = 0,
  NACK,
  OK_AND_ACK,
  STOP_GAME,
  FILE_SIZE,
  DATA,
  TXT_FILE_NAME_ACK,
  VIDEO_FILE_NAME_ACK,
  IMG_FILE_NAME_ACK,
  END_OF_FILE,
  MOVE_RIGHT,
  MOVE_UP,
  MOVE_DOWN,
  MOVE_LEFT,
  INVERT,
  ERROR
};

const char NO_SPACE_ERROR[] = "1";

struct KermitPackage {
  unsigned char initMark;
  unsigned short size    : 7;
  unsigned short idx     : 5;
  unsigned short type    : 4;
  unsigned char checkSum;
  unsigned char data[DATA_SIZE];
}__attribute__((packed));

/* Setting fixed buf len to be double the size of a KermitPackage as
 * 0xff bytes must be inserted after every 0x88/0x81 sequence */
const unsigned long MAX_PACKAGE_SIZE = sizeof(KermitPackage) * 2;

class PackageHandler {
  private:
    CustomSocket::RawSocket *sokt;
    struct KermitPackage pkgs[2];
    struct KermitPackage *prevPkg;
    struct KermitPackage *currentPkg;
    unsigned char rawBytes[MAX_PACKAGE_SIZE];
    unsigned char currentPkgIdx;
    unsigned char lastRecvIdx;
    unsigned char lastUsedIdx;

    /* Fill currentPkg.initMark with 01111110 binary sequence */
    void SetInitMarkPkg();
    /* Append bytes that may be discard signal to network chip with 0xff.
     * It writes modified pkg in rawBytes array */
    void Append0xff(struct KermitPackage *pkg);
    /* Remove 0xff sequence after every 0x81/0x88 byte sequences. It reads from
     * rawBytes array and writes to pkg */
    void Remove0xff(struct KermitPackage *pkg);
    /* Return checksum. Make sure to call this after all other fields were
     * loaded */
    unsigned char ChecksumResolver();
    /* size of KermitPackage - DATA_SIZE + currentPkg.size */
    size_t GetPkgSize(struct KermitPackage *pkg);
    /* Verify if bytes pointed by currentPkg represent a KermitPackage */
    bool IsMsgKermitPackage();
    /* Send current package. Make sure to initialize it with InitPackage.
     * It returns whatever RecvPackage returns after Send is called */
    int SendPackage(struct KermitPackage *pkg);
    /* Verify checksum field of currentPkg */
    bool VerifyChecksum();
    /* make prevPkg point to pkgs[currentPkgIdx] and update currentPkg */
     void SwapPkg();
  public:
    PackageHandler(const char *netIntName);
    ~PackageHandler();
    /* Initialize current package with type (message type), data (pointer
     * to data) and number of data bytes (<= 128). If there is no data to be
     * sent fill data with NULL and len with 0 */
    int InitPackage(unsigned char type, void *data, unsigned short len);
    /* Send package pointed by currentPkg */
    int SendCurrentPkg();
    /* Send package pointed by prevPkg */
    int SendPreviousPkg();
    /* Receive package in currentPkg. This method implements timeout. It may
     * return REPEATED_MSG, TIMEOUT_REACHED, VALID_NEW_MSG or INVALID_NEW_MSG. */
    int RecvPackage();
    /* It returns const pointer to last received/initiated package */
    const struct KermitPackage *GetCurrentPkg();
};

class NetworkHandler {
  private:
    PackageHandler *pkgHandler;
    bool isFirstRecv;

    /* Return name of network interface. Make sure to free pointer */
    const char *GetEthIntName();

    void TransferData(const KermitPackage *retPkg, void *ptr, size_t *len);

  public:
    NetworkHandler();
    ~NetworkHandler();
    /* Send len bytes in ptr. Wait until acknoledgment or error */
    void SendGenericData(MsgType msg, void *ptr, size_t len);
    /* Does not block. Just unpacks currentPkg */
    MsgType RecvGenericData(void *ptr, size_t *len);
    /* Used by receiver to send response of a message (e.g. ACK). Blocks until
     * new valid message arrives */
    void SendResponse(MsgType msg);
    /* Used by sender to get response of a sent message
     * Does not block. Just unpacks currentPkg */
    MsgType RecvResponse(void *ptr, size_t *len);
    /* Receiver calls this method to become sender */
    void InvertToSender();
    /* Sender calls this method to become receiver */
    void InvertToReceiver();
};

}

#endif