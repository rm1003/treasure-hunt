/* ARQUIVO PARA TESTE DE RawSocket lib */

#include "../src/libs/RawSocket.hpp"
#include "../src/libs/Logging.hpp"
#include <cstdio>

#include <cstring>

int main(int argc, char **argv) {
  if (argc <= 1) {
    ERROR_PRINT("argc pequeno\n");
    exit(1);
  }

  char string[1024];
  CustomSocket::RawSocket sokt(argv[1]);
  while (1) {
    int ret;
    ret = sokt.Recv(string, strlen(string) + 1);

    if (ret != 0) {continue;}
    printf("%s\n", string);
  }
}