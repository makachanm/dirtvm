#ifndef OBJECT_H
#define OBJECT_H

enum D_TYPE {
    BIT_8,
    BIT_16,
    BIT_32,
    BIT_64,
    BIT_128,
};

class stack_data {
private:
    D_TYPE d_type;
    __uint128_t data;
    
public:
    stack_data(D_TYPE d_type, __uint128_t data);
    ~stack_data();

    D_TYPE get_d_type() const;
    __uint128_t get_data() const;
};

#endif // OBJECT_H
