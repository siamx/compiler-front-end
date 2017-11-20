//
// Created by ahmed on 11/14/17.
//

#include "DFA_Reducer.h"

DFA_Reducer::DFA_Reducer(Graph *dfa, set<char> language_chars) {
    this->dfa = dfa;
    this->language_chars = std::move(language_chars);
}

Graph *DFA_Reducer::get_dfa() const {
    return this->dfa;
}

void DFA_Reducer::minimize() {
    printf("\n\nMinimizing DFA:\n\n");
    // removes redundancies
    this->remove_redundancies();

    // merging states using disjoint sets
    int partition_count = this->merge_non_distinguishable();
    this->build_min_dfa(partition_count);
}

void DFA_Reducer::print() {
    set<State *> states = this->dfa->get_states();

    string line = "---------------";
    for (int i = 0; i < language_chars.size(); ++i)
        line += "-------";

    printf("%s\n               ", line.c_str());
    for (auto &input: this->language_chars) {
        printf("| %-5c", input);
    }

    printf("\n%s\n", line.c_str());
    for (auto &state: states) {
        if (state->is_input_state())
            printf(" ➜ ");
        else
            printf("   ");

        if (state->is_accept_state())
            printf("*");
        else
            printf(" ");

        printf("%5d  =>  ", state->get_id());
        for (auto &next_state: state->get_transitions()) {
            printf("| %-5d", next_state.second->get_id());
        }
        printf("\n");
    }
    printf("%s\n", line.c_str());
}

void DFA_Reducer::remove_redundancies() {
    printf("1- Eliminating redundancies\n============================\n");
    set<State *> states = dfa->get_states();
    map<State *, State *> redundant;
    set<State *> updated;

    for (auto &A: states) {
        if (updated.find(A) != updated.end()) continue;
        for (auto &B: states) {
            if (A == B || updated.find(B) != updated.end()) continue;

            // Two states are equivalent if => `input & accept & token_name & transitions` are equal
            bool condition = A->get_transitions().size() == B->get_transitions().size();
            condition = condition && (A->is_accept_state() == B->is_accept_state());
            condition = condition && (A->get_token_name() == B->get_token_name());
            condition = condition && (A->is_input_state() == B->is_input_state());

            if (condition) {
                auto A_transitions = A->get_transitions();
                auto B_transitions = B->get_transitions();
                bool valid = true;

                // Match all transitions on both states
                for (auto nextA = A_transitions.begin(), nextB = B_transitions.begin();
                     nextA != A_transitions.end() && nextB != B_transitions.end();
                     ++nextA, ++nextB) {
                    if (nextA->first != nextB->first || nextA->second != nextB->second) {
                        valid = false;
                        break;
                    }
                }
                if (valid) {
                    // Replace redundancies in the states set
                    printf("Redundant: %-3d ==> %3d\n", A->get_id(), B->get_id());
                    this->replace_redundant(A, B);
                    updated.insert(B);
                }
            }
        }
    }
}

void DFA_Reducer::replace_redundant(State *A, State *B) {
    for (auto &state: this->dfa->get_states()) {
        for (auto &next_state: state->get_transitions()) {
            if (next_state.second == B) {
                state->add_transition(next_state.first, A);
            }
        }
    }
}

int DFA_Reducer::merge_non_distinguishable() {
    printf("\n2- Merging non-distinguishable states\n=====================================\n");
    int partition_count = 2; // starting with two partitions accept states & non-accept states
    map<int, set<State *> > disjoint_set;

    // Create initial two sets, accepting and non-accepting states
    for (auto &state: this->get_dfa()->get_states()) {
        if (state->is_accept_state()) {
            disjoint_set[1].insert(state);
            this->old_state_mapper[state] = 1;
        } else {
            disjoint_set[2].insert(state);
            this->old_state_mapper[state] = 2;
        }
    }

    bool flag = true;
    while (flag) {
        flag = false;
        for (auto &cur_set: disjoint_set) {
            for (auto &stateA: cur_set.second) {
                for (auto &stateB: cur_set.second) {
                    if (stateA == stateB) continue;
                    for (auto &transition: stateA->get_transitions()) {
                        if (this->old_state_mapper[transition.second] !=
                            this->old_state_mapper[stateB->get_transitions()[transition.first]]) {
                            if (this->old_state_mapper[stateA] != this->old_state_mapper[transition.second]) {
                                this->old_state_mapper[stateA] = ++partition_count;
                                cur_set.second.erase(stateA);
                                disjoint_set[partition_count].insert(stateA);
                            } else {
                                this->old_state_mapper[stateB] = ++partition_count;
                                cur_set.second.erase(stateB);
                                disjoint_set[partition_count].insert(stateB);
                            }
                            flag = true;
                            break;
                        }
                    }
                }
                if (flag)
                    break;
            }
            if (flag)
                break;
        }
    }
    return partition_count;
}

void DFA_Reducer::build_min_dfa(int partition_count) {
    set<int> min_states;

    // Form transition_table table
    for (auto &state: this->old_state_mapper) {
        min_states.insert(state.second);
        for (auto &move: state.first->get_transitions()) {
            pair<int, char> cur_pair = make_pair(this->old_state_mapper[state.first], move.first);
            this->transition_table[cur_pair] = this->old_state_mapper[move.second];
        }
    }

    // Building new min_dfa
    delete (this->dfa);
    this->dfa = new Graph();

    // Add states to the new min_dfa
    for (unsigned int i = 1; i <= partition_count; ++i) {
        auto *dummy = new State();
        dummy->set_id(i);
        dfa->insert_state(dummy);
        for (auto &state: this->old_state_mapper) {
            if (state.second == i) {
                if (state.first->is_input_state()) {
                    dummy->set_input_state(true);
                    this->dfa->set_start_state(dummy);
                }
                if (state.first->is_accept_state()) {
                    dummy->set_accept_state(state.first->get_token_name());
                    // TODO: (in case of overlapping accept states)
                    // TODO: set token type in accepting state according to priority
                }
            }
        }
        this->new_state_mapper[i] = dummy;
    }

    // Add transitions to each state
    for (auto &state_id: min_states) {
        State *cur = this->new_state_mapper.at(state_id);
        for (auto &input: this->language_chars) {
            State *next_state = this->new_state_mapper[this->transition_table[make_pair(state_id, input)]];
            cur->add_transition(input, next_state);
        }
    }

    // Release old DFA pointers
    for (auto &old_state: this->old_state_mapper) {
        delete (old_state.first);
    }
}