#include "KermitProtocol.hpp"
#include "Logging.hpp"

#include <cstring>
extern "C" {
#include <net/if.h>
#include <netinet/in.h>
#include <stdlib.h>
}

CustomProtocol::PackageHandler::PackageHandler(char *netIntName) {
  this->sokt = new CustomSocket::RawSocket(netIntName);
  this->SetInitMarkPkg();
}

void CustomProtocol::PackageHandler::SetInitMarkPkg() {
  this->currentPkg.initMark = INIT_MARK;
}

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
}