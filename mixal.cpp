#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include "dbg.h"
#include "core.h"

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
std::vector<int> locals = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
// "Daisy chain instructions" (key -> linked list) that need to
// be substituted once the future global/local is defined.
// The list next pointer is sneakily stored in the address field of each
// assembled instruction to make it easy to traverse and
// rewrite as needed.
// -1 = end of list marker
std::map<std::string, int> fglobals = {};
std::vector<int> flocals = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

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
// Return 0 on success, -1 on failure (invalid local symbol)
int get_local(std::string sym, bool loc_context, int &i, bool &is_future) {
  if (sym.size() == 2 && is09(sym[0])) {
    if (sym[1] == 'H') {
      if (loc_context) {
        i = ctoi(sym[0]);
        is_future = false;
        return 0;
      } else {
        D2("Invalid appearance of local H symbol in addr context! ", sym);
        return -1;
      }
    } else if (sym[1] == 'F' || sym[1] == 'B') {
      if (!loc_context) {
        i = ctoi(sym[0]);
        is_future = (sym[1] == 'F');
        return 0;
      } else {
        D2("Invalid appearance of local BF symbol in loc context! ", sym);
        return -1;
      }
    }
  }
  i = -1;
  is_future = false;
  return 0;
}

Word build_word(int a, int i, int f, int c) {
  Word out_a = a;
  Word w {out_a.sgn(), {out_a.b(4), out_a.b(5),
    (Byte) i, (Byte) f, (Byte) c}};
  D2("Assembled word: ", w);
  return w;
}

// Add symbol to global/local symbol tables
// return 0 on success, -1 on failure
// NOTE that this also corrects any daisy chained future symbols
int define_symbol(std::string sym, int val) {

  D4("Adding symbol definition: ", sym, " = ", val);
  // Will be set to the most recent appearance if there
  // are earlier appearances of this symbol (before definition)
  int future_chain = -1;

  int ix;
  bool is_future;
  if (get_local(sym, true, ix, is_future) < 0)
    return -1;

  if (ix >= 0) { // Local
    locals[ctoi(sym[0])] = val;

    future_chain = flocals[ctoi(sym[0])];
    flocals[ctoi(sym[0])] = -1;
  } else { // Global
    if (globals.find(sym) != globals.end()) {
      D4("Error: global symbol already defined! ",
          sym, " = ", globals[sym]);
      return -1;
    }
    globals[sym] = val;
    if (fglobals.find(sym) != fglobals.end()) {
       future_chain = fglobals[sym];
    }
    fglobals.erase(sym);
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
  return 0;
}

// Find symbol in global/local symbol tables
// Return value in &val
// Return 0 on success, -1 on failure
int lookup_symbol(std::string sym, int &val) {
  D2("Looking up symbol: ", sym);
  int ix;
  bool is_future;
  if (get_local(sym, false, ix, is_future) < 0)
    return -1;

  // Cannot lookup future locals yet
  if (is_future)
    return -1;

  if (ix >= 0) { // Local
    return locals[ctoi(sym[0])];
  } else { // Global
    if (globals.find(sym) == globals.end())
      return -1;
    else
      return globals[sym];
  }
  return 0;
}

// Add a future symbol daisy chain entry
// Return the value that should be stored in addr for this row
// that can be used for daisy chaining in &a
// Return 0 on success, -1 on failure
int add_future(std::string sym, int &a) {
  D4("Adding future symbol reference: ", sym, " at ", star);
  int ix;
  bool is_future;
  if (get_local(sym, false, ix, is_future) < 0)
    return -1;

  if (ix >= 0) { // Local
    if (!is_future) {
      D2("Local past symbol has no existing definition!", sym);
      return -1;
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
  return 0;
}

// Clean futures by adding additional rows
// (starting at *) that define the appropriate symbols
// called at END
int clean_futures() {
  D("Cleaning up remaining future symbols...");
  for (int i : flocals) {
    if (i != -1) {
      D3("Error! Undefined future local reference at", i, flocals[i]);
      return -1;
    }
  }
  for (auto p : fglobals) {
    // Fake instruction:
    // <SYM> CON 0
    if (define_symbol(p.first, star) < 0)
      return -1;
    words[star++] = 0;
  }
  return 0;
}


// Parse "expression" (see Knuth)
// Return 0 on success, -1 on error
// output is stored in e
// Note that expressions must have fully determined numerical values!
int parse_exp(std::string s, int &e) {
  // TODO
  return 0;
}

// Parse "W-value" (see Knuth)
// Return 0 on success, -1 on error
int parse_w(std::string s, int &w) {
  // TODO
  return 0;
}

// Parse "A-part", "I-part", and "F-part" (see Knuth)
// A-part is returned in:
//   a if it's an expression
//   future_a if it's a future value
//   literal_a if it's a literal value
// Any unused cases are set to -1/empty string
// Return 0 on success, -1 on error
int parse_aif(std::string s, int &a, std::string &future_a,
    int &literal_a, int &i, int &f) {
  // TODO
  return 0;
}


// Assemble next row of input
int assemble_next(std::string s) {
  for (char c : s) {
    if (!(is09(c) || isAZ(c) ||
          c == ' ' || c == '*' || c == '/' ||
          c == '+' || c == '-' || c == ':' ||
          c == '=' || 
          c == '(' || c == ')' || c == ',')) {
      D2("Found invalid character: ", c);
      return -1;
    }
  }
  // Omit blank lines and comments
  if (s.size() == 0 || s[0] == '*')
    return 0;
  unsigned i = 0;
  while (i < s.size() && s[i] != ' ') i++;
  if (i == s.size()) {
    D2("Instruction must contain an opcode, given: ", s);
    return -1;
  }
  std::string loc {s, 0, i};
  while (i < s.size() && s[i] == ' ') i++;
  unsigned op_start = i;
  while (i < s.size() && s[i] != ' ') i++;
  if (i == s.size()) {
    D2("Instruction must contain an opcode, given: ", s);
    return -1;
  }
  std::string op {s, op_start, i};
  std::string addr;
  // Special case -- ALF operator can ingest spaces
  if (op == "ALF") {
    i++;
    if (i < s.size() && s[i] == ' ') i++;
    if (i+5 < s.size()) {
      addr = {s, i, i+5};
    } else {
      D2("ALF instruction: address too short: ", s);
      return -1;
    }
  } else {
    while (i < s.size() && s[i] == ' ') i++;
    if (i == s.size()) {
      addr = "";
    } else {
      unsigned addr_start = i;
      while (i < s.size() && s[i] != ' ') i++;
      addr = {s, addr_start, i};
    }
  }

  // At this point, we've tokenized the line
  // into loc (maybe empty), op, and addr (maybe empty).

  // Handle different cases for different operators.

  // Special operators
  int w = 0;
  if (op == "EQU" || op == "ORIG" ||
      op == "CON" || op == "END") {
    if (parse_w(addr, w) < 0) {
      D("Error parsing W value");
      return -1;
    }

    // EQU and ORIG handled at end
    if (op == "CON") {
      words[star++] = w;
    } else if (op == "END") {
      if (clean_futures() < 0)
        return -1;
      ended = true;
    }
    return 0;
  } else if (op == "ALF") {
    if (CHAR_TABLE.find(addr[0]) == CHAR_TABLE.end() ||
        CHAR_TABLE.find(addr[1]) == CHAR_TABLE.end() ||
        CHAR_TABLE.find(addr[2]) == CHAR_TABLE.end() ||
        CHAR_TABLE.find(addr[3]) == CHAR_TABLE.end() ||
        CHAR_TABLE.find(addr[4]) == CHAR_TABLE.end()) {
      D2("Unprintable characters passed to ALF:", addr);
      return -1;
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
      return -1;
    }
    auto op_p = OP_TABLE[op];
    int a;
    std::string future_a;
    int literal_a;
    int i;
    int f;
    if (parse_aif(addr, a, future_a, literal_a, i, f) < 0) {
      D("Error parsing A,I,F values");
      return -1;
    }
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
        return -1;
      }
      has_az = (has_az || isAZ(c));
    }
    if (!has_az) {
      D2("Invalid symbol! ", loc);
      D("Symbol must contain at least one letter.");
      return -1;
    }
    if (op != "EQU") {
      if (define_symbol(loc, star) < 0)
        return -1;
    }
  }
  // Handle special operator
  if (op == "EQU") {
    if (loc == "") {
      D("Invalid empty loc field for EQU operator");
      return -1;
    }
    if (define_symbol(loc, w) < 0)
      return -1;
  }

  // Finally, ORIG takes effect after defining the location
  if (op == "ORIG") {
    star = w;
  }
  return 0;
}

int assemble_all(std::istream &in) {
  for (std::string s; getline(in, s); ) {
    int ret;
    if ((ret = assemble_next(s)) < 0)
      return ret;
  }
  if (!ended) {
    D("Never encountered END instruction");
    return -1;
  }
  return 0;
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
  if (assemble_all(in) < 0) {
    D("Assembly failed!");
    return 1;
  }
  dump(out_file);
  DBG_CLOSE();
  return 0;
}
