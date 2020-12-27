#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include "io.h"
#include "dbg.h"

enum class Sign { POS, NEG };
enum class Overflow { OFF, ON };
enum class Comp { LESS, EQUAL, GREATER };

/*
 * A MIX byte (6 bits) will be passed around in a char.
 */
using Byte = unsigned char;

constexpr int BYTE_MAX = 077;
constexpr int ADDR_MAX = 07777;
constexpr int WORD_MAX = 07777777777;
constexpr long long DWORD_MAX = 077777777777777777777;

/*
 * Represents a MIX word (5 unsigned 6-bit bytes, and a sign.)
 * Representation:
 * - word value stored in native int
 * - MIX bytes and sign are "simulated" ie computed on demand
 *
 * Known incompatibility:
 *  - native cannot represent +0 and -0 separately.
 *    programs relying on -0 will behave incorrectly.
 *
 * Note this is a lightweight conversion class that is "trusting"
 * ie it assumes all input is well structured and nothing will
 * overflow. If any inputs are overflowing, result is going to
 * be a big mess (ie undefined).
 */
class Word {
public:
  Word(int w = 0) : w(w) {}
  Word(Sign s, std::vector<Byte> b) {
    int aw = 0;
    for (Byte x: b) {
      aw = (aw << 6) | x;
    }
    if (s == Sign::NEG) {
      aw = -aw;
    }
    w = aw;
  }
  operator int() const {
    return w;
  }
  // Sign = byte 0
  Sign sgn() { return (w >= 0) ? Sign::POS : Sign::NEG; }
  // Bytes numbered from 1 to 5
  Byte b(int i) {
    int aw = (w >= 0) ? w : -w;
    return (Byte) ((aw >> (6 * (5 - i))) & BYTE_MAX);
  }
  Word field(
      int l,
      int r,
      bool shift_left = false,
      bool shift_right = false);
private:
  int w;
};

/*
 * Fetch the field of the word associated with the field
 * specifier (l:r).
 * If shift_left is set, return the bytes shifted as far
 * to the left as possible. For instance, if (l:r) = (4:5)
 * return
 * + b4 b5 0 0 0
 * (instead of the default behavior, + 0 0 0 b4 b5)
 * Similarly for shifT_right
 */
Word Word::field(int l, int r, bool shift_left, bool shift_right) {
  if (l < 0 || r < 0 || l > 5 || r > 5) {
    D3("BAD FIELD! ", l, r);
  }
  Sign s = Sign::POS;
  if (l == 0) {
    s = this->sgn();
    l = 1;
  }
  std::vector<Byte> b(5);
  for (int i = l; i <= r; i++) {
    if (shift_left) {
      b[i-l] = this->b(i);
    } else if (shift_right) {
      b[5-r+i-1] = this->b(i);
    } else {
      b[i-1] = this->b(i);
    }
  }
  return Word(s, b);
}

// I/O helpers
// in "pretty print format"
std::istream& operator>>(std::istream& in, Word &w) {
  std::string raw_sgn;
  std::vector<Byte> b(5);
  in >> raw_sgn;
  if (in.fail()) {
    return in;
  }
  Sign s;
  if (raw_sgn == "+") {
    s = Sign::POS;
  } else if (raw_sgn == "-") {
    s = Sign::NEG;
  } else {
    in.setstate(std::ios_base::failbit);
    return in;
  }
  for (int i = 0; i < 5; i++) {
    unsigned next_byte;
    in >> next_byte;
    if (in.fail()) {
      return in;
    }
    b[i] = (Byte) next_byte;
  }
  w = {s, b};
  return in;
}

std::ostream& operator<<(std::ostream& out, Word w) {
  if (w.sgn() == Sign::POS) {
    out << "+ ";
  } else {
    out << "- ";
  }
  for (int i = 1; i < 6; i++) {
    out.width(2);
    out.fill('0');
    out << (unsigned) w.b(i);
    if (i < 5)
      out << " ";
  }
  return out;
}

/*
 * Represent MIX address as a full word for simplicity.
 * (so we can reuse the above helpers).
 * Only use bytes 1 and 2, and the sign.
 */
using Addr = Word;

constexpr int MEM_SIZE = 4000;
constexpr int CORE_SIZE = MEM_SIZE + 16;

struct MixCore {
  // Registers
  Word a;
  Word x;
  Addr i[6];
  Addr j;
  // Flags
  Overflow overflow;
  Comp comp;
  // Padding so memory is more aligned
  // and core dumps are easy to read!
  Word pad[5];
  // Memory starts at 16 words (0x40 bytes in xxd)
  // Memory
  Word memory[MEM_SIZE];
};

class Mix {
public:
  // In-memory core (owned by caller)
  Mix(MixCore *core, int pc = 0) : core(core), pc(pc) {};
  // Mapped core (owned by class)
  Mix(std::string core_file, int pc = 0) {
    void *raw_core = nullptr;
    open_and_map(
      core_file,
      sizeof(MixCore),
      raw_core,
      this->core_fd
    );
    this->core = static_cast<MixCore *>(raw_core);
  }
  ~Mix() {
    if (core_fd != -1) {
      unmap_and_close(core, sizeof(MixCore), core_fd);
    }
  }
  void load(std::string filename);
  void dump(std::string filename);
  std::string to_str(
      bool include_registers = true,
      bool include_memory = false,
      bool include_zeros = false);
  int execute(Word w);
  void step();
  void run();
  // Manually set some test values to check orchestration code
  void test();
private:
  MixCore *core;
  int pc;
  bool panic = false;
  std::string panic_msg = "";
  int core_fd = -1;
};

/*
 * Load a core dump or program listing into the current
 * Mix machine. Skip all invalid lines.
 */
void Mix::load(std::string filename) {
  D2("loading ", filename);
  std::ifstream fs {filename};
  for (std::string s; fs >> s; ) {
    // Registers
    if (s[0] == 'A') {
      fs >> core->a;
    } else if (s[0] == 'X') {
      fs >> core->x;
    } else if (s[0] == 'I') {
      int i = stoi(s.substr(2, 1));
      if (i >= 1 && i <= 6) {
        fs >> core->i[i];
      } else {
        fs.setstate(std::ios_base::failbit);
      }
    } else if (s[0] == 'J') {
      fs >> core->j;
    // Memory
    } else {
      int i = stoi(s);
      if (i >= 0 && i < MEM_SIZE) {
        fs >> core->memory[i];
      } else {
        fs.setstate(std::ios_base::failbit);
      }
    }
    if (fs.fail()) {
      fs.clear();
      // discard remainder of line and continue
      fs.unget();
      getline(fs, s);
    }
  }
  fs.close();
}

/*
 * Convert core fields of the Mix machine to a string
 * If include_registers is set, include registers in the string.
 * If include_memory is set, include memory in the string.
 * If include_zeros is set, keep lines for memory
 * rows that are zero.
 */
std::string Mix::to_str(
    bool include_registers,
    bool include_memory,
    bool include_zeros) {
  std::stringstream ss;
  // Registers
  if (include_registers) {
    ss << "   A: " << core->a << std::endl;
    ss << "   X: " << core->x << std::endl;
    for (int i = 0; i < 6; i++) {
      ss << "I[" << (i+1) << "]: " << core->i[i] << std::endl;
    }
    ss << "   J: " << core->j << std::endl;
  }
  if (include_memory) {
    for (int i = 0; i < MEM_SIZE; i++) {
      if (core->memory[i] != 0 || include_zeros) {
        ss.width(4);
        ss.fill('0');
        ss << i << ": " << core->memory[i] << std::endl;
      }
    }
  }
  return ss.str();
}

/*
 * Dump core fields of the Mix machine to a file.
 * See above.
 */
void Mix::dump(std::string filename) {
  D2("dumping to ", filename);
  std::ofstream fs {filename};
  fs << to_str(true, true, true);
  fs.close();
}

/*
 * Given a word, execute that word as though it's the current
 * instruction. Return the new value of the program counter.
 * ie, the address of the next instruction to execute.
 * Note that the word can be something other than the current
 * instruction (for debugging purposes).
 */
int Mix::execute(Word w) {
}

void Mix::step() {
  pc = execute(core->memory[pc]);
}

void Mix::run() {
  // TODO
}

void Mix::test() {
  core->a = 4;
  core->x = 5;
  core->i[0] = 3;
  core->i[1] = 9;
  core->i[2] = 27;
  core->i[3] = 81;
  core->overflow = Overflow::ON;
  core->memory[0] = 0xdeadbeef;
  core->memory[3999] = 0xdeadbeef;

}

void test_core() {
  D("test_core");
  Mix mix("./test.core");
  mix.test();
  // manually verify core file to check that it's good
  // $ xxd test.core | less
}

void test_dump() {
  D("test_dump");
  MixCore core;
  Mix m(&core);
  // set some values
  m.test();
  m.dump("./test_dump.txt");
  // load into new machine
  Mix m2("./test_dump.core");
  m2.load("./test_dump.txt");
  // manually verify core file to check that it's good
  // $ xxd test_dump.core | less
}

int main() {
  DBG_INIT();
  test_core();
  test_dump();
  DBG_CLOSE();
  return 0;
}
