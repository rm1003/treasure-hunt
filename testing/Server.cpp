/* ARQUIVO PARA TESTE DE RawSocket lib */

#include "../src/libs/KermitProtocol.hpp"
#include "../src/libs/Logging.hpp"
#include <cstdio>

#include <cstring>
#include <unistd.h>

using CustomProtocol::MsgType;

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
  const char *ethName = GetEthIntName();
  if (ethName == NULL) {
    ERROR_PRINT("Could not get interface name. Exiting.\n");
    exit(1);
  } else {
    DEBUG_PRINT("Interface name [%s]\n", ethName);
  }
  CustomProtocol::PackageHandler pkgHandler(ethName);
  char string[] = "Oi meu nome é fernando, essa eh uma mensagem!";
  while(1) {
    pkgHandler.InitPackage(MsgType::DATA, string, strlen(string) + 1);
    int ret = pkgHandler.SendCurrentPkg();
    sleep(1);
    DEBUG_PRINT("SendCurrentPkg ret [%d]\n", ret);
  }
  free((void*)ethName);
}