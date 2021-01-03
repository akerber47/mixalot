#include <string>
#include <sstream>
#include <vector>
#include "dbg.h"
#include "sys.h"
#include "core.h"
#include "io.h"
#include "cpu.h"
#include "clock.h"

constexpr int PC_ERR = -1;
constexpr int PC_HLT = -2;

MixCPU::MixCPU(MixCore *core) {
  this->core = core;
}

void MixCPU::init(MixClock *clock, MixIO *io) {
  this->clock = clock;
  this->io = io;
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

int MixCPU::execute(Word w) {
  Word aa = w.field(0, 2);
  int i = w.b(3);
  int f = w.b(4);
  int c = w.b(5);
  // validate i
  if (i < 0 || i > 6) {
    D3("invalid i, (i,w) = ", i, w);
    return PC_ERR;
  }

  Word m = aa;
  if (i > 0) {
    m = m + core->i[i-1];
  }
  // Note: if m == 0, m has same sign as aa

  // validate m
  if (
      // All arithmetic, memory, jump, cmp, and MOVE
      // ops require M to be a valid memory address
      ((arithop(c) || memop(c) || jmpop(c) ||
        cmpop(c) || (c == 7)) &&
       (m < 0 || m >= MEM_SIZE)) ||
      // Shift op requires non negative m
      (c == 6 && m < 0)) {
    D3("Invalid m, (m,w) = ", m, w);
    return PC_ERR;
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
      // Global jump ops require F in [0,9]
      (c == 39 && f > 9) ||
      // Register-based jump ops require F in [0,5]
      ((jmpop(c) && c != 34 && c != 38 && c != 39) && f > 6) ||
      // Transfer ops require F in [0,3]
      (transop(c) && f > 3)) {
    D3("invalid field, (f,w) = ", f, w);
    return PC_ERR;
  }

  // If we've made it this far, the instruction is valid.
  // Execute it.
  D6("Executing op #C M(L:R) F = ", c, m, l, r, f);

  Word& reg =
    (c % 8 == 0) ? core->a :
    (c % 8 == 7) ? core->x :
    core->i[(c % 8) - 1];
  Word dummy = 0; // for mem to reference if it's unused
  Word& mem = (m >= 0 && m < MEM_SIZE) ? core->memory[m] : dummy;

  int next_pc = (pc + 1) % MEM_SIZE;
  if (c == 0) {
    // NOP
  } else if (c == 1) {
    // ADD
    core->a = core->a + mem;
  } else if (c == 2) {
    // SUB
    core->a = core->a + (-mem);
  } else if (c == 3) {
    // MUL
    long long out = ((long long) core->a) * ((long long) mem);
    bool neg = (out < 0);
    unsigned long long ax = neg ? -out : out;
    int a = (ax >> 30);
    int x = (ax & WORD_MAX);
    core->a = neg ? -a : a;
    core->x = neg ? -x : x;
  } else if (c == 4) {
    // DIV
    if (mem == 0) {
      D("Divide by zero, setting overflow");
      core->overflow = Overflow::ON;
    } else {
      bool neg = (core->a < 0);
      unsigned long long ax = neg ? -core->a : core->a;
      ax = (ax << 30) | ((core->x < 0) ? -core->x : core->x);
      bool mneg = (mem < 0);
      unsigned long long v = mneg ? -mem : mem;
      unsigned long long q = ax / v;
      unsigned long long r = ax % v;
      if (q > WORD_MAX || r > WORD_MAX)
        core->overflow = Overflow::ON;
      int ua = (int)(q & WORD_MAX);
      int ux = (int)(r & WORD_MAX);
      core->a = ((neg && !mneg) || (!neg && mneg)) ? -ua : ua;
      core->x = neg ? -ux : ux;
    }
  } else if (c == 5) {
    switch (f) {
      case 0: // NUM
      {
        unsigned long long num = 0;
        for (int i = 1; i <= 5; i++)
          num = (num * 10) + (core->a.b(i) % 10);
        for (int i = 1; i <= 5; i++)
          num = (num * 10) + (core->x.b(i) % 10);
        if (num > WORD_MAX)
          core->overflow = Overflow::ON;
        Word w = (num % (WORD_MAX + 1));
        std::vector<Byte> newa = {w.b(1), w.b(2), w.b(3), w.b(4), w.b(5)};
        core->a = {core->a.sgn(), newa};
        break;
      }
      case 1: // CHR
      {
        int num = (core->a >= 0 ? core->a : -core->a);
        std::vector<Byte> newa = {0, 0, 0, 0, 0};
        std::vector<Byte> newx = {0, 0, 0, 0, 0};
        for (int i = 4; i >= 0; i--) {
          newx[i] = 30 + (num % 10);
          num = num / 10;
        }
        for (int i = 4; i >= 0; i--) {
          newa[i] = 30 + (num % 10);
          num = num / 10;
        }
        core->a = {core->a.sgn(), newa};
        core->x = {core->x.sgn(), newx};
        break;
      }
      case 2: // HLT
        D("Halt!");
        return PC_HLT;
    }
  } else if (c == 6) {
    // SL* vs SR* (negative vs positive index offset)
    int sm = (f % 2 == 0) ? -m : m;
    if (f < 2) { // SLA, SRA
      std::vector<Byte> newa = {0, 0, 0, 0, 0};
      for (int i = 0; i < 5; i++) {
        if (i + sm >= 0 && i + sm < 5)
          newa[i+sm] = core->a.b(i+1);
      }
      core->a = {core->a.sgn(), newa};
    } else if (f >= 2 && f < 4) { // SLAX, SRAX
      std::vector<Byte> newa = {0, 0, 0, 0, 0};
      std::vector<Byte> newx = {0, 0, 0, 0, 0};
      for (int i = 0; i < 10; i++) {
        Byte bi = (i < 5) ? core->a.b(i+1) : core->x.b(i-5+1);
        if (i + sm >= 0 && i + sm < 5)
          newa[i+sm] = bi;
        else if (i + sm >= 5 && i + sm < 10)
          newx[i+sm-5] = bi;
      }
      core->a = {core->a.sgn(), newa};
      core->x = {core->x.sgn(), newx};
    } else { // SLC, SRC
      std::vector<Byte> newa = {0, 0, 0, 0, 0};
      std::vector<Byte> newx = {0, 0, 0, 0, 0};
      for (int i = 0; i < 10; i++) {
        Byte bi = (i < 5) ? core->a.b(i+1) : core->x.b(i-5+1);
        if ((i+sm) % 10 < 5)
          newa[(i+sm) % 10] = bi;
        else
          newx[((i+sm) % 10) - 5] = bi;
      }
      core->a = {core->a.sgn(), newa};
      core->x = {core->x.sgn(), newx};
    }
  } else if (c == 7) {
    // MOVE
    for (int k = 0; k < f; k++) {
      int k0 = ((int) m) + k;
      int k1 = ((int) core->i[0]) + k;
      if (k0 < 0 || k1 < 0) {
        D("Move command underflowed memory");
        return PC_ERR;
      }
      if (k0 >= 4000 || k1 >= 4000) {
        D("Move command overflowed memory");
        return PC_ERR;
      }
      core->memory[k1] = core->memory[k0];
    }
    core->i[0] = core->i[0] + (Word)f;
  } else if (c >= 8 && c < 16) {
    // Load (LD*)
    reg = mem.field(l, r);
  } else if (c >= 16 && c < 24) {
    // Load negative (LD*N)
    reg = (-mem).field(l, r);
  } else if (c >= 24 && c < 32) {
    // Store (ST*)
    mem = mem.with_field(reg, l, r);
  } else if (c == 32) {
    // STJ
    mem = mem.with_field(core->j, l, r);
  } else if (c == 33) {
    // STZ
    mem = mem.with_field(0, l, r);
  } else if (c >= 34 && c < 39) {
    D("Calling IO coprocessor");
    io->execute(w);
  } else if (c == 39) {
    // Global jumps
    if (f == 1) {
      // JSJ
      next_pc = m;
    } else if (f == 2 && core->overflow == Overflow::ON) {
      // JOV
      core->overflow = Overflow::OFF;
      core->j = next_pc;
      next_pc = m;
    } else if (
        (f == 0) || // JMP
        (f == 3 && core->overflow == Overflow::OFF) || // JNOV
        (f == 4 && core->comp == Comp::LESS) || // JL
        (f == 5 && core->comp == Comp::EQUAL) || // JE
        (f == 6 && core->comp == Comp::GREATER) || // JG
        (f == 7 && core->comp != Comp::LESS) || // JGE
        (f == 8 && core->comp != Comp::EQUAL) || // JNE
        (f == 9 && core->comp != Comp::GREATER)) { // JLE
      core->j = next_pc;
      next_pc = m;
    }
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
    switch (f) {
      case 0: // INC*
        reg = reg + m; break;
      case 1: // DEC*
        reg = reg + (-m); break;
      case 2: // ENT*
        reg = m; break;
      case 3: // ENN*
        reg = -m; break;
    }
  } else {
    // Comparison operators
    Word rf = reg.field(l, r);
    Word mf = mem.field(l, r);
    if (rf < mf)
      core->comp = Comp::LESS;
    else if (rf == mf)
      core->comp = Comp::EQUAL;
    else
      core->comp = Comp::GREATER;
  }

  // check/validate I overflow
  for (int i = 0; i < 6; i++) {
    if (core->i[i].iov() == Overflow::ON) {
      D3("Overflowed I register, undefined, (i,reg i)",
          i,
          core->i[i]);
      return PC_ERR;
    }
  }

  // check A/X overflow
  if (core->a.ov() == Overflow::ON) {
    D("Overflowed A register");
    core->overflow = Overflow::ON;
    core->a = core->a.with_nov();
  }
  if (core->x.ov() == Overflow::ON) {
    D("Overflowed X register");
    core->overflow = Overflow::ON;
    core->x = core->x.with_nov();
  }

  return next_pc;
}

int MixCPU::tick() {
  if (clock->ts() < get_ts(core->memory[pc]))
    return 0;
  int next_pc = execute(core->memory[pc]);
  // set previous ts for execution
  previous_ts = clock->ts();
  if (next_pc < 0)
    return next_pc;
  return 0;
}

int MixCPU::next_ts() {
  return get_ts(core->memory[pc]);
}

int MixCPU::get_ts(Word w) {
  int c = w.b(5);
  int f = w.b(4);
  int ts = previous_ts;
  D5("Computing ts for word W (with C, F) given previous ts = ", w, c, f, ts);
  if ((c == 1 || c == 2) || // ADD, SUB
      (c == 6) || // Shift
      (c >= 8 && c < 33) || // LD*, ST*
      (c >= 56)) { // CMP*
    ts += 2;
  } else if ((c == 3) || // MUL
      (c == 5 && (f == 0 || f == 1))) { // NUM, CHR
    ts += 10;
  } else if (c == 4) { // DIV
    ts += 12;
  } else if (c == 7) { // MOVE
    ts += (1 + 2*f);
  } else if ((c >= 35 && c < 38) || // blocking IO
      (c == 34 && w.b(3) == 0 && w.field(0,2) == pc)) { // JBUS *
    // Execute after device is free
    int free_ts = io->free_ts(f);
    if (free_ts < 0) {
      ts += 1;
    } else {
      ts = free_ts + 1;
    }
  } else {
    ts += 1;
  }
  D2("Found execution time ts", ts);
  return ts;
}
