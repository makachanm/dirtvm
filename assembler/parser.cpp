#include "parser.h"
#include <sstream>   // For std::stringstream
#include <algorithm> // For std::find_if
#include <cctype>    // For std::isspace
#include <stdexcept> // For std::out_of_range, std::invalid_argument
#include <iostream>  // For std::cerr
#include <vector>    // For std::vector
#include <string>    // For std::string
#include <cstdint>   // For uint16_t, uint64_t etc.

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
    trim(input); // Trim leading/trailing whitespace

    if (input.empty()) {
        return; // Skip empty lines
    }

    // Handle comments: assume ';' denotes a comment until end of line
    size_t comment_pos = input.find(';');
    if (comment_pos != std::string::npos) {
        input = input.substr(0, comment_pos); // Truncate at comment
        trim(input); // Trim again in case comment was preceded by spaces
    }

    if (input.empty()) {
        return; // Skip lines that were only comments or whitespace
    }

    std::stringstream ss(input);
    std::string token_str;
    while (ss >> token_str) {
        this->tokens.push_back(token_str);
    }
}

// Add stubs for other Parser methods to ensure the file compiles,
// assuming they will be implemented later or are not critical for this specific task.
Parser::Parser() {}
Parser::~Parser() {}

void Parser::parse(std::string input_assembly_code) {
    // Clear previous tokens before parsing new input
    this->tokens.clear();

    std::stringstream ss(input_assembly_code);
    std::string line;
    while (std::getline(ss, line)) {
        split_token(line);
    }
}

std::vector<uint16_t> Parser::get_bytecode() {
    std::vector<uint16_t> bytecode;
    // Iterate through tokens using an index to allow consuming multiple tokens (opcode + data)
    for (size_t i = 0; i < tokens.size(); ++i) {
        const std::string& token = tokens[i];

        if (token == "add") {
            bytecode.push_back(0b000001);
        } else if (token == "sub") {
            bytecode.push_back(0b000010);
        } else if (token == "mul") {
            bytecode.push_back(0b000011);
        } else if (token == "div") {
            bytecode.push_back(0b000100);
        } else if (token == "pop") {
            bytecode.push_back(0b000110);
        } else if (token == "dup") {
            bytecode.push_back(0b000111);
        } else if (token == "jmp") {
            bytecode.push_back(0b001000);
            if (++i >= tokens.size()) { // Move to next token (data)
                std::cerr << "Error: Expected 128-bit data after pushd128." << std::endl;
                exit(1);
            }
            std::vector<uint16_t> data_parts = token_to_data(tokens[i], 128);
            bytecode.insert(bytecode.end(), data_parts.begin(), data_parts.end());
        } else if (token == "jz") {
            bytecode.push_back(0b001001);
        } else if (token == "jnz") {
            bytecode.push_back(0b001010);
        } else if (token == "call") {
            bytecode.push_back(0b001011);
        } else if (token == "ret") {
            bytecode.push_back(0b001100);
        } else if (token == "eq") {
            bytecode.push_back(0b001101);
        } else if (token == "lt") {
            bytecode.push_back(0b001110);
        } else if (token == "gt") {
            bytecode.push_back(0b001111);
        } else if (token == "gload") {
            bytecode.push_back(0b010000);
        } else if (token == "gstore") {
            bytecode.push_back(0b010001);
        } else if (token == "lload") {
            bytecode.push_back(0b010010);
        } else if (token == "lstore") {
            bytecode.push_back(0b010011);
        } else if (token == "pushd8") {
            bytecode.push_back(0b010100);
            if (++i >= tokens.size()) { // Move to next token (data)
                std::cerr << "Error: Expected 8-bit data after pushd8." << std::endl;
                exit(1);
            }
            std::vector<uint16_t> data_parts = token_to_data(tokens[i], 8);
            bytecode.insert(bytecode.end(), data_parts.begin(), data_parts.end());
        } else if (token == "pushd16") {
            bytecode.push_back(0b010101);
            if (++i >= tokens.size()) { // Move to next token (data)
                std::cerr << "Error: Expected 16-bit data after pushd16." << std::endl;
                exit(1);
            }
            std::vector<uint16_t> data_parts = token_to_data(tokens[i], 16);
            bytecode.insert(bytecode.end(), data_parts.begin(), data_parts.end());
        } else if (token == "pushd32") {
            bytecode.push_back(0b010110);
            if (++i >= tokens.size()) { // Move to next token (data)
                std::cerr << "Error: Expected 32-bit data after pushd32." << std::endl;
                exit(1);
            }
            std::vector<uint16_t> data_parts = token_to_data(tokens[i], 32);
            bytecode.insert(bytecode.end(), data_parts.begin(), data_parts.end());
        } else if (token == "pushd64") {
            bytecode.push_back(0b010111);
            if (++i >= tokens.size()) { // Move to next token (data)
                std::cerr << "Error: Expected 64-bit data after pushd64." << std::endl;
                exit(1);
            }
            std::vector<uint16_t> data_parts = token_to_data(tokens[i], 64);
            bytecode.insert(bytecode.end(), data_parts.begin(), data_parts.end());
        } else if (token == "pushd128") {
            bytecode.push_back(0b011000);
            if (++i >= tokens.size()) { // Move to next token (data)
                std::cerr << "Error: Expected 128-bit data after pushd128." << std::endl;
                exit(1);
            }
            std::vector<uint16_t> data_parts = token_to_data(tokens[i], 128);
            bytecode.insert(bytecode.end(), data_parts.begin(), data_parts.end());
        } else if (token == "syscall") {
            bytecode.push_back(0b011001);
        } else {
            // If it's not a recognized opcode, it's an error.
            std::cerr << "Unknown token: " << token << std::endl;
            exit(1);
        }
    }

    return bytecode;
}

// Function to parse a 128-bit hexadecimal string into two uint64_t
// Assumes input string is like "0x..." or just "..." (hex).
// Returns true on success, false on failure. Stores result in high and low.
bool parseUInt128Hex(const std::string& s, uint64_t& high, uint64_t& low) {
    std::string hex_str = s;
    if (hex_str.rfind("0x", 0) == 0 || hex_str.rfind("0X", 0) == 0) {
        hex_str = hex_str.substr(2); // Remove "0x" prefix
    }

    if (hex_str.empty() || hex_str.length() > 32) { // Max 32 hex chars for 128 bits
        return false;
    }

    // Pad with leading zeros if necessary
    if (hex_str.length() < 32) {
        hex_str = std::string(32 - hex_str.length(), '0') + hex_str;
    }

    try {
        high = std::stoull(hex_str.substr(0, 16), nullptr, 16); // First 16 hex chars for high 64 bits
        low = std::stoull(hex_str.substr(16, 16), nullptr, 16); // Next 16 hex chars for low 64 bits
    } catch (const std::invalid_argument& e) {
        return false;
    } catch (const std::out_of_range& e) {
        return false; // Should not happen with 16 hex chars
    }

    return true;
}


std::vector<uint16_t> Parser::token_to_data(std::string input, size_t size) {
    std::vector<uint16_t> result;
    uint8_t char_val;

    // Try to parse as a character literal first, especially if size is 8
    if (size == 8 && parseCharLiteral(input, char_val)) {
        result.push_back(static_cast<uint16_t>(char_val));
        return result;
    }

    // Handle 128-bit integers as a special case before stoull
    if (size == 128) {
        uint64_t high_64 = 0, low_64 = 0;
        if (parseUInt128Hex(input, high_64, low_64)) {
            // Append low_64 first (little-endian for uint16_t parts)
            result.push_back(static_cast<uint16_t>(low_64 & 0xFFFF));
            result.push_back(static_cast<uint16_t>((low_64 >> 16) & 0xFFFF));
            result.push_back(static_cast<uint16_t>((low_64 >> 32) & 0xFFFF));
            result.push_back(static_cast<uint16_t>((low_64 >> 48) & 0xFFFF));
            // Then high_64
            result.push_back(static_cast<uint16_t>(high_64 & 0xFFFF));
            result.push_back(static_cast<uint16_t>((high_64 >> 16) & 0xFFFF));
            result.push_back(static_cast<uint16_t>((high_64 >> 32) & 0xFFFF));
            result.push_back(static_cast<uint16_t>((high_64 >> 48) & 0xFFFF));
            return result;
        } else {
            // Fallback for 128-bit if not a valid hex, try stoull but expect failure
            // This caters to decimal 128-bit numbers that are too big for unsigned long long
            // and where `parseUInt128Hex` fails.
            std::cerr << "Invalid or unsupported 128-bit numeric format (only hex supported for 128-bit literals currently): " << input << std::endl;
            exit(1);
        }
    }


    unsigned long long value;
    try {
        value = std::stoull(input, nullptr, 0); // Base 0 to auto-detect decimal/hex
    } catch (const std::invalid_argument& e) {
        std::cerr << "Invalid numeric format for data: " << input << std::endl;
        exit(1);
    } catch (const std::out_of_range& e) {
        // This should primarily catch decimal numbers larger than ULLONG_MAX,
        // as hex numbers up to 64-bit should be handled.
        std::cerr << "Numeric data out of range for unsigned long long (for sizes < 128): " << input << std::endl;
        exit(1);
    }

    // Determine how to store the value based on the size
    switch (size) {
        case 8:
            if (value > 0xFF) {
                std::cerr << "Value " << input << " exceeds 8-bit capacity." << std::endl;
                exit(1);
            }
            result.push_back(static_cast<uint16_t>(value & 0xFF));
            break;
        case 16:
            if (value > 0xFFFF) {
                std::cerr << "Value " << input << " exceeds 16-bit capacity." << std::endl;
                exit(1);
            }
            result.push_back(static_cast<uint16_t>(value & 0xFFFF));
            break;
        case 32:
            if (value > 0xFFFFFFFFULL) { // Use ULL suffix for comparison with unsigned long long
                std::cerr << "Value " << input << " exceeds 32-bit capacity." << std::endl;
                exit(1);
            }
            result.push_back(static_cast<uint16_t>(value & 0xFFFF));         // Lower 16 bits
            result.push_back(static_cast<uint16_t>((value >> 16) & 0xFFFF)); // Upper 16 bits
            break;
        case 64:
            // No explicit overflow check needed here as value is already unsigned long long
            // and we are extracting all 64 bits.
            result.push_back(static_cast<uint16_t>(value & 0xFFFF));
            result.push_back(static_cast<uint16_t>((value >> 16) & 0xFFFF));
            result.push_back(static_cast<uint16_t>((value >> 32) & 0xFFFF));
            result.push_back(static_cast<uint16_t>((value >> 48) & 0xFFFF));
            break;
        default:
            std::cerr << "Unsupported data size in token_to_data: " << size << std::endl;
            exit(1);
    }

    return result;
}