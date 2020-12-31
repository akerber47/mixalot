
class MixIO {
public:
  // In-memory core (owned by caller)
  // Custom device file maps
  MixIO(
      MixCore *core,
      std::string tape_prefix = "./dev/t",
      int tape_n = 8,
      std::string disk_prefix = "./dev/d",
      int disk_n = 8,
      std::string card_punch = "./dev/cp0",
      std::string card_reader = "./dev/cr0",
      std::string line_printer = "./dev/lp0",
      std::string paper_tape = "./dev/pt0"
  );
  ~MixIO();

  /*
   * Load a debug dump and output it through the given
   * I/O device. Only valid for output-capable devices.
   * Skip all invalid lines.
   */
  void load_out(int dev, int blocknum, std::string filename);

  /*
   * Dump a debug dump of the given I/O device input.
   * Only valid for input-capable devices.
   * See above.
   */
  void in_dump(int dev, int blocknum, std::string filename);

  /*
   * Execute I/O instructions.
   */
  int in(int dev, int blocknum);
  int out(int dev, int blocknum);
  int ioc(int dev, int f);

private:
  MixCore *core;

};

enum clas Format = { BINARY, CHAR, CARD };

class MixDev {
public:
  MixDev(std::string filename, Format fmt);
private:
  Format fmt;
  bool supports_in;
  bool supports_out;
  bool random_access;
  int fd = -1;



std::vector<char> CHR_TABLE = {
  ' ', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I',
  '^', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R',
  '&', '#', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
  '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
  '.', ',', '(', ')', '+', '-', '*', '/', '=', '$',
  '<', '>', '@', ';', ':', '\''
};
