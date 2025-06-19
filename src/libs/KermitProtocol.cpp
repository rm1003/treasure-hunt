#include "KermitProtocol.hpp"
#include "Logging.hpp"

#include <bits/types/struct_timeval.h>
#include <cassert>
#include <cstddef>
#include <cstring>
extern "C" {
#include <net/if.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/time.h>
}

using CustomProtocol::KermitPackage;

unsigned long timestamp() {
  /* Static to avoid multiple allocations */
  static struct timeval tp;
  gettimeofday(&tp, NULL);
  return tp.tv_sec * 1000 + tp.tv_usec / 1000;
}

void PrintErrorMsgType(CustomProtocol::MsgType msg, const char *location) {
  ERROR_PRINT("Unexpected message type [%d] in [%s]\n", msg, location);
}

//=================================================================//
// CustomProtocol::PackageHandler
//=================================================================//

CustomProtocol::PackageHandler::PackageHandler(const char *netIntName) {
  this->sokt = new CustomSocket::RawSocket(netIntName);
  this->SetInitMarkPkg();
  this->lastRecvIdx = this->lastUsedIdx = 0;
}

CustomProtocol::PackageHandler::~PackageHandler() {
  delete this->sokt;
}

//===================================================================
// SetSendPkgData
//===================================================================

void CustomProtocol::PackageHandler::InitSendPackage(unsigned short type, void *data,
                                                     unsigned short len) {
  /* data */
  if (len > DATA_SIZE) {
    ERROR_PRINT("Data is too long to fit a package. Exiting.\n");
    exit(1);
  }
  if (data != NULL) {
    memcpy(this->sendPkg.data, data, len);
    this->sendPkg.size = len;
  }
  /* type */
  this->sendPkg.type = type;
  /* sequence number */
  this->sendPkg.idx = NEXT_IDX(this->lastUsedIdx);
  this->lastUsedIdx = this->sendPkg.idx;
  /* checksum */
  this->sendPkg.checkSum = this->ChecksumResolver(SEND_PKG);
  /* 0xff */
  this->Append0xff();
}

//===================================================================
// SendPackage
//===================================================================
int CustomProtocol::PackageHandler::SendPackage() {
  return this->sokt->Send(this->rawBytes, sizeof(this->rawBytes));
}

//===================================================================
// RecvPackage
//===================================================================
int CustomProtocol::PackageHandler::RecvPackage() {
  int ret;
  unsigned long init = timestamp();

  do {
    ret = this->sokt->Recv(this->rawBytes, sizeof(this->rawBytes));
    if (ret == -1) {
      continue;
    }
    if (this->IsMsgKermitPackage()) {
      this->Remove0xff();
      if (!this->VerifyChecksum()) {
        DEBUG_PRINT("Invalid new message arrived.\n");
        return INVALID_NEW_MSG;
      }
      if (this->recvPkg.idx == this->lastRecvIdx) {
        DEBUG_PRINT("Repeated message. Maybe sender did not get ACK.\n");
        return REPEATED_MSG;
      }
      if (this->recvPkg.idx != this->lastRecvIdx + 1) {
        ERROR_PRINT("Invalid sequence number. Exiting.\n");
        exit(1);
      }
      this->lastRecvIdx = this->recvPkg.idx;
      return VALID_NEW_MSG;
    }
  } while (timestamp() - init < TIMEOUT_LEN);

  return TIMEOUT_REACHED;
}

//===================================================================
// GetCurrentPkg
//===================================================================
const struct KermitPackage* CustomProtocol::PackageHandler::GetRecvPkg() {
  return &this->recvPkg;
}

//===================================================================
// SetInitMarkPkg
//===================================================================
void CustomProtocol::PackageHandler::SetInitMarkPkg() {
  this->sendPkg.initMark = INIT_MARK;
}

//===================================================================
// ChecksumResolver
//===================================================================
unsigned char CustomProtocol::PackageHandler::ChecksumResolver(int pkg) {
  struct KermitPackage *currentPkg;
  switch (pkg) {
    case SEND_PKG:
      currentPkg = &this->sendPkg;
      break;
    case RECV_PKG:
      currentPkg = &this->recvPkg;
      break;
    default:
      ERROR_PRINT("Invalid [pkg] in ChecksumResolver. Exiting.\n");
      exit(1);
  }

  unsigned short sum = 0;

  sum += currentPkg->size;
  sum += currentPkg->idx;
  sum += currentPkg->type;
  for (size_t it = 0; it < currentPkg->size; it++) {
    sum += currentPkg->data[it];
  }

  return (unsigned char)sum;
}

//===================================================================
// Append0xff
//===================================================================
void CustomProtocol::PackageHandler::Append0xff() {
  unsigned char *pkgBytes = (unsigned char *)(&this->sendPkg);
  unsigned long pkgIt = 0;
  unsigned long rawBytesIt = 0;

  memset(this->rawBytes, 0, sizeof(this->rawBytes));

  for (; pkgIt < sizeof(KermitPackage); rawBytesIt++, pkgIt++) {
    this->rawBytes[rawBytesIt] = pkgBytes[pkgIt];
    if (pkgBytes[pkgIt] == 0x88 || pkgBytes[pkgIt] == 0x81) {
      this->rawBytes[++rawBytesIt] = 0xff;
    }
  }
}

//===================================================================
// Remove0xff
//===================================================================
void CustomProtocol::PackageHandler::Remove0xff() {
  unsigned char *pkgBytes = (unsigned char *)(&this->recvPkg);
  unsigned long rawBytesIt = 0;
  unsigned long pkgIt = 0;

  for (; pkgIt < sizeof(KermitPackage); pkgIt++, rawBytesIt++) {
    pkgBytes[pkgIt] = this->rawBytes[rawBytesIt];
    if (pkgBytes[pkgIt] == 0x88 || pkgBytes[pkgIt] == 0x81) {
      rawBytesIt++;
    }
  }
}

//===================================================================
// VerifyChecksum
//===================================================================
bool CustomProtocol::PackageHandler::VerifyChecksum() {
  bool check = this->recvPkg.checkSum == this->ChecksumResolver(RECV_PKG);
  if (check == false) {DEBUG_PRINT("[VerifyChecksum] yield false\n");}
  return check;
}

//===================================================================
// IsMsgKermitPackage
//===================================================================
bool CustomProtocol::PackageHandler::IsMsgKermitPackage() {
  return (rawBytes[0] == INIT_MARK);
}


//=================================================================//
// CustomProtocol::NetworkHandler
//=================================================================//

//===================================================================
// NetworkHandler
//===================================================================
CustomProtocol::NetworkHandler::NetworkHandler() {
  const char *ethIntName = this->GetEthIntName();
  this->pkgHandler = new PackageHandler(ethIntName);
  free((void*)ethIntName);
  this->isFirstRecv = true;
}

CustomProtocol::NetworkHandler::~NetworkHandler() {
  delete this->pkgHandler;
}

//===================================================================
// GetEthIntName
//===================================================================
const char *CustomProtocol::NetworkHandler::GetEthIntName() {
  struct if_nameindex *ifArr;
  struct if_nameindex *ifIt;
  const char *ptr;

  ifArr = if_nameindex();
  if (!ifArr) {
    ERROR_PRINT("Could not get eth interface name arr\n");
    exit(1);
  }

  for (ifIt = &ifArr[0]; ifIt->if_name != NULL; ifIt++) {
    if (strstr(ifIt->if_name, "eth") || strstr(ifIt->if_name, "enp")) {
      ptr = strdup(ifIt->if_name);
      if_freenameindex(ifArr);
      return ptr;
    }
  }

  if_freenameindex(ifArr);

  return NULL;
}


//===================================================================
// Recv
//===================================================================
void CustomProtocol::NetworkHandler::TransferData(const KermitPackage *retPkg,
                                          void *ptr, size_t *len) {
  /* If ptr and len is NULL return MsgType (Invert case) */
  if (ptr) {
    memcpy(ptr, retPkg->data, retPkg->size);
    if (!len) return;
    *len = retPkg->size;
  }
}

//===================================================================
// SendGeneticData
//===================================================================
void CustomProtocol::NetworkHandler::SendGenericData(MsgType msg, void *ptr, size_t len) {
  this->pkgHandler->InitSendPackage(msg, ptr, len);
  this->pkgHandler->SendPackage();
  int feedBack = this->pkgHandler->RecvPackage();
  unsigned short type = this->pkgHandler->GetRecvPkg()->type;

  while (feedBack != VALID_NEW_MSG || type == NACK) {
    this->pkgHandler->SendPackage();
    feedBack = this->pkgHandler->RecvPackage();
    type = this->pkgHandler->GetRecvPkg()->type;
  }
}

//===================================================================
// RecvGenericData
//===================================================================
CustomProtocol::MsgType CustomProtocol::NetworkHandler::RecvGenericData(void *ptr,
                                                                        size_t *len) {
  const KermitPackage *retPkg;
  int feedBack;

  if (this->isFirstRecv) {
    /* create NACK pkg in case it is needed */
    this->pkgHandler->InitSendPackage(NACK, NULL, 0);
    do {
      feedBack = this->pkgHandler->RecvPackage();
      if (feedBack == INVALID_NEW_MSG)
        this->pkgHandler->SendPackage();
    } while (feedBack != VALID_NEW_MSG);
    this->isFirstRecv = false;
  }

  retPkg = this->pkgHandler->GetRecvPkg();
  this->TransferData(retPkg, ptr, len);

  return (MsgType)retPkg->type;
}

//===================================================================
// SendResponse
//===================================================================
void CustomProtocol::NetworkHandler::SendResponse(MsgType msg) {
  int feedBack;

  this->pkgHandler->InitSendPackage(msg, NULL, 0);
  this->pkgHandler->SendPackage();
  do {
    feedBack = this->pkgHandler->RecvPackage();
    switch (feedBack) {
      case REPEATED_MSG:
        this->pkgHandler->SendPackage();
        break;
      case INVALID_NEW_MSG:
        /* create NACK pkg then revert to original */
        this->pkgHandler->InitSendPackage(NACK, NULL, 0);
        this->pkgHandler->SendPackage();
        this->pkgHandler->InitSendPackage(msg, NULL, 0);
    }
  } while (feedBack != VALID_NEW_MSG);
}

//===================================================================
// RecvResponse
//===================================================================
CustomProtocol::MsgType CustomProtocol::NetworkHandler::RecvResponse(void *ptr,
                                                                    size_t *len) {
  const KermitPackage *retPkg;
  retPkg = this->pkgHandler->GetRecvPkg();
  this->TransferData(retPkg, ptr, len);
  return (MsgType)retPkg->type;
}

//===================================================================
// InvertToSender
//===================================================================
void CustomProtocol::NetworkHandler::InvertToSender() {
  /* Since receiver called SendResponse(), feedBack should be INVERT */
  MsgType feedBack = this->RecvResponse(NULL, NULL);
  if (feedBack != INVERT) {
    ERROR_PRINT("Got %d in [InvertToSender]. Exiting.\n", feedBack);
    exit(1);
  }
}

//===================================================================
// InvertToReceiver
//===================================================================
void CustomProtocol::NetworkHandler::InvertToReceiver() {
  this->SendGenericData(INVERT, NULL, 0);
}