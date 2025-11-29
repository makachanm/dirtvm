#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>

#include "../assembler/parser.h"
#include "../engine/vm.h"
#include "../engine/object.h" // For stack_data

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

int main(int argc, char* argv[]) {
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

    std::cout << "Mode: ";
    if (mode == CliMode::ASSEMBLE) std::cout << "Assemble";
    else if (mode == CliMode::RUN) std::cout << "Run";
    else if (mode == CliMode::ASSEMBLE_AND_RUN) std::cout << "Assemble and Run";
    std::cout << std::endl;
    std::cout << "Input file: " << input_file << std::endl;
    if (mode == CliMode::ASSEMBLE) {
        std::cout << "Output file: " << output_file << std::endl;
    }

    // Actual logic for selected mode
    if (mode == CliMode::ASSEMBLE) {
        std::ifstream ifs(input_file);
        if (!ifs.is_open()) {
            std::cerr << "Error: Could not open input file " << input_file << std::endl;
            return 1;
        }
        std::string assembly_code((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
        ifs.close();

        Parser parser;
        parser.parse(assembly_code);
        std::vector<uint16_t> bytecode = parser.get_bytecode();

        std::ofstream ofs(output_file, std::ios::binary);
        if (!ofs.is_open()) {
            std::cerr << "Error: Could not open output file " << output_file << std::endl;
            return 1;
        }
        for (uint16_t instruction : bytecode) {
            ofs.write(reinterpret_cast<const char*>(&instruction), sizeof(instruction));
        }
        ofs.close();
        std::cout << "Assembly successful. Bytecode written to " << output_file << std::endl;
    } else if (mode == CliMode::RUN) {
        std::ifstream ifs(input_file, std::ios::binary);
        if (!ifs.is_open()) {
            std::cerr << "Error: Could not open input bytecode file " << input_file << std::endl;
            return 1;
        }

        std::vector<uint16_t> bytecode;
        uint16_t instruction;
        while (ifs.read(reinterpret_cast<char*>(&instruction), sizeof(instruction))) {
            bytecode.push_back(instruction);
        }
        ifs.close();

        vm dirt_vm(bytecode);
        dirt_vm.run();
        std::cout << "Execution finished." << std::endl;
    } else if (mode == CliMode::ASSEMBLE_AND_RUN) {
        std::ifstream ifs(input_file);
        if (!ifs.is_open()) {
            std::cerr << "Error: Could not open input file " << input_file << std::endl;
            return 1;
        }
        std::string assembly_code((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
        ifs.close();

        Parser parser;
        parser.parse(assembly_code);
        std::vector<uint16_t> bytecode = parser.get_bytecode();

        vm dirt_vm(bytecode);
        dirt_vm.run();
        std::cout << "Execution finished." << std::endl;
    }

    return 0;
}
