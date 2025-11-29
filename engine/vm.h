#include <stack>
#include <vector>
#include <cstdint>

#include "object.h"

class vm
{
private:
    __uint128_t pc;
    std::stack<stack_data> stack;
    std::stack<__uint128_t> call_stack;
    std::vector<stack_data> global_memory;
    std::vector<std::vector<stack_data>> local_memory;
    std::vector<uint16_t> raw_bytecode;


    void push(stack_data);
    void handle_syscall();

public:
    vm(std::vector<uint16_t> raw_bytecode);
    void run();
    ~vm();

    stack_data pop();
    stack_data& top();
};