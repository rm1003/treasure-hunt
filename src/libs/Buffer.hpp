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
    /* Gets the amount of data according to len and informs with actualSize the 
     * actual amount read, returning a pointer to the buffer + offset or NULL
     * if no more data in the buffer */
    void *GetData(size_t len, size_t *actualSize);
    /* Retrive new buffer from file. activeSize stores the amount of data
     * read. If number of bytes read is equal to 0 return 1, indicating 
     * that there are no more bytes in the file */
    int RetrieveBuffer();
    /* If possible, appends len bytes pointer by ptr to buffer, otherwise returns 1 */
    int AppendToBuffer(void *ptr, size_t len);
    /* Flush buffer to file */
    void FlushBuffer();
};

} // namespace Data

#endif