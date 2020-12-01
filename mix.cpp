#include <vector>
#include <string>
#include <fstream>
#include "io.h"

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
  Sign sgn() { return (w >= 0) ? Sign::POS : Sign::NEG; }
  Byte b(int i) {
    int aw = (w >= 0) ? w : -w;
    return (Byte) ((aw >> (6 * (5 - i))) & BYTE_MAX);
  }
  Word field(int a, int b);
private:
  int w;
};

/*
 * Fetch the field of the word associated with the field
 * specifier (l:r)
 */
Word Word::field(int l, int r) {
  Sign s = Sign::POS;
  if (l == 0) {
    s = this->sgn();
    l = 1;
  }
  std::vector<Byte> b(5);
  for (int i = l; i < r; i++) {
    b[i-1] = this->b(i);
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
  for (int i = 0; i < 5; i++) {
    out.width(2);
    out.fill('0');
    out << (unsigned) w.b(i);
    if (i < 4)
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
  void dump(
      std::string filename,
      bool include_registers = true,
      bool include_zeros = true);
  void step();
  void run();
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
 * Dump a full core image of the Mix machine.
 * If include_zeros is set, omit lines for memory
 * rows that are zero.
 * If include_registers is set, include registers in the dump.
 */
void Mix::dump(
    std::string filename,
    bool include_registers,
    bool include_zeros) {
  std::ofstream fs {filename};
  // Registers
  if (include_registers) {
    fs << "   A: " << core->a << std::endl;
    fs << "   X: " << core->x << std::endl;
    for (int i = 0; i < 6; i++) {
      fs << "I[" << (i+1) << "]: " << core->i[i] << std::endl;
    }
    fs << "   J: " << core->j << std::endl;
  }
  for (int i = 0; i < MEM_SIZE; i++) {
    if (core->memory[i] != 0 || include_zeros) {
      fs.width(4);
      fs.fill('0');
      fs << i << ": " << core->memory[i] << std::endl;
    }
  }
  fs.close();
}

void Mix::step() {
  // TODO
}

void Mix::run() {
  // TODO
}

void Mix::test() {
  core->a = 4;
  core->x = 5;
  core->overflow = Overflow::ON;
  core->memory[0] = 0xdeadbeef;
  core->memory[3999] = 0xdeadbeef;
}

int main() {
  Mix mix("./core");
  mix.test();
  mix.step();
  mix.dump("./dump-test");
  mix.dump("./dump-test-2", true, false);
  return 0;
}
