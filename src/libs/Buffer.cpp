#include "Buffer.hpp"

Data::Buffer::Buffer() {
  this->store = new unsigned char[BUFFER_SIZE];
  this->eofFound = false;
  this->offset = 0;
}

Data::Buffer::~Buffer() {
  delete[] this->store; 
}