#ifndef LOGGING_HPP_
#define LOGGING_HPP_

#define ERROR_PRINT(...)                 \
  {                                      \
    fprintf(stderr, "[ERROR] ");         \
    fprintf(stderr, __VA_ARGS__);        \
  }

#define DEBUG_PRINT(...)                 \
  {                                      \
    fprintf(stderr, "[DEBUG] ");         \
    fprintf(stderr, __VA_ARGS__);        \
  }

#endif