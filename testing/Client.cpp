/* ARQUIVO PARA TESTE DE RawSocket lib */

#include "RawSocket.hpp"
#include "Logging.hpp"
#include <cstdio>

#include <cstring>
#include <net/if.h>
#include <netinet/in.h>

int main(int argc, char **argv) {


  struct if_nameindex *ifArr, *ifIt;

  ifArr = if_nameindex();
  if (!ifArr) {
    ERROR_PRINT("Could not get if name arr\n");
    return 1;
  }
  for (ifIt = &ifArr[0]; ifIt->if_name != NULL; ifIt++) {
    DEBUG_PRINT("%s\n", ifIt->if_name);
  }
}