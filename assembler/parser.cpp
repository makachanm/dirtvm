#include "parser.h"
#include <sstream>   // For std::stringstream
#include <algorithm> // For std::find_if
#include <cctype>    // For std::isspace
#include <stdexcept> // For std::out_of_range, std::invalid_argument
#include <iostream>  // For std::cerr
#include <vector>    // For std::vector
#include <string>    // For std::string
#include <cstdint>
#include <map>
#include <variant>

__uint128_t string_to_uint128(const std::string &s) {
    __uint128_t res = 0;
    size_t i = 0;
    bool is_hex = false;

    if (s.rfind("0x", 0) == 0) {
        is_hex = true;
        i = 2;
    }

    for (; i < s.length(); ++i) {
        char c = s[i];
        if (is_hex) {
            if (c >= '0' && c <= '9') {
                res = res * 16 + (c - '0');
            } else if (c >= 'a' && c <= 'f') {
                res = res * 16 + (c - 'a' + 10);
            } else if (c >= 'A' && c <= 'F') {
                res = res * 16 + (c - 'A' + 10);
            }
        } else {
            if (c >= '0' && c <= '9') {
                res = res * 10 + (c - '0');
            }
        }
    }
    return res;
}

// Opcodes map
const std::map<std::string, uint16_t> opcodes = {
    {"add", 0b0000010000000000},
    {"sub", 0b0000100000000000},
    {"mul", 0b0000110000000000},
    {"div", 0b0001000000000000},
    {"pop", 0b0001100000000000},
    {"dup", 0b0001110000000000},
    {"jmp", 0b0010000000000000},
    {"jz", 0b0010010000000000},
    {"jnz", 0b0010100000000000},
    {"call", 0b0010110000000000},
    {"ret", 0b0011000000000000},
    {"eq", 0b0011010000000000},
    {"lt", 0b0011100000000000},
    {"gt", 0b0011110000000000},
    {"gload", 0b0100000000000000},
    {"gstore", 0b0100010000000000},
    {"lload", 0b0100100000000000},
    {"lstore", 0b0100110000000000},
    {"pushd8", 0b0101000000000000},
    {"pushd16", 0b0101010000000000},
    {"pushd32", 0b0101100000000000},
    {"pushd64", 0b0101110000000000},
    {"pushd128", 0b0110000000000000},
    {"syscall", 0b0110010000000000},
};

std::string unescape_string(const std::string& s) {
    std::string res;
    for (size_t i = 0; i < s.length(); ++i) {
        if (s[i] == '\\' && i + 1 < s.length()) {
            switch (s[++i]) {
                case 'n': res += '\n'; break;
                case 't': res += '\t'; break;
                case 'r': res += '\r'; break;
                case '0': res += '\0'; break;
                case '\\': res += '\\'; break;
                case '\'': res += '\''; break;
                case '"': res += '"'; break;
                default: res += s[i]; break;
            }
        } else {
            res += s[i];
        }
    }
    return res;
}


// Helper function to trim whitespace from a string
static inline void trim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

// Helper to parse a character literal like 'a' or '\n'
// Returns true on success, false on failure. Stores result in 'out_char'.
bool parseCharLiteral(const std::string& s, uint8_t& out_char) {
    if (s.length() < 3 || s.front() != '\'' || s.back() != '\'') {
        return false; // Must start and end with single quote, and have at least one character content (e.g., 'x')
    }

    std::string content = s.substr(1, s.length() - 2);

    if (content.empty()) {
        return false; // Empty character literal like '' is invalid
    }

    if (content.length() == 1) {
        // Simple single character literal, e.g., 'a' or ''' (if content is just '\'')
        out_char = static_cast<uint8_t>(content[0]);
        return true;
    } else  {
        std::cout << "Escape" << std::endl;
        // Escape sequence
        char escape_char = content[1];
        switch (escape_char) {
            case 'n': out_char = '\n'; return true;
            case 't': out_char = '\t'; return true;
            case 'r': out_char = '\r'; return true;
            case '0': out_char = '\0'; return true;
            case '\\': out_char = '\\'; return true;
            case '\'': out_char = '\''; return true;
            case '"': out_char = '"'; return true;
            default: return false;
        }
    }

    return false; // Not a recognized character literal format
}


void Parser::split_token(std::string input) {
    trim(input);
    size_t comment_pos = input.find(';');
    if (comment_pos != std::string::npos) {
        input = input.substr(0, comment_pos);
        trim(input);
    }
    if (input.empty()) {
        return;
    }

    std::string current_token;
    bool in_string = false;
    for (size_t i = 0; i < input.length(); ++i) {
        char c = input[i];

        if (c == '"' && (i == 0 || input[i-1] != '\\')) {
            if (in_string) {
                // End of string
                current_token += c;
                tokens.push_back(current_token);
                current_token.clear();
                in_string = false;
            } else {
                // Start of string
                if (!current_token.empty()) {
                    tokens.push_back(current_token);
                    current_token.clear();
                }
                current_token += c;
                in_string = true;
            }
        } else if ((std::isspace(c) || c == ',') && !in_string) {
            if (!current_token.empty()) {
                tokens.push_back(current_token);
                current_token.clear();
            }
        } else {
            current_token += c;
        }
    }

    if (!current_token.empty()) {
        if (in_string) {
            std::cerr << "Error: Unterminated string literal." << std::endl;
            exit(1);
        }
        tokens.push_back(current_token);
    }
}

Parser::Parser() {}
Parser::~Parser() {}
std::map<std::string, __uint128_t> label_addresses;

void Parser::parse(std::string input_assembly_code) {
    this->tokens.clear();
    this->instructions.clear();

    std::stringstream ss(input_assembly_code);
    std::string line;
    while (std::getline(ss, line)) {
        split_token(line);
    }
    first_pass();
    token_to_data();
}

void Parser::token_to_data() {
    instructions.clear(); // 두 번째 패스를 위해 비웁니다.
    for (size_t i = 0; i < tokens.size(); ++i) {
        const std::string& token = tokens[i];
        if (token == ".string") {
            if (i + 2 >= tokens.size()) { std::cerr << "Error: .string requires an address and a string literal." << std::endl; exit(1); }
            
            uint16_t address = std::stoul(tokens[++i]);
            std::string str_literal = tokens[++i];

            if (str_literal.front() != '"' || str_literal.back() != '"') { std::cerr << "Error: Expected string literal for .string." << std::endl; exit(1); }
            
            Instruction instr;
            instr.type = InstructionType::STRING;
            String s;
            s.address = address;
            s.value = unescape_string(str_literal.substr(1, str_literal.length() - 2));
            instr.args = s;
            instructions.push_back(instr);
        } else if (token == "pushd8") {
            if (++i >= tokens.size()) { std::cerr << "Error: Expected 8-bit data after pushd8." << std::endl; exit(1); }
            uint8_t val;
            if (parseCharLiteral(tokens[i], val)) {
                instructions.push_back({InstructionType::PUSH8, Pushd8{val}});
            } else {
                instructions.push_back({InstructionType::PUSH8, Pushd8{(uint8_t)std::stoul(tokens[i], nullptr, 0)}});
            }
        } else if (token == "pushd16") {
            if (++i >= tokens.size()) { std::cerr << "Error: Expected 16-bit data after pushd16." << std::endl; exit(1); }
            instructions.push_back({InstructionType::PUSH16, Pushd16{(uint16_t)std::stoul(tokens[i], nullptr, 0)}});
        } else if (token == "pushd128") {
            if (++i >= tokens.size()) { std::cerr << "Error: Expected 128-bit data after pushd128." << std::endl; exit(1); }
            instructions.push_back({InstructionType::PUSH128, Pushd128{string_to_uint128(tokens[i])}});
        } else if (token == "pushd32") {
            if (++i >= tokens.size()) { std::cerr << "Error: Expected 32-bit data after pushd32." << std::endl; exit(1); }
            instructions.push_back({InstructionType::PUSH32, Pushd32{(uint32_t)std::stoul(tokens[i], nullptr, 0)}});
        } else if (token == "pushd64") {
            if (++i >= tokens.size()) { std::cerr << "Error: Expected 64-bit data after pushd64." << std::endl; exit(1); }
            instructions.push_back({InstructionType::PUSH64, Pushd64{(uint64_t)string_to_uint128(tokens[i])}});
        } else if (token == "jmp") { 
            if (++i >= tokens.size()) { std::cerr << "Error: Expected 128-bit data after jmp." << std::endl; exit(1); }
            if (label_addresses.count(tokens[i])) {
                instructions.push_back({InstructionType::JMP, Pushd128{label_addresses.at(tokens[i])}});
            } else {
                instructions.push_back({InstructionType::JMP, Pushd128{string_to_uint128(tokens[i])}});
            }
        } else if (token == "gstore") {
            instructions.push_back({InstructionType::GSTORE, Gstore{}});
        } else if (token == "syscall") {
            if (++i >= tokens.size()) { std::cerr << "Error: Expected syscall number after syscall." << std::endl; exit(1); }
            instructions.push_back({InstructionType::SYSCALL, Syscall{(uint8_t)std::stoul(tokens[i], nullptr, 0)}});
        } else if (token == "lload") {
            if (++i >= tokens.size()) { std::cerr << "Error: Expected 10-bit tag after lload." << std::endl; exit(1); }
            instructions.push_back({InstructionType::LLOAD, Lload{(uint16_t)std::stoul(tokens[i], nullptr, 0)}});
        } else if (token == "lstore") {
            if (++i >= tokens.size()) { std::cerr << "Error: Expected 10-bit tag after lstore." << std::endl; exit(1); }
            instructions.push_back({InstructionType::LSTORE, Lstore{(uint16_t)std::stoul(tokens[i], nullptr, 0)}});
        } else if (token == "call" || token == "jz" || token == "jnz") {
            if (++i >= tokens.size()) { std::cerr << "Error: Expected address after " << token << std::endl; exit(1); }
            __uint128_t address;
            if (label_addresses.count(tokens[i])) {
                address = label_addresses.at(tokens[i]);
            } else {
                address = string_to_uint128(tokens[i]);
            }
            if (token == "call") instructions.push_back({InstructionType::CALL, Pushd128{address}});
            else if (token == "jz") instructions.push_back({InstructionType::JZ, Pushd128{address}});
            else if (token == "jnz") instructions.push_back({InstructionType::JNZ, Pushd128{address}});
        } else {
             if (opcodes.count(token)) {
                Instruction instr;
                instr.type = InstructionType::OPCODE;
                instr.args = Opcode{opcodes.at(token)};
                instructions.push_back(instr);
             } else if (token.back() == ':') {
                // 라벨 정의는 두 번째 패스에서 무시합니다.
                continue;
             } else {
                std::cerr << "Unknown token: " << token << std::endl;
                exit(1);
            }
        }
    }
}

void Parser::first_pass() {
    label_addresses.clear();
    __uint128_t current_address = 0;

    for (size_t i = 0; i < tokens.size(); ++i) {
        const std::string& token = tokens[i];

        if (token.back() == ':') {
            std::string label = token.substr(0, token.length() - 1);
            label_addresses[label] = current_address;
        } else if (token == ".string") {
            if (i + 2 >= tokens.size()) { std::cerr << "Error: .string requires an address and a string literal." << std::endl; exit(1); }
            std::string str_literal = tokens[i+2];
            std::string unescaped = unescape_string(str_literal.substr(1, str_literal.length() - 2));
            current_address += unescaped.length() * 5; // pushd8(2) + pushd16(2) + gstore(1) = 5 words per char
            i += 2;
        } else if (token == "pushd8") {
            current_address += 2; // Instruction word + data word
            i += 1;
        } else if (token == "pushd16") {
            current_address += 2;
            i += 1;
        } else if (token == "pushd32") {
            current_address += 3;
            i += 1;
        } else if (token == "pushd64") {
            current_address += 5;
            i += 1;
        } else if (token == "pushd128") {
            current_address += 9;
            i += 1;
        } else if (token == "jmp" || token == "call" || token == "jz" || token == "jnz") {
            current_address += 9; // opcode + 8-word address
            i += 1;
        } else if (token == "syscall") {
            current_address += 1;
            i += 1;
        } else if (token == "lload" || token == "lstore") {
            current_address += 1;
            i += 1;
        } else if (opcodes.count(token)) {
            current_address += 1;
        } else {
            // Unknown token or already handled (like label value)
        }
    }
}


std::vector<uint16_t> Parser::get_bytecode() {
    std::vector<uint16_t> bytecode;
    for (const auto& instr : instructions) {
        switch (instr.type) {
            case InstructionType::PUSH8:
                bytecode.push_back(opcodes.at("pushd8"));
                bytecode.push_back(std::get<Pushd8>(instr.args).value);
                break;
            case InstructionType::PUSH16:
                bytecode.push_back(opcodes.at("pushd16"));
                bytecode.push_back(std::get<Pushd16>(instr.args).value);
                break;
            case InstructionType::PUSH32:
                {
                    bytecode.push_back(opcodes.at("pushd32"));
                    uint32_t value = std::get<Pushd32>(instr.args).value;
                    bytecode.push_back(value & 0xFFFF);
                    bytecode.push_back((value >> 16) & 0xFFFF);
                }
                break;
            case InstructionType::PUSH64:
                {
                    bytecode.push_back(opcodes.at("pushd64"));
                    uint64_t value = std::get<Pushd64>(instr.args).value;
                    bytecode.push_back(value & 0xFFFF);
                    bytecode.push_back((value >> 16) & 0xFFFF);
                    bytecode.push_back((value >> 32) & 0xFFFF);
                    bytecode.push_back((value >> 48) & 0xFFFF);
                }
                break;
            case InstructionType::PUSH128:
                {
                    bytecode.push_back(opcodes.at("pushd128"));
                    __uint128_t value = std::get<Pushd128>(instr.args).value;
                    uint64_t low = (uint64_t)value;
                    uint64_t high = (uint64_t)(value >> 64);
                    bytecode.push_back(low & 0xFFFF);
                    bytecode.push_back((low >> 16) & 0xFFFF);
                    bytecode.push_back((low >> 32) & 0xFFFF);
                    bytecode.push_back((low >> 48) & 0xFFFF);
                    bytecode.push_back(high & 0xFFFF);
                    bytecode.push_back((high >> 16) & 0xFFFF);
                    bytecode.push_back((high >> 32) & 0xFFFF);
                    bytecode.push_back((high >> 48) & 0xFFFF);
                }
                break;
            case InstructionType::CALL:
            case InstructionType::JZ:
            case InstructionType::JNZ:
                {
                    if (instr.type == InstructionType::CALL) bytecode.push_back(opcodes.at("call"));
                    else if (instr.type == InstructionType::JZ) bytecode.push_back(opcodes.at("jz"));
                    else if (instr.type == InstructionType::JNZ) bytecode.push_back(opcodes.at("jnz"));

                    __uint128_t value = std::get<Pushd128>(instr.args).value;
                    uint64_t low = (uint64_t)value;
                    uint64_t high = (uint64_t)(value >> 64);
                    bytecode.push_back(low & 0xFFFF);
                    bytecode.push_back((low >> 16) & 0xFFFF);
                    bytecode.push_back((low >> 32) & 0xFFFF);
                    bytecode.push_back((low >> 48) & 0xFFFF);
                    bytecode.push_back(high & 0xFFFF);
                    bytecode.push_back((high >> 16) & 0xFFFF);
                    bytecode.push_back((high >> 32) & 0xFFFF);
                    bytecode.push_back((high >> 48) & 0xFFFF);
                }
                break;
            case InstructionType::JMP:
                {
                bytecode.push_back(opcodes.at("jmp"));
                __uint128_t value = std::get<Pushd128>(instr.args).value;
                uint64_t jlow = (uint64_t)value;
                uint64_t jhigh = (uint64_t)(value >> 64);
                bytecode.push_back(jlow & 0xFFFF);
                bytecode.push_back((jlow >> 16) & 0xFFFF);
                bytecode.push_back((jlow >> 32) & 0xFFFF);
                bytecode.push_back((jlow >> 48) & 0xFFFF);
                bytecode.push_back(jhigh & 0xFFFF);
                bytecode.push_back((jhigh >> 16) & 0xFFFF);
                bytecode.push_back((jhigh >> 32) & 0xFFFF);
                bytecode.push_back((jhigh >> 48) & 0xFFFF);
                }
                break;
            case InstructionType::GSTORE:
                bytecode.push_back(opcodes.at("gstore"));
                break;
            case InstructionType::SYSCALL:
                // syscall opcode와 syscall 번호를 하나의 16비트 명령어로 합칩니다.
                // Opcode: 6 bits, Syscall number: 10 bits
                bytecode.push_back(opcodes.at("syscall") | (std::get<Syscall>(instr.args).value & 0x03FF));
                break;
            case InstructionType::LLOAD:
                {
                    uint16_t tag = std::get<Lload>(instr.args).tag;
                    bytecode.push_back(opcodes.at("lload") | (tag & 0x3FF));
                }
                break;
            case InstructionType::LSTORE:
                {
                    uint16_t tag = std::get<Lstore>(instr.args).tag;
                    bytecode.push_back(opcodes.at("lstore") | (tag & 0x3FF));
                }
                break;
            case InstructionType::STRING:
                {
                    const auto& s = std::get<String>(instr.args);
                    uint16_t current_address = s.address;
                    for (char c : s.value) {
                        // pushd8 'c'
                        bytecode.push_back(opcodes.at("pushd8"));
                        bytecode.push_back(static_cast<uint8_t>(c));
                        // pushd16 address
                        bytecode.push_back(opcodes.at("pushd16"));
                        bytecode.push_back(current_address++);
                        // gstore
                        bytecode.push_back(opcodes.at("gstore"));
                    }
                }
                break;
            case InstructionType::OPCODE:
                bytecode.push_back(std::get<Opcode>(instr.args).code);
                break;
        }
    }
    return bytecode;
}
