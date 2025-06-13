/* ARQUIVO PARA TESTE DE RawSocket lib */

#include "../src/libs/KermitProtocol.hpp"
#include "../src/libs/Buffer.hpp"
#include "../src/libs/Logging.hpp"
#include <cassert>
#include <cstddef>
#include <cstdio>
#include <string>
#include <cstdlib>

const std::string MP4_PLAYER  = "vlc";
const std::string MP4_OPTIONS = "--play-and-exit";
const std::string JPG_PLAYER  = "eog";
const std::string TXT_PLAYER  = "xed";

char INPUT_FILE[] = "video.mp4";

int main() {
  Data::Buffer buffer;
  CustomProtocol::NetworkHandler netHandler;
  CustomProtocol::MsgType msgRet;
  
  size_t dataLen;
  int intRet;
  unsigned char data[CustomProtocol::DATA_SIZE];

  buffer.OpenFileForWrite(INPUT_FILE);
  while (1) {
    msgRet = netHandler.RecvGenericData((void*)data, &dataLen);

    switch (msgRet) {
      case CustomProtocol::DATA:
        netHandler.SendResponse(CustomProtocol::ACK);
        intRet = buffer.AppendToBuffer(data, dataLen);
        if (intRet == Data::APPEND_IMPOSSIBLE) {
          buffer.FlushBuffer();
          buffer.AppendToBuffer(data, dataLen);
        }
        break;
      case CustomProtocol::END_OF_FILE:
        buffer.FlushBuffer();
        buffer.CloseFile();
        break;
      default:
        DEBUG_PRINT("Not expected [%d]. Exiting\n", msgRet);
        exit(1);
    }

    if (msgRet == CustomProtocol::END_OF_FILE)
      break;
  }

  buffer.CloseFile();
  std::string command = MP4_PLAYER + INPUT_FILE + MP4_OPTIONS;
  std::system(command.c_str());
}