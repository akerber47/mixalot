#include <string>
#include <vector>
#include "dbg.h"
#include "sys.h"
#include "core.h"
#include "io.h"
#include "cpu.h"
#include "clock.h"


constexpr int NUM_DEVICES = 21;

/*
 * Memory: 4x10^3 words
 * Tape: 4x10^6 words
 * Disk: 1x10^5 words
 * Paper tape: 1x10^5 words
 */

DevInfo DEV_MAGNETIC_TAPE = {
  Format::BINARY,
  StorageType::FIXED_SIZE,
  100,
  40000,
  true,
  true,
  500,
  1000,
};

DevInfo DEV_DISK = {
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
  for (int i = 0; i < NUM_DEVICES; i++) {
    do_io_ts.push_back(-1);
    finish_ts.push_back(-1);
    cur_inst.push_back(0);
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
    if ((f < 8 && (((int)m) + pos[f] < 0 ||((int)m) + pos[f] >= info[f].num_blocks)) ||
        // IOC for disk, printer, and paper tape devices
        (f >= 8 && m != 0)) {
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
    return IO_BLK;
  }

  do_io_ts[f] = clock->ts() + info[f].time_to_do_io;
  finish_ts[f] = clock->ts() + info[f].time_to_finish;
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
