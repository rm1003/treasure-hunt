/* ARQUIVO PARA TESTE DE RawSocket lib */

#include "../src/libs/KermitProtocol.hpp"
#include "../src/libs/Buffer.hpp"
#include "../src/libs/Logging.hpp"
#include <cassert>
#include <cstddef>
#include <cstdio>
#include <string>
#include <cstdlib>

const std::string MP4_PLAYER  = "vlc ";
const std::string JPG_PLAYER  = "eog ";
const std::string TXT_PLAYER  = "xed ";

char INPUT_FILE[] = "video.txt";

int main() {
  Data::Buffer buffer;
  CustomProtocol::NetworkHandler netHandler;
  CustomProtocol::MsgType msgRet;

  size_t dataLen;
  unsigned char data[CustomProtocol::DATA_SIZE];

  buffer.OpenFileForWrite(INPUT_FILE);
  do {
    msgRet = netHandler.RecvGenericData((void*)data, &dataLen);

    switch (msgRet) {
      case CustomProtocol::DATA:
        netHandler.SendResponse(CustomProtocol::ACK);
        if (buffer.AppendToBuffer(data, dataLen)) {
          buffer.FlushBuffer();
          buffer.AppendToBuffer(data, dataLen);
        }
        break;
      case CustomProtocol::END_OF_FILE:
        buffer.FlushBuffer();
        buffer.CloseFile();
        break;
      default:
        ERROR_PRINT("Not expected [%d]. Exiting\n", msgRet);
        exit(1);
    }
  } while (msgRet != CustomProtocol::END_OF_FILE);

  // std::string command = TXT_PLAYER + INPUT_FILE;
  // std::system(command.c_str());
}