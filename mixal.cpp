#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include "dbg.h"
#include "core.h"

class Asm_error {
public:
  Asm_error(int err) : err(err) {}
  int err;
};

std::map<std::string, std::pair<int, int>> OP_TABLE = {
  {"NOP", {000, 0}},
  {"ADD", {001, 5}},
  {"SUB", {002, 5}},
  {"MUL", {003, 5}},
  {"DIV", {004, 5}},
  {"NUM", {005, 0}},
  {"CHR", {005, 1}},
  {"HLT", {005, 2}},
  {"SLA", {006, 0}},
  {"SRA", {006, 1}},
  {"SLAX", {006, 2}},
  {"SRAX", {006, 3}},
  {"SLC", {006, 4}},
  {"SRC", {006, 5}},
  {"MOVE", {007, 1}},
  {"LDA", {010, 5}},
  {"LD1", {011, 5}},
  {"LD2", {012, 5}},
  {"LD3", {013, 5}},
  {"LD4", {014, 5}},
  {"LD5", {015, 5}},
  {"LD6", {016, 5}},
  {"LDX", {017, 5}},
  {"LDAN", {020, 5}},
  {"LD1N", {021, 5}},
  {"LD2N", {022, 5}},
  {"LD3N", {023, 5}},
  {"LD4N", {024, 5}},
  {"LD5N", {025, 5}},
  {"LD6N", {026, 5}},
  {"LDXN", {027, 5}},
  {"STA", {030, 5}},
  {"ST1", {031, 5}},
  {"ST2", {032, 5}},
  {"ST3", {033, 5}},
  {"ST4", {034, 5}},
  {"ST5", {035, 5}},
  {"ST6", {036, 5}},
  {"STX", {037, 5}},
  {"STJ", {040, 2}},
  {"STZ", {041, 5}},
  {"JBUS", {042, 0}},
  {"IOC", {043, 0}},
  {"IN", {044, 0}},
  {"OUT", {045, 0}},
  {"JRED", {046, 0}},
  {"JMP", {047, 0}},
  {"JSJ", {047, 1}},
  {"JOV", {047, 2}},
  {"JNOV", {047, 3}},
  {"JL", {047, 4}},
  {"JE", {047, 5}},
  {"JG", {047, 6}},
  {"JGE", {047, 7}},
  {"JNE", {047, 8}},
  {"JLE", {047, 9}},
  {"JAN", {050, 0}},
  {"J1N", {051, 0}},
  {"J2N", {052, 0}},
  {"J3N", {053, 0}},
  {"J4N", {054, 0}},
  {"J5N", {055, 0}},
  {"J6N", {056, 0}},
  {"JXN", {057, 0}},
  {"JAZ", {050, 1}},
  {"J1Z", {051, 1}},
  {"J2Z", {052, 1}},
  {"J3Z", {053, 1}},
  {"J4Z", {054, 1}},
  {"J5Z", {055, 1}},
  {"J6Z", {056, 1}},
  {"JXZ", {057, 1}},
  {"JAP", {050, 2}},
  {"J1P", {051, 2}},
  {"J2P", {052, 2}},
  {"J3P", {053, 2}},
  {"J4P", {054, 2}},
  {"J5P", {055, 2}},
  {"J6P", {056, 2}},
  {"JXP", {057, 2}},
  {"JANN", {050, 3}},
  {"J1NN", {051, 3}},
  {"J2NN", {052, 3}},
  {"J3NN", {053, 3}},
  {"J4NN", {054, 3}},
  {"J5NN", {055, 3}},
  {"J6NN", {056, 3}},
  {"JXNN", {057, 3}},
  {"JANZ", {050, 4}},
  {"J1NZ", {051, 4}},
  {"J2NZ", {052, 4}},
  {"J3NZ", {053, 4}},
  {"J4NZ", {054, 4}},
  {"J5NZ", {055, 4}},
  {"J6NZ", {056, 4}},
  {"JXNZ", {057, 4}},
  {"JANP", {050, 5}},
  {"J1NP", {051, 5}},
  {"J2NP", {052, 5}},
  {"J3NP", {053, 5}},
  {"J4NP", {054, 5}},
  {"J5NP", {055, 5}},
  {"J6NP", {056, 5}},
  {"JXNP", {057, 5}},
  {"INCA", {060, 0}},
  {"INC1", {061, 0}},
  {"INC2", {062, 0}},
  {"INC3", {063, 0}},
  {"INC4", {064, 0}},
  {"INC5", {065, 0}},
  {"INC6", {066, 0}},
  {"INCX", {067, 0}},
  {"DECA", {060, 1}},
  {"DEC1", {061, 1}},
  {"DEC2", {062, 1}},
  {"DEC3", {063, 1}},
  {"DEC4", {064, 1}},
  {"DEC5", {065, 1}},
  {"DEC6", {066, 1}},
  {"DECX", {067, 1}},
  {"ENTA", {060, 2}},
  {"ENT1", {061, 2}},
  {"ENT2", {062, 2}},
  {"ENT3", {063, 2}},
  {"ENT4", {064, 2}},
  {"ENT5", {065, 2}},
  {"ENT6", {066, 2}},
  {"ENTX", {067, 2}},
  {"ENNA", {060, 3}},
  {"ENN1", {061, 3}},
  {"ENN2", {062, 3}},
  {"ENN3", {063, 3}},
  {"ENN4", {064, 3}},
  {"ENN5", {065, 3}},
  {"ENN6", {066, 3}},
  {"ENNX", {067, 3}},
  {"CMPA", {070, 5}},
  {"CMP1", {071, 5}},
  {"CMP2", {072, 5}},
  {"CMP3", {073, 5}},
  {"CMP4", {074, 5}},
  {"CMP5", {075, 5}},
  {"CMP6", {076, 5}},
  {"CMPX", {077, 5}}
};

std::map<char,Byte> CHAR_TABLE {
  {' ', 0},
  {'A', 1},
  {'B', 2},
  {'C', 3},
  {'D', 4},
  {'E', 5},
  {'F', 6},
  {'G', 7},
  {'H', 8},
  {'I', 9},
  {'J', 11},
  {'K', 12},
  {'L', 13},
  {'M', 14},
  {'N', 15},
  {'O', 16},
  {'P', 17},
  {'Q', 18},
  {'R', 19},
  {'S', 22},
  {'T', 23},
  {'U', 24},
  {'V', 25},
  {'W', 26},
  {'X', 27},
  {'Y', 28},
  {'Z', 29},
  {'0', 30},
  {'1', 31},
  {'2', 32},
  {'3', 33},
  {'4', 34},
  {'5', 35},
  {'6', 36},
  {'7', 37},
  {'8', 38},
  {'9', 39},
};

int star = 0;
bool ended = false;
std::map<int, Word> words = {};
// Literal values (key -> value) for defined globals/locals
std::map<std::string, int> globals = {};
std::map<int, int> locals = {};
// "Daisy chain instructions" (key -> linked list) that need to
// be substituted once the future global/local is defined.
// The list next pointer is sneakily stored in the address field of each
// assembled instruction to make it easy to traverse and
// rewrite as needed.
// -1 = end of list marker
std::map<std::string, int> fglobals = {};
std::map<int, int> flocals = {};

int next_literal = 0;
std::vector<int> literals;


inline bool is09(char c) {
  return (c >= '0' && c <= '9');
}

inline bool isAZ(char c) {
  return (c >= 'A' && c <= 'Z');
}

inline int ctoi(char c) {
  return (c - '0');
}

// Check if a symbol is local, and if so put the local index in &i
// if it's a future local symbol (#F), set &is_future to true.
// If loc_context is true, look for (#H) symbols not #F/#B
//   it's an error to use #H in addr context or #F/#B in loc context
// If it's not a local symbol, put -1 in the i argument.
// Throw on failure.
void get_local(std::string sym, bool loc_context, int &i, bool &is_future) {
  if (sym.size() == 2 && is09(sym[0])) {
    if (sym[1] == 'H') {
      if (loc_context) {
        i = ctoi(sym[0]);
        is_future = false;
      } else {
        D2("Invalid appearance of local H symbol in addr context! ", sym);
        throw Asm_error(-1);
      }
    } else if (sym[1] == 'F' || sym[1] == 'B') {
      if (!loc_context) {
        i = ctoi(sym[0]);
        is_future = (sym[1] == 'F');
      } else {
        D2("Invalid appearance of local BF symbol in loc context! ", sym);
        throw Asm_error(-1);
      }
    }
  }
  i = -1;
  is_future = false;
}

Word build_word(int a, int i, int f, int c) {
  Word out_a = a;
  Word w {out_a.sgn(), {out_a.b(4), out_a.b(5),
    (Byte) i, (Byte) f, (Byte) c}};
  D2("Assembled word: ", w);
  return w;
}

// Add symbol to global/local symbol tables
// NOTE that this also corrects any daisy chained future symbols
void define_symbol(std::string sym, int val) {

  D4("Adding symbol definition: ", sym, " = ", val);
  // Will be set to the most recent appearance if there
  // are earlier appearances of this symbol (before definition)
  int future_chain = -1;

  int ix;
  bool is_future;
  get_local(sym, true, ix, is_future);

  if (ix >= 0) { // Local
    locals[ix] = val;

    if (flocals.find(ix) != flocals.end()) {
      future_chain = flocals[ix];
      flocals.erase(ix);
    }
  } else { // Global
    if (globals.find(sym) != globals.end()) {
      D4("Error: global symbol already defined! ",
          sym, " = ", globals[sym]);
      throw Asm_error(-1);
    }
    globals[sym] = val;
    if (fglobals.find(sym) != fglobals.end()) {
      future_chain = fglobals[sym];
      fglobals.erase(sym);
    }
  }

  // chase future definition chain
  while (future_chain != -1) {
    D2("Found previous appearance of this as a future symbol at ",
        future_chain);
    int tmp = words[future_chain].field(0,2);
    words[future_chain] = build_word(
        val,
        words[future_chain].b(3),
        words[future_chain].b(4),
        words[future_chain].b(5));
    future_chain = tmp;
  }
}

// Find symbol in global/local symbol tables
// Return value in &val
// Return 0 on success, -1 on failure (not found)
int lookup_symbol(std::string sym, int &val) {
  D2("Looking up symbol: ", sym);
  int ix;
  bool is_future;
  get_local(sym, false, ix, is_future);

  if (ix >= 0) { // Local
    // Cannot lookup future locals yet
    if (is_future)
      return -1;
    if (locals.find(ix) == locals.end()) {
      D2("Failed to find definition for past local", sym);
      throw Asm_error(-1);
    }
    val = locals[ix];
    return 0;
  } else { // Global
    if (globals.find(sym) == globals.end()) {
      return -1;
    } else {
      val = globals[sym];
      return 0;
    }
  }
}

// Add a future symbol daisy chain entry
// Return the value that should be stored in addr for this row
// that can be used for daisy chaining in &a
// Throw on error.
void add_future(std::string sym, int &a) {
  D4("Adding future symbol reference: ", sym, " at ", star);
  int ix;
  bool is_future;
  get_local(sym, false, ix, is_future);

  if (ix >= 0) { // Local
    if (!is_future) {
      D2("Inconsistent! Trying to add non future local as future",
          sym);
      throw Asm_error(-1);
    }
    a = flocals[ctoi(sym[0])];
    flocals[ctoi(sym[0])] = star;
  } else { // Global
    if (fglobals.find(sym) != fglobals.end()) {
      // daisy chain
      a = fglobals[sym];
    } else {
      a = -1;
    }
    fglobals[sym] = star;
  }
}

// Clean futures by adding additional rows
// (starting at *) that define the appropriate symbols
// called at END
// throw on error
void clean_futures() {
  D("Cleaning up remaining future symbols...");
  for (auto p : flocals) {
    D3("Error! Undefined future local reference at END at",
        p.first, p.second);
    throw Asm_error(-1);
  }
  for (auto p : fglobals) {
    // Fake instruction:
    // <SYM> CON 0
    define_symbol(p.first, star);
    words[star++] = 0;
  }
}


// Parse "expression" (see Knuth) and return its value
// Throw on error
// Note that expressions must have fully determined numerical values!
int parse_exp(std::string s) {
  D2("Parsing expression from: ", s);
  if (s == "") {
    D("Expression cannot be empty!");
    throw Asm_error(-1);
  }
  unsigned long i = 0;
  int e = 0;
  std::string binop = "+";
  std::string unop = "+";
  while (i < s.size()) {
    // Ingest next binop
    if (i == 0) {
      // Hack: on first iteration, fake "+" binop
      // (so operation will be 0 + atom)
      // and move to unop/atom
      binop = "+";
    } else {
      // search: +, -, *, /, //, :
      if (s[i] == '/') {
        i++;
        if (i < s.size() && s[i] == '/') {
          binop = "//";
          i++;
        } else {
          binop = "/";
        }
      } else {
        binop = s[i++];
      }
    }

    // Check for dangling binary operator (missing RHS)
    if (i == s.size()) {
      D2("Expected atom following operator ", binop);
      D2(" in expression ", s);
      throw Asm_error(-1);
    }

    //Ingest next unop, if any
    if (s[i] == '+' || s[i] == '-') {
      unop = s[i++];
    } else {
      unop = "+";
    }

    // Ingest next atom
    int atom = 0;
    unsigned long atom_start = i;
    if (s[i] == '*') { // * atom
      atom = star;
      i++;
    } else {
      bool has_az = false;
      while (i < s.size() && (is09(s[i]) || isAZ(s[i]))) {
        if (isAZ(s[i]))
          has_az = true;
        i++;
      }
      std::string atom_sym = {s, atom_start, i-atom_start};
      if (atom_sym == "") {
        D2("Expected atom following operator ", binop);
        D2(" in expression ", s);
        throw Asm_error(-1);
      }
      if (has_az) { // symbol atom
        if (lookup_symbol(atom_sym, atom) < 0) {
          D3("Undefined symbol in expression!", atom_sym, s);
          throw Asm_error(-1);
        }
      } else {
        atom = std::stoi(atom_sym);
      }
    }

    // Finally, do the arithmetic
    int rhs = (unop == "-") ? -atom : atom;
    if (binop == "+") {
      e += rhs;
    } else if (binop == "-") {
      e -= rhs;
    } else if (binop == "*") {
      e *= rhs;
    } else if (binop == "/") {
      e /= rhs;
    } else if (binop == "//") {
      // TODO this isn't right
      e %= rhs;
    } else if (binop == ":") {
      e = (8*e) + rhs;
    }
  }
  return e;
}

// Parse "W-value" (see Knuth) and return its value
// throw on error
int parse_w(std::string s) {
  D2("Parsing W-value from: ", s);
  unsigned long pos = 0;
  unsigned long next_pos = s.find(',', pos);
  Word w = 0;
  do {
    std::string s_term = {s, pos, next_pos-pos};
    auto lpos = s_term.find('(');
    auto rpos = s_term.find(')');
    if (lpos == std::string::npos &&
        rpos == std::string::npos) {
      int e = parse_exp(s_term);
      w = e;
    } else if (lpos != std::string::npos &&
        rpos != std::string::npos &&
        rpos == s_term.size()-1 &&
        lpos < rpos) {
      int e = parse_exp({s_term, 0, lpos});
      int f = parse_exp({s_term, lpos+1, rpos-lpos-1});
      int l = f / 8;
      int r = f % 8;
      if (l > r || r > 5) {
        D2("Bad field! ", f);
        throw Asm_error(-1);
      }
      w = w.with_field(e, l, r);
    }
    pos = next_pos + 1;
    next_pos = s.find(',', pos);
  } while (next_pos != std::string::npos);
  return w;
}

// Parse "A-part", "I-part", and "F-part" (see Knuth)
// A-part is returned in:
//   a if it's an expression
//   future_a if it's a future value
//   literal_a if it's a literal value
// Any unused cases are set to -1/empty string
// i set to 0 if not specified
// f set to -1 if not specified
// throw on error
void parse_aif(std::string s, int &a, std::string &future_a,
    int &literal_a, int &i, int &f) {
  D2("Parsing A,I, and F-values (opcode RHS) from: ", s);
  // default values
  a = -1;
  future_a = "";
  literal_a = -1;
  i = 0;
  f = -1;

  // Split into A, I, and F parts
  std::string ap = s, ip = "", fp = "";
  unsigned long i_end;
  if ((i_end = s.find('(')) != std::string::npos) {
    if (s.find(')' != s.size()-1)) {
      D2("Bad field in op address: ", s);
      throw Asm_error(-1);
    }
    fp = {s, i_end+1, s.size()-i_end-2};
    ap = {s, 0, i_end};
    s = {s, 0, i_end};
  }
  unsigned long a_end;
  if ((a_end = s.find(',')) != std::string::npos) {
    ip = {s, i_end+1, s.size()-i_end-2};
    ap = {s, 0, i_end};
  }

  // Vacuous case
  if (ap == "") {
    a = 0;
  }
  // Literal case
  if (ap[0] == '=') {
    if (ap.find('=', 1) != ap.size()-1) {
      D2("Bad literal in address part!", s);
      throw Asm_error(-1);
    }
    literal_a = parse_exp({ap, 1, ap.size()-2});
  } else {
    bool has_az = false;
    bool has_spec = false;
    for (char c : ap) {
      if (isAZ(c))
        has_az = true;
      if (!isAZ(c) && !is09(c))
        has_spec = true;
    }
    // entire A part is a single symbol
    if (has_az && !has_spec) {
      int val;
      if (lookup_symbol(ap, val) == 0)
        // already-defined single symol
        a = val;
      else
        // future case
        future_a = ap;
    } else {
      // Expression case
      a = parse_exp(ap);
    }
  }

  if (ip != "") {
    i = parse_exp(ip);
  }

  if (fp != "") {
    f = parse_exp(fp);
  }
}


// Assemble next row of input
// throw on error
void assemble_next(std::string s) {
  for (char c : s) {
    if (!(is09(c) || isAZ(c) ||
          c == ' ' || c == '*' || c == '/' ||
          c == '+' || c == '-' || c == ':' ||
          c == '=' || 
          c == '(' || c == ')' || c == ',')) {
      D2("Found invalid character: ", c);
      throw Asm_error(-1);
    }
  }
  // Omit blank lines and comments
  if (s.size() == 0 || s[0] == '*')
    return;
  unsigned i = 0;
  while (i < s.size() && s[i] != ' ') i++;
  if (i == s.size()) {
    D2("Instruction must contain an opcode, given: ", s);
    throw Asm_error(-1);
  }
  std::string loc {s, 0, i};
  D2("Loc is ", loc);
  while (i < s.size() && s[i] == ' ') i++;
  unsigned op_start = i;
  while (i < s.size() && s[i] != ' ') i++;
  if (i == s.size()) {
    D2("Instruction must contain an opcode, given: ", s);
    throw Asm_error(-1);
  }
  std::string op {s, op_start, i-op_start};
  D2("Opcode is ", op);
  std::string addr;
  // Special case -- ALF operator can ingest spaces
  if (op == "ALF") {
    i++;
    if (i < s.size() && s[i] == ' ') i++;
    if (i+5 < s.size()) {
      addr = {s, i, 5};
    } else {
      D2("ALF instruction: address too short: ", s);
      throw Asm_error(-1);
    }
  } else {
    while (i < s.size() && s[i] == ' ') i++;
    if (i == s.size()) {
      addr = "";
    } else {
      unsigned addr_start = i;
      while (i < s.size() && s[i] != ' ') i++;
      addr = {s, addr_start, i-addr_start};
    }
  }
  D2("Addr is ", addr);

  // At this point, we've tokenized the line
  // into loc (maybe empty), op, and addr (maybe empty).

  // Handle different cases for different operators.

  // Special operators
  int w = 0;
  if (op == "EQU" || op == "ORIG" ||
      op == "CON" || op == "END") {
    w = parse_w(addr);

    // EQU and ORIG handled at end
    if (op == "CON") {
      words[star++] = w;
    } else if (op == "END") {
      clean_futures();
      ended = true;
    }
  } else if (op == "ALF") {
    if (CHAR_TABLE.find(addr[0]) == CHAR_TABLE.end() ||
        CHAR_TABLE.find(addr[1]) == CHAR_TABLE.end() ||
        CHAR_TABLE.find(addr[2]) == CHAR_TABLE.end() ||
        CHAR_TABLE.find(addr[3]) == CHAR_TABLE.end() ||
        CHAR_TABLE.find(addr[4]) == CHAR_TABLE.end()) {
      D2("Unprintable characters passed to ALF:", addr);
      throw Asm_error(-1);
    }
    Word alfw {Sign::POS, {
      CHAR_TABLE[addr[0]],
      CHAR_TABLE[addr[1]],
      CHAR_TABLE[addr[2]],
      CHAR_TABLE[addr[3]],
      CHAR_TABLE[addr[4]]}};
    words[star++] = alfw;
  } else {
    // look up opcode
    if (OP_TABLE.find(op) == OP_TABLE.end()) {
      D2("Unknown opcode: ", op);
      throw Asm_error(-1);
    }
    auto op_p = OP_TABLE[op];
    int a;
    std::string future_a;
    int literal_a;
    int i;
    int f;
    parse_aif(addr, a, future_a, literal_a, i, f);
    // Literal case: create a new "fake symbol"
    // and handle same as future symbol
    if (literal_a != -1) {
      future_a = "*LIT" + std::to_string(next_literal++);
      literals.push_back(literal_a);
    }
    if (future_a != "") {
    }
    // After this point, a stores the value we want to assemble
    // (which is the special list pointer in future cases)

    // Use default value of f if not specified
    if (f == -1)
      f = op_p.second;
    int c = op_p.first;
    D6("Adding new word to assembled map: Star, A, I, F, C =",
        star, a, i, f, c);
    words[star++] = build_word(a, i, f, c);
  }

  // Finally, define the location
  // validate the LOC
  bool has_az = false;
  if (loc != "") {
    for (char c : loc) {
      if (!(is09(c) || isAZ(c))) {
        D2("Invalid character in symbol: ", c);
        throw Asm_error(-1);
      }
      has_az = (has_az || isAZ(c));
    }
    if (!has_az) {
      D2("Invalid symbol! ", loc);
      D("Symbol must contain at least one letter.");
      throw Asm_error(-1);
    }
    if (op != "EQU") {
      define_symbol(loc, star);
    }
  }
  // Handle special operator
  if (op == "EQU") {
    if (loc == "") {
      D("Invalid empty loc field for EQU operator");
      throw Asm_error(-1);
    }
    define_symbol(loc, w);
  }

  // Finally, ORIG takes effect after defining the location
  if (op == "ORIG") {
    star = w;
  }
}

void assemble_all(std::istream &in) {
  for (std::string s; getline(in, s); ) {
    assemble_next(s);
  }
  if (!ended) {
    D("Never encountered END instruction");
    throw Asm_error(-1);
  }
}

/*
 * Dump all assembled rows into an output file.
 */
void dump(std::string out_file) {
  std::ofstream fs {out_file};
  for (auto p : words) {
    fs.width(4);
    fs.fill('0');
    fs << p.first << ": " << p.second << std::endl;
  }
}

int main(int argc, char **argv) {
  if (argc < 3) {
    std::cout << "Usage: mixal <input.mixal> <output.mix>"
      << std::endl;
    return 2;
  }
  DBG_INIT();
  std::string in_file = argv[1];
  std::string out_file = argv[2];
  std::ifstream in {in_file};
  assemble_all(in);
  dump(out_file);
  DBG_CLOSE();
  return 0;
}
