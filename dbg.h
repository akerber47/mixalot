#include <iostream>
#include <string>
#include <fstream>
#define LOG_DEBUG 0

#ifdef LOG_DEBUG
extern std::ofstream *log_fs;
void log_dbg_init();
void log_dbg_close();

#define DBG_INIT() log_dbg_init()
#define _DBG_STREAM *log_fs
#define DBG_CLOSE() log_dbg_close()

#else

#define DBG_INIT()
#define _DBG_STREAM std::cerr
#define DBG_CLOSE()

#endif

#define _DBG_WRAP(exp) ( \
    _DBG_STREAM \
    << __FILE__ \
    << ":" \
    << __LINE__ \
    << ": " \
    << exp \
    << std::endl \
)

#define D(a) _DBG_WRAP(a)
#define D2(a,b) D(a << ", " << b)
#define D3(a,b,c) D2(a << ", " << b, c)
#define D4(a,b,c,d) D3(a << ", " << b, c, d)
#define D5(a,b,c,d,e) D4(a << ", " << b, c, d, e)
#define D6(a,b,c,d,e,f) D5(a << ", " << b, c, d, e, f)

