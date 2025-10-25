#ifndef CELLULAR_AUTOMATON_H
#define CELLULAR_AUTOMATON_H

#include <vector>
#include <cstdint>  // Pour uint32_t


class CellularAutomaton1D {
private:
    std::vector<bool> state;
    uint32_t current_rule;
    
    bool apply_rule(bool left, bool center, bool right);
    
public:
    CellularAutomaton1D(uint32_t rule = 30);
    void init_state(const std::vector<bool>& initial_state);
    void evolve();
    void evolve(int generations);
    const std::vector<bool>& get_state() const;
};

#endif