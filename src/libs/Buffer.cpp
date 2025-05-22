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
    ERROR_PRINT("Failed to open file.\n Exiting...\n");
    exit(0);
  }
  return;
}

void Data::Buffer::OpenFileForWrite(char *filePath) {
  this->fd = open(filePath, O_CREAT | O_WRONLY | O_TRUNC, 0644);
  if (this->fd == -1) {
    ERROR_PRINT("Failed to open file for writing.\n Exiting...\n");
    exit(1);
  }
}

void Data::Buffer::CloseFile(char *filePath) {
  if (this->fd == 1) {
    ERROR_PRINT("Failed to close file.\n Exiting...\n");
    exit(0);
  }
  FlushBuffer();
  close(this->fd);
  return;
}

void *Data::Buffer::GetData(size_t len, size_t *remainingSize) {
  void *data;

  if (this->fd == -1) {
    ERROR_PRINT("No file opened.\n Exiting...\n");
    exit(1);
  }

  if (this->eofFound && ((this->offset + len) > this->eofLocation)) {
    *remainingSize = this->eofLocation - this->offset;
    data = this->store + this->offset;
    this->offset = this->eofLocation;
    return data;
  }

  *remainingSize = BUFFER_SIZE - offset;
  data = this->store + this->offset;

  if (this->offset + len > BUFFER_SIZE) {
    this->offset = BUFFER_SIZE;
    return data;
  }

  this->offset += len;
  return data;
}

int Data::Buffer::RetrieveBuffer() {
  if (this->fd == -1) {
    ERROR_PRINT("No file opened.\n Exiting...\n");
    exit(1);
  }

  if (this->eofFound) return 1;

  ssize_t numBytes = read(this->fd, this->store, BUFFER_SIZE);
  if (numBytes < 0) {
    ERROR_PRINT("Error reading file.\n Exiting...\n");
    exit(1);
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
    ERROR_PRINT("Poiter is NULL.\n Exiting...\n");
    exit(1);
  }

  if (this->offset + len > BUFFER_SIZE) return 1;

  memcpy(this->store + this->offset, ptr, len);
  this->offset += len;
  return 0;
}

void Data::Buffer::FlushBuffer() {
  if (this->fd == -1) {
    ERROR_PRINT("No file opened.\n Exiting...\n");
    exit(1);
  }

  if (this->offset == 0) return;

  ssize_t written = write(this->fd, this->store, this->offset);
  if (written != (ssize_t)this->offset) {
    ERROR_PRINT("Error writing to file.\n Exiting...\n");
    exit(1);
  }

  this->offset = 0;
  this->eofFound = false;
  this->eofLocation = 0;

  return;
}