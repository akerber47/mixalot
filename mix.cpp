#include <vector>
#include <string>

enum class Sign { POS, NEG };
enum class Overflow { ON, OFF };
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

struct MixCore {
  // Registers
  Word a;
  Word x;
  Addr i[6];
  Addr j;
  // Flags
  Overflow overflow;
  Comp comp;
  // Memory
  Word memory[MEM_SIZE];
};

class Mix {
public:
  Mix(MixCore core, int pc = 0) : core(core), pc(pc) {};
  Mix(std::string core_file, int pc = 0);
  void step();
  void run();
private:
  MixCore core;
  int pc;
  bool panic = false;
  std::string panic_msg = "";
};

Mix::Mix(std::string core_file, int pc) {
  // TODO
}

void Mix::step() {
  // TODO
}

void Mix::run() {
  // TODO
}

int main() {
  return 0;
}
