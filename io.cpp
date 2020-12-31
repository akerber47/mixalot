#include <string>
#include <vector>
#include "sys.h"

enum class Format { BINARY, CHAR, CARD };
enum class StorageType { FIXED_SIZE, STREAM };

// High level info per device
struct DevInfo {
  Format fmt;
  StorageType storage;
  int block_size;
  int num_blocks; // only for FIXED_SIZE devices
  bool can_input;
  bool can_output;
};

DevInfo DEV_MAGNETIC_TAPE = {
  Format::BINARY,
  StorageType::FIXED_SIZE,
  100,
  1000000, // TODO look up realistic numbers
  true,
  true,
};

DevInfo DEV_DISK = {
  Format::BINARY,
  StorageType::FIXED_SIZE,
  100,
  100000, // TODO look up realistic numbers
  true,
  true,
};

DevInfo DEV_CARD_READER = {
  Format::CARD,
  StorageType::STREAM,
  16,
  0,
  true,
  false,
};

DevInfo DEV_CARD_PUNCH = {
  Format::CARD,
  StorageType::STREAM,
  16,
  0,
  false,
  true,
};

DevInfo DEV_LINE_PRINTER = {
  Format::CHAR,
  StorageType::STREAM,
  24,
  0,
  false,
  true,
};

DevInfo DEV_TERMINAL = {
  Format::CHAR,
  StorageType::STREAM,
  14,
  0,
  true,
  true,
};

DevInfo DEV_PAPER_TAPE = {
  Format::CHAR,
  StorageType::FIXED_SIZE,
  14,
  100000, // TODO look up realistic numbers
  true,
  true,
};

/*
 * Lightweight low-level resource object per device
 * to handle file descriptor read/write/seek
 *
 * Don't handle errors gracefully, just throw errors -> terminate.
 */
class MixDev {
public:
  // FIXED_SIZE -> open and set size to sz
  // STREAM -> open with append mode, and don't set size
  MixDev(std::string filename, StorageType storage, size_t sz);
  ~MixDev();
  // Read data into the given dest of the given size
  // (in bytes), from the given offset in the device file.
  // If off is -1, don't seek before reading.
  void read_block(void *dest, int off, size_t sz);
  // Write data from the given src of the given size
  // (in bytes), to the given offset in the device file.
  // If off is -1, don't seek before writing.
  void write_block(void *src, int off, size_t sz);
private:
  int fd = -1;
};

MixDev::MixDev(std::string filename, StorageType storage, size_t sz) {
  if (storage == StorageType::FIXED_SIZE)
    fd = open_and_resize(filename, sz);
  else
    fd = open_append(filename);
}

MixDev::~MixDev() {
  if (fd != -1)
    close(fd);
}

void MixDev::read_block(void *dest, int off, size_t sz) {
  (void) seek_read(fd, dest, off, sz);
}

void MixDev::write_block(void *src, int off, size_t sz) {
  (void) seek_write(fd, src, off, sz);
}

class MixIO {
public:
  MixIO(
      MixCore *core, // owned by caller
      std::string tape_prefix = "./dev/t",
      int tape_n = 8,
      std::string disk_prefix = "./dev/d",
      int disk_n = 8,
      std::string card_punch = "./dev/cp0",
      std::string card_reader = "./dev/cr0",
      std::string line_printer = "./dev/lp0",
      std::string paper_tape = "./dev/pt0"
  );

  int execute(Word w);
  int begin_execute(Word w);
  int tick(int ts);

private:
  MixCore *core;
  int num_devs;
  // Per device controller data
  std::vector<MixDev> dev;
  std::vector<DevInfo> info;
  // ongoing execution
  std::vector<int> finish_ts;
  std::vector<Word> cur_inst;
};

std::vector<char> CHR_TABLE = {
  ' ', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I',
  '^', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R',
  '&', '#', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
  '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
  '.', ',', '(', ')', '+', '-', '*', '/', '=', '$',
  '<', '>', '@', ';', ':', '\''
};
