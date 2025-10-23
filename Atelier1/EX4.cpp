#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <random>
#include <algorithm>
#include "picosha2.h"

using namespace std;
using namespace chrono;

// ============================================================================
// UTILITAIRES
// ============================================================================

string sha256(const string& data) {
    return picosha2::hash256_hex_string(data);
}

// ============================================================================
// PARTIE 1 : TRANSACTION ET MERKLE TREE
// ============================================================================

class Transaction {
public:
    string id;
    string sender;
    string receiver;
    double amount;
    
    Transaction(string i, string s, string r, double a) 
        : id(i), sender(s), receiver(r), amount(a) {}
    
    string toString() const {
        stringstream ss;
        ss << id << sender << receiver << fixed << setprecision(2) << amount;
        return ss.str();
    }
    
    void display() const {
        cout << "  📄 TX [" << id << "] " << sender << " → " << receiver 
             << " : " << amount << "€" << endl;
    }
};

class MerkleTree {
private:
    vector<string> leaves;
    string root;
    
    string hashPair(const string& left, const string& right) {
        return sha256(left + right);
    }
    
    string buildTree(vector<string> hashes) {
        if(hashes.empty()) return "";
        if(hashes.size() == 1) return hashes[0];
        
        vector<string> newLevel;
        for(size_t i = 0; i < hashes.size(); i += 2) {
            if(i + 1 < hashes.size()) {
                newLevel.push_back(hashPair(hashes[i], hashes[i+1]));
            } else {
                newLevel.push_back(hashPair(hashes[i], hashes[i]));
            }
        }
        return buildTree(newLevel);
    }
    
public:
    MerkleTree(const vector<Transaction>& transactions) {
        for(const auto& tx : transactions) {
            leaves.push_back(sha256(tx.toString()));
        }
        root = buildTree(leaves);
    }
    
    string getRoot() const { return root; }
};

// ============================================================================
// PARTIE 2 : VALIDATEURS (pour PoS)
// ============================================================================

class Validator {
public:
    string name;
    double stake;
    int blocksValidated;
    
    Validator(string n, double s) : name(n), stake(s), blocksValidated(0) {}
    
    void display() const {
        cout << "  👤 " << left << setw(12) << name 
             << " | Stake: " << setw(8) << stake 
             << " | Blocs: " << blocksValidated << endl;
    }
};

// ============================================================================
// PARTIE 3 : BLOCK
// ============================================================================

class Block {
private:
    int index;
    long long timestamp;
    string previousHash;
    string merkleRoot;
    int nonce;
    string hash;
    vector<Transaction> transactions;
    string validatorName;  // Pour PoS
    bool usedPoW;          // true = PoW, false = PoS
    
public:
    Block(int idx, string prevHash, vector<Transaction> txs, bool usePoW = true, string validator = "") 
        : index(idx), previousHash(prevHash), transactions(txs), 
          nonce(0), usedPoW(usePoW), validatorName(validator) {
        
        timestamp = duration_cast<milliseconds>(
            system_clock::now().time_since_epoch()
        ).count();
        
        // Calculer le Merkle Root
        MerkleTree tree(transactions);
        merkleRoot = tree.getRoot();
        
        hash = calculateHash();
    }
    
    string calculateHash() const {
        stringstream ss;
        ss << index << timestamp << previousHash << merkleRoot << nonce << validatorName;
        return sha256(ss.str());
    }
    
    // Proof of Work
    void mineBlock(int difficulty) {
        string target(difficulty, '0');
        
        auto start = high_resolution_clock::now();
        
        while(hash.substr(0, difficulty) != target) {
            nonce++;
            hash = calculateHash();
        }
        
        auto end = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(end - start);
        
        cout << "  ⛏️  Bloc #" << index << " miné (PoW) - Nonce: " << nonce 
             << " - Temps: " << duration.count() << " ms" << endl;
    }
    
    // Getters
    string getHash() const { return hash; }
    string getPreviousHash() const { return previousHash; }
    string getMerkleRoot() const { return merkleRoot; }
    int getIndex() const { return index; }
    bool isPoW() const { return usedPoW; }
    string getValidator() const { return validatorName; }
    const vector<Transaction>& getTransactions() const { return transactions; }
    
    void display() const {
        cout << "\n┌─────────────────────────────────────────────────────────┐" << endl;
        cout << "│ 🔷 Block #" << index << setw(45) << "│" << endl;
        cout << "├─────────────────────────────────────────────────────────┤" << endl;
        cout << "│ Consensus: " << left << setw(44) << (usedPoW ? "Proof of Work (PoW)" : "Proof of Stake (PoS)") << "│" << endl;
        if(!usedPoW) {
            cout << "│ Validateur: " << setw(43) << validatorName << "│" << endl;
        } else {
            cout << "│ Nonce: " << setw(48) << nonce << "│" << endl;
        }
        cout << "│ Transactions: " << setw(41) << transactions.size() << "│" << endl;
        cout << "│ Merkle Root: " << merkleRoot.substr(0, 32) << "..." << setw(9) << "│" << endl;
        cout << "│ Hash: " << hash.substr(0, 32) << "..." << setw(16) << "│" << endl;
        cout << "└─────────────────────────────────────────────────────────┘" << endl;
        
        for(const auto& tx : transactions) {
            tx.display();
        }
    }
};

// ============================================================================
// PARTIE 4 : BLOCKCHAIN
// ============================================================================

class Blockchain {
private:
    vector<Block> chain;
    vector<Validator> validators;
    int difficulty;
    mt19937 rng;
    
    // Statistiques
    long long totalPoWTime;
    long long totalPoSTime;
    int powBlocks;
    int posBlocks;
    
    Validator& selectValidator() {
        double totalStake = 0;
        for(const auto& v : validators) {
            totalStake += v.stake;
        }
        
        uniform_real_distribution<double> dist(0, totalStake);
        double randomValue = dist(rng);
        
        double cumulativeStake = 0;
        for(auto& v : validators) {
            cumulativeStake += v.stake;
            if(randomValue <= cumulativeStake) {
                return v;
            }
        }
        
        return validators[0];
    }
    
public:
    Blockchain(int diff = 3) : difficulty(diff), totalPoWTime(0), totalPoSTime(0), 
                                 powBlocks(0), posBlocks(0) {
        rng.seed(time(nullptr));
        
        // Initialiser les validateurs
        validators.push_back(Validator("Alice", 1000));
        validators.push_back(Validator("Bob", 500));
        validators.push_back(Validator("Charlie", 300));
        validators.push_back(Validator("Dave", 200));
        
        // Bloc Genesis
        vector<Transaction> genesisTx;
        genesisTx.push_back(Transaction("0", "System", "Network", 0));
        
        Block genesis(0, "0", genesisTx, true, "");
        chain.push_back(genesis);
        
        cout << "🔗 Blockchain initialisée (Difficulté PoW: " << difficulty << ")" << endl;
    }
    
    Block getLastBlock() const {
        return chain.back();
    }
    
    // Ajouter un bloc avec PoW
    void addBlockPoW(vector<Transaction> transactions) {
        cout << "\n📦 Ajout d'un bloc avec Proof of Work..." << endl;
        
        auto start = high_resolution_clock::now();
        
        Block newBlock(chain.size(), getLastBlock().getHash(), transactions, true, "");
        newBlock.mineBlock(difficulty);
        
        auto end = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(end - start);
        
        chain.push_back(newBlock);
        totalPoWTime += duration.count();
        powBlocks++;
        
        cout << "  ✅ Bloc ajouté avec succès" << endl;
    }
    
    // Ajouter un bloc avec PoS
    void addBlockPoS(vector<Transaction> transactions) {
        cout << "\n📦 Ajout d'un bloc avec Proof of Stake..." << endl;
        
        auto start = high_resolution_clock::now();
        
        Validator& validator = selectValidator();
        cout << "  🎲 Validateur sélectionné: " << validator.name 
             << " (Stake: " << validator.stake << ")" << endl;
        
        Block newBlock(chain.size(), getLastBlock().getHash(), transactions, false, validator.name);
        
        auto end = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(end - start);
        
        chain.push_back(newBlock);
        validator.blocksValidated++;
        totalPoSTime += duration.count();
        posBlocks++;
        
        cout << "  ✅ Bloc validé en " << duration.count() << " ms" << endl;
    }
    
    bool isChainValid() const {
        for(size_t i = 1; i < chain.size(); i++) {
            const Block& current = chain[i];
            const Block& previous = chain[i-1];
            
            // Vérifier le hash
            if(current.getHash() != current.calculateHash()) {
                return false;
            }
            
            // Vérifier le lien
            if(current.getPreviousHash() != previous.getHash()) {
                return false;
            }
            
            // Vérifier la difficulté pour PoW
            if(current.isPoW()) {
                string target(difficulty, '0');
                if(current.getHash().substr(0, difficulty) != target) {
                    return false;
                }
            }
        }
        return true;
    }
    
    void displayChain() const {
        cout << "\n╔═════════════════════════════════════════════════════════╗" << endl;
        cout << "║            CONTENU DE LA BLOCKCHAIN                     ║" << endl;
        cout << "╚═════════════════════════════════════════════════════════╝" << endl;
        
        for(const auto& block : chain) {
            block.display();
        }
    }
    
    void displayStats() const {
        cout << "\n╔═════════════════════════════════════════════════════════╗" << endl;
        cout << "║          ANALYSE COMPARATIVE PoW vs PoS                 ║" << endl;
        cout << "╠═════════════════════════════════════════════════════════╣" << endl;
        cout << "║                                                          ║" << endl;
        
        cout << "║  📊 STATISTIQUES GÉNÉRALES                              ║" << endl;
        cout << "║  ─────────────────────────────────────────────────────  ║" << endl;
        cout << "║  Nombre total de blocs: " << setw(30) << chain.size() << " ║" << endl;
        cout << "║  Blocs PoW: " << setw(42) << powBlocks << " ║" << endl;
        cout << "║  Blocs PoS: " << setw(42) << posBlocks << " ║" << endl;
        cout << "║                                                          ║" << endl;
        
        if(powBlocks > 0) {
            double avgPoW = (double)totalPoWTime / powBlocks;
            cout << "║  ⛏️  PROOF OF WORK                                      ║" << endl;
            cout << "║  ─────────────────────────────────────────────────────  ║" << endl;
            cout << "║  Temps total: " << setw(38) << totalPoWTime << " ms ║" << endl;
            cout << "║  Temps moyen par bloc: " << setw(29) << fixed << setprecision(2) << avgPoW << " ms ║" << endl;
            cout << "║  Difficulté: " << setw(41) << difficulty << " ║" << endl;
            cout << "║                                                          ║" << endl;
        }
        
        if(posBlocks > 0) {
            double avgPoS = (double)totalPoSTime / posBlocks;
            cout << "║  🎲 PROOF OF STAKE                                      ║" << endl;
            cout << "║  ─────────────────────────────────────────────────────  ║" << endl;
            cout << "║  Temps total: " << setw(38) << totalPoSTime << " ms ║" << endl;
            cout << "║  Temps moyen par bloc: " << setw(29) << fixed << setprecision(2) << avgPoS << " ms ║" << endl;
            cout << "║                                                          ║" << endl;
        }
        
        if(powBlocks > 0 && posBlocks > 0) {
            double avgPoW = (double)totalPoWTime / powBlocks;
            double avgPoS = (double)totalPoSTime / posBlocks;
            double speedup = avgPoW / avgPoS;
            
            cout << "║  📈 COMPARAISON                                          ║" << endl;
            cout << "║  ─────────────────────────────────────────────────────  ║" << endl;
            cout << "║  PoS est " << setw(40) << fixed << setprecision(0) << speedup << "x plus rapide ║" << endl;
            cout << "║  Consommation CPU: PoW >> PoS                           ║" << endl;
            cout << "║  Facilité d'implémentation: PoS > PoW                   ║" << endl;
            cout << "║                                                          ║" << endl;
        }
        
        cout << "║  👥 VALIDATEURS                                          ║" << endl;
        cout << "║  ─────────────────────────────────────────────────────  ║" << endl;
        for(const auto& v : validators) {
            cout << "║  ";
            v.display();
        }
        cout << "║                                                          ║" << endl;
        cout << "╚═════════════════════════════════════════════════════════╝" << endl;
    }
    
    int getChainLength() const { return chain.size(); }
};

// ============================================================================
// MAIN : TESTS ET DÉMONSTRATIONS
// ============================================================================

int main() {
    cout << "╔═══════════════════════════════════════════════════════════╗" << endl;
    cout << "║   EXERCICE 4 : MINI-BLOCKCHAIN COMPLÈTE FROM SCRATCH     ║" << endl;
    cout << "╚═══════════════════════════════════════════════════════════╝\n" << endl;
    
    // Créer la blockchain
    Blockchain blockchain(3);
    
    // ========== PARTIE 2 : Ajouter des blocs avec PoW ==========
    cout << "\n\n" << string(65, '=') << endl;
    cout << "PARTIE 2 : Ajout de blocs avec Proof of Work" << endl;
    cout << string(65, '=') << endl;
    
    vector<Transaction> tx1;
    tx1.push_back(Transaction("tx001", "Alice", "Bob", 100.0));
    tx1.push_back(Transaction("tx002", "Bob", "Charlie", 50.0));
    blockchain.addBlockPoW(tx1);
    
    vector<Transaction> tx2;
    tx2.push_back(Transaction("tx003", "Charlie", "Dave", 25.0));
    tx2.push_back(Transaction("tx004", "Dave", "Alice", 10.0));
    blockchain.addBlockPoW(tx2);
    
    vector<Transaction> tx3;
    tx3.push_back(Transaction("tx005", "Alice", "Charlie", 75.0));
    blockchain.addBlockPoW(tx3);
    
    // ========== PARTIE 3 : Ajouter des blocs avec PoS ==========
    cout << "\n\n" << string(65, '=') << endl;
    cout << "PARTIE 3 : Ajout de blocs avec Proof of Stake" << endl;
    cout << string(65, '=') << endl;
    
    vector<Transaction> tx4;
    tx4.push_back(Transaction("tx006", "Bob", "Dave", 30.0));
    tx4.push_back(Transaction("tx007", "Charlie", "Alice", 40.0));
    blockchain.addBlockPoS(tx4);
    
    vector<Transaction> tx5;
    tx5.push_back(Transaction("tx008", "Dave", "Bob", 15.0));
    blockchain.addBlockPoS(tx5);
    
    vector<Transaction> tx6;
    tx6.push_back(Transaction("tx009", "Alice", "Dave", 60.0));
    tx6.push_back(Transaction("tx010", "Bob", "Charlie", 20.0));
    blockchain.addBlockPoS(tx6);
    
    // Afficher la blockchain complète
    blockchain.displayChain();
    
    // Vérifier l'intégrité
    cout << "\n\n" << string(65, '=') << endl;
    cout << "VÉRIFICATION DE L'INTÉGRITÉ" << endl;
    cout << string(65, '=') << endl;
    cout << (blockchain.isChainValid() ? "✅ La blockchain est VALIDE" : "❌ La blockchain est INVALIDE") << endl;
    
    // ========== PARTIE 4 : Analyse comparative ==========
    blockchain.displayStats();
    
    cout << "\n\n╔═══════════════════════════════════════════════════════════╗" << endl;
    cout << "║                  CONCLUSIONS                              ║" << endl;
    cout << "╠═══════════════════════════════════════════════════════════╣" << endl;
    cout << "║                                                            ║" << endl;
    cout << "║  ✅ Arbre de Merkle: Résume efficacement les transactions ║" << endl;
    cout << "║  ✅ Proof of Work: Sécurise la blockchain mais lent       ║" << endl;
    cout << "║  ✅ Proof of Stake: Rapide et économe en énergie          ║" << endl;
    cout << "║  ✅ Blockchain: Intégrité vérifiée avec succès            ║" << endl;
    cout << "║                                                            ║" << endl;
    cout << "╚═══════════════════════════════════════════════════════════╝\n" << endl;
    
    return 0;
}
