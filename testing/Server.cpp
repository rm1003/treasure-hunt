/* ARQUIVO PARA TESTE DE RawSocket lib */

#include "RawSocket.hpp"
#include "Logging.hpp"
#include <cstdio>

#include <cstring>

int main(int argc, char **argv) {
  if (argc <= 1) {
    ERROR_PRINT("argc pequeno\n");
    exit(1);
  }

  CustomSocket::RawSocket sokt(argv[1]);
  while (1) {
    char string[] = "Hello World";
    sokt.Send(string, strlen(string) + 1);
  }
}