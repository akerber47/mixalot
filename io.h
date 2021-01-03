class MixDev;
struct DevInfo;
class MixClock;

constexpr int IO_ERR = -1;
constexpr int IO_BLK = -2;

class MixIO {
public:
  MixIO(
      MixCore *core, // owned by caller
      std::string tape_prefix = "./dev/t",
      std::string disk_prefix = "./dev/d",
      std::string card_punch = "./dev/cp0",
      std::string card_reader = "./dev/cr0",
      std::string line_printer = "./dev/lp0",
      std::string terminal = "./dev/term0",
      std::string paper_tape = "./dev/pt0"
  );
  void init (MixClock *clock);
  int execute(Word w);
  int do_io(Word w);
  int tick();
  int next_ts();

private:
  MixCore *core;
  MixClock *clock = nullptr;
  // Per device controller data
  std::vector<MixDev> dev;
  std::vector<DevInfo> info;
  // ongoing execution
  std::vector<int> do_io_ts;
  std::vector<int> finish_ts;
  std::vector<Word> cur_inst;
  // only used for block devices
  std::vector<int> pos;
};

// Information below needed for compilation

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
  int time_to_do_io;
  int time_to_finish;
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
