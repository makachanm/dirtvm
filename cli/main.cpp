#include <iostream>
#include <fstream>
#include <vector>
#include <iterator>
#include <string>
#include <map>

#include "../assembler/parser.h"
#include "../engine/vm.h"

enum class CliMode {
    NONE,
    ASSEMBLE,
    RUN,
    ASSEMBLE_AND_RUN
};

void print_help() {
    std::cout << "Usage: dirtvm_cli [options] <input_file>" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -a, --assemble       Assemble the input assembly file and output bytecode to <output_file> (default: a.out)" << std::endl;
    std::cout << "  -r, --run            Run the input bytecode file" << std::endl;
    std::cout << "  -ar, --assemble-run  Assemble and immediately run the input assembly file" << std::endl;
    std::cout << "  -o <file>            Specify output file for assembly (used with -a)" << std::endl;
    std::cout << "  -h, --help           Display this help message" << std::endl;
}

// 파일에서 어셈블리 코드를 읽어옵니다.
std::string read_source_file(const std::string& filename) {
    std::ifstream ifs(filename);
    if (!ifs.is_open()) {
        std::cerr << "Error: Could not open input file " << filename << std::endl;
        exit(1);
    }
    return std::string((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
}

// 바이트코드 파일에서 바이트코드를 읽어옵니다.
std::vector<uint16_t> read_bytecode_file(const std::string& filename) {
    std::ifstream ifs(filename, std::ios::binary);
    if (!ifs.is_open()) {
        std::cerr << "Error: Could not open input bytecode file " << filename << std::endl;
        exit(1);
    }
    std::vector<uint16_t> bytecode;
    uint16_t instruction;
    while (ifs.read(reinterpret_cast<char*>(&instruction), sizeof(instruction))) {
        bytecode.push_back(instruction);
    }
    return bytecode;
}

// 어셈블리 코드를 바이트코드로 변환합니다.
std::vector<uint16_t> assemble(const std::string& assembly_code) {
    Parser parser;
    parser.parse(assembly_code);
    return parser.get_bytecode();
}

// 바이트코드를 파일에 씁니다.
void write_bytecode(const std::string& filename, const std::vector<uint16_t>& bytecode) {
    std::ofstream ofs(filename, std::ios::binary);
    if (!ofs.is_open()) {
        std::cerr << "Error: Could not open output file " << filename << std::endl;
        exit(1);
    }
    for (uint16_t instruction : bytecode) {
        ofs.write(reinterpret_cast<const char*>(&instruction), sizeof(instruction));
    }
    std::cout << "Assembly successful. Bytecode written to " << filename << std::endl;
}

// VM을 실행합니다.
void run_vm(const std::vector<uint16_t>& bytecode) {
    vm dirt_vm(bytecode);
    dirt_vm.run();
    std::cout << "Execution finished." << std::endl;
}

int main(int argc, char* argv[]) {
    // C++ 스트림과 C 표준 스트림의 동기화를 비활성화하여 입출력 성능을 향상시킵니다.
    std::ios_base::sync_with_stdio(false);

    CliMode mode = CliMode::NONE;
    std::string input_file;
    std::string output_file = "a.out"; // Default output file for assembly

    // Parse command-line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            print_help();
            return 0;
        } else if (arg == "-a" || arg == "--assemble") {
            mode = CliMode::ASSEMBLE;
        } else if (arg == "-r" || arg == "--run") {
            mode = CliMode::RUN;
        } else if (arg == "-ar" || arg == "--assemble-run") {
            mode = CliMode::ASSEMBLE_AND_RUN;
        } else if (arg == "-o") {
            if (i + 1 < argc) {
                output_file = argv[++i];
            } else {
                std::cerr << "Error: -o option requires an argument." << std::endl;
                return 1;
            }
        } else {
            // Assume it's the input file
            if (input_file.empty()) {
                input_file = arg;
            } else {
                std::cerr << "Error: Multiple input files specified." << std::endl;
                return 1;
            }
        }
    }

    if (input_file.empty()) {
        std::cerr << "Error: No input file specified." << std::endl;
        print_help();
        return 1;
    }

    if (mode == CliMode::NONE) {
        std::cerr << "Error: No mode specified. Please use -a, -r, or -ar." << std::endl;
        print_help();
        return 1;
    }

    switch (mode) {
        case CliMode::ASSEMBLE: {
            std::cout << "Mode: Assemble" << std::endl;
            std::cout << "Input file: " << input_file << std::endl;
            std::cout << "Output file: " << output_file << std::endl;
            std::string assembly_code = read_source_file(input_file);
            std::vector<uint16_t> bytecode = assemble(assembly_code);
            write_bytecode(output_file, bytecode);
            break;
        }
        case CliMode::RUN: {
            std::cout << "Mode: Run" << std::endl;
            std::cout << "Input file: " << input_file << std::endl;
            std::vector<uint16_t> bytecode = read_bytecode_file(input_file);
            run_vm(bytecode);
            break;
        }
        case CliMode::ASSEMBLE_AND_RUN: {
            std::cout << "Mode: Assemble and Run" << std::endl;
            std::cout << "Input file: " << input_file << std::endl;
            std::string assembly_code = read_source_file(input_file);
            std::vector<uint16_t> bytecode = assemble(assembly_code);
            run_vm(bytecode);
            break;
        }
        case CliMode::NONE:
            // 이 경우는 이미 위에서 처리되었지만, 안전을 위해 추가합니다.
            break;
    }

    return 0;
}
