#include <vector>

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
 * - MIX bytes and sign are stored directly.
 * - value is converted to a native int on demand.
 *
 * Note that a Word is a POD (plain old data) in C++
 * and fits in 32 bits of memory.
 */
class Word {
public:
  Word() = default;
  /*
   * Build a new word from a native integer value.
   * Native zero becomes +0.
   * If the native integer doesn't fit, truncate higher
   * bits and set the overload flag.
   */
  Word(int w);
  /*
   * Build a new word from 5 bytes and a sign.
   */
  Word(Sign sgn, std::vector<Byte> b);
  /*
   * Quick helpers to return individual fields of the word.
   * Sign = byte 0
   * Bytes numbered from 1 to 5
   * Overflow = overflow flag
   */
  Sign sgn() const;
  Byte b(int i) const;
  Overflow ov() const;
  /*
   * Return Overflow::ON if the word doesn't fit into
   * the final 2 bytes (b4 and b5).
   * That is, if it would overflow an index register.
   */
  Overflow iov() const;

  /*
   * A copy of this word, with overflow set to false.
   */
  Word with_nov() const;

  /*
   * Fetch the field of the word associated with the field
   * specifier (l:r).
   * shift_left and shift_right behave similarly to with_field
   * function below.
   * However, the default behavior of field() is to shift right
   * since we're building a new word.
   * If sign is not included in the field, it defaults to +
   * Note: w.field(...) == w0.with_field(w, ...)
   * where w0 is the zero word
   */
  Word field(
      int l,
      int r,
      bool shift_left = false,
      bool shift_right = true) const;

  /*
   * Build a new word by "copying the specified fields from src"
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
   * (instead of the unshifted behavior, * * * b4 b5)
   * Similarly for shift_right.
   * The default behavior is to shift_left.
   * Note that signs are not affected by shifts.
   *
   */
  Word with_field(Word src, int l, int r,
      bool default_positive = false,
      bool shift_left = true,
      bool shift_right = false);
  /*
   * Overloaded operators:
   * operator int() converts to a native integer
   *   (reverse of the one-argument constructor above)
   *   note that -0 is converted to native 0 (+0)
   * operator+ performs ordinary integer addition with some
   * special behavior:
   *   If the result is 0, the sign is set by the
   *   sign of the first argument to be either +0 or -0
   *   If the result doesn't fit, the outcome is truncated
   *   and the overflow flag is set.
   * operator- (unary) flips the sign only
   * operator<<, operator>> perform input/output pretty printing
   *   when the first argument is ostream/istream
   */
  operator int() const;
  Word operator+(Word w) const;
  Word operator-() const;
private:
  // true if overflow occurred while computing/storing this value.
  bool _ov : 1;
  // true if negative
  bool _s : 1;
  Byte _b1 : 6;
  Byte _b2 : 6;
  Byte _b3 : 6;
  Byte _b4 : 6;
  Byte _b5 : 6;
};

// More overloaded operators
std::istream& operator>>(std::istream& in, Word &w);
std::ostream& operator<<(std::ostream& out, Word w);


constexpr int MEM_SIZE = 4000;
constexpr int CORE_SIZE = MEM_SIZE + 16;

struct MixCore {
  // Registers
  Word a;
  Word x;
  Word i[6];
  Word j;
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

