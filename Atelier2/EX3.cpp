#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <ctime>
#include "EX2.h"

// ========== ENUM POUR MODE DE HACHAGE ==========
enum class HashMode {
    SHA256,
    AC_HASH
};

std::string hash_mode_to_string(HashMode mode) {
    return (mode == HashMode::SHA256) ? "SHA256" : "AC_HASH";
}

// ========== STRUCTURE BLOC ==========
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
        hash = calculate_hash();
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
    
    void mine_block(int difficulty) {
        std::string target(difficulty, '0');
        
        std::cout << "Minage du bloc #" << index << " en cours";
        int progress = 0;
        do {
            nonce++;
            hash = calculate_hash();
            
            // Affichage de progression tous les 10000 essais
            if (nonce % 10000 == 0) {
                std::cout << "." << std::flush;
                progress++;
                
                // Limite de sécurité pour éviter boucle infinie
                if (progress > 1000) {
                    std::cout << "\n[ERREUR] Trop d'iterations, hash invalide!" << std::endl;
                    return;
                }
            }
        } while (hash.substr(0, difficulty) != target);
        
        std::cout << " OK!" << std::endl;
        std::cout << "Hash: " << hash << std::endl;
        std::cout << "Nonce: " << nonce << std::endl;
    }
    
    void print() const {
        std::cout << "\n========== BLOC #" << index << " ==========" << std::endl;
        std::cout << "Mode: " << hash_mode_to_string(hash_mode) << std::endl;
        std::cout << "Timestamp: " << timestamp << std::endl;
        std::cout << "Data: " << data << std::endl;
        std::cout << "Previous Hash: " << previous_hash.substr(0, 16) << "..." << std::endl;
        std::cout << "Hash: " << hash.substr(0, 16) << "..." << std::endl;
        std::cout << "Nonce: " << nonce << std::endl;
        std::cout << "================================" << std::endl;
    }
};

// ========== CLASSE BLOCKCHAIN ==========
class Blockchain {
private:
    std::vector<Block> chain;
    int difficulty;
    HashMode current_hash_mode;
    
public:
    Blockchain(int diff = 2, HashMode mode = HashMode::SHA256) 
        : difficulty(diff), current_hash_mode(mode) {
        chain.emplace_back(0, "Genesis Block", "0", current_hash_mode);
        std::cout << "Blockchain initialisee avec " 
                  << hash_mode_to_string(current_hash_mode) 
                  << " (difficulte: " << difficulty << ")" << std::endl;
    }
    
    void set_hash_mode(HashMode mode) {
        current_hash_mode = mode;
        std::cout << "Mode de hachage change en: " 
                  << hash_mode_to_string(mode) << std::endl;
    }
    
    HashMode get_hash_mode() const {
        return current_hash_mode;
    }
    
    Block& get_latest_block() {
        return chain.back();
    }
    
    void add_block(const std::string& data) {
        Block new_block(chain.size(), data, 
                       get_latest_block().hash, 
                       current_hash_mode);
        
        new_block.mine_block(difficulty);
        chain.push_back(new_block);
    }
    
    bool is_chain_valid() {
        for (size_t i = 1; i < chain.size(); ++i) {
            Block& current = chain[i];
            Block& previous = chain[i - 1];
            
            if (current.hash != current.calculate_hash()) {
                std::cout << "Erreur: Hash invalide pour le bloc #" 
                         << current.index << std::endl;
                return false;
            }
            
            if (current.previous_hash != previous.hash) {
                std::cout << "Erreur: Chainage rompu entre blocs #" 
                         << previous.index << " et #" << current.index << std::endl;
                return false;
            }
            
            std::string target(difficulty, '0');
            if (current.hash.substr(0, difficulty) != target) {
                std::cout << "Erreur: Preuve de travail invalide pour le bloc #" 
                         << current.index << std::endl;
                return false;
            }
        }
        return true;
    }
    
    void print_chain() {
        std::cout << "\n========== BLOCKCHAIN ==========" << std::endl;
        std::cout << "Difficulte: " << difficulty << std::endl;
        std::cout << "Nombre de blocs: " << chain.size() << std::endl;
        
        for (const auto& block : chain) {
            block.print();
        }
    }
};

// ========== FONCTIONS DE TEST ==========
void test_sha256_blockchain() {
    std::cout << "\n=== TEST BLOCKCHAIN AVEC SHA256 ===" << std::endl;
    Blockchain bc(2, HashMode::SHA256);
    bc.add_block("Transaction 1: Alice -> Bob 50 BTC");
    bc.add_block("Transaction 2: Bob -> Charlie 30 BTC");
    
    std::cout << "\nValidation: " 
              << (bc.is_chain_valid() ? "VALIDE ✓" : "INVALIDE ✗") << std::endl;
}

void test_ac_hash_blockchain() {
    std::cout << "\n=== TEST BLOCKCHAIN AVEC AC_HASH ===" << std::endl;
    Blockchain bc(2, HashMode::AC_HASH);
    bc.add_block("Transaction 1: Alice -> Bob 50 BTC");
    bc.add_block("Transaction 2: Bob -> Charlie 30 BTC");
    
    bc.print_chain();
    
    std::cout << "\nValidation: " 
              << (bc.is_chain_valid() ? "VALIDE ✓" : "INVALIDE ✗") << std::endl;
}

void test_mixed_blockchain() {
    std::cout << "\n=== TEST BLOCKCHAIN MIXTE ===" << std::endl;
    Blockchain bc(2, HashMode::SHA256);
    bc.add_block("Bloc 1 avec SHA256");
    
    bc.set_hash_mode(HashMode::AC_HASH);
    bc.add_block("Bloc 2 avec AC_HASH");
    bc.add_block("Bloc 3 avec AC_HASH");
    
    bc.print_chain();
    
    std::cout << "\nValidation: " 
              << (bc.is_chain_valid() ? "VALIDE ✓" : "INVALIDE ✗") << std::endl;
}

// ========== MAIN ==========
int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "  EXERCICE 3 - BLOCKCHAIN" << std::endl;
    std::cout << "  Integration AC_HASH" << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    // Test 1: Blockchain SHA256
    test_sha256_blockchain();
    
    // Test 2: Blockchain AC_HASH (commenté par défaut car lent)
    // test_ac_hash_blockchain();
    
    // Test 3: Blockchain mixte (commenté par défaut)
    // test_mixed_blockchain();
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "         TOUS LES TESTS TERMINES" << std::endl;
    std::cout << "========================================" << std::endl;
    
    return 0;
}