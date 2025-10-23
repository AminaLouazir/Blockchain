#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>

class CellularAutomaton1D {
private:
    std::vector<bool> state;
    uint32_t current_rule;
    
    bool apply_rule(bool left, bool center, bool right) {
        int index = (left << 2) | (center << 1) | right;
        return (current_rule >> index) & 1;
    }
    
public:
    CellularAutomaton1D(uint32_t rule = 30) : current_rule(rule) {}
    
    void init_state(const std::vector<bool>& initial_state) {
        state = initial_state;
    }
    
    void evolve() {
        if (state.empty()) return;
        
        std::vector<bool> new_state(state.size());
        
        for (size_t i = 0; i < state.size(); ++i) {
            bool left = state[(i - 1 + state.size()) % state.size()];
            bool center = state[i];
            bool right = state[(i + 1) % state.size()];
            
            new_state[i] = apply_rule(left, center, right);
        }
        
        state = new_state;
    }
    
    void evolve(int generations) {
        for (int i = 0; i < generations; ++i) {
            evolve();
        }
    }
    
    const std::vector<bool>& get_state() const {
        return state;
    }
};

// 2.2. Convertit une chaîne en vecteur de bits
std::vector<bool> string_to_bits(const std::string& input) {
    std::vector<bool> bits;
    for (unsigned char c : input) {
        for (int i = 7; i >= 0; --i) {
            bits.push_back((c >> i) & 1);
        }
    }
    return bits;
}

// 2.3. Extrait 256 bits de l'état avec échantillonnage espacé
std::string extract_hash(const std::vector<bool>& state) {
    std::stringstream ss;
    
    // Échantillonnage espacé pour mieux distribuer le chaos
    for (size_t i = 0; i < 64; ++i) {  // 64 nibbles = 256 bits
        int nibble = 0;
        for (int j = 0; j < 4; ++j) {
            // Utilise un pas de 13 pour éviter les patterns répétitifs
            size_t bit_idx = ((i * 4 + j) * 13 + i * 7) % state.size();
            nibble = (nibble << 1) | state[bit_idx];
        }
        ss << std::hex << nibble;
    }
    
    return ss.str();
}

// 2.1. Fonction de hachage principale
std::string ac_hash(const std::string& input, uint32_t rule, size_t steps) {
    // Conversion texte → bits
    std::vector<bool> bits = string_to_bits(input);
    
    // Padding pour avoir au moins 512 bits (meilleure diffusion)
    while (bits.size() < 512) {
        // Padding avec pattern basé sur la longueur pour différencier
        bits.push_back((bits.size() % 3) == 0);
    }
    
    // Initialisation de l'automate avec la règle spécifiée
    CellularAutomaton1D ca(rule);
    ca.init_state(bits);
    
    // Augmentation du nombre d'évolutions pour plus de chaos
    size_t effective_steps = std::max(steps, size_t(200));
    ca.evolve(effective_steps);
    
    // Extraction du hash 256 bits en hexadécimal
    return extract_hash(ca.get_state());
}

// 2.4. Test de collision
void test_hash_function() {
    std::cout << "=== Test de la fonction de hachage ===" << std::endl;
    
    std::string input1 = "Hello World";
    std::string input2 = "Hello World!";
    std::string input3 = "Bonjour";
    
    uint32_t rule = 30;
    size_t steps = 100;
    
    std::cout << "Regle: " << rule << ", Evolutions: >= " << steps << "\n" << std::endl;
    
    std::string hash1 = ac_hash(input1, rule, steps);
    std::string hash2 = ac_hash(input2, rule, steps);
    std::string hash3 = ac_hash(input3, rule, steps);
    
    std::cout << "Input: \"" << input1 << "\"" << std::endl;
    std::cout << "Hash:  " << hash1 << "\n" << std::endl;
    
    std::cout << "Input: \"" << input2 << "\"" << std::endl;
    std::cout << "Hash:  " << hash2 << "\n" << std::endl;
    
    std::cout << "Input: \"" << input3 << "\"" << std::endl;
    std::cout << "Hash:  " << hash3 << "\n" << std::endl;
    
    // Vérification des collisions
    std::cout << "Hash1 == Hash2? " << (hash1 == hash2 ? "OUI (COLLISION!)" : "NON (OK)") << std::endl;
    std::cout << "Hash1 == Hash3? " << (hash1 == hash3 ? "OUI (COLLISION!)" : "NON (OK)") << std::endl;
    std::cout << "Hash2 == Hash3? " << (hash2 == hash3 ? "OUI (COLLISION!)" : "NON (OK)") << std::endl;
    
    // Test avec différentes règles
    std::cout << "\n=== Comparaison de regles ===" << std::endl;
    std::string test_input = "Test";
    
    std::cout << "Input: \"" << test_input << "\"" << std::endl;
    std::cout << "Rule 30:  " << ac_hash(test_input, 30, 100) << std::endl;
    std::cout << "Rule 90:  " << ac_hash(test_input, 90, 100) << std::endl;
    std::cout << "Rule 110: " << ac_hash(test_input, 110, 100) << std::endl;
    
    // Test d'avalanche (petite modification → grand changement)
    std::cout << "\n=== Test d'effet avalanche ===" << std::endl;
    std::string msg1 = "password";
    std::string msg2 = "Password";  // Juste une majuscule
    
    std::string h1 = ac_hash(msg1, 30, 100);
    std::string h2 = ac_hash(msg2, 30, 100);
    
    std::cout << "\"" << msg1 << "\" -> " << h1 << std::endl;
    std::cout << "\"" << msg2 << "\" -> " << h2 << std::endl;
    
    // Compte les différences
    int diff_count = 0;
    for (size_t i = 0; i < h1.size(); ++i) {
        if (h1[i] != h2[i]) diff_count++;
    }
    std::cout << "Differences: " << diff_count << "/64 caracteres (" 
              << (diff_count * 100.0 / 64) << "%)" << std::endl;
}

int main() {
    test_hash_function();
    return 0;
}