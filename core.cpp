#include <string>
#include "core.h"
#include "dbg.h"

Word::Word(int w) {
  _s = (w < 0);
  int aw = _s ? -w : w;
  _b1 = (Byte)((aw >> 24) & BYTE_MAX);
  _b2 = (Byte)((aw >> 18) & BYTE_MAX);
  _b3 = (Byte)((aw >> 12) & BYTE_MAX);
  _b4 = (Byte)((aw >> 6) & BYTE_MAX);
  _b5 = (Byte)(aw & BYTE_MAX);

  _ov = ((aw >> 30) > 0);
}

Word::Word(Sign sgn, std::vector<Byte> b) {
  _s = (sgn == Sign::NEG);
  _b1 = b[0] & BYTE_MAX;
  _b2 = b[1] & BYTE_MAX;
  _b3 = b[2] & BYTE_MAX;
  _b4 = b[3] & BYTE_MAX;
  _b5 = b[4] & BYTE_MAX;
  _ov = false;
  for (int i = 0; i < 5; i++) {
    if (b[i] > BYTE_MAX)
      _ov = true;
  }
}


Sign Word::sgn() const {
  return _s ? Sign::NEG : Sign::POS;
}

Byte Word::b(int i) const {
  switch (i) {
    case 1:
      return _b1;
    case 2:
      return _b2;
    case 3:
      return _b3;
    case 4:
      return _b4;
    case 5:
      return _b5;
    default:
      D3("Bad byte! (b,w) = ", i, *this);
      return -1;
  }
}

Overflow Word::ov() const {
  return _ov ? Overflow::ON : Overflow::OFF;
}

Overflow Word::iov() const {
  return (_ov || _b1 || _b2 || _b3) ? Overflow::ON : Overflow::OFF;
}

Word Word::with_nov() const {
  Word w = *this;
  w._ov = false;
  return w;
}

Word Word::field(int l, int r, bool shift_left, bool shift_right) const {
  Word w0(0);
  return w0.with_field(*this, l, r, false, shift_left, shift_right);
}

Word Word::with_field(Word src, int l, int r,
    bool default_positive, bool shift_left, bool shift_right) {
  if (l < 0 || r < 0 || l > 5 || r > 5) {
    D3("BAD FIELD! ", l, r);
  }
  Sign s = default_positive ? Sign::POS : this->sgn();
  if (l == 0) {
    s = src.sgn();
    l = 1;
  }
  std::vector<Byte> b =
    {this->b(1), this->b(2), this->b(3), this->b(4), this->b(5)};
  for (int i = l; i <= r; i++) {
    if (shift_left) {
      b[i-1] = src.b(5-r+i);
    } else if (shift_right) {
      b[5-r+i-1] = src.b(i);
    } else {
      b[i-1] = src.b(i);
    }
  }
  Word w(s,b);
  w._ov = src._ov || this->_ov;
  return w;
}

Word::operator int() const {
  int w = _b1;
  w = (w << 6) | _b2;
  w = (w << 6) | _b3;
  w = (w << 6) | _b4;
  w = (w << 6) | _b5;
  if (_s)
    w = -w;
  return w;
}

Word Word::operator+(Word w) const {
  Word sum = ((int) *this) + ((int) w);
  if (*this == 0) {
    sum._s = this->_s;
  }
  return sum;
}

Word Word::operator-() const {
  Word w = *this;
  w._s = !w._s;
  return w;
}

// I/O helpers
// in "book print format"
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

