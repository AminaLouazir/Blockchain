#include "EX2.h"
#include "EX1.h"
#include <sstream>
#include <iomanip>
#include <algorithm>

std::vector<bool> string_to_bits(const std::string& input) {
    std::vector<bool> bits;
    for (unsigned char c : input) {
        for (int i = 7; i >= 0; --i) {
            bits.push_back((c >> i) & 1);
        }
    }
    return bits;
}

std::string extract_hash(const std::vector<bool>& state) {
    std::stringstream ss;
    
    for (size_t i = 0; i < 64; ++i) {
        int nibble = 0;
        for (int j = 0; j < 4; ++j) {
            size_t bit_idx = ((i * 4 + j) * 13 + i * 7) % state.size();
            nibble = (nibble << 1) | state[bit_idx];
        }
        ss << std::hex << nibble;
    }
    
    return ss.str();
}

std::string ac_hash(const std::string& input, uint32_t rule, size_t steps) {
    std::vector<bool> bits = string_to_bits(input);
    
    while (bits.size() < 512) {
        bits.push_back((bits.size() % 3) == 0);
    }
    
    CellularAutomaton1D ca(rule);
    ca.init_state(bits);
    
    size_t effective_steps = std::max(steps, size_t(200));
    ca.evolve(effective_steps);
    
    return extract_hash(ca.get_state());
}

std::string sha256_simple(const std::string& input) {
    unsigned long long hash = 5381;
    unsigned long long hash2 = 1;
    
    for (size_t i = 0; i < input.length(); i++) {
        hash = ((hash << 5) + hash) + input[i];
        hash2 = hash2 * 31 + input[i];
    }
    
    std::stringstream ss;
    ss << std::hex << std::setw(16) << std::setfill('0') << hash;
    ss << std::hex << std::setw(16) << std::setfill('0') << hash2;
    ss << std::hex << std::setw(16) << std::setfill('0') << (hash ^ hash2);
    ss << std::hex << std::setw(16) << std::setfill('0') << (hash + hash2);
    
    return ss.str();
}