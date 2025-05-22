#ifndef BUFFER_HPP_
#define BUFFER_HPP_

#include <cstddef>
#include <cstdio>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <cstring>

#include "Logging.hpp"

namespace Data {

/* 1Mib */
const unsigned long BUFFER_SIZE = 1 << 20;

class Buffer {
  private:
    /* File Descriptor */
    int fd;
    unsigned char *store;
    size_t offset;          // Actual position in the buffer
    size_t activeSize;      // Valid size in the buffer
    
  public:
    Buffer();
    ~Buffer();
    /* Open the file for read-only */
    void OpenFileForRead(char *filePath);
    /* Create the file for write */
    void OpenFileForWrite(char *filePath);
    /* Close the file */
    void CloseFile(char *filePath);
    /* Returns store + offset and increments offset by len. If offset becomes 
     * than buffer size, it returns actual size of buffer in actualSize */
    void *GetData(size_t len, size_t *actualSize);
    /* Retrive new buffer from file. If number of bytes read is not equal to
     * BUFFER_SIZE, eofFound becomes true and number of read bytes is assigned to
     * eofLocation. If eofFound is true and this operation is called, retuns 1 */
    int RetrieveBuffer();
    /* If possible, appends len bytes pointer by ptr to buffer, otherwise returns 1 */
    int AppendToBuffer(void *ptr, size_t len);
    /* Flush buffer to file */
    void FlushBuffer();
};

} // namespace Data

#endif