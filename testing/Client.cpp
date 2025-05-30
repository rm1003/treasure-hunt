/* ARQUIVO PARA TESTE DE RawSocket lib */

#include "../src/libs/KermitProtocol.hpp"
#include "../src/libs/Logging.hpp"
#include <cstdio>

#include <cstring>

using CustomProtocol::MsgType;
using CustomProtocol::KermitPackage;

const char *GetEthIntName() {
  struct if_nameindex *ifArr, *ifIt;
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

int main() {
  CustomProtocol::NetworkHandler netHandler;

  netHandler.SendGenericData(CustomProtocol::DATA, (void*)"Oi", 3);
}