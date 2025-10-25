#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <cstdlib>
#include <ctime>
#include "EX2.h"

// ========== FONCTIONS UTILITAIRES ==========

// Convertir un hash hexadécimal en bits
std::vector<bool> hex_to_bits(const std::string& hex_hash) {
    std::vector<bool> bits;
    for (char c : hex_hash) {
        int val;
        if (c >= '0' && c <= '9') {
            val = c - '0';
        } else if (c >= 'a' && c <= 'f') {
            val = c - 'a' + 10;
        } else if (c >= 'A' && c <= 'F') {
            val = c - 'A' + 10;
        } else {
            continue;
        }
        
        // Convertir chaque caractère hex (4 bits)
        for (int i = 3; i >= 0; --i) {
            bits.push_back((val >> i) & 1);
        }
    }
    return bits;
}

// 5.1. Calculer le pourcentage de bits différents entre deux hashes
double calculate_bit_difference_percentage(const std::string& hash1, const std::string& hash2) {
    std::vector<bool> bits1 = hex_to_bits(hash1);
    std::vector<bool> bits2 = hex_to_bits(hash2);
    
    if (bits1.size() != bits2.size()) {
        std::cerr << "Erreur: Les hashes n'ont pas la même taille!" << std::endl;
        return 0.0;
    }
    
    int different_bits = 0;
    for (size_t i = 0; i < bits1.size(); ++i) {
        if (bits1[i] != bits2[i]) {
            different_bits++;
        }
    }
    
    return (double)different_bits / bits1.size() * 100.0;
}

// Inverser un bit dans une chaîne à une position donnée
std::string flip_bit_in_string(const std::string& input, size_t bit_position) {
    std::string result = input;
    
    // Calculer la position du caractère et du bit
    size_t char_index = bit_position / 8;
    size_t bit_index = 7 - (bit_position % 8);
    
    if (char_index >= result.length()) {
        std::cerr << "Erreur: Position de bit hors limites!" << std::endl;
        return result;
    }
    
    // Inverser le bit
    result[char_index] ^= (1 << bit_index);
    
    return result;
}

// ========== STRUCTURE POUR RÉSULTATS ==========
struct AvalancheResult {
    std::string original_message;
    int bit_flipped;
    std::string modified_message;
    std::string original_hash;
    std::string modified_hash;
    double difference_percentage;
    
    void print() const {
        std::cout << "\n--- Test #" << bit_flipped << " ---" << std::endl;
        std::cout << "Message original: \"" << original_message << "\"" << std::endl;
        std::cout << "Bit inverse: " << bit_flipped << std::endl;
        std::cout << "Hash original:  " << original_hash.substr(0, 32) << "..." << std::endl;
        std::cout << "Hash modifie:   " << modified_hash.substr(0, 32) << "..." << std::endl;
        std::cout << "Difference: " << std::fixed << std::setprecision(2) 
                  << difference_percentage << "% des bits" << std::endl;
    }
};

// ========== TESTS D'AVALANCHE ==========

// 5.1. Test avec AC_HASH
void test_avalanche_ac_hash(const std::string& message, int num_tests) {
    std::cout << "\n========================================" << std::endl;
    std::cout << "  TEST AVALANCHE - AC_HASH" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Message de test: \"" << message << "\"" << std::endl;
    std::cout << "Nombre de tests: " << num_tests << std::endl;
    
    std::vector<AvalancheResult> results;
    double total_difference = 0.0;
    
    // Hash original
    std::string original_hash = ac_hash(message, 30, 250);
    
    // Tester en inversant différents bits
    int total_bits = message.length() * 8;
    
    for (int i = 0; i < num_tests; ++i) {
        // Choisir une position de bit aléatoire ou séquentielle
        int bit_pos = (i < total_bits) ? i : (rand() % total_bits);
        
        // Inverser le bit
        std::string modified_message = flip_bit_in_string(message, bit_pos);
        
        // Calculer le nouveau hash
        std::string modified_hash = ac_hash(modified_message, 30, 250);
        
        // Calculer la différence
        double diff_percentage = calculate_bit_difference_percentage(original_hash, modified_hash);
        
        // Stocker les résultats
        AvalancheResult result;
        result.original_message = message;
        result.bit_flipped = bit_pos;
        result.modified_message = modified_message;
        result.original_hash = original_hash;
        result.modified_hash = modified_hash;
        result.difference_percentage = diff_percentage;
        
        results.push_back(result);
        total_difference += diff_percentage;
        
        // Afficher quelques exemples
        if (i < 3 || i == num_tests - 1) {
            result.print();
        }
    }
    
    // Calculer et afficher la moyenne
    double average_difference = total_difference / num_tests;
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "  RESULTATS AC_HASH" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Nombre de tests: " << num_tests << std::endl;
    std::cout << "Difference moyenne: " << std::fixed << std::setprecision(2) 
              << average_difference << "%" << std::endl;
    std::cout << "Ideal (bon effet avalanche): ~50%" << std::endl;
    
    // Analyse
    if (average_difference >= 45.0 && average_difference <= 55.0) {
        std::cout << "Verdict: EXCELLENT effet avalanche! ✓" << std::endl;
    } else if (average_difference >= 40.0 && average_difference <= 60.0) {
        std::cout << "Verdict: BON effet avalanche." << std::endl;
    } else if (average_difference >= 30.0 && average_difference <= 70.0) {
        std::cout << "Verdict: Effet avalanche ACCEPTABLE." << std::endl;
    } else {
        std::cout << "Verdict: Effet avalanche FAIBLE! ✗" << std::endl;
    }
}

// Test avec SHA256 (pour comparaison)
void test_avalanche_sha256(const std::string& message, int num_tests) {
    std::cout << "\n========================================" << std::endl;
    std::cout << "  TEST AVALANCHE - SHA256" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Message de test: \"" << message << "\"" << std::endl;
    std::cout << "Nombre de tests: " << num_tests << std::endl;
    
    std::vector<AvalancheResult> results;
    double total_difference = 0.0;
    
    // Hash original
    std::string original_hash = sha256_simple(message);
    
    // Tester en inversant différents bits
    int total_bits = message.length() * 8;
    
    for (int i = 0; i < num_tests; ++i) {
        int bit_pos = (i < total_bits) ? i : (rand() % total_bits);
        std::string modified_message = flip_bit_in_string(message, bit_pos);
        std::string modified_hash = sha256_simple(modified_message);
        double diff_percentage = calculate_bit_difference_percentage(original_hash, modified_hash);
        
        AvalancheResult result;
        result.original_message = message;
        result.bit_flipped = bit_pos;
        result.modified_message = modified_message;
        result.original_hash = original_hash;
        result.modified_hash = modified_hash;
        result.difference_percentage = diff_percentage;
        
        results.push_back(result);
        total_difference += diff_percentage;
        
        if (i < 3 || i == num_tests - 1) {
            result.print();
        }
    }
    
    double average_difference = total_difference / num_tests;
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "  RESULTATS SHA256" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Nombre de tests: " << num_tests << std::endl;
    std::cout << "Difference moyenne: " << std::fixed << std::setprecision(2) 
              << average_difference << "%" << std::endl;
    std::cout << "Ideal (bon effet avalanche): ~50%" << std::endl;
    
    if (average_difference >= 45.0 && average_difference <= 55.0) {
        std::cout << "Verdict: EXCELLENT effet avalanche! ✓" << std::endl;
    } else if (average_difference >= 40.0 && average_difference <= 60.0) {
        std::cout << "Verdict: BON effet avalanche." << std::endl;
    } else if (average_difference >= 30.0 && average_difference <= 70.0) {
        std::cout << "Verdict: Effet avalanche ACCEPTABLE." << std::endl;
    } else {
        std::cout << "Verdict: Effet avalanche FAIBLE! ✗" << std::endl;
    }
}

// Tableau comparatif
void print_comparison_table(const std::string& message, int num_tests) {
    std::cout << "\n========================================" << std::endl;
    std::cout << "  TABLEAU COMPARATIF" << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    // Calculer pour AC_HASH
    std::string original_hash_ac = ac_hash(message, 30, 250);
    double total_diff_ac = 0.0;
    
    for (int i = 0; i < num_tests; ++i) {
        int bit_pos = i % (message.length() * 8);
        std::string modified = flip_bit_in_string(message, bit_pos);
        std::string modified_hash = ac_hash(modified, 30, 250);
        total_diff_ac += calculate_bit_difference_percentage(original_hash_ac, modified_hash);
    }
    
    // Calculer pour SHA256
    std::string original_hash_sha = sha256_simple(message);
    double total_diff_sha = 0.0;
    
    for (int i = 0; i < num_tests; ++i) {
        int bit_pos = i % (message.length() * 8);
        std::string modified = flip_bit_in_string(message, bit_pos);
        std::string modified_hash = sha256_simple(modified);
        total_diff_sha += calculate_bit_difference_percentage(original_hash_sha, modified_hash);
    }
    
    double avg_ac = total_diff_ac / num_tests;
    double avg_sha = total_diff_sha / num_tests;
    
    std::cout << std::left << std::setw(25) << "Metrique" 
              << std::setw(20) << "AC_HASH" 
              << std::setw(20) << "SHA256" << std::endl;
    std::cout << std::string(65, '-') << std::endl;
    
    std::cout << std::left << std::setw(25) << "Nombre de tests" 
              << std::setw(20) << num_tests
              << std::setw(20) << num_tests << std::endl;
    
    std::cout << std::left << std::setw(25) << "Diff moyenne (%)" 
              << std::setw(20) << std::fixed << std::setprecision(2) << avg_ac
              << std::setw(20) << std::fixed << std::setprecision(2) << avg_sha << std::endl;
    
    std::cout << std::left << std::setw(25) << "Ideal" 
              << std::setw(20) << "~50%"
              << std::setw(20) << "~50%" << std::endl;
    
    std::cout << std::string(65, '-') << std::endl;
}

// ========== MAIN ==========
int main() {
    srand(time(0));
    
    std::cout << "========================================" << std::endl;
    std::cout << "  EXERCICE 5 - EFFET AVALANCHE" << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    std::string test_message = "Hello World!";
    int num_tests = 20;  // Tester 20 inversions de bits
    
    // Test 1: AC_HASH
    test_avalanche_ac_hash(test_message, num_tests);
    
    // Test 2: SHA256 (pour comparaison)
    test_avalanche_sha256(test_message, num_tests);
    
    // Test 3: Tableau comparatif
    print_comparison_table(test_message, num_tests);
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "         TESTS TERMINES" << std::endl;
    std::cout << "========================================" << std::endl;
    
    return 0;
}