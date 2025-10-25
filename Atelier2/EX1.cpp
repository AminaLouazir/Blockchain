#include "EX1.h"
#include <cstddef>  // Pour size_t (optionnel, normalement inclus via vector)

bool CellularAutomaton1D::apply_rule(bool left, bool center, bool right) {
    int index = (left << 2) | (center << 1) | right;
    return (current_rule >> index) & 1;
}

CellularAutomaton1D::CellularAutomaton1D(uint32_t rule) : current_rule(rule) {}

void CellularAutomaton1D::init_state(const std::vector<bool>& initial_state) {
    state = initial_state;
}

void CellularAutomaton1D::evolve() {
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

void CellularAutomaton1D::evolve(int generations) {
    for (int i = 0; i < generations; ++i) {
        evolve();
    }
}

const std::vector<bool>& CellularAutomaton1D::get_state() const {
    return state;
}