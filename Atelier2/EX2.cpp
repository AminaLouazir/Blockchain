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
    
    size_t effective_steps = std::max(steps, size_t(50));  // Réduit pour la vitesse
    ca.evolve(effective_steps);
    
    return extract_hash(ca.get_state());
}

// VERSION AMÉLIORÉE de sha256_simple avec meilleure distribution
std::string sha256_simple(const std::string& input) {
    // Utilisation de plusieurs hash combinés pour meilleure distribution
    unsigned long long h1 = 0x6a09e667f3bcc908ULL;
    unsigned long long h2 = 0xbb67ae8584caa73bULL;
    unsigned long long h3 = 0x3c6ef372fe94f82bULL;
    unsigned long long h4 = 0xa54ff53a5f1d36f1ULL;
    
    // Hash chaque caractère avec des constantes différentes
    for (size_t i = 0; i < input.length(); i++) {
        unsigned char c = input[i];
        
        h1 = ((h1 << 5) + h1) ^ c;
        h1 = h1 * 0x5bd1e995;
        h1 ^= h1 >> 15;
        
        h2 = ((h2 << 7) + h2) ^ (c * 31);
        h2 = h2 * 0x85ebca6b;
        h2 ^= h2 >> 13;
        
        h3 = ((h3 << 3) + h3) + (c * 37);
        h3 = h3 * 0xc2b2ae35;
        h3 ^= h3 >> 17;
        
        h4 = ((h4 << 11) + h4) ^ (c * 41);
        h4 = h4 * 0x27d4eb2d;
        h4 ^= h4 >> 11;
    }
    
    // Mélange final
    h1 ^= h1 >> 16;
    h2 ^= h2 >> 16;
    h3 ^= h3 >> 16;
    h4 ^= h4 >> 16;
    
    // Formatage en hex (64 caractères = 256 bits)
    std::stringstream ss;
    ss << std::hex << std::setw(16) << std::setfill('0') << h1;
    ss << std::hex << std::setw(16) << std::setfill('0') << h2;
    ss << std::hex << std::setw(16) << std::setfill('0') << h3;
    ss << std::hex << std::setw(16) << std::setfill('0') << h4;
    
    return ss.str();
}