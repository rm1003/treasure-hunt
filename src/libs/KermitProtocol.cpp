#include "KermitProtocol.hpp"

CustomProtocol::PackageHandler::PackageHandler(char *netIntName) {
  this->sokt = new CustomSocket::RawSocket(netIntName);
  this->SetInitMarkPkg();
}

void CustomProtocol::PackageHandler::SetInitMarkPkg() {
  this->currentPkg.initMark = INIT_MARK;
}