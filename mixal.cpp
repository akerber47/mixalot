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

int star = 0;
bool ended = false;
std::map<int, Word> words = {};
std::map<std::string, int> globals = {};
std::map<std::string, int> fglobals = {};
std::vector<int> locals = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
std::vector<int> flocals = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// Assemble next row of input
int assemble_next(std::string s) {
  std::getline(in, s);
  for (char c : s) {
    if (!((c >= 'A' && c <= 'Z') ||
          (c >= '0' && c <= '9') ||
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
  int i = 0;
  while (i < s.size() && s[i] != ' ') i++;
  if (i == s.size()) {
    D2("Instruction must contain an opcode, given: " s);
    return -1;
  }
  std::string loc {s, 0, i};
  while (i < s.size() && s[i] == ' ') i++;
  int op_start = i;
  while (i < s.size() && s[i] != ' ') i++;
  if (i == s.size()) {
    D2("Instruction must contain an opcode, given: " s);
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
      int addr_start = i;
      while (i < s.size() && s[i] != ' ') i++;
      addr = {s, addr_start, i};
    }
  }

  // At this point, we've tokenized the line
  // into loc (maybe empty), op, and addr (maybe empty).
  // Handle different cases for different operators.

  // Special operators
  if (op == "EQU" || op == "ORIG" ||
      op == "CON" || op == "END") {
    int w;
    if (parse_w(addr, w) < 0) {
      D("Error parsing W value");
      return -1;
    }
    // TODO handle special opcode with w
  } else if (op == "ALF") {
    // TODO handle ALF
  } else {
    int a;
    std::string future_a;
    int i;
    int f;
    if (parse_a(addr, a, future_a, i, f) < 0) {
      D("Error parsing A,I,F values");
      return -1;
    }
    // TODO handle general opcode
  }
}

int assemble_all(std::istream in) {
  for (std::string s; getline(in, s); ) {
    if ((int ret = assemble_next(s)) < 0)
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
void dump(std::string out_file);

int main(int argc, char **argv) {
  if (argc < 3) {
    std::cout << "Usage: mixal <input.mixal> <output.mix>" << std::endl;
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
