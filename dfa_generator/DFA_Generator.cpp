//
// Created by ahmed on 14/11/17.
//

#include "DFA_Generator.h"

/* Add new DFA Entry */
int DFA::AddEntry(vector<int> entry) {
    entries.push_back(entry);
    isMarked.push_back(false);
    return (int) (entries.size() - 1);
}

/* Return the next unMarked entry */
int DFA::GetNextUnMarkedIndex() {
    for (int index = 0; index < isMarked.size(); index++) {
        if (!isMarked.at(index)) {
            return index;
        }
    }
    /* All is marked */
    return -1;
}

/* Mark entry with given index */
void DFA::MarkEntry(int index) {
    isMarked.at(index) = true;
}

/* Get Entry using it's index */
vector<int> DFA::GetEntry(int index) {
    return entries.at(index);
}

/* Search for entry */
int DFA::FindEntry(vector<int> entry) {
    for (int i = 0; i < entries.size(); i++) {
        vector<int> it = entries.at(i);
        if (it == entry) {
            return i;
        }
    }
    /* Not Found */
    return -1;
}

void DFA::SetNFAFinalState(int nfa_fs) {
    for (int i = 0; i < entries.size(); i++) {
        vector<int> entry = entries.at(i);

        for (int j = 0; j < entry.size(); j++) {
            int vertex = entry.at(j);
            if (vertex == nfa_fs) {
                finalStates.push_back(i);
            }
        }
    }
}

void DFA::SetDFAFinalState(int dfa_fs, string token_name) {
    finalStates.push_back(dfa_fs);
    finalStateTokenNames.push_back(token_name);
}

string DFA::GetFinalState() {
    return join(finalStates, ",");
}

/* Set new transition values */
void DFA::SetTransition(int from, int to, char value) {
    transition trans;
    trans.from = from;
    trans.to = to;
    trans.value = value;
    transitions.push_back(trans);
}

void DFA::display() {
    transition newTransition;
    cout << "\n";
    for (int i = 0; i < transitions.size(); i++) {
        newTransition = transitions.at(i);
        cout << "q" << newTransition.from << " {" << join(entries.at(newTransition.from), ",")
             << "} -> q" << newTransition.to << " {" << join(entries.at(newTransition.to), ",")
             << "} : Symbol - " << newTransition.value << endl;
    }
    cout << "\nThe final state is q : " << join(finalStates, ",") << endl;
}

set<int> DFA::vector_to_set(vector<int> vec) {
    set<int> result;

    for (int element : vec)
        result.insert(element);

    return result;
}

/**
 * Implements the subset construction algorithm as described in the reference
 */
DFA::DFA(NFA *n, set<char> language) {
    // initially e-closure(S0) is the only state in Dstates, and it is unmarked
    set<int> start;
    start.insert(n->get_start_state());
    set<int> s = n->epsilon_closure(start);
    AddEntry(set_to_vector(s));

    // while ( there is an unmarked state T in Dstates )
    int T_index;
    while ((T_index = GetNextUnMarkedIndex()) != -1) {
        // Mark T
        MarkEntry(T_index);
        vector<int> T_vec = GetEntry(T_index);
        set<int> T_set = vector_to_set(T_vec);

        // for each input symbol a
        for (char a : language) {
            // U = e-closure(move(T,a))
            set<int> U_set = n->epsilon_closure(n->move(T_set, a));
            vector<int> U_vec = set_to_vector(U_set);

            // if ( U is not in Dstates )
            int U_index;
            if ((U_index = FindEntry(U_vec)) == -1)
                // add U as an unmarked state to Dstates
                U_index = AddEntry(U_vec);
            SetTransition(T_index, U_index, a);
        }

    }

    // mark accepting states
    for (int i = 0; i < entries.size(); i++) {

        vector<int> entry = entries[i];

        bool accepting = false;
        string token_name;

        for (int j : entry) {
            if (n->is_accepting(j)) {
                if (accepting) {
                    // error (not really an error)
                    // found to NFA accepting states mapping to the same DFA state
                    cout << "found to NFA accepting states mapping to the same DFA state. This means that one of "
                            "the tokens will never be detected" << endl;
                } else {
                    accepting = true;
                    token_name = n->get_accepting_token_name(j);
                    cout << "NFA state with ID: " << j << " is accepting, it's token name is: " << token_name << endl;
                }
            }
        }

        if (accepting)
            SetDFAFinalState(i, token_name);
    }

}

vector<int> DFA::set_to_vector(set<int> set) {
    vector<int> result;

    for (int element : set)
        result.push_back(element);

    return result;
}

string DFA::join(vector<int> vector, string delimiter) {
    string result;

    for (int i : vector)
        result += (to_string(i) + delimiter);

    return result.substr(0, result.length() - 1);
}

Graph *DFA::as_graph() {
    // create a state for each DFA entry
    vector<State *> states;
    for (int i = 0; i < entries.size(); i++) {
        // side note: a State and it's corresponding DFA have the same index in both vectors
        State *s = new State();
        states.push_back(s);
    }

    for (transition trans : transitions) {
        State *from = states[trans.from];
        State *to = states[trans.to];

        from->add_transition(trans.value, to);
    }

    for (int i = 0; i < finalStates.size(); i++) {
        int dfa_fs_index = finalStates[i];
        string token_name = finalStateTokenNames[i];

        State *s = states[dfa_fs_index];

        s->set_accept_state(token_name);
//        s->set_token_name(token_name);
    }

    Graph *g = new Graph();
    for (State *s : states) {
        g->insert_state(s);
    }

    return g;
}


