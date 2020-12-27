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
 *  - TODO fix -0
 *
 * Note this is a lightweight conversion class that is "trusting"
 * ie it assumes all input is well structured and nothing will
 * overflow. If any inputs are overflowing, result is going to
 * be a big mess (ie undefined).
 */
class Word {
public:
  /*
   * Build a new word from a native integer value.
   * Native zero becomes +0.
   */
  Word(int w = 0) : w(w) {}
  /*
   * Build a new word from 5 bytes and a sign.
   */
  Word(Sign s, std::vector<Byte> b);
  /*
   * Build a new word by "copying the specified fields from
   * src to dest".
   * In other words, start with a copy of the word dest, and
   * then copy over fields from the word src corresponding the
   * specifier (l:r).
   * If default_positive is set, and the field does not include 0,
   * the sign will be automatically set to +. (Otherwise, it will
   * stay the same as the sign of dest.)
   * If shift_left is set, copy over the bytes shifted as far
   * to the left as possible. For instance, if (l:r) = (4:5)
   * copy over
   * b4 b5 * * *
   * (instead of the default behavior, * * * b4 b5)
   * Similarly for shift_right.
   * Note that signs are not affected by shifts.
   *
   */
  Word(Word dest, Word src, int l, int r,
      bool default_positive = false,
      bool shift_left = false,
      bool shift_right = false);
  operator int() const {
    return w;
  }
  /*
   * Quick helpers to return individual fields of the word.
   * Sign = byte 0
   * Bytes numbered from 1 to 5
   */
  Sign sgn() { return (w >= 0) ? Sign::POS : Sign::NEG; }
  Byte b(int i) {
    int aw = (w >= 0) ? w : -w;
    return (Byte) ((aw >> (6 * (5 - i))) & BYTE_MAX);
  }
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
  Word field(
      int l,
      int r,
      bool shift_left = false,
      bool shift_right = false);
private:
  int w;
};

Word::Word(Sign s, std::vector<Byte> b) {
  int aw = 0;
  for (Byte x: b) {
    aw = (aw << 6) | x;
  }
  if (s == Sign::NEG) {
    aw = -aw;
  }
  w = aw;
}

Word::Word(Word dest, Word src, int l, int r,
    bool default_positive, bool shift_left, bool shift_right) {
  if (l < 0 || r < 0 || l > 5 || r > 5) {
    D3("BAD FIELD! ", l, r);
  }
  Sign s = default_positive ? Sign::POS : dest.sgn();
  if (l == 0) {
    s = src.sgn();
    l = 1;
  }
  std::vector<Byte> b =
    {dest.b(1), dest.b(2), dest.b(3), dest.b(4), dest.b(5)};
  for (int i = l; i <= r; i++) {
    if (shift_left) {
      b[i-l] = src.b(i);
    } else if (shift_right) {
      b[5-r+i-1] = src.b(i);
    } else {
      b[i-1] = src.b(i);
    }
  }
  Word(s,b);
}
Word Word::field(int l, int r, bool shift_left, bool shift_right) {
  return {0, *this, l, r, false, shift_left, shift_right};
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
  /*
   * Load a core dump or program listing into the current
   * Mix machine. Skip all invalid lines.
   */
  void load(std::string filename);
  /*
   * Dump core fields of the Mix machine to a file.
   * See above.
   */
  void dump(std::string filename);
  /*
   * Convert core fields of the Mix machine to a string
   * If include_registers is set, include registers in the string.
   * If include_memory is set, include memory in the string.
   * If include_zeros is set, keep lines for memory
   * rows that are zero.
   */
  std::string to_str(
      bool include_registers = true,
      bool include_memory = false,
      bool include_zeros = false);
  /*
   * Given a word, execute that word as though it's the current
   * instruction. Return the new value of the program counter.
   * ie, the address of the next instruction to execute.
   * Note that the word can be something other than the current
   * instruction (for debugging purposes).
   */
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

std::string Mix::to_str(
    bool include_registers,
    bool include_memory,
    bool include_zeros) {
  std::stringstream ss;
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

void Mix::dump(std::string filename) {
  D2("dumping to ", filename);
  std::ofstream fs {filename};
  fs << to_str(true, true, true);
  fs.close();
}

// operator classification helpers
// Arithmetic: ADD, SUB, MUL, DIV
bool arithop(int c) {
  return (c >= 1 && c <= 4);
}
// Memory: LD*, ST*
bool memop(int c) {
  return (c >= 7 && c <= 33);
}
// Jump: J*
// note JRED and JBUS are also I/O ops
bool jmpop(int c) {
  return (c == 34) ||
    (c >= 38 && c <= 47);
}
// I/O: IN, OUT, IOC, JRED, JBUS
// note JRED and JBUS are also jump ops
bool ioop(int c) {
  return (c >= 34 && c <= 38);
}
// Transfer: EN*, INC*, DEC*
bool transop(int c) {
  return (c >= 48 && c <= 55);
}
// Comparison: CMP*
bool cmpop(int c) {
  return (c >= 56);
}

int Mix::execute(Word w) {
  Word aa = w.field(0, 2, false, true);
  int i = w.b(3);
  int f = w.b(4);
  int c = w.b(5);
  // validate i
  if (i < 0 || i > 6) {
    D3("invalid i, (i,w) = ", i, w);
    return -1;
  }

  int m = (int) aa;
  if (i > 0) {
    m += (int) core->i[i-1];
  }
  // validate m
  if (
      // All arithmetic, memory, jump, cmp, and MOVE
      // ops require M to be a valid memory address
      ((arithop(c) || memop(c) || jmpop(c) ||
        cmpop(c) || (c == 7)) &&
       (m < 0 || m >= 4000)) ||
      // Shift op requires non negative m
      (c == 6 && m < 0)) {
    D3("Invalid m, (m,w) = ", m, w);
    return -1;
  }

  // validate f
  // Note that f is an (unsigned) byte from b(),
  // so it's guaranteed to be from 0 to 63
  int l = f / 8;
  int r = f % 8;
  if (
      // All arithmetic, memory, and cmp ops require
      // a valid field specification (L:R)
      // ie, 0 <= L <= R <= 5
      ((arithop(c) || memop(c) || cmpop(c)) &&
       (l > r || r > 5)) ||
      // Special ops require F = 0 (NUM), 1 (CHAR), or 2 (HLT)
      (c == 5 && f > 2) ||
      // Shift ops require F in [0,5]
      (c == 6 && f > 6) ||
      // IO ops require F to be an IO unit [0,20]
      (ioop(c) && f > 20) ||
      // Global jump ops require F in [0,9]
      (c == 39 && f > 9) ||
      // Register-based jump ops require F in [0,5]
      ((jmpop(c) && c != 34 && c != 38 && c != 39) && f > 6) ||
      // Transfer ops require F in [0,3]
      (transop(c) && f > 3)) {
    D3("invalid field, (f,w) = ", f, w);
    return -1;
  }


  // If we've made it this far, the instruction is valid.
  // Execute it.
  D6("Executing op #C M(L:R) F = ", c, m, l, r, f);

  Word& reg =
    (c % 8 == 0) ? core->a :
    (c % 8 == 7) ? core->x :
    core->i[(c % 8) - 1];
  Word mem_dummy = 0;
  Word& mem = (m >= 0 && m < 4000) ? core->memory[m] : mem_dummy;

  int next_pc = pc + 1;
  if (c == 0) {
    // NOP
  } else if (c >= 8 && c < 16) {
    // Load (LD*)
    reg = {0, mem, l, r, true, false, true};
  } else if (c >= 16 && c < 24) {
    // Load negative (LD*N)
    reg = {0, -(int)mem, l, r, true, false, true};
    // TODO: undefined behavior on index registers.
  } else if (c >= 24 && c < 32) {
    // Store (ST*)
    mem = {mem, reg, l, r, false, true, false};
  } else if (c >= 40 && c < 48) {
    // Register based jumps (J**)
    if ((f == 0 && reg < 0) || // J*N
        (f == 1 && reg == 0) || // J*Z
        (f == 2 && reg > 0) || // J*P
        (f == 3 && reg >= 0) || // J*NN
        (f == 4 && reg != 0) || // J*NZ
        (f == 5 && reg <= 0)) { // J*NP
      core->j = next_pc;
      next_pc = m;
    }
  } else if (c >= 48 && c < 56) {
    // Transfer operators
    // TODO ...
  }

  return next_pc;
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
