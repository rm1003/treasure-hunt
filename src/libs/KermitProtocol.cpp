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

using CustomProtocol::MsgType;

unsigned long timestamp() {
  // Static to avoid multiple allocations
  static struct timeval tp;
  gettimeofday(&tp, NULL);
  return tp.tv_sec * 1000 + tp.tv_usec / 1000;
}

//=================================================================//
// CustomProtocol::PackageHandler
//=================================================================//

CustomProtocol::PackageHandler::PackageHandler(char *netIntName) {
  this->sokt = new CustomSocket::RawSocket(netIntName);
  this->sokt->SetFixedBufLen(sizeof(KermitPackage));
  this->currentPkgIdx = 0;
  this->currentPkg = &this->pkgs[this->currentPkgIdx];
  this->SetInitMarkPkg();
  this->lastUsedIdx = 0;
}

CustomProtocol::PackageHandler::~PackageHandler() {

}

int CustomProtocol::PackageHandler::InitPackage(unsigned char type, 
                                                unsigned char *data, 
                                                size_t len) {
  if (len > DATA_SIZE) {
    ERROR_PRINT("Data is too long to fit a package\n");
    return 1;
  }
  this->currentPkg->type = type;
  this->currentPkg->idx = this->lastUsedIdx;
  this->currentPkg->size = len;
  memcpy(this->currentPkg->data, data, len);
  this->ChecksumResolver();

  this->lastUsedIdx = NEXT_IDX(this->lastUsedIdx);

  return 0;
}

void CustomProtocol::PackageHandler::SendPackage() {
  int ret;
  while(1) {
    this->sokt->Send(this->currentPkg, this->GetCurrentPkgSize());
    ret = this->RecvPackage();
    if (ret != -1) {
      if (this->GetMsgTypeOfCurrentPkg() == ACK) {
        break;
      } else if (this->GetMsgTypeOfCurrentPkg() == ERROR) {
        ERROR_PRINT("Error in sequence. Logic done wrong\n");
        exit(1);
      }
    }
    this->currentPkg = this->prevPkg;q
  }

}

int CustomProtocol::PackageHandler::RecvPackage() {
  int ret;
  unsigned long init = timestamp();

  this->prevPkg = this->currentPkg;
  INC_MOD_K(this->currentPkgIdx, 2);
  this->currentPkg = &this->pkgs[this->currentPkgIdx];
  do {
    ret = this->sokt->Recv(this->currentPkg, sizeof(KermitPackage));
    if (ret == -1) {continue;}
    if (this->IsMsgKermitPackage()) {
      return 0;
    }
  } while (timestamp() - init < TIMEOUT_LEN);
  
  return -1;
}

MsgType CustomProtocol::PackageHandler::GetMsgTypeOfCurrentPkg() {

}

void CustomProtocol::PackageHandler::GetDataInCurrentPkg(unsigned char *ptr, 
                                                         size_t *len) {
  
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
  while(offset < DATA_SIZE) {
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

}

bool CustomProtocol::PackageHandler::IsMsgKermitPackage() {
  return (currentPkg->initMark == INIT_MARK);
}

size_t CustomProtocol::PackageHandler::GetCurrentPkgSize() {
  size_t sizePkgNoData = sizeof(KermitPackage) - sizeof(KermitPackage::data);
  return sizePkgNoData + this->currentPkg->size;
}

//=================================================================//
// CustomProtocol::NetworkHandler
//=================================================================//

const char *CustomProtocol::NetworkHandler::GetEthIntName() {
  struct if_nameindex *ifArr, *ifIt;

  ifArr = if_nameindex();
  if (!ifArr) {
    ERROR_PRINT("Could not get eth interface name arr\n");
    exit(1);
  }

  for (ifIt = &ifArr[0]; ifIt->if_name != NULL; ifIt++) {
    int notEqualEth = strcmp(ifIt->if_name, "eth");
    if (!notEqualEth) {
      if_freenameindex(ifArr);
      return strdup(ifIt->if_name);
    }
    int notEqualEnp = strcmp(ifIt->if_name, "enp");
    if (!notEqualEnp) {
      if_freenameindex(ifArr);
      return strdup(ifIt->if_name);
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