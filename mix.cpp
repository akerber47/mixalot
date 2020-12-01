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
    w = 0;
    for (Byte x: b) {
      w = (w << 6) | x;
    }
    if (s == Sign::NEG) {
      w = -w;
    }
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
  std::ifstream fs{filename, std::ios_base::in};
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
        // drop rest of line
        (void) getline(fs, s);
      }
    } else if (s[0] == 'J') {
      fs >> core->j;
    // Memory
    } else {
      int i = stoi(s);
      if (i >= 0 && i < MEM_SIZE) {
        fs >> core->memory[i];
      } else {
        // drop rest of line
        getline(fs, s);
      }
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
  std::ostream fs{filename, std::ios_base::out};
  // Registers
  if (include_registers) {
    fs << "A: " << core->a << std::endl;
    fs << "X: " << core->x << std::endl;
    for (int i = 0; i < 6; i++) {
      fs << "I[" << (i+1) << "]: " << core->i[i] << std::endl;
    }
    fs << "J: " << core->j << std::endl;
  }
  for (int i = 0; i < MEM_SIZE; i++) {
    if (core->memory[i] != 0 || include_zeros) {
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

int main() {
  Mix mix("./core");
  mix.core->a = 4;
  mix.core->x = 5;
  mix.core->overflow = Overflow::ON;
  mix.core->memory[0] = 0xdeadbeef;
  mix.core->memory[3999] = 0xdeadbeef;
  mix.step();
  return 0;
}
