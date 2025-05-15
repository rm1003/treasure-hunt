#ifndef LOGGING_HPP_
#define LOGGING_HPP_

#define ERROR_PRINT(...)                 \
  {                                      \
    fprintf(stderr, "[ERROR] ");         \
    fprintf(stderr, __VA_ARGS__);        \
  }

#ifndef DEBUG
#define DEBUG_PRINT(...)                 \
  {                                      \
    while(0);                            \
  }
#else
#define DEBUG_PRINT(...)                 \
  {                                      \
    fprintf(stderr, "[DEBUG] ");         \
    fprintf(stderr, __VA_ARGS__);        \
  }
#endif

#endif