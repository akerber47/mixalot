
enum class Sign { POS, NEG };
enum class Comp { LESS, EQUAL, GREATER };

/*
 * Representation:
 * - value stored in native int
 * - MIX bytes and sign are "simulated" ie computed on demand
 *
 * Known incompatibility:
 *  - native cannot represent +0 and -0 separately.
 *    programs relying on -0 will behave incorrectly.
 */
class Word {
public:
  Word(int w = 0) : w(w) {}
  operator int() const {
    return w;
  }
  Sign sgn() { return (w >= 0) ? Sign.POS : Sign.NEG; }
  unsigned b(int i) {
    int aw = (w < 0) ? -w : w;
  Word field(int a, int b);
private:
  int w;
}

class Addr {
public:
  Addr(int a = 0) : a(a) {}
  operator int() const {
    return a;
  }
  bool sgn = false;
  std::vector<unsigned char> b(2);
}

constexpr int MEM_SIZE = 4000;

class Mix {
  // 
  void load_memory(std::vector<unsigned i
private:
  // Registers
  Word a;
  Word x;
  std::vector<Addr> i(6);
  Addr j;
  // Memory
  std::vector<Word> memory(MEM_SIZE);
  // Implementation
  Addr pc;

