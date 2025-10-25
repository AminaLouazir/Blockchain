#ifndef HASH_FUNCTIONS_H
#define HASH_FUNCTIONS_H

#include <string>
#include <vector>
#include <cstdint>

// Fonction de conversion texte -> bits
std::vector<bool> string_to_bits(const std::string& input);

// Fonction d'extraction du hash 256 bits
std::string extract_hash(const std::vector<bool>& state);

// Fonction de hachage basée sur automate cellulaire
std::string ac_hash(const std::string& input, uint32_t rule, size_t steps);

// Fonction SHA256 simplifiée (sans OpenSSL)
std::string sha256_simple(const std::string& input);

#endif