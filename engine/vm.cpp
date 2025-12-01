#include "vm.h"
#include <iostream> // For debugging, can be removed later
#include <algorithm> // For std::max

// Helper function to read a 128-bit address from the bytecode
__uint128_t read_address(const std::vector<uint16_t>& bytecode, __uint128_t& pc) {
    __uint128_t address = 0;
    for (int i = 0; i < 8; ++i) {
        if (static_cast<size_t>(pc + i) < bytecode.size()) {
            address |= (__uint128_t)bytecode[pc + i] << (16 * i);
        } else {
            std::cerr << "Unexpected end of bytecode when reading an address" << std::endl;
            return 0;
        }
    }
    pc += 8;
    return address;
}

vm::vm(std::vector<uint16_t> bytecode)
    : raw_bytecode(std::move(bytecode)), pc(0) {

}

vm::~vm() {
    // Destructor
}

void vm::push(stack_data data) {
    stack.push(data);
}

stack_data vm::pop() {
    if (stack.empty()) {
        // Error: stack underflow
        // This should be handled more gracefully
        std::cerr << "stack underflow at data stack" << std::endl;
        exit(1);
    }
    stack_data val = stack.top();
    stack.pop();
    return val;
}

stack_data& vm::top() {
     if (stack.empty()) {
        // Error: stack underflow
        // This should be handled more gracefully
        std::cerr << "stack underflow at data stack" << std::endl;
        exit(1);
    }
    return stack.top();
}


void vm::run() {
    while (static_cast<size_t>(pc) < raw_bytecode.size()) {
        uint16_t instruction = raw_bytecode[pc++];
        uint8_t opcode = instruction >> 10;
        uint16_t operand1 = instruction & 0x03FF;

        switch (opcode) {
            case 0b000001: { // add
                stack_data b = pop();
                stack_data a = pop();
                push(stack_data(a.get_d_type(), a.get_data() + b.get_data()));
                break;
            }
            case 0b000010: { // sub
                stack_data b = pop();
                stack_data a = pop();
                push(stack_data(a.get_d_type(), a.get_data() - b.get_data()));
                break;
            }
            case 0b000011: { // mul
                stack_data b = pop();
                stack_data a = pop();
                push(stack_data(a.get_d_type(), a.get_data() * b.get_data()));
                break;
            }
            case 0b000100: { // div
                stack_data b = pop();
                stack_data a = pop();
                if (b.get_data() == 0) {
                    // Division by zero error
                    std::cerr << "Division by zero error" << std::endl;
                    exit(1);
                }
                push(stack_data(a.get_d_type(), a.get_data() / b.get_data()));
                break;
            }
            case 0b000110: { // pop
                pop();
                break;
            }
            case 0b000111: { // dup
                push(top());
                break;
            }
            case 0b001000: { // jmp
                pc = read_address(raw_bytecode, pc);
                continue; // pc가 이미 설정되었으므로 루프의 끝에서 pc++를 건너뜁니다.
            }
            case 0b001001: { // jz
                __uint128_t dest = read_address(raw_bytecode, pc);
                stack_data val = pop();
                if (val.get_data() == 0) {
                    pc = dest;
                    continue;
                }
                break;
            }
            case 0b001010: { // jnz
                __uint128_t dest = read_address(raw_bytecode, pc);
                stack_data val = pop();
                if (val.get_data() != 0) {
                    pc = dest;
                    continue;
                }
                break;
            }
            case 0b001011: { // call
                __uint128_t dest = read_address(raw_bytecode, pc);
                call_stack.push(pc);
                pc = dest;
                continue; // pc가 이미 설정되었으므로 루프의 끝에서 pc++를 건너뜁니다.
            }
            case 0b001100: { // ret
                if (call_stack.empty()) {
                    // Return from main program body, treat as HALT
                    return;
                }
                pc = call_stack.top();
                call_stack.pop();
                break;
            }
            case 0b001101: { // eq
                stack_data b = pop();
                stack_data a = pop();
                push(stack_data(D_TYPE::BIT_8, a.get_data() == b.get_data()));
                break;
            }
            case 0b001110: { // lt
                stack_data b = pop();
                stack_data a = pop();
                push(stack_data(D_TYPE::BIT_8, a.get_data() < b.get_data()));
                break;
            }
            case 0b001111: { // gt
                stack_data b = pop();
                stack_data a = pop();
                push(stack_data(D_TYPE::BIT_8, a.get_data() > b.get_data()));
                break;
            }
            case 0b010000: { // gload
                stack_data addr = pop();
                __uint128_t address = addr.get_data();
                if (address >= global_memory.size()) {
                    // Error: out of bounds global memory access
                    exit(1);
                }
                push(global_memory[static_cast<size_t>(address)]);
                break;
            }
            case 0b010001: { // gstore
                stack_data addr = pop();
                stack_data val = pop();
                __uint128_t address = addr.get_data();
                if (address >= global_memory.size()) {
                    global_memory.resize(static_cast<size_t>(address) + 1, stack_data(D_TYPE::BIT_8, 0));
                }
                global_memory[static_cast<size_t>(address)] = val;
                break;
            }
            case 0b010010: { // lload
                uint16_t tag = operand1;
                stack_data addr = pop();
                __uint128_t address = addr.get_data();
                if (tag >= local_memory.size() || address >= local_memory[tag].size()) {
                    // Error: out of bounds local memory access
                    exit(1);
                }
                push(local_memory[tag][static_cast<size_t>(address)]);
                break;
            }
            case 0b010011: { // lstore
                uint16_t tag = operand1;
                stack_data addr = pop();
                stack_data val = pop();
                __uint128_t address = addr.get_data();
                if (tag >= local_memory.size()) {
                    local_memory.resize(tag + 1);
                }
                if (address >= local_memory[tag].size()) {
                    local_memory[tag].resize(static_cast<size_t>(address) + 1, stack_data(D_TYPE::BIT_8, 0));
                }
                local_memory[tag][static_cast<size_t>(address)] = val;
                break;
            }
            case 0b010100: { // pushd8
                push(stack_data(D_TYPE::BIT_8, raw_bytecode[pc] & 0xFF));
                pc += 1; // Consume data word
                break;
            }
            case 0b010101: { // pushd16
                push(stack_data(D_TYPE::BIT_16, raw_bytecode[pc]));
                pc += 1; // 데이터 1워드.
                break;
            }
            case 0b010110: { // pushd32
                __uint128_t data = 0;
                for(int i = 1; i >= 0; i--) {
                    data <<= 16;
                    data |= raw_bytecode[pc + i];
                }
                push(stack_data(D_TYPE::BIT_32, data));
                pc += 2; // 데이터 2워드.
                break;
            }
            case 0b010111: { // pushd64
                __uint128_t data = 0;
                for(int i = 0; i < 4; i++) {
                    data <<= 16;
                    data |= raw_bytecode[pc + (3-i)];
                }
                push(stack_data(D_TYPE::BIT_64, data));
                pc += 4; // 데이터 4워드.
                break;
            }
            case 0b011000: { // pushd128
                __uint128_t data = 0;
                 for(int i = 0; i < 8; i++) {
                    data <<= 16;
                    data |= raw_bytecode[pc + (7-i)];
                }
                push(stack_data(D_TYPE::BIT_128, data));
                pc += 8; // 데이터 8워드.
                break;
            }
            case 0b011001: { // syscall
                handle_syscall(operand1);
                break;
            }
            default:
                // Unknown opcode
                std::cerr << "Unknown opcode: " << std::hex << (int)opcode << std::endl;
                continue; // or exit, or throw an exception
        }
    }
}
