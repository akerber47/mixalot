#include "dbg.h"
#ifdef LOG_DEBUG
std::ofstream *log_fs = nullptr;

void log_dbg_init() {
  log_fs = new std::ofstream("./out/debug.log");
}

void log_dbg_close() {
  delete log_fs;
}
#endif
