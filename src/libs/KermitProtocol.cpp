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
  // Static to avoid multiple allocations
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
  DEBUG_PRINT("Created new Raw Socket.\n");
  this->currentPkgIdx = 0;
  DEBUG_PRINT("Set current package index to 0.\n");
  this->currentPkg = &this->pkgs[this->currentPkgIdx];
  DEBUG_PRINT("Set initial current package.\n");
  this->SetInitMarkPkg();
  DEBUG_PRINT("Set message init mark.\n");
  this->lastUsedIdx = this->lastRecvIdx = 0;
  DEBUG_PRINT("Set last used index to 0.\n");
  memset(this->rawBytes, 0, sizeof(this->rawBytes));
  DEBUG_PRINT("Initialized all rawBytes arr bytes to 0.\n");
}

CustomProtocol::PackageHandler::~PackageHandler() {
  delete this->sokt;
}

//===================================================================
// InitPackage
//===================================================================
int CustomProtocol::PackageHandler::InitPackage(unsigned char type,
                                                void *data,
                                                size_t len) {
  if (len > DATA_SIZE) {
    ERROR_PRINT("Data is too long to fit a package. Exiting.\n");
    exit(1);
  }
  this->currentPkg->type = type;
  this->currentPkg->idx = this->lastUsedIdx;
  this->currentPkg->size = len;
  if (data != NULL) {
    memcpy(this->currentPkg->data, data, len);
  }
  this->currentPkg->checkSum = this->ChecksumResolver();
  this->lastUsedIdx = NEXT_IDX(this->lastUsedIdx);
  DEBUG_PRINT("Updated last used index [%d].\n", this->lastUsedIdx);

  return 0;
}

//===================================================================
// SendCurrentPkg
//===================================================================
int CustomProtocol::PackageHandler::SendCurrentPkg() {
  int ret;
  ret = this->SendPackage(this->currentPkg);
  this->SwapPkg();
  return ret;
}

//===================================================================
// SendPreviousPkg
//===================================================================
int CustomProtocol::PackageHandler::SendPreviousPkg() {
  return this->SendPackage(this->prevPkg);
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
    this->Remove0xff(this->currentPkg);
    if (this->IsMsgKermitPackage()) {
      if (!this->VerifyChecksum()) {
        DEBUG_PRINT("Invalid new message arrived.\n");
        return INVALID_NEW_MSG;
      }
      if (this->currentPkg->idx == this->lastRecvIdx) {
        DEBUG_PRINT("Repeated message. Maybe sender did not get ACK.\n");
        return REPEATED_MSG;
      }
      this->lastRecvIdx = this->currentPkg->idx;
      return VALID_NEW_MSG;
    }
  } while (timestamp() - init < TIMEOUT_LEN);

  return TIMEOUT_REACHED;
}

//===================================================================
// SwapPkg
//===================================================================
void CustomProtocol::PackageHandler::SwapPkg() {
  this->prevPkg = this->currentPkg;
  this->currentPkgIdx = INC_MOD_K(this->currentPkgIdx, 2);
  this->currentPkg = &this->pkgs[this->currentPkgIdx];
}

//===================================================================
// GetCurrentPkg
//===================================================================
const struct KermitPackage* CustomProtocol::PackageHandler::GetCurrentPkg() {
  return this->currentPkg;
}

//===================================================================
// SetInitMarkPkg
//===================================================================
void CustomProtocol::PackageHandler::SetInitMarkPkg() {
  this->currentPkg->initMark = INIT_MARK;
}

//===================================================================
// ChecksumResolver
//===================================================================
unsigned char CustomProtocol::PackageHandler::ChecksumResolver() {
  unsigned long sum;
  size_t it = 0;
  const size_t checksumTotalBits = sizeof(KermitPackage::checkSum) * 8;

  sum = this->currentPkg->size;
  sum += this->currentPkg->idx;
  sum += (sum >> checksumTotalBits);
  sum += this->currentPkg->type;
  sum += (sum >> checksumTotalBits);
  /* This loop only works since checkSum field has 1 byte in size */
  while(it < this->currentPkg->size) {
    sum += this->currentPkg->data[it];
    sum += (sum >> checksumTotalBits);
    it++;
  }

  return (unsigned char)sum;
}

//===================================================================
// Append0xff
//===================================================================
void CustomProtocol::PackageHandler::Append0xff(struct KermitPackage *pkg) {
  unsigned char *pkgBytes = (unsigned char *)(pkg);
  unsigned long pkgIt = 0;
  unsigned long rawBytesIt = 0;
  unsigned long pkgSize = this->GetPkgSize(pkg);

  for (; pkgIt < pkgSize; rawBytesIt++, pkgIt++) {
    this->rawBytes[rawBytesIt] = pkgBytes[pkgIt];
    if (pkgBytes[pkgIt] == 0x88 || pkgBytes[pkgIt] == 0x81) {
      this->rawBytes[++rawBytesIt] = 0xff;
    }
  }

  DEBUG_PRINT("Final rawBytesArr it value [%lu]\n", rawBytesIt);
  DEBUG_PRINT("Final pkg it value [%lu]\n", pkgIt);
}

//===================================================================
// Remove0xff
//===================================================================
void CustomProtocol::PackageHandler::Remove0xff(struct KermitPackage *pkg) {
  unsigned long rawBytesIt = 0;
  unsigned long pkgIt = 0;
  unsigned char *pkgBytes = (unsigned char *)(pkg);

  for (; rawBytesIt < sizeof(KermitPackage); pkgIt++, rawBytesIt++) {
    pkgBytes[pkgIt] = this->rawBytes[rawBytesIt];
    if (pkgBytes[pkgIt] == 0x81 || pkgBytes[pkgIt] == 0x88) {
      rawBytesIt++;
    }
  }

  DEBUG_PRINT("Final rawBytesArr it value [%lu]\n", rawBytesIt);
  DEBUG_PRINT("Final pkg it value [%lu]\n", pkgIt);
}

//===================================================================
// VerifyChecksum
//===================================================================
bool CustomProtocol::PackageHandler::VerifyChecksum() {
  return (this->currentPkg->checkSum == this->ChecksumResolver());
}

//===================================================================
// IsMsgKermitPackage
//===================================================================
bool CustomProtocol::PackageHandler::IsMsgKermitPackage() {
  return (currentPkg->initMark == INIT_MARK);
}

//===================================================================
// GetPkgSize
//===================================================================
size_t CustomProtocol::PackageHandler::GetPkgSize(struct KermitPackage *pkg) {
  size_t sizePkgNoData = sizeof(KermitPackage) - sizeof(KermitPackage::data);
  return sizePkgNoData + pkg->size;
}

//===================================================================
// SendPackage
//===================================================================
int CustomProtocol::PackageHandler::SendPackage(struct KermitPackage *pkg) {
  this->Append0xff(pkg);
  return this->sokt->Send(this->rawBytes, sizeof(this->rawBytes));
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
  this->isFirstPkg = true;
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
// RecvPackage
//===================================================================
void RecvPackage(const KermitPackage *retPkg, void *ptr, size_t *len) {
  // If ptr and len is NULL return MsgType (Invert case)
  if (!ptr && !len) {
    return;
  }

  memcpy(ptr, retPkg->data, retPkg->size);
  *len = retPkg->size;
  return ;
}

//===================================================================
// SendGeneticData
//===================================================================
void CustomProtocol::NetworkHandler::SendGenericData(MsgType msg, void *ptr, size_t len) {

  int feedBack;

  if (len > DATA_SIZE) {
    ERROR_PRINT("len is larger than DATA_SIZE.\n Exiting...\n");
    exit(1);
  }
  if (len < 0) {
    ERROR_PRINT("len is smaller than 0.\n Exiting...\n");
    exit(1);
  }

  this->pkgHandler->InitPackage(msg, ptr, len);
  this->pkgHandler->SendCurrentPkg();

  while (1) {
    feedBack = this->pkgHandler->RecvPackage();

    if (feedBack == TIMEOUT_REACHED || feedBack == INVALID_NEW_MSG) {
      this->pkgHandler->SendPreviousPkg();
    } else if (feedBack == VALID_NEW_MSG) {
      break;
    }
  }
  return;
}

//===================================================================
// RecvGenericData
//===================================================================
CustomProtocol::MsgType CustomProtocol::NetworkHandler::RecvGenericData(void *ptr, size_t *len) {

  const KermitPackage *retPkg;
  int feedBack;
  if (this->isFirstPkg) {
    while (1) {
      feedBack = this->pkgHandler->RecvPackage();
      if (feedBack == VALID_NEW_MSG) {
        retPkg = this->pkgHandler->GetCurrentPkg();
        this->RecvPackage(retPkg, ptr, len);
        this->isFirstPkg = false;
        return  static_cast<MsgType>(retPkg->type);
      }
    }
  } else {
    // Loop ?
    retPkg = this->pkgHandler->GetCurrentPkg();
    this->RecvPackage(retPkg, ptr, len);
    return  static_cast<MsgType>(retPkg->type);
  }
}

//===================================================================
// SendResponse
//===================================================================
void CustomProtocol::NetworkHandler::SendResponse(MsgType msg) {

  int feedBack;

  this->pkgHandler->InitPackage(msg, NULL, NULL);
  this->pkgHandler->SendCurrentPkg();
  while (1) {
    feedBack = this->pkgHandler->RecvPackage();

    if (feedBack == REPEATED_MSG) {
      this->pkgHandler->SendPreviousPkg();
    } else if (feedBack == VALID_NEW_MSG){
      return; 
    } else {
      continue;
    }
  }
}

//===================================================================
// RecvResponse
//===================================================================
CustomProtocol::MsgType CustomProtocol::NetworkHandler::RecvResponse(void *ptr, size_t *len) {
  const KermitPackage *retPkg;
  retPkg = this->pkgHandler->GetCurrentPkg();
  this->RecvPackage(retPkg, ptr, len);
  return static_cast<MsgType>(retPkg->type);
}

//===================================================================
// InvertToSender
//===================================================================
void CustomProtocol::NetworkHandler::InvertToSender() {
  this->SendGenericData(INVERT_REQUEST, NULL, 0);
  MsgType feedBack = this->RecvResponse(NULL, NULL);

  if (feedBack != ACK) {
    ERROR_PRINT("Expected ACK in [InvertToSender], but got %d. Exiting.\n", feedBack);
    exit(1);
  }
  return;
}

//===================================================================
// InvertToReceiver
//===================================================================
void CustomProtocol::NetworkHandler::InvertToReceiver() {
  MsgType ret;
  ret = this->RecvGenericData(NULL, NULL);
  if (ret != INVERT_REQUEST) {
    ERROR_PRINT("Not expected message type in [InvertToReceiver]. Exiting.\n");
    exit(1);
  }
  this->SendResponse(ACK);
  return;
}