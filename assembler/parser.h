#include <string>
#include <vector>
#include <iostream>
#include <cstdint>

class Parser {
private:
    std::vector<std::string> tokens;
    std::vector<uint16_t> bytecode;

    void split_token(std::string input);
    std::vector<uint16_t> token_to_data(std::string input, size_t size);

public:
    Parser();
    ~Parser();

    void parse(std::string input);
    std::vector<uint16_t> get_bytecode();
};