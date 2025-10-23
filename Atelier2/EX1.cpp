#include <iostream>
#include <vector>
#include <bitset>
#include <string>

class CellularAutomaton1D {
private:
    std::vector<bool> state;
    
    // Applique la Règle 30 pour une configuration de 3 cellules
    // Règle 30: 00011110 en binaire
    bool apply_rule_30(bool left, bool center, bool right) {
        // Convertit la configuration en index (0-7)
        int index = (left << 2) | (center << 1) | right;
        // Rule 30 = 30 = 0b00011110
        return (30 >> index) & 1;
    }
    
public:
    CellularAutomaton1D() {}
    
    // 1.1. Initialise l'état à partir d'un vecteur de bits
    void init_state(const std::vector<bool>& initial_state) {
        state = initial_state;
    }
    
    // Initialise avec une seule cellule active au centre
    void init_single_cell(int size) {
        state.clear();
        state.resize(size, false);
        state[size / 2] = true;
    }
    
    // 1.2. Applique la règle de transition (une génération)
    void evolve() {
        if (state.empty()) return;
        
        std::vector<bool> new_state(state.size());
        
        for (size_t i = 0; i < state.size(); ++i) {
            // Gestion des bords avec conditions périodiques
            bool left = state[(i - 1 + state.size()) % state.size()];
            bool center = state[i];
            bool right = state[(i + 1) % state.size()];
            
            new_state[i] = apply_rule_30(left, center, right);
        }
        
        state = new_state;
    }
    
    // Évolue pour n générations
    void evolve(int generations) {
        for (int i = 0; i < generations; ++i) {
            evolve();
        }
    }
    
    // Affiche l'état actuel
    void print_state() const {
        for (bool cell : state) {
            std::cout << (cell ? "#" : ".");  // ou '*' et ' '
        }
        std::cout << std::endl;
    }
    
    // Retourne l'état actuel
    const std::vector<bool>& get_state() const {
        return state;
    }
};

// 1.3. Fonction de test pour vérifier la Règle 30
void test_rule_30() {
    std::cout << "=== Test de la Règle 30 ===" << std::endl;
    std::cout << "Configuration de test: cellule unique au centre\n" << std::endl;
    
    CellularAutomaton1D ca;
    ca.init_single_cell(31);
    
    // Affiche 15 générations
    for (int i = 0; i < 15; ++i) {
        std::cout << "Gen " << i << ": ";
        ca.print_state();
        ca.evolve();
    }
    
    std::cout << "\n=== Test avec état initial personnalisé ===" << std::endl;
    std::vector<bool> custom_state = {0,0,0,1,1,1,0,0,0};
    ca.init_state(custom_state);
    
    std::cout << "État initial: ";
    ca.print_state();
    
    for (int i = 1; i <= 5; ++i) {
        ca.evolve();
        std::cout << "Génération " << i << ": ";
        ca.print_state();
    }
}

int main() {
    test_rule_30();
    return 0;
}