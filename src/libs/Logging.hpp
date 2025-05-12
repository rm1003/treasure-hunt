#ifndef LOGGING_HPP_
#define LOGGING_HPP_

#define ERROR_PRINT(msg)                 \
  {                                      \
    fprintf(stderr, "[ERROR] %s", #msg); \
  }

#define DEBUG_PRINT(msg)                 \
  {                                      \
    fprintf(stderr, "[DEBUG] %s", #msg); \
  }

#endif