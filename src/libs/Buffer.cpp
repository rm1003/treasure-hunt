#include "Buffer.hpp"

Data::Buffer::Buffer() {
  this->store = new unsigned char[BUFFER_SIZE];
  this->eofFound = false;
  this->offset = 0;
  this->eofLocation = 0;
}

Data::Buffer::~Buffer() {
  delete[] this->store; 
}

void Data::Buffer::OpenFileForRead(char *filePath) {
  this->fd = open(filePath, O_RDONLY);
  if (this->fd == -1) {
    ERROR_PRINT("Failed to open file.\n");
    exit(1);
  }
  return;
}

void Data::Buffer::OpenFileForWrite(char *filePath) {
  this->fd = open(filePath, O_CREAT | O_WRONLY | O_TRUNC, 0644);
  if (this->fd == -1) {
    ERROR_PRINT("Failed to open file for writing.\n");
    exit(1);
  }
}

void Data::Buffer::CloseFile(char *filePath) {
  if (this->fd == -1) return;
  FlushBuffer();
  close(this->fd);
  return;
}

size_t Data::Buffer::GetLastReadSize() {
    return this->eofFound ? this->eofLocation : BUFFER_SIZE;
}


/* Provides a pointer to the data in the buffer and advances the current position */
void *Data::Buffer::GetData(size_t len, size_t *remainingSize) {
  void *data;

  if (this->fd == -1) {
    ERROR_PRINT("No file opened.\n");
    return NULL;
  }

  if (this->offset + len > BUFFER_SIZE) {
    *remainingSize = BUFFER_SIZE - offset;
    data = this->store + this->offset;
    this->offset = BUFFER_SIZE;
    return data;
  }

  data = this->store + offset;
  this->offset += len;
  *remainingSize = 0;
  return data;
}

int Data::Buffer::RetrieveBuffer() {
  if (this->fd == -1) {
    ERROR_PRINT("No file opened.\n");
    return -1;
  }
  if (this->eofFound) return 1;

  ssize_t numBytes = read(this->fd, this->store, BUFFER_SIZE);
  if (numBytes < 0) {
    ERROR_PRINT("Error reading file.\n");
    return -1;
  }

  this->offset = 0;

  if (numBytes != BUFFER_SIZE) {
    this->eofFound = true;
    this->eofLocation = numBytes;
    return 1;
  }

  return 0;
}

int Data::Buffer::AppendToBuffer(void *ptr, size_t len) {
  if (ptr == NULL) {
    ERROR_PRINT("Poiter is NULL.\n");
    exit(1);
  }

  if (this->offset + len > BUFFER_SIZE) {
    return 1;
  }
  memcpy(this->store + this->offset, ptr, len);
  this->offset += len;
  return 0;
}

void Data::Buffer::FlushBuffer() {
  if (this->fd == -1) {
    ERROR_PRINT("No file opened.\n");
    return;
  }

  if (this->offset == 0) return;

  ssize_t written = write(this->fd, this->store, this->offset);
  if (written != (ssize_t)this->offset) {
    ERROR_PRINT("Error writing to file.\n");
    exit(1);
  }


  this->offset = 0;
  return;
}