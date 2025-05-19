#ifndef BUFFER_HPP_
#define BUFFER_HPP_

#include <cstddef>
#include <cstdio>

namespace Data {

/* 1Mib */
const unsigned long BUFFER_SIZE = 1 << 20;

class Buffer {
  private:
    /* File Descriptor */
    int fd;
    unsigned char *store;
    /* Initialized with 0. Becomes 0 after every Retrieve/Flush operation */
    size_t offset;
    bool eofFound;
    size_t eofLocation;
    
  public:
    Buffer();
    ~Buffer();
    /* Returns store + offset and increments offset by len. If offset becomes 
     * than buffer size, it returns remaining size of buffer in remainingSize */
    void *GetData(size_t len, size_t *remainingSize);
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