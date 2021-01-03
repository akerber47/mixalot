#include <string>
#include <vector>
#include <map>
#include "dbg.h"
#include "sys.h"
#include "core.h"
#include "io.h"
#include "cpu.h"
#include "clock.h"


constexpr int NUM_DEVICES = 21;

/*
 * How much faster disk instructions will execute
 * if the disk is already in the right place
 * (ie, current position == requested position)
 */
constexpr int DISK_SEEK_FACTOR = 10;

/*
 * Memory: 4x10^3 words
 * Tape: 1x10^6 words
 * Disk: 1x10^5 words
 * Paper tape: 1x10^5 words
 */

DevInfo DEV_MAGNETIC_TAPE = {
  DevType::MAGNETIC_TAPE,
  Format::BINARY,
  StorageType::FIXED_SIZE,
  100,
  1000,
  true,
  true,
  500,
  1000,
};

DevInfo DEV_DISK = {
  DevType::DISK,
  Format::BINARY,
  StorageType::FIXED_SIZE,
  100,
  100,
  true,
  true,
  500,
  1000
};

DevInfo DEV_CARD_READER = {
  DevType::CARD_READER,
  Format::CARD,
  StorageType::STREAM,
  16,
  0,
  true,
  false,
  5000,
  10000
};

DevInfo DEV_CARD_PUNCH = {
  DevType::CARD_PUNCH,
  Format::CARD,
  StorageType::STREAM,
  16,
  0,
  false,
  true,
  10000,
  20000
};

DevInfo DEV_LINE_PRINTER = {
  DevType::LINE_PRINTER,
  Format::CHAR,
  StorageType::STREAM,
  24,
  0,
  false,
  true,
  3750,
  7500
};

DevInfo DEV_TERMINAL = {
  DevType::TERMINAL,
  Format::CHAR,
  StorageType::STREAM,
  14,
  0,
  true,
  true,
  3750,
  7500
};

DevInfo DEV_PAPER_TAPE = {
  DevType::PAPER_TAPE,
  Format::CHAR,
  StorageType::FIXED_SIZE,
  14,
  1000,
  true,
  true,
  3750,
  7500
};


MixDev::MixDev(std::string filename, StorageType storage, size_t sz) {
  D2("Initializing device file ", filename);
  if (storage == StorageType::FIXED_SIZE)
    fd = open_and_resize(filename, sz);
  else
    fd = open_append(filename);
}

MixDev::~MixDev() {
  if (fd != -1)
    close_noerr(fd);
}

void MixDev::read_block(void *dest, int off, size_t sz) {
  (void) seek_read(fd, dest, off, sz);
}

void MixDev::write_block(void *src, int off, size_t sz) {
  (void) seek_write(fd, src, off, sz);
}

MixIO::MixIO(
    MixCore *core, // owned by caller
    std::string tape_prefix,
    std::string disk_prefix,
    std::string card_punch,
    std::string card_reader,
    std::string line_printer,
    std::string terminal,
    std::string paper_tape
) {
  this->core = core;
  D2("Initializing device files, num = ", NUM_DEVICES);
  for (int i = 0; i < NUM_DEVICES; i++) {
    do_io_ts.push_back(-1);
    finish_ts.push_back(-1);
    cur_inst.push_back(0);
    pos.push_back(0);
    std::string filename;
    if (i >= 0 && i < 8) {
      info.push_back(DEV_MAGNETIC_TAPE);
      filename = tape_prefix + std::to_string(i);
    } else if (i >= 8 && i < 16) {
      info.push_back(DEV_DISK);
      filename = disk_prefix + std::to_string(i-8);
    } else if (i == 16) {
      info.push_back(DEV_CARD_READER);
      filename = card_reader;
    } else if (i == 17) {
      info.push_back(DEV_CARD_PUNCH);
      filename = card_punch;
    } else if (i == 18) {
      info.push_back(DEV_LINE_PRINTER);
      filename = line_printer;
    } else if (i == 19) {
      info.push_back(DEV_TERMINAL);
      filename = terminal;
    } else if (i == 20) {
      info.push_back(DEV_PAPER_TAPE);
      filename = paper_tape;
    }

    if (info[i].storage == StorageType::FIXED_SIZE) {
      dev.emplace_back(
          filename,
          StorageType::FIXED_SIZE,
          info[i].block_size * info[i].num_blocks * sizeof(Word)
      );
    } else {
      dev.emplace_back(
          filename,
          StorageType::STREAM,
          0
      );
    }
  }
}

void MixIO::init(MixClock *clock) {
  this->clock = clock;
}

int MixIO::execute(Word w) {
  Word aa = w.field(0, 2);
  int i = w.b(3); // already validated
  int f = w.b(4);
  int c = w.b(5);

  // validate f
  if (f >= NUM_DEVICES) {
    D3("Invalid f", f, w);
    return IO_ERR;
  }

  // validate m
  Word m = aa;
  if (i > 0) {
    m = m + core->i[i-1];
  }
  if ((c != 35) && (m < 0 || m >= MEM_SIZE)) {
    D3("Invalid m, (m,w) = ", m, w);
    return IO_ERR;
  }


  if (c == 35) {
    // IOC for tape devices
    if (info[f].type == DevType::MAGNETIC_TAPE) {
      if (((int)m) + pos[f] < 0 ||
         ((int)m) + pos[f] >= info[f].num_blocks) {
        D3("Invalid m for IOC:", m, w);
        return IO_ERR;
      }
    // IOC for disk, printer, and paper tape devices
    } else if (info[f].type == DevType::DISK ||
        info[f].type == DevType::LINE_PRINTER ||
        info[f].type == DevType::PAPER_TAPE) {
      if (m != 0) {
        D3("Invalid m for IOC:", m, w);
        return IO_ERR;
      }
    // IOC not supported for other devices
    } else {
      D3("Invalid m for IOC:", m, w);
      return IO_ERR;
    }
  }

  // validate x (for disk devices)
  if (c <=  36 && f >= 8 && f < 16 &&
      (core->x < 0 || core->x >= info[f].num_blocks)) {
    D3("Invalid x for disk device", core->x, w);
    return IO_ERR;
  }

  if (finish_ts[f] != -1) {
    D("Executing blocked I/O instruction! Should NEVER happen!");
    return IO_BLK;
  }

  D4("Staging io op #C M F = ", c, m, f);
  // Special case: if f is a disk and is already in the right
  // place, time to execute is cut by DISK_SEEK_FACTOR
  if (info[f].type == DevType::DISK && core->x == pos[f]) {
    do_io_ts[f] = clock->ts() + (info[f].time_to_do_io/DISK_SEEK_FACTOR);
    finish_ts[f] = clock->ts() + (info[f].time_to_finish/DISK_SEEK_FACTOR);
  } else {
    do_io_ts[f] = clock->ts() + info[f].time_to_do_io;
    finish_ts[f] = clock->ts() + info[f].time_to_finish;
  }
  D2("Io op will run at", do_io_ts[f]);
  D2("Io device will be unblocked at", finish_ts[f]);
  cur_inst[f] = w;
  return 0;
}

int MixIO::tick() {
  int tick_ret = 0;
  for (int d = 0; d < NUM_DEVICES; d++) {
    if (clock->ts() == do_io_ts[d]) {
      int ret;
      if ((ret = do_io(cur_inst[d])) < 0)
        tick_ret = ret;
      do_io_ts[d] = -1;
    }
    if (clock->ts() == finish_ts[d]) {
      finish_ts[d] = -1;
      cur_inst[d] = 0;
    }
  }
  return tick_ret;
}


int MixIO::next_ts() {
  int min = WORD_MAX;
  for (auto t : do_io_ts) {
    min = (t < min && t >= clock->ts()) ? t : min;
  }
  for (auto t : finish_ts) {
    min = (t < min && t >= clock->ts()) ? t : min;
  }
  return min;
}

int MixIO::free_ts(int f) {
  if (f < 0 || f >= NUM_DEVICES) {
    // invalid f, just pretend it's free to avoid weird IO block
    return -1;
  }
  return finish_ts[f];
}


std::vector<char> CHR_TABLE = {
  ' ', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I',
  '^', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R',
  '&', '#', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
  '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
  '.', ',', '(', ')', '+', '-', '*', '/', '=', '$',
  '<', '>', '@', ';', ':', '\''
};

std::map<char,int> CHR_REV_TABLE {
  {' ', 0},
  {'A', 1},
  {'B', 2},
  {'C', 3},
  {'D', 4},
  {'E', 5},
  {'F', 6},
  {'G', 7},
  {'H', 8},
  {'I', 9},
  {'^', 10},
  {'J', 11},
  {'K', 12},
  {'L', 13},
  {'M', 14},
  {'N', 15},
  {'O', 16},
  {'P', 17},
  {'Q', 18},
  {'R', 19},
  {'&', 20},
  {'#', 21},
  {'S', 22},
  {'T', 23},
  {'U', 24},
  {'V', 25},
  {'W', 26},
  {'X', 27},
  {'Y', 28},
  {'Z', 29},
  {'0', 30},
  {'1', 31},
  {'2', 32},
  {'3', 33},
  {'4', 34},
  {'5', 35},
  {'6', 36},
  {'7', 37},
  {'8', 38},
  {'9', 39},
  {'.', 40},
  {',', 41},
  {'(', 42},
  {')', 43},
  {'+', 44},
  {'-', 45},
  {'*', 46},
  {'/', 47},
  {'=', 48},
  {'$', 49},
  {'<', 50},
  {'>', 51},
  {'@', 52},
  {';', 53},
  {':', 54},
  {'\'', 55}
};

const char LINE_PRINTER_CLEAR[] =
  "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
  "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n";

int MixIO::do_io(Word w) {
  // All already validated
  Word aa = w.field(0, 2);
  int i = w.b(3);
  int f = w.b(4);
  int c = w.b(5);
  Word m = aa;
  if (i > 0) {
    m = m + core->i[i-1];
  }
  D4("Running io op #C M F = ", c, m, f);
  if (c == 35 || c == 36) { // IN, OUT
    int blocknum = -1;
    if (info[f].storage == StorageType::FIXED_SIZE) {
      if (info[f].type == DevType::DISK) {
        // disks support random access
        blocknum = core->x;
      } else {
        blocknum = pos[f];
      }
      pos[f] += 1; // block number will be incremented after read/write
    }
    if (info[f].fmt == Format::BINARY) {
      if (c == 35) { // IN, binary
        dev[f].read_block(
            (void *)&core->memory[m],
            blocknum * info[f].block_size * sizeof(Word),
            info[f].block_size * sizeof(Word));
      } else { // OUT, binary
        dev[f].write_block(
            (void *)&core->memory[m],
            blocknum * info[f].block_size * sizeof(Word),
            info[f].block_size * sizeof(Word));
      }
    } else if (info[f].fmt == Format::CHAR) {
      // TODO
    } else if (info[f].fmt == Format::CARD) {
      // TODO
    }
  } else if (c == 37) { // IOC
    if (info[f].type == DevType::MAGNETIC_TAPE) {
      if (m == 0)
        pos[f] = 0;
      else
        pos[f] += m;
    } else if (info[f].type == DevType::DISK) {
      pos[f] = core->x;
    } else if (info[f].type == DevType::LINE_PRINTER) {
      dev[f].write_block(
          (void *)&LINE_PRINTER_CLEAR[0],
          -1,
          sizeof(LINE_PRINTER_CLEAR));
    } else if (info[f].type == DevType::PAPER_TAPE) {
      pos[f] = 0;
    }
  }
  return 0;
}

