#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <chrono>
#include "picosha2.h"

using namespace std;
using namespace chrono;

// Fonction pour calculer le hash SHA256
string sha256(const string& data) {
    return picosha2::hash256_hex_string(data);
}

// Classe Block pour Proof of Work
class Block {
private:
    int index;
    string previousHash;
    string data;
    long long timestamp;
    int nonce;
    string hash;
    
public:
    Block(int idx, string prevHash, string d) 
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
    
    // Proof of Work : Miner le bloc
    void mineBlock(int difficulty) {
        string target(difficulty, '0');
        
        cout << "\n🔨 Mining block " << index << " avec difficulté " << difficulty << "..." << endl;
        cout << "Recherche d'un hash commençant par: " << target << endl;
        
        auto start = high_resolution_clock::now();
        
        while(hash.substr(0, difficulty) != target) {
            nonce++;
            hash = calculateHash();
            
            // Afficher progression tous les 100000 essais
            if(nonce % 100000 == 0) {
                cout << "  Nonce: " << nonce << " - Hash: " << hash.substr(0, 20) << "..." << endl;
            }
        }
        
        auto end = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(end - start);
        
        cout << "✅ Block miné!" << endl;
        cout << "  Nonce trouvé: " << nonce << endl;
        cout << "  Hash: " << hash << endl;
        cout << "  Temps d'exécution: " << duration.count() << " ms" << endl;
    }
    
    // Getters
    string getHash() const { return hash; }
    int getIndex() const { return index; }
    int getNonce() const { return nonce; }
    string getPreviousHash() const { return previousHash; }
    string getData() const { return data; }
    
    void display() const {
        cout << "\n┌─────────────────────────────────────────────────────┐" << endl;
        cout << "│ Block #" << index << setw(44) << "│" << endl;
        cout << "├─────────────────────────────────────────────────────┤" << endl;
        cout << "│ Données: " << left << setw(42) << data.substr(0, 42) << "│" << endl;
        cout << "│ Hash précédent: " << previousHash.substr(0, 32) << "..." << setw(2) << "│" << endl;
        cout << "│ Hash: " << hash.substr(0, 32) << "..." << setw(15) << "│" << endl;
        cout << "│ Nonce: " << right << setw(44) << nonce << "│" << endl;
        cout << "│ Timestamp: " << setw(40) << timestamp << "│" << endl;
        cout << "└─────────────────────────────────────────────────────┘" << endl;
    }
};

// Classe Blockchain
class Blockchain {
private:
    vector<Block> chain;
    int difficulty;
    
public:
    Blockchain(int diff = 2) : difficulty(diff) {
        // Créer le bloc Genesis
        cout << "\n🔗 Création de la Blockchain avec difficulté " << difficulty << endl;
        Block genesis(0, "0", "Genesis Block");
        genesis.mineBlock(difficulty);
        chain.push_back(genesis);
    }
    
    Block getLastBlock() const {
        return chain.back();
    }
    
    void addBlock(string data) {
        Block newBlock(chain.size(), getLastBlock().getHash(), data);
        newBlock.mineBlock(difficulty);
        chain.push_back(newBlock);
    }
    
    bool isChainValid() const {
        for(size_t i = 1; i < chain.size(); i++) {
            const Block& currentBlock = chain[i];
            const Block& previousBlock = chain[i-1];
            
            // Vérifier que le hash est correct
            if(currentBlock.getHash() != currentBlock.calculateHash()) {
                cout << "❌ Hash invalide pour le bloc " << i << endl;
                return false;
            }
            
            // Vérifier la liaison avec le bloc précédent
            if(currentBlock.getPreviousHash() != previousBlock.getHash()) {
                cout << "❌ Chaîne brisée au bloc " << i << endl;
                return false;
            }
            
            // Vérifier la difficulté
            string target(difficulty, '0');
            if(currentBlock.getHash().substr(0, difficulty) != target) {
                cout << "❌ Difficulté non respectée pour le bloc " << i << endl;
                return false;
            }
        }
        return true;
    }
    
    void displayChain() const {
        cout << "\n╔═══════════════════════════════════════════════════════╗" << endl;
        cout << "║              CONTENU DE LA BLOCKCHAIN                 ║" << endl;
        cout << "╚═══════════════════════════════════════════════════════╝" << endl;
        
        for(const auto& block : chain) {
            block.display();
        }
    }
    
    int getChainLength() const { return chain.size(); }
};

// Test de différentes difficultés
void testDifficulties() {
    cout << "\n\n" << string(70, '=') << endl;
    cout << "TEST : Comparaison des temps de minage selon la difficulté" << endl;
    cout << string(70, '=') << endl;
    
    vector<int> difficulties = {1, 2, 3, 4, 5};
    vector<long long> times;
    
    for(int diff : difficulties) {
        cout << "\n\n>>> DIFFICULTÉ : " << diff << " <<<" << endl;
        
        auto start = high_resolution_clock::now();
        
        Block testBlock(1, "0000000000000000", "Test block");
        testBlock.mineBlock(diff);
        
        auto end = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(end - start);
        times.push_back(duration.count());
    }
    
    // Afficher le tableau récapitulatif
    cout << "\n\n╔═══════════════════════════════════════════════════════╗" << endl;
    cout << "║         TABLEAU RÉCAPITULATIF DES TEMPS              ║" << endl;
    cout << "╠═══════════════════════════════════════════════════════╣" << endl;
    cout << "║ Difficulté │ Temps (ms) │ Facteur multiplicatif    ║" << endl;
    cout << "╠════════════╪════════════╪══════════════════════════╣" << endl;
    
    for(size_t i = 0; i < difficulties.size(); i++) {
        cout << "║ " << setw(10) << difficulties[i] << " │ ";
        cout << setw(10) << times[i] << " │ ";
        if(i == 0) {
            cout << setw(24) << "référence" << " ║" << endl;
        } else {
            double factor = (double)times[i] / times[0];
            cout << "x" << setw(23) << fixed << setprecision(2) << factor << " ║" << endl;
        }
    }
    cout << "╚════════════╧════════════╧══════════════════════════╝" << endl;
    
    cout << "\n📊 Analyse:" << endl;
    cout << "- Chaque augmentation de difficulté multiplie le temps par ~16" << endl;
    cout << "- Ceci est dû au fait que chaque zéro supplémentaire réduit" << endl;
    cout << "  les possibilités de 1/16 (car hash en hexadécimal)" << endl;
}

int main() {
    cout << "╔════════════════════════════════════════════════════════╗" << endl;
    cout << "║          EXERCICE 2 : PROOF OF WORK (PoW)             ║" << endl;
    cout << "╚════════════════════════════════════════════════════════╝" << endl;
    
    // ========== EXEMPLE 1 : Création d'une blockchain simple ==========
    cout << "\n\n" << string(60, '=') << endl;
    cout << "EXEMPLE 1 : Création d'une blockchain avec PoW" << endl;
    cout << string(60, '=') << endl;
    
    Blockchain blockchain(3);
    
    blockchain.addBlock("Transaction: Alice -> Bob 100€");
    blockchain.addBlock("Transaction: Bob -> Charlie 50€");
    blockchain.addBlock("Transaction: Charlie -> Dave 25€");
    
    blockchain.displayChain();
    
    // Vérifier l'intégrité
    cout << "\n\n" << string(60, '=') << endl;
    cout << "Vérification de l'intégrité de la blockchain" << endl;
    cout << string(60, '=') << endl;
    cout << (blockchain.isChainValid() ? "✅ La blockchain est VALIDE" : "❌ La blockchain est INVALIDE") << endl;
    
    // ========== EXEMPLE 2 : Test des différentes difficultés ==========
    testDifficulties();
    
    cout << "\n\n╔════════════════════════════════════════════════════════╗" << endl;
    cout << "║            FIN DE L'EXERCICE 2                         ║" << endl;
    cout << "╚════════════════════════════════════════════════════════╝\n" << endl;
    
    return 0;
}
