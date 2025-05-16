#include "KermitProtocol.hpp"
#include "Logging.hpp"

#include <bits/types/struct_timeval.h>
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

//=================================================================//
// CustomProtocol::PackageHandler
//=================================================================//

CustomProtocol::PackageHandler::PackageHandler(const char *netIntName) {
  this->sokt = new CustomSocket::RawSocket(netIntName);
  DEBUG_PRINT("Created new Raw Socket.\n");
  this->sokt->SetFixedBufLen(sizeof(KermitPackage));
  DEBUG_PRINT("Set Raw Socket fixed buffer len to KermitPackage size.\n");
  this->currentPkgIdx = 0;
  DEBUG_PRINT("Set current package index to 0.\n");
  this->currentPkg = &this->pkgs[this->currentPkgIdx];
  DEBUG_PRINT("Set initial current package.\n");
  this->SetInitMarkPkg();
  DEBUG_PRINT("Set message init mark.\n");
  this->lastUsedIdx = 0;
  DEBUG_PRINT("Set last used index to 0.\n");
}

CustomProtocol::PackageHandler::~PackageHandler() {
  delete this->sokt;
}

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
    DEBUG_PRINT("Data copied to currentPkg data [InitPackage].\n");
  }
  this->ChecksumResolver();
  DEBUG_PRINT("Solver for checksum [InitPackage].\n");
  this->lastUsedIdx = NEXT_IDX(this->lastUsedIdx);
  DEBUG_PRINT("Updating last used index [InitPackage].\n");

  return 0;
}

int CustomProtocol::PackageHandler::SendCurrentPkg() {
  return this->SendPackage(this->currentPkg);
}

int CustomProtocol::PackageHandler::SendPreviousPkg() {
  return this->SendPackage(this->prevPkg);
}

int CustomProtocol::PackageHandler::RecvPackage() {
  int ret;
  unsigned long init = timestamp();

  do {
    ret = this->sokt->Recv(this->currentPkg, sizeof(KermitPackage));
    if (ret == -1) {continue;}
    if (this->IsMsgKermitPackage()) {
      if (!this->VerifyChecksum()) {
        DEBUG_PRINT("Invalid new message arrived.\n");
        return INVALID_NEW_MSG;
      }
      if (this->currentPkg->idx == this->lastRecvIdx) {
        DEBUG_PRINT("Repeated message. Maybe sender did not get ACK.\n")
        return REPEATED_MSG;
      }
      return VALID_NEW_MSG;
    }
  } while (timestamp() - init < TIMEOUT_LEN);

  return TIMEOUT_REACHED;
}

void CustomProtocol::PackageHandler::SwapPkg() {
  this->prevPkg = this->currentPkg;
  this->currentPkgIdx = INC_MOD_K(this->currentPkgIdx, 2);
  this->currentPkg = &this->pkgs[this->currentPkgIdx];
}

const struct KermitPackage* CustomProtocol::PackageHandler::GetCurrentPkg() {
  return this->currentPkg;
}

void CustomProtocol::PackageHandler::SetInitMarkPkg() {
  this->currentPkg->initMark = INIT_MARK;
}

void CustomProtocol::PackageHandler::ChecksumResolver() {
  unsigned long sum = 0;
  size_t offset = 0;
  sum += this->currentPkg->size;
  sum += (sum >> sizeof(KermitPackage::checkSum));
  sum += this->currentPkg->idx;
  sum += (sum >> sizeof(KermitPackage::checkSum));
  sum += this->currentPkg->type;
  sum += (sum >> sizeof(KermitPackage::checkSum));
  while(offset < this->currentPkg->size) {
    /* Gets type of KermitPackage::checkSum and casts this->currentPkg->data +
     * offset to a pointer to this type and then dereferences it */
    sum += *(decltype(KermitPackage::checkSum)*)(this->currentPkg->data + offset);
    sum += (sum >> sizeof(KermitPackage::checkSum));
    offset += sizeof(KermitPackage::checkSum);
  }
  this->currentPkg->checkSum = (unsigned char)sum;
}

void CustomProtocol::PackageHandler::Append0xffToPkg() {

}

void CustomProtocol::PackageHandler::Remove0xffInsertedInPkg() {

}

bool CustomProtocol::PackageHandler::VerifyChecksum() {
  return true;
}

bool CustomProtocol::PackageHandler::IsMsgKermitPackage() {
  return (currentPkg->initMark == INIT_MARK);
}

size_t CustomProtocol::PackageHandler::GetPkgSize(struct KermitPackage *pkg) {
  size_t sizePkgNoData = sizeof(KermitPackage) - sizeof(KermitPackage::data);
  return sizePkgNoData + pkg->size;
}

int CustomProtocol::PackageHandler::SendPackage(struct KermitPackage *pkg) {
  return this->sokt->Send(pkg, this->GetPkgSize(pkg));
}

//=================================================================//
// CustomProtocol::NetworkHandler
//=================================================================//

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

CustomProtocol::NetworkHandler::NetworkHandler() {
  const char *ethIntName = this->GetEthIntName();
  // alguma coisa
  free((void*)ethIntName);

  this->buffer = new unsigned char[DATA_BUFFER_SIZE];
  this->bufferOffset = 0;
}

CustomProtocol::NetworkHandler::~NetworkHandler() {
  delete[] this->buffer;
}