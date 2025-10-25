#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <cmath>  // Pour fabs()
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
        
        // Convertir chaque caractère hex en 4 bits
        for (int i = 3; i >= 0; --i) {
            bits.push_back((val >> i) & 1);
        }
    }
    return bits;
}

// Générer un message aléatoire
std::string generate_random_message(int length) {
    std::string message;
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 !@#$%^&*()";
    
    for (int i = 0; i < length; ++i) {
        message += charset[rand() % (sizeof(charset) - 1)];
    }
    
    return message;
}

// ========== STRUCTURE POUR RÉSULTATS ==========
struct BitDistributionResult {
    std::string hash_method;
    int total_hashes;
    long long total_bits;
    long long bits_one;
    long long bits_zero;
    double percentage_one;
    double percentage_zero;
    
    BitDistributionResult() 
        : total_hashes(0), total_bits(0), bits_one(0), bits_zero(0), 
          percentage_one(0.0), percentage_zero(0.0) {}
    
    void calculate_percentages() {
        if (total_bits > 0) {
            percentage_one = (double)bits_one / total_bits * 100.0;
            percentage_zero = (double)bits_zero / total_bits * 100.0;
        }
    }
    
    bool is_balanced() const {
        // Distribution équilibrée si entre 49% et 51%
        return (percentage_one >= 49.0 && percentage_one <= 51.0);
    }
    
    void print() const {
        std::cout << "\n========================================" << std::endl;
        std::cout << "  RESULTATS - " << hash_method << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << "Nombre de hashes testes: " << total_hashes << std::endl;
        std::cout << "Nombre total de bits: " << total_bits << std::endl;
        std::cout << std::string(40, '-') << std::endl;
        std::cout << "Bits a 1: " << bits_one 
                  << " (" << std::fixed << std::setprecision(4) 
                  << percentage_one << "%)" << std::endl;
        std::cout << "Bits a 0: " << bits_zero 
                  << " (" << std::fixed << std::setprecision(4) 
                  << percentage_zero << "%)" << std::endl;
        std::cout << std::string(40, '-') << std::endl;
        
        // Évaluation
        std::cout << "Distribution ideale: 50.00% de 1" << std::endl;
        std::cout << "Ecart par rapport a l'ideal: " 
                  << std::fixed << std::setprecision(4) 
                  << std::abs(percentage_one - 50.0) << "%" << std::endl;
        
        if (is_balanced()) {
            std::cout << "Verdict: Distribution EQUILIBREE ✓" << std::endl;
        } else if (percentage_one >= 48.0 && percentage_one <= 52.0) {
            std::cout << "Verdict: Distribution ACCEPTABLE" << std::endl;
        } else if (percentage_one >= 45.0 && percentage_one <= 55.0) {
            std::cout << "Verdict: Distribution PASSABLE" << std::endl;
        } else {
            std::cout << "Verdict: Distribution DESEQUILIBREE ✗" << std::endl;
        }
    }
};

// ========== ANALYSE DE DISTRIBUTION ==========

// 6.1. Analyser la distribution des bits pour AC_HASH
BitDistributionResult analyze_bit_distribution_ac_hash(int num_samples) {
    std::cout << "\n========================================" << std::endl;
    std::cout << "  ANALYSE AC_HASH" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Generation de " << num_samples << " hashes..." << std::endl;
    
    BitDistributionResult result;
    result.hash_method = "AC_HASH";
    result.total_hashes = num_samples;
    
    // Générer des hashes et compter les bits
    for (int i = 0; i < num_samples; ++i) {
        // Générer un message aléatoire
        std::string message = generate_random_message(10 + (rand() % 20));
        
        // Calculer le hash
        std::string hash = ac_hash(message, 30, 250);
        
        // Convertir en bits et compter
        std::vector<bool> bits = hex_to_bits(hash);
        
        for (bool bit : bits) {
            result.total_bits++;
            if (bit) {
                result.bits_one++;
            } else {
                result.bits_zero++;
            }
        }
        
        // Afficher la progression
        if ((i + 1) % 100 == 0 || i == 0) {
            std::cout << "Progression: " << (i + 1) << "/" << num_samples 
                      << " (" << std::fixed << std::setprecision(1) 
                      << ((i + 1) * 100.0 / num_samples) << "%)" << std::endl;
        }
    }
    
    result.calculate_percentages();
    return result;
}

// Analyser la distribution des bits pour SHA256
BitDistributionResult analyze_bit_distribution_sha256(int num_samples) {
    std::cout << "\n========================================" << std::endl;
    std::cout << "  ANALYSE SHA256" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Generation de " << num_samples << " hashes..." << std::endl;
    
    BitDistributionResult result;
    result.hash_method = "SHA256";
    result.total_hashes = num_samples;
    
    for (int i = 0; i < num_samples; ++i) {
        std::string message = generate_random_message(10 + (rand() % 20));
        std::string hash = sha256_simple(message);
        std::vector<bool> bits = hex_to_bits(hash);
        
        for (bool bit : bits) {
            result.total_bits++;
            if (bit) {
                result.bits_one++;
            } else {
                result.bits_zero++;
            }
        }
        
        if ((i + 1) % 100 == 0 || i == 0) {
            std::cout << "Progression: " << (i + 1) << "/" << num_samples 
                      << " (" << std::fixed << std::setprecision(1) 
                      << ((i + 1) * 100.0 / num_samples) << "%)" << std::endl;
        }
    }
    
    result.calculate_percentages();
    return result;
}

// ========== TABLEAU COMPARATIF ==========
void print_comparison_table(const BitDistributionResult& ac_result, 
                            const BitDistributionResult& sha_result) {
    std::cout << "\n========================================" << std::endl;
    std::cout << "  TABLEAU COMPARATIF" << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    std::cout << std::left << std::setw(30) << "Metrique" 
              << std::setw(20) << "AC_HASH" 
              << std::setw(20) << "SHA256" << std::endl;
    std::cout << std::string(70, '-') << std::endl;
    
    std::cout << std::left << std::setw(30) << "Nombre de hashes" 
              << std::setw(20) << ac_result.total_hashes
              << std::setw(20) << sha_result.total_hashes << std::endl;
    
    std::cout << std::left << std::setw(30) << "Total de bits" 
              << std::setw(20) << ac_result.total_bits
              << std::setw(20) << sha_result.total_bits << std::endl;
    
    std::cout << std::left << std::setw(30) << "Bits a 1" 
              << std::setw(20) << ac_result.bits_one
              << std::setw(20) << sha_result.bits_one << std::endl;
    
    std::cout << std::left << std::setw(30) << "% de bits a 1" 
              << std::setw(20) << (std::to_string(ac_result.percentage_one).substr(0, 7) + "%")
              << std::setw(20) << (std::to_string(sha_result.percentage_one).substr(0, 7) + "%") << std::endl;
    
    std::cout << std::left << std::setw(30) << "Distribution ideale" 
              << std::setw(20) << "50.00%"
              << std::setw(20) << "50.00%" << std::endl;
    
    std::cout << std::left << std::setw(30) << "Ecart par rapport ideal" 
              << std::setw(20) << (std::to_string(std::abs(ac_result.percentage_one - 50.0)).substr(0, 6) + "%")
              << std::setw(20) << (std::to_string(std::abs(sha_result.percentage_one - 50.0)).substr(0, 6) + "%") << std::endl;
    
    std::cout << std::left << std::setw(30) << "Equilibree (49-51%)?" 
              << std::setw(20) << (ac_result.is_balanced() ? "OUI ✓" : "NON ✗")
              << std::setw(20) << (sha_result.is_balanced() ? "OUI ✓" : "NON ✗") << std::endl;
    
    std::cout << std::string(70, '-') << std::endl;
}

// ========== ANALYSE DÉTAILLÉE PAR POSITION ==========
void analyze_bit_positions(int num_samples) {
    std::cout << "\n========================================" << std::endl;
    std::cout << "  ANALYSE PAR POSITION DE BIT" << std::endl;
    std::cout << "========================================" << std::endl;
    
    const int HASH_BITS = 256;
    std::vector<int> bit_counts(HASH_BITS, 0);
    
    std::cout << "Analyse de " << num_samples << " hashes AC_HASH..." << std::endl;
    
    for (int i = 0; i < num_samples; ++i) {
        std::string message = generate_random_message(15);
        std::string hash = ac_hash(message, 30, 250);
        std::vector<bool> bits = hex_to_bits(hash);
        
        for (size_t j = 0; j < bits.size() && j < HASH_BITS; ++j) {
            if (bits[j]) {
                bit_counts[j]++;
            }
        }
    }
    
    // Calculer les statistiques par position
    std::cout << "\nStatistiques par position de bit:" << std::endl;
    std::cout << "Position | % de 1 | Ecart ideal" << std::endl;
    std::cout << std::string(40, '-') << std::endl;
    
    int positions_to_show = 10;  // Montrer les 10 premières positions
    for (int i = 0; i < positions_to_show; ++i) {
        double percentage = (double)bit_counts[i] / num_samples * 100.0;
        double deviation = std::abs(percentage - 50.0);
        
        std::cout << std::setw(8) << i << " | "
                  << std::setw(6) << std::fixed << std::setprecision(2) << percentage << "% | "
                  << std::setw(6) << std::fixed << std::setprecision(2) << deviation << "%" << std::endl;
    }
    std::cout << "..." << std::endl;
}

// ========== MAIN ==========
int main() {
    srand(time(0));
    
    std::cout << "========================================" << std::endl;
    std::cout << "  EXERCICE 6 - DISTRIBUTION DES BITS" << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    // 6.1. Objectif: au moins 10^5 bits
    // Avec des hashes de 256 bits, on a besoin de: 10^5 / 256 ≈ 391 hashes
    // Utilisons 500 hashes pour avoir ~128,000 bits (> 10^5)
    
    int num_samples = 200;
    
    std::cout << "Objectif: Analyser au moins 100,000 bits" << std::endl;
    std::cout << "Nombre de hashes a generer: " << num_samples << std::endl;
    std::cout << "Bits totaux attendus: " << (num_samples * 256) << std::endl;
    std::cout << "\nNote: AC_HASH est lent, cela peut prendre plusieurs minutes..." << std::endl;
    
    // Test 1: Distribution AC_HASH
    BitDistributionResult ac_result = analyze_bit_distribution_ac_hash(num_samples);
    ac_result.print();
    
    // Test 2: Distribution SHA256 (pour comparaison)
    BitDistributionResult sha_result = analyze_bit_distribution_sha256(num_samples);
    sha_result.print();
    
    // Test 3: Tableau comparatif
    print_comparison_table(ac_result, sha_result);
    
    // Test 4: Analyse par position (optionnel, rapide)
    std::cout << "\nAnalyse supplementaire par position de bit..." << std::endl;
    analyze_bit_positions(100);  // Petit échantillon pour cette analyse
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "         ANALYSE TERMINEE" << std::endl;
    std::cout << "========================================" << std::endl;
    
    return 0;
}