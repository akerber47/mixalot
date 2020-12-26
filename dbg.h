#include <iostream>
#include <string>
#include <fstream>
#define LOG_DEBUG 1

#ifdef LOG_DEBUG

std::ofstream *log_fs = nullptr;

void log_dbg_init() {
  log_fs = new std::ofstream("./debug.log");
}

void log_dbg_close() {
  delete log_fs;
}
#define DBG_INIT() log_dbg_init()
#define DBG(x) ( \
    *log_fs \
    << __FILE__ \
    << ":" \
    << __LINE__ \
    << ": " \
    << (x) \
    << std::endl)
#define DBG_CLOSE() log_dbg_close()

#else

#define DBG_INIT()
#define DBG(x) ( \
    std::cerr \
    << __FILE__ \
    << ":" \
    << __LINE__ \
    << ": " \
    << (x) \
    << std::endl)
#define DBG_CLOSE()

#endif
