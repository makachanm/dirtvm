#include <string>
#include <vector>
#include <iostream>
#include <cstdint>
#include <variant>

enum class InstructionType {
    PUSH8,
    PUSH16,
    PUSH32,
    PUSH64,
    PUSH128,
    LLOAD,
    LSTORE,
    GSTORE,
    SYSCALL,
    JMP,
    JZ,
    JNZ,
    CALL,
    STRING,
    OPCODE,
};

struct Pushd8 {
    uint8_t value;
};

struct Pushd16 {
    uint16_t value;
};

struct Pushd32 {
    uint32_t value;
};

struct Pushd64 {
    uint64_t value;
};

struct Pushd128 {
    __uint128_t value;
};

struct Jmp {
    __uint128_t address;
};

struct Opcode {
    uint16_t code;
};

struct Gstore {};

struct Syscall {
    uint16_t value;
};

struct String {
    uint16_t address;
    std::string value;
};

struct Lload {
    uint16_t tag;
};

struct Lstore {
    uint16_t tag;
};


struct Instruction {
    InstructionType type;
    std::variant<Pushd8, Pushd16, Pushd32, Pushd64, Pushd128, Lload, Lstore, Gstore, Syscall, String, Opcode> args;
};

class Parser {
private:
    std::vector<std::string> tokens;
    std::vector<uint16_t> bytecode;
    std::vector<Instruction> instructions;

    void split_token(std::string input);
    void token_to_data();
    void first_pass();
    std::vector<uint16_t> token_to_data(std::string input, size_t size);

public:
    Parser();
    ~Parser();

    void parse(std::string input);
    std::vector<uint16_t> get_bytecode();
};