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

// Fonction pour calculer le hash SHA256
string sha256(const string& data) {
    return picosha2::hash256_hex_string(data);
}

// Classe Validator (Validateur)
class Validator {
public:
    string name;
    double stake;  // Montant misé
    int blocksValidated;
    
    Validator(string n, double s) : name(n), stake(s), blocksValidated(0) {}
    
    void display() const {
        cout << "  👤 " << left << setw(15) << name 
             << " | Stake: " << setw(8) << stake << " coins"
             << " | Blocs validés: " << blocksValidated << endl;
    }
};

// Classe Block pour PoS
class BlockPoS {
private:
    int index;
    string previousHash;
    string data;
    long long timestamp;
    string hash;
    string validatorName;
    
public:
    BlockPoS(int idx, string prevHash, string d, string validator) 
        : index(idx), previousHash(prevHash), data(d), validatorName(validator) {
        timestamp = duration_cast<milliseconds>(
            system_clock::now().time_since_epoch()
        ).count();
        hash = calculateHash();
    }
    
    string calculateHash() const {
        stringstream ss;
        ss << index << previousHash << data << timestamp << validatorName;
        return sha256(ss.str());
    }
    
    // Getters
    string getHash() const { return hash; }
    int getIndex() const { return index; }
    string getPreviousHash() const { return previousHash; }
    string getData() const { return data; }
    string getValidator() const { return validatorName; }
    
    void display() const {
        cout << "\n┌─────────────────────────────────────────────────────┐" << endl;
        cout << "│ Block #" << index << setw(44) << "│" << endl;
        cout << "├─────────────────────────────────────────────────────┤" << endl;
        cout << "│ Données: " << left << setw(42) << data.substr(0, 42) << "│" << endl;
        cout << "│ Validateur: " << setw(39) << validatorName.substr(0, 39) << "│" << endl;
        cout << "│ Hash: " << hash.substr(0, 32) << "..." << setw(15) << "│" << endl;
        cout << "│ Timestamp: " << setw(40) << timestamp << "│" << endl;
        cout << "└─────────────────────────────────────────────────────┘" << endl;
    }
};

// Classe BlockchainPoS
class BlockchainPoS {
private:
    vector<BlockPoS> chain;
    vector<Validator> validators;
    mt19937 rng;
    
    // Sélection du validateur basée sur le stake (pondéré)
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
        
        return validators[0]; // Fallback
    }
    
public:
    BlockchainPoS() {
        rng.seed(time(nullptr));
        
        cout << "\n🔗 Création de la Blockchain avec Proof of Stake" << endl;
        
        // Initialiser les validateurs
        validators.push_back(Validator("Alice", 1000));
        validators.push_back(Validator("Bob", 500));
        validators.push_back(Validator("Charlie", 300));
        validators.push_back(Validator("Dave", 200));
        
        cout << "\n📋 Validateurs initiaux:" << endl;
        for(const auto& v : validators) {
            v.display();
        }
        
        // Créer le bloc Genesis
        BlockPoS genesis(0, "0", "Genesis Block", "System");
        chain.push_back(genesis);
        cout << "\n✅ Bloc Genesis créé" << endl;
    }
    
    BlockPoS getLastBlock() const {
        return chain.back();
    }
    
    void addBlock(string data) {
        auto start = high_resolution_clock::now();
        
        // Sélectionner un validateur
        Validator& selectedValidator = selectValidator();
        
        cout << "\n🎲 Validateur sélectionné: " << selectedValidator.name 
             << " (Stake: " << selectedValidator.stake << " coins)" << endl;
        
        // Créer le nouveau bloc
        BlockPoS newBlock(chain.size(), getLastBlock().getHash(), data, selectedValidator.name);
        
        auto end = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(end - start);
        
        chain.push_back(newBlock);
        selectedValidator.blocksValidated++;
        
        cout << "✅ Bloc ajouté en " << duration.count() << " μs (microsecondes)" << endl;
    }
    
    bool isChainValid() const {
        for(size_t i = 1; i < chain.size(); i++) {
            const BlockPoS& currentBlock = chain[i];
            const BlockPoS& previousBlock = chain[i-1];
            
            if(currentBlock.getHash() != currentBlock.calculateHash()) {
                cout << "❌ Hash invalide pour le bloc " << i << endl;
                return false;
            }
            
            if(currentBlock.getPreviousHash() != previousBlock.getHash()) {
                cout << "❌ Chaîne brisée au bloc " << i << endl;
                return false;
            }
        }
        return true;
    }
    
    void displayChain() const {
        cout << "\n╔═══════════════════════════════════════════════════════╗" << endl;
        cout << "║         CONTENU DE LA BLOCKCHAIN (PoS)                ║" << endl;
        cout << "╚═══════════════════════════════════════════════════════╝" << endl;
        
        for(const auto& block : chain) {
            block.display();
        }
    }
    
    void displayValidatorStats() const {
        cout << "\n╔═══════════════════════════════════════════════════════╗" << endl;
        cout << "║         STATISTIQUES DES VALIDATEURS                  ║" << endl;
        cout << "╠═══════════════════════════════════════════════════════╣" << endl;
        
        for(const auto& v : validators) {
            v.display();
        }
        
        cout << "╚═══════════════════════════════════════════════════════╝" << endl;
    }
    
    int getChainLength() const { return chain.size(); }
};

// Classe Block pour PoW (pour comparaison)
class BlockPoW {
private:
    int index;
    string previousHash;
    string data;
    long long timestamp;
    int nonce;
    string hash;
    
public:
    BlockPoW(int idx, string prevHash, string d) 
        : index(idx), previousHash(prevHash), data(d), nonce(0) {
        timestamp = duration_cast<milliseconds>(
            system_clock::now().time_since_epoch()
        ).count();
        hash = calculateHash();
    }
    
    string calculateHash() const {
        stringstream ss;
        ss << index << previousHash << data << timestamp << nonce;
        return sha256(ss.str());
    }
    
    void mineBlock(int difficulty) {
        string target(difficulty, '0');
        while(hash.substr(0, difficulty) != target) {
            nonce++;
            hash = calculateHash();
        }
    }
    
    string getHash() const { return hash; }
};

// Classe BlockchainPoW (pour comparaison)
class BlockchainPoW {
private:
    vector<BlockPoW> chain;
    int difficulty;
    
public:
    BlockchainPoW(int diff) : difficulty(diff) {
        BlockPoW genesis(0, "0", "Genesis Block");
        genesis.mineBlock(difficulty);
        chain.push_back(genesis);
    }
    
    void addBlock(string data) {
        BlockPoW newBlock(chain.size(), chain.back().getHash(), data);
        newBlock.mineBlock(difficulty);
        chain.push_back(newBlock);
    }
};

// Fonction de comparaison PoW vs PoS
void comparePoWvsPoS() {
    cout << "\n\n" << string(70, '=') << endl;
    cout << "COMPARAISON : Proof of Work vs Proof of Stake" << endl;
    cout << string(70, '=') << endl;
    
    int numBlocks = 10;
    int difficulty = 4;
    
    // Test PoW
    cout << "\n>>> Test avec Proof of Work (difficulté " << difficulty << ") <<<" << endl;
    auto startPoW = high_resolution_clock::now();
    
    BlockchainPoW blockchainPoW(difficulty);
    for(int i = 1; i <= numBlocks; i++) {
        cout << "Mining bloc " << i << "..." << endl;
        blockchainPoW.addBlock("Transaction " + to_string(i));
    }
    
    auto endPoW = high_resolution_clock::now();
    auto durationPoW = duration_cast<milliseconds>(endPoW - startPoW);
    
    cout << "✅ PoW terminé en " << durationPoW.count() << " ms" << endl;
    
    // Test PoS
    cout << "\n>>> Test avec Proof of Stake <<<" << endl;
    auto startPoS = high_resolution_clock::now();
    
    BlockchainPoS blockchainPoS;
    for(int i = 1; i <= numBlocks; i++) {
        blockchainPoS.addBlock("Transaction " + to_string(i));
    }
    
    auto endPoS = high_resolution_clock::now();
    auto durationPoS = duration_cast<milliseconds>(endPoS - startPoS);
    
    cout << "\n✅ PoS terminé en " << durationPoS.count() << " ms" << endl;
    
    // Afficher les résultats
    cout << "\n\n╔═══════════════════════════════════════════════════════╗" << endl;
    cout << "║         RÉSULTATS DE LA COMPARAISON                   ║" << endl;
    cout << "╠═══════════════════════════════════════════════════════╣" << endl;
    cout << "║                                                        ║" << endl;
    cout << "║  Méthode          │ Temps          │ Vitesse relative ║" << endl;
    cout << "║  ════════════════╪════════════════╪═════════════════ ║" << endl;
    
    cout << "║  Proof of Work   │ " << setw(10) << durationPoW.count() << " ms │ ";
    cout << "1x (référence)   ║" << endl;
    
    cout << "║  Proof of Stake  │ " << setw(10) << durationPoS.count() << " ms │ ";
    double speedup = (double)durationPoW.count() / durationPoS.count();
    cout << setw(7) << fixed << setprecision(0) << speedup << "x plus rapide ║" << endl;
    
    cout << "║                                                        ║" << endl;
    cout << "╚═══════════════════════════════════════════════════════╝" << endl;
    
    cout << "\n📊 Analyse:" << endl;
    cout << "✅ PoS est environ " << speedup << "x plus rapide que PoW" << endl;
    cout << "✅ PoS consomme beaucoup moins de ressources CPU" << endl;
    cout << "✅ PoS ne nécessite pas de calculs intensifs" << endl;
    cout << "⚠️  PoS nécessite un mécanisme de sélection équitable" << endl;
    
    // Afficher les stats des validateurs
    blockchainPoS.displayValidatorStats();
}

int main() {
    cout << "╔════════════════════════════════════════════════════════╗" << endl;
    cout << "║          EXERCICE 3 : PROOF OF STAKE (PoS)            ║" << endl;
    cout << "╚════════════════════════════════════════════════════════╝" << endl;
    
    // ========== EXEMPLE 1 : Création d'une blockchain PoS ==========
    cout << "\n\n" << string(60, '=') << endl;
    cout << "EXEMPLE 1 : Création d'une blockchain avec PoS" << endl;
    cout << string(60, '=') << endl;
    
    BlockchainPoS blockchain;
    
    cout << "\n--- Ajout de blocs ---" << endl;
    blockchain.addBlock("Transaction: Alice -> Bob 100€");
    blockchain.addBlock("Transaction: Bob -> Charlie 50€");
    blockchain.addBlock("Transaction: Charlie -> Dave 25€");
    blockchain.addBlock("Transaction: Dave -> Alice 10€");
    blockchain.addBlock("Transaction: Alice -> Charlie 75€");
    
    blockchain.displayChain();
    
    // Vérifier l'intégrité
    cout << "\n\n" << string(60, '=') << endl;
    cout << "Vérification de l'intégrité de la blockchain" << endl;
    cout << string(60, '=') << endl;
    cout << (blockchain.isChainValid() ? "✅ La blockchain est VALIDE" : "❌ La blockchain est INVALIDE") << endl;
    
    // Afficher les statistiques des validateurs
    blockchain.displayValidatorStats();
    
    // ========== EXEMPLE 2 : Comparaison PoW vs PoS ==========
    comparePoWvsPoS();
    
    // ========== EXEMPLE 3 : Probabilité de sélection ==========
    cout << "\n\n" << string(60, '=') << endl;
    cout << "EXEMPLE 3 : Test de probabilité de sélection" << endl;
    cout << string(60, '=') << endl;
    
    cout << "\nAjout de 50 blocs pour analyser la distribution..." << endl;
    BlockchainPoS testChain;
    
    for(int i = 0; i < 50; i++) {
        testChain.addBlock("Test transaction " + to_string(i));
    }
    
    testChain.displayValidatorStats();
    
    cout << "\n💡 Observation:" << endl;
    cout << "Les validateurs avec plus de stake sont sélectionnés plus souvent," << endl;
    cout << "ce qui est cohérent avec le principe du Proof of Stake." << endl;
    
    cout << "\n\n╔════════════════════════════════════════════════════════╗" << endl;
    cout << "║            FIN DE L'EXERCICE 3                         ║" << endl;
    cout << "╚════════════════════════════════════════════════════════╝\n" << endl;
    
    return 0;
}
