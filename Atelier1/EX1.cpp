#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include "picosha2.h"

using namespace std;

// Fonction pour calculer le hash SHA256
string sha256(const string& data) {
    return picosha2::hash256_hex_string(data);
}

// Classe pour l'arbre de Merkle
class MerkleTree {
private:
    vector<string> leaves;      // Feuilles de l'arbre (hashes des transactions)
    vector<vector<string>> levels;  // Tous les niveaux de l'arbre
    string root;                // Racine de l'arbre
    
    // Combine deux hashes
    string hashPair(const string& left, const string& right) {
        return sha256(left + right);
    }
    
    // Construit l'arbre récursivement
    void buildTree() {
        if(leaves.empty()) {
            root = "";
            return;
        }
        
        levels.clear();
        levels.push_back(leaves);
        
        vector<string> currentLevel = leaves;
        
        while(currentLevel.size() > 1) {
            vector<string> nextLevel;
            
            for(size_t i = 0; i < currentLevel.size(); i += 2) {
                if(i + 1 < currentLevel.size()) {
                    // Paire normale
                    nextLevel.push_back(hashPair(currentLevel[i], currentLevel[i+1]));
                } else {
                    // Nombre impair : dupliquer le dernier élément
                    nextLevel.push_back(hashPair(currentLevel[i], currentLevel[i]));
                }
            }
            
            levels.push_back(nextLevel);
            currentLevel = nextLevel;
        }
        
        root = currentLevel[0];
    }
    
public:
    // Constructeur avec des transactions
    MerkleTree(const vector<string>& transactions) {
        cout << "\n=== Construction de l'Arbre de Merkle ===" << endl;
        cout << "Nombre de transactions: " << transactions.size() << endl;
        
        // Créer les feuilles (hash de chaque transaction)
        for(size_t i = 0; i < transactions.size(); i++) {
            string hash = sha256(transactions[i]);
            leaves.push_back(hash);
            cout << "Transaction " << i+1 << ": " << transactions[i] << endl;
            cout << "  Hash: " << hash.substr(0, 16) << "..." << endl;
        }
        
        buildTree();
    }
    
    string getRoot() const { 
        return root; 
    }
    
    // Afficher l'arbre complet
    void display() {
        cout << "\n=== Structure de l'Arbre ===" << endl;
        for(int i = levels.size() - 1; i >= 0; i--) {
            cout << "Niveau " << i << " (" << levels[i].size() << " noeuds):" << endl;
            for(size_t j = 0; j < levels[i].size(); j++) {
                cout << "  [" << j << "] " << levels[i][j].substr(0, 16) << "..." << endl;
            }
        }
        cout << "\nMerkle Root: " << root << endl;
    }
    
    // Vérifier si une transaction est dans l'arbre
    bool verifyTransaction(const string& transaction) {
        string txHash = sha256(transaction);
        for(const auto& leaf : leaves) {
            if(leaf == txHash) {
                return true;
            }
        }
        return false;
    }
    
    // Générer la preuve de Merkle pour une transaction
    vector<string> generateProof(int txIndex) {
        vector<string> proof;
        
        if(txIndex < 0 || txIndex >= leaves.size()) {
            return proof;
        }
        
        int index = txIndex;
        for(size_t level = 0; level < levels.size() - 1; level++) {
            int pairIndex = (index % 2 == 0) ? index + 1 : index - 1;
            
            if(pairIndex < levels[level].size()) {
                proof.push_back(levels[level][pairIndex]);
            } else {
                proof.push_back(levels[level][index]);
            }
            
            index = index / 2;
        }
        
        return proof;
    }

    // Vérifier une preuve de Merkle
    static bool verifyProof(const string& transaction, const vector<string>& proof, const string& root) {
        string currentHash = sha256(transaction);
        
        for(const auto& p : proof) {
            // L'ordre (gauche/droite) est important. On essaie les deux.
            // Dans une vraie implémentation, on stockerait la position (gauche/droite).
            string hash1 = sha256(currentHash + p);
            string hash2 = sha256(p + currentHash);
            currentHash = sha256(p + currentHash); 
        }
        
        // Le hash calculé doit correspondre à la racine de l'arbre
        return currentHash == root;
    }
};

// Exemples d'exécution
int main() {
    cout << "╔════════════════════════════════════════════════════════╗" << endl;
    cout << "║     EXERCICE 1 : ARBRE DE MERKLE - FROM SCRATCH       ║" << endl;
    cout << "╚════════════════════════════════════════════════════════╝" << endl;
    
    // ========== EXEMPLE 1 : 4 transactions ==========
    cout << "\n\n" << string(60, '=') << endl;
    cout << "EXEMPLE 1 : Arbre avec 4 transactions" << endl;
    cout << string(60, '=') << endl;
    
    vector<string> transactions1 = {
        "Alice -> Bob: 50€",
        "Bob -> Charlie: 30€",
        "Charlie -> Dave: 20€",
        "Dave -> Alice: 10€"
    };
    
    MerkleTree tree1(transactions1);
    tree1.display();
    
    // Test de vérification
    cout << "\n--- Test de vérification ---" << endl;
    cout << "Transaction 'Alice -> Bob: 50€' est dans l'arbre? " 
         << (tree1.verifyTransaction("Alice -> Bob: 50€") ? "OUI" : "NON") << endl;
    cout << "Transaction 'Eve -> Frank: 100€' est dans l'arbre? " 
         << (tree1.verifyTransaction("Eve -> Frank: 100€") ? "OUI" : "NON") << endl;
    
    // ========== EXEMPLE 2 : 5 transactions (nombre impair) ==========
    cout << "\n\n" << string(60, '=') << endl;
    cout << "EXEMPLE 2 : Arbre avec 5 transactions (nombre impair)" << endl;
    cout << string(60, '=') << endl;
    
    vector<string> transactions2 = {
        "Tx1: Alice pays 100",
        "Tx2: Bob pays 200",
        "Tx3: Charlie pays 150",
        "Tx4: Dave pays 75",
        "Tx5: Eve pays 50"
    };
    
    MerkleTree tree2(transactions2);
    tree2.display();
    
    // ========== EXEMPLE 3 : Preuve de Merkle ==========
    cout << "\n\n" << string(60, '=') << endl;
    cout << "EXEMPLE 3 : Génération de preuve de Merkle" << endl;
    cout << string(60, '=') << endl;
    
    vector<string> transactions3 = {
        "Transaction A",
        "Transaction B",
        "Transaction C",
        "Transaction D",
        "Transaction E",
        "Transaction F",
        "Transaction G",
        "Transaction H"
    };
    
    MerkleTree tree3(transactions3);
    
    cout << "\nGénération de la preuve pour Transaction C (index 2):" << endl;
    vector<string> proof = tree3.generateProof(2);
    cout << "Chemin de preuve:" << endl;
    for(size_t i = 0; i < proof.size(); i++) {
        cout << "  Niveau " << i << ": " << proof[i].substr(0, 16) << "..." << endl;
    }
    
    // Vérification de la preuve
    cout << "\nVérification de la preuve pour 'Transaction C':" << endl;
    bool isProofValid = MerkleTree::verifyProof("Transaction C", proof, tree3.getRoot());
    cout << "La preuve est-elle valide? " << (isProofValid ? "OUI" : "NON") << endl;
    
    // Tentative de vérification avec une mauvaise transaction
    cout << "\nVérification de la preuve pour 'Transaction X' (invalide):" << endl;
    bool isProofValid2 = MerkleTree::verifyProof("Transaction X", proof, tree3.getRoot());
    cout << "La preuve est-elle valide? " << (isProofValid2 ? "OUI" : "NON") << endl;
    
    // ========== EXEMPLE 4 : Comparaison de deux arbres ==========
    cout << "\n\n" << string(60, '=') << endl;
    cout << "EXEMPLE 4 : Comparaison de Merkle Roots" << endl;
    cout << string(60, '=') << endl;
    
    vector<string> setA = {"Tx1", "Tx2", "Tx3", "Tx4"};
    vector<string> setB = {"Tx1", "Tx2", "Tx3", "Tx4"};
    vector<string> setC = {"Tx1", "Tx2", "Tx3", "Tx5"}; // Dernière transaction différente
    
    MerkleTree treeA(setA);
    MerkleTree treeB(setB);
    MerkleTree treeC(setC);
    
    cout << "\nSet A Root: " << treeA.getRoot().substr(0, 32) << "..." << endl;
    cout << "Set B Root: " << treeB.getRoot().substr(0, 32) << "..." << endl;
    cout << "Set C Root: " << treeC.getRoot().substr(0, 32) << "..." << endl;
    
    cout << "\nSet A == Set B? " << (treeA.getRoot() == treeB.getRoot() ? "OUI (identiques)" : "NON") << endl;
    cout << "Set A == Set C? " << (treeA.getRoot() == treeC.getRoot() ? "OUI" : "NON (différents)") << endl;
    
    cout << "\n\n╔════════════════════════════════════════════════════════╗" << endl;
    cout << "║            FIN DE L'EXERCICE 1                         ║" << endl;
    cout << "╚════════════════════════════════════════════════════════╝\n" << endl;
    
    return 0;
}

/* 
COMPILATION:
g++ -o EX1 EX1.cpp -std=c++11

EXECUTION:
./EX1
*/