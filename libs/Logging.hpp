#define ERROR_PRINT(msg)                 \
  {                                      \
    fprintf(stderr, "[ERROR] %s", #msg); \
  }

#define DEBUG_PRINT(msg)                 \
  {                                      \
    fprintf(stderr, "[DEBUG] %s", #msg); \
  }
