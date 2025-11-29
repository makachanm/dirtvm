#include "object.h"

stack_data::stack_data(D_TYPE type, __uint128_t val) : d_type(type), data(val) {}

stack_data::~stack_data() {}

D_TYPE stack_data::get_d_type() const {
    return d_type;
}

__uint128_t stack_data::get_data() const {
    return data;
}
