#include <iostream>
#include <vector>
#include <cassert>
#include "vm.h"
#include "object.h"

// --- Opcode Definitions for Readability ---
#define OPC_ADD      (0b000001 << 10)
#define OPC_SUB      (0b000010 << 10)
#define OPC_MUL      (0b000011 << 10)
#define OPC_DIV      (0b000100 << 10)
#define OPC_POP      (0b000110 << 10)
#define OPC_DUP      (0b000111 << 10)
#define OPC_JMP      (0b001000 << 10)
#define OPC_JZ       (0b001001 << 10)
#define OPC_JNZ      (0b001010 << 10)
#define OPC_CALL     (0b001011 << 10)
#define OPC_RET      (0b001100 << 10)
#define OPC_EQ       (0b001101 << 10)
#define OPC_LT       (0b001110 << 10)
#define OPC_GT       (0b001111 << 10)
#define OPC_GLOAD    (0b010000 << 10)
#define OPC_GSTORE   (0b010001 << 10)
#define OPC_LLOAD    (0b010010 << 10)
#define OPC_LSTORE   (0b010011 << 10)
#define OPC_PUSHD8   (0b010100 << 10)
#define OPC_PUSHD16  (0b010101 << 10)
#define OPC_PUSHD32  (0b010110 << 10)
#define OPC_PUSHD64  (0b010111 << 10)
#define OPC_PUSHD128 (0b011000 << 10)
#define OPC_SYSCALL  (0b011001 << 10)

// Helper to access the internal stack for testing purposes.
// This requires a friend declaration in vm.h or making the stack public.
// For now, let's assume we can modify vm.h to add: friend class VMTester;
// Since we can't modify vm.h, we will test by what's left on the stack.

void test_arithmetic() {
    std::cout << "Testing Arithmetic..." << std::endl;
    // Test ADD: 5 + 10 = 15
    std::vector<uint16_t> bytecode_add = {
        OPC_PUSHD16, 10,
        OPC_PUSHD16, 5,
        OPC_ADD
    };
    vm vm_add(bytecode_add);
    vm_add.run();
    assert(vm_add.pop().get_data() == 15);

    // Test SUB: 10 - 5 = 5
    std::vector<uint16_t> bytecode_sub = {
        OPC_PUSHD16, 10,
        OPC_PUSHD16, 5,
        OPC_SUB
    };
    vm vm_sub(bytecode_sub);
    vm_sub.run();
    assert(vm_sub.pop().get_data() == 5);
    
    // Test MUL: 10 * 5 = 50
    std::vector<uint16_t> bytecode_mul = {
        OPC_PUSHD16, 5,
        OPC_PUSHD16, 10,
        OPC_MUL
    };
    vm vm_mul(bytecode_mul);
    vm_mul.run();
    assert(vm_mul.pop().get_data() == 50);

    // Test DIV: 10 / 5 = 2
    std::vector<uint16_t> bytecode_div = {
        OPC_PUSHD16, 10,
        OPC_PUSHD16, 5,
        OPC_DIV
    };
    vm vm_div(bytecode_div);
    vm_div.run();
    assert(vm_div.pop().get_data() == 2);
    std::cout << "Arithmetic Tests Passed!" << std::endl;
}

void test_stack_ops() {
    std::cout << "Testing Stack Ops..." << std::endl;
    // Test DUP
    std::vector<uint16_t> bytecode_dup = {
        OPC_PUSHD16, 123,
        OPC_DUP
    };
    vm vm_dup(bytecode_dup);
    vm_dup.run();
    assert(vm_dup.pop().get_data() == 123);
    assert(vm_dup.pop().get_data() == 123);

    // Test POP
     std::vector<uint16_t> bytecode_pop = {
        OPC_PUSHD16, 1,
        OPC_PUSHD16, 2,
        OPC_POP
    };
    vm vm_pop(bytecode_pop);
    vm_pop.run();
    assert(vm_pop.pop().get_data() == 1);

    std::cout << "Stack Ops Tests Passed!" << std::endl;
}

void test_control_flow() {
    std::cout << "Testing Control Flow..." << std::endl;
    // Test JMP
    std::vector<uint16_t> bytecode_jmp = {
        OPC_JMP, 0,0,0,0,0,0,0,11, // JMP to address 11
        OPC_PUSHD16, 1, // Should be skipped
        OPC_PUSHD16, 99 // Target
    };
    vm vm_jmp(bytecode_jmp);
    vm_jmp.run();
    assert(vm_jmp.pop().get_data() == 99);

    // Test JZ (jump)
    std::vector<uint16_t> bytecode_jz_true = {
        OPC_PUSHD16, 0,
        OPC_JZ, 0,0,0,0,0,0,0,13, // JZ to address 13
        OPC_PUSHD16, 1, // Skipped
        OPC_PUSHD16, 99 // Target
    };
    vm vm_jz_true(bytecode_jz_true);
    vm_jz_true.run();
    assert(vm_jz_true.pop().get_data() == 99);
    
    // Test CALL/RET
    std::vector<uint16_t> bytecode_call = {
        OPC_CALL, 0,0,0,0,0,0,0,12, // Call address 12
        OPC_PUSHD16, 55,           // After return
        OPC_RET,                   // Should not be executed here
        OPC_PUSHD16, 123,          // In function
        OPC_RET                    // Return
    };
    vm vm_call(bytecode_call);
    vm_call.run();
    assert(vm_call.pop().get_data() == 55);
    assert(vm_call.pop().get_data() == 123);
    
    std::cout << "Control Flow Tests Passed!" << std::endl;
}

void test_comparison() {
    std::cout << "Testing Comparison..." << std::endl;
    // Test EQ
    std::vector<uint16_t> bytecode_eq_true = { OPC_PUSHD16, 5, OPC_PUSHD16, 5, OPC_EQ };
    vm vm_eq_true(bytecode_eq_true);
    vm_eq_true.run();
    assert(vm_eq_true.pop().get_data() == 1);

    // Test LT
    std::vector<uint16_t> bytecode_lt_true = { OPC_PUSHD16, 5, OPC_PUSHD16, 10, OPC_LT };
    vm vm_lt_true(bytecode_lt_true);
    vm_lt_true.run();
    assert(vm_lt_true.pop().get_data() == 1);

    // Test GT
    std::vector<uint16_t> bytecode_gt_true = { OPC_PUSHD16, 10, OPC_PUSHD16, 5, OPC_GT };
    vm vm_gt_true(bytecode_gt_true);
    vm_gt_true.run();
    assert(vm_gt_true.pop().get_data() == 1);

    std::cout << "Comparison Tests Passed!" << std::endl;
}

void test_memory() {
    std::cout << "Testing Memory..." << std::endl;
    // Test GSTORE / GLOAD
    std::vector<uint16_t> bytecode_gmem = {
        OPC_PUSHD16, 123,       // Value to store
        OPC_PUSHD16, 2,         // Address
        OPC_GSTORE,             // Store 123 at address 2
        OPC_PUSHD16, 2,         // Address to load from
        OPC_GLOAD               // Load from address 2
    };
    vm vm_gmem(bytecode_gmem);
    vm_gmem.run();
    assert(vm_gmem.pop().get_data() == 123);

    // Test LSTORE / LLOAD
    uint16_t tag = 5;
    std::vector<uint16_t> bytecode_lmem = {
        OPC_PUSHD16, 456,            // Value to store
        OPC_PUSHD16, 1,              // Address
        (uint16_t)(OPC_LSTORE | tag),            // Store 456 at loc(tag=5, addr=1)
        OPC_PUSHD16, 1,              // Address to load
        (uint16_t)(OPC_LLOAD | tag)              // Load from loc(tag=5, addr=1)
    };
     vm vm_lmem(bytecode_lmem);
    vm_lmem.run();
    assert(vm_lmem.pop().get_data() == 456);

    std::cout << "Memory Tests Passed!" << std::endl;
}

void test_push() {
    std::cout << "Testing Push Operations..." << std::endl;
    // Test PUSHD8
    std::vector<uint16_t> code_d8 = { OPC_PUSHD8, 0xAB };
    vm vm_d8(code_d8);
    vm_d8.run();
    assert(vm_d8.pop().get_data() == 0xAB);
    
    // Test PUSHD16
    std::vector<uint16_t> code_d16 = { OPC_PUSHD16, 0xABCD };
    vm vm_d16(code_d16);
    vm_d16.run();
    assert(vm_d16.pop().get_data() == 0xABCD);

    // Test PUSHD32
    std::vector<uint16_t> code_d32 = { OPC_PUSHD32, 0xCDAB, 0xEF89 };
    vm vm_d32(code_d32);
    vm_d32.run();
    assert(vm_d32.pop().get_data() == 0xEF89CDAB);

    std::cout << "Push Tests Passed!" << std::endl;
}

void test_syscall() {
    std::cout << "Testing Syscall..." << std::endl;
    // 1. Store "hello" in global memory
    // 2. Setup stack for write(1, &"hello", 5)
    // 3. Call syscall
    std::vector<uint16_t> bytecode = {
        // Store "h" at address 0
        OPC_PUSHD8, 'h',
        OPC_PUSHD16, 0,
        OPC_GSTORE,
        // Store "e" at address 1
        OPC_PUSHD8, 'e',
        OPC_PUSHD16, 1,
        OPC_GSTORE,
        // Store "l" at address 2
        OPC_PUSHD8, 'l',
        OPC_PUSHD16, 2,
        OPC_GSTORE,
        // Store "l" at address 3
        OPC_PUSHD8, 'l',
        OPC_PUSHD16, 3,
        OPC_GSTORE,
        // Store "o" at address 4
        OPC_PUSHD8, 'o',
        OPC_PUSHD16, 4,
        OPC_GSTORE,

        // Setup stack for syscall
        OPC_PUSHD16, 5,      // arg 3: count = 5
        OPC_PUSHD16, 0,      // arg 2: buffer address = 0
        OPC_PUSHD16, 1,      // arg 1: fd = 1 (stdout)
        OPC_PUSHD16, 1,      // syscall number for write
        OPC_SYSCALL
    };

    vm vm_syscall(bytecode);
    vm_syscall.run();

    assert(vm_syscall.pop().get_data() == 5);
    std::cout << "Syscall Test Passed!" << std::endl;
}

int main() {
    test_arithmetic();
    test_stack_ops();
    test_control_flow();
    test_comparison();
    test_memory();
    test_push();
    test_syscall();

    std::cout << "\nAll tests passed successfully!" << std::endl;
    return 0;
}
