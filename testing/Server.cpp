/* ARQUIVO PARA TESTE DE RawSocket lib */

#include "../src/libs/KermitProtocol.hpp"
#include "../src/libs/Buffer.hpp"
#include "../src/libs/Logging.hpp"
#include <cstddef>

char INPUT_FILE[] = "cat.mp4";

int main() {
  Data::Buffer buffer;
  CustomProtocol::NetworkHandler netHandler;
  void *ptr;
  size_t actualSize;
  size_t bytesRetrived;
  CustomProtocol::MsgType resp;

  buffer.OpenFileForRead(INPUT_FILE);
  while(1) {
    ptr = buffer.GetData(CustomProtocol::DATA_SIZE, &actualSize);
    if (ptr == NULL) {
      bytesRetrived = buffer.RetrieveBuffer();
      if (bytesRetrived == 0) {
        netHandler.SendGenericData(CustomProtocol::END_OF_FILE, NULL, 0);
        break;
      }
      ptr = buffer.GetData(CustomProtocol::DATA_SIZE, &actualSize);
    }

    netHandler.SendGenericData(CustomProtocol::DATA, ptr, actualSize);
    resp = netHandler.RecvResponse(NULL, NULL);
    if (resp != CustomProtocol::ACK) {
      DEBUG_PRINT("Got [%d]. Exiting.\n\n", resp);
      exit(1);
    }
  }
  buffer.CloseFile();
}