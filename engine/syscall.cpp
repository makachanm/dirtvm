#include <unistd.h>
#include <iostream>
#include <vector>
#include <sys/syscall.h> // For syscall numbers like SYS_write

#include "vm.h"
#include "object.h"

void vm::handle_syscall(uint16_t operand1) {
    long syscall_num = operand1;
    long ret = 0;

    switch (syscall_num) {
        case SYS_read: { // syscall 0
            stack_data fd_data = pop();
            stack_data buf_addr_data = pop();
            stack_data count_data = pop();
            // Reading into VM memory is complex, so not fully implemented.
            std::cerr << "Syscall error: read is not implemented" << std::endl;
            ret = -1;
            break;
        }
        case SYS_write: { // syscall 1
            stack_data fd_data = pop();
            stack_data buf_addr_data = pop();
            stack_data count_data = pop();

            long fd = (long)fd_data.get_data();
            __uint128_t buf_addr = buf_addr_data.get_data();
            size_t count = (size_t)count_data.get_data();

            if (buf_addr + count > global_memory.size()) {
                 std::cerr << "Syscall error: write buffer out of bounds" << std::endl;
                 ret = -1;
            } else {
                std::vector<char> buffer;
                buffer.reserve(count);
                for(size_t i = 0; i < count; i++) {
                    buffer.push_back(static_cast<char>(global_memory[static_cast<size_t>(buf_addr + i)].get_data()));
                }
                ret = write(fd, buffer.data(), count);
            }
            break;
        }
        case SYS_exit: { // syscall 60
            stack_data status_data = pop();
            exit((int)status_data.get_data());
            break; 
        }
        default: {
            std::cerr << "Unsupported syscall: " << syscall_num << std::endl;
            ret = -1;
            break;
        }
    }

    // For syscalls that don't return (like exit), this won't be reached.
    // For others, push the return value.
    if (syscall_num != SYS_exit) {
        push(stack_data(D_TYPE::BIT_64, ret));
    }
}
