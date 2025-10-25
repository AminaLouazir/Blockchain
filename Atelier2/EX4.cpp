#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <ctime>
#include <chrono>
#include <iomanip>
#include "EX2.h"

// ========== ENUM POUR MODE DE HACHAGE ==========
enum class HashMode {
    SHA256,
    AC_HASH
};

std::string hash_mode_to_string(HashMode mode) {
    return (mode == HashMode::SHA256) ? "SHA256" : "AC_HASH";
}

// ========== STRUCTURE POUR STATISTIQUES ==========
struct MiningStats {
    double total_time_ms;
    long long total_iterations;
    int blocks_mined;
    
    MiningStats() : total_time_ms(0), total_iterations(0), blocks_mined(0) {}
    
    double avg_time_per_block() const {
        return blocks_mined > 0 ? total_time_ms / blocks_mined : 0;
    }
    
    double avg_iterations_per_block() const {
        return blocks_mined > 0 ? (double)total_iterations / blocks_mined : 0;
    }
};

// ========== STRUCTURE BLOC SIMPLIFIÉE ==========
struct Block {
    int index;
    std::string timestamp;
    std::string data;
    std::string previous_hash;
    int nonce;
    std::string hash;
    HashMode hash_mode;
    
    Block(int idx, const std::string& d, const std::string& prev_hash, HashMode mode)
        : index(idx), data(d), previous_hash(prev_hash), nonce(0), hash_mode(mode) {
        timestamp = get_current_timestamp();
    }
    
    std::string get_current_timestamp() {
        time_t now = time(0);
        char buf[80];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
        return std::string(buf);
    }
    
    std::string calculate_hash() {
        std::stringstream ss;
        ss << index << timestamp << data << previous_hash << nonce;
        std::string block_content = ss.str();
        
        if (hash_mode == HashMode::SHA256) {
            return sha256_simple(block_content);
        } else {
            return ac_hash(block_content, 30, 250);
        }
    }
    
    // 4.2. Version modifiée pour compter les itérations
    long long mine_block_with_stats(int difficulty, double& time_taken_ms) {
        std::string target(difficulty, '0');
        long long iterations = 0;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        do {
            nonce++;
            iterations++;
            hash = calculate_hash();
        } while (hash.substr(0, difficulty) != target);
        
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration = end - start;
        time_taken_ms = duration.count();
        
        return iterations;
    }
};

// ========== CLASSE BLOCKCHAIN POUR TESTS ==========
class TestBlockchain {
private:
    std::vector<Block> chain;
    int difficulty;
    HashMode current_hash_mode;
    
public:
    TestBlockchain(int diff, HashMode mode) 
        : difficulty(diff), current_hash_mode(mode) {
        chain.emplace_back(0, "Genesis Block", "0", current_hash_mode);
    }
    
    Block& get_latest_block() {
        return chain.back();
    }
    
    // 4.1. Ajouter un bloc avec statistiques
    void add_block_with_stats(const std::string& data, MiningStats& stats) {
        Block new_block(chain.size(), data, 
                       get_latest_block().hash, 
                       current_hash_mode);
        
        double time_ms;
        long long iterations = new_block.mine_block_with_stats(difficulty, time_ms);
        
        stats.total_time_ms += time_ms;
        stats.total_iterations += iterations;
        stats.blocks_mined++;
        
        chain.push_back(new_block);
        
        std::cout << "Bloc #" << new_block.index 
                  << " - Temps: " << std::fixed << std::setprecision(2) << time_ms << " ms"
                  << " - Iterations: " << iterations << std::endl;
    }
};

// ========== FONCTION DE TEST COMPARATIVE ==========
void compare_hash_methods(int difficulty, int num_blocks) {
    std::cout << "\n========================================" << std::endl;
    std::cout << "  COMPARAISON ac_hash vs SHA256" << std::endl;
    std::cout << "  Difficulte: " << difficulty << std::endl;
    std::cout << "  Nombre de blocs: " << num_blocks << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    MiningStats sha256_stats;
    MiningStats ac_hash_stats;
    
    // Test avec SHA256
    std::cout << "--- Test avec SHA256 ---" << std::endl;
    TestBlockchain bc_sha(difficulty, HashMode::SHA256);
    for (int i = 1; i <= num_blocks; i++) {
        std::stringstream ss;
        ss << "Transaction " << i << ": Test data";
        bc_sha.add_block_with_stats(ss.str(), sha256_stats);
    }
    
    std::cout << "\n--- Test avec AC_HASH ---" << std::endl;
    TestBlockchain bc_ac(difficulty, HashMode::AC_HASH);
    for (int i = 1; i <= num_blocks; i++) {
        std::stringstream ss;
        ss << "Transaction " << i << ": Test data";
        bc_ac.add_block_with_stats(ss.str(), ac_hash_stats);
    }
    
    // 4.3. Affichage des résultats dans un tableau
    std::cout << "\n========================================" << std::endl;
    std::cout << "           RESULTATS" << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    std::cout << std::left << std::setw(25) << "Metrique" 
              << std::setw(20) << "SHA256" 
              << std::setw(20) << "AC_HASH" << std::endl;
    std::cout << std::string(65, '-') << std::endl;
    
    std::cout << std::left << std::setw(25) << "Blocs mines" 
              << std::setw(20) << sha256_stats.blocks_mined
              << std::setw(20) << ac_hash_stats.blocks_mined << std::endl;
    
    std::cout << std::left << std::setw(25) << "Temps total (ms)" 
              << std::setw(20) << std::fixed << std::setprecision(2) << sha256_stats.total_time_ms
              << std::setw(20) << std::fixed << std::setprecision(2) << ac_hash_stats.total_time_ms << std::endl;
    
    std::cout << std::left << std::setw(25) << "Temps moyen/bloc (ms)" 
              << std::setw(20) << std::fixed << std::setprecision(2) << sha256_stats.avg_time_per_block()
              << std::setw(20) << std::fixed << std::setprecision(2) << ac_hash_stats.avg_time_per_block() << std::endl;
    
    std::cout << std::left << std::setw(25) << "Iterations totales" 
              << std::setw(20) << sha256_stats.total_iterations
              << std::setw(20) << ac_hash_stats.total_iterations << std::endl;
    
    std::cout << std::left << std::setw(25) << "Iterations moy/bloc" 
              << std::setw(20) << std::fixed << std::setprecision(0) << sha256_stats.avg_iterations_per_block()
              << std::setw(20) << std::fixed << std::setprecision(0) << ac_hash_stats.avg_iterations_per_block() << std::endl;
    
    std::cout << std::string(65, '-') << std::endl;
    
    // Comparaison relative
    double time_ratio = ac_hash_stats.avg_time_per_block() / sha256_stats.avg_time_per_block();
    double iter_ratio = ac_hash_stats.avg_iterations_per_block() / sha256_stats.avg_iterations_per_block();
    
    std::cout << "\n--- Analyse comparative ---" << std::endl;
    std::cout << "AC_HASH est " << std::fixed << std::setprecision(2) 
              << time_ratio << "x " 
              << (time_ratio > 1 ? "plus lent" : "plus rapide") 
              << " que SHA256" << std::endl;
    
    std::cout << "AC_HASH necessite " << std::fixed << std::setprecision(2) 
              << iter_ratio << "x " 
              << (iter_ratio > 1 ? "plus" : "moins") 
              << " d'iterations que SHA256" << std::endl;
}

// ========== TESTS AVEC DIFFÉRENTES DIFFICULTÉS ==========
void run_comprehensive_tests() {
    std::cout << "\n=======================================" << std::endl;
    std::cout << "  EXERCICE 4 - ANALYSE COMPARATIVE" << std::endl;
    std::cout << "=======================================" << std::endl;
    
    // Test 1: Difficulté faible (2)
    compare_hash_methods(2, 10);
    
    // Test 2: Difficulté moyenne (3)
    std::cout << "\n\n";
    compare_hash_methods(3, 10);
    
    // Test optionnel: Difficulté élevée (4) - Attention, peut être long!
    // compare_hash_methods(4, 5);
}

// ========== MAIN ==========
int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "  DEMARRAGE DES TESTS DE PERFORMANCE" << std::endl;
    std::cout << "========================================" << std::endl;
    
    run_comprehensive_tests();
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "         TESTS TERMINES" << std::endl;
    std::cout << "========================================" << std::endl;
    
    return 0;
}
