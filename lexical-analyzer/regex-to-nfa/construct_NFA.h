//
// Created by abdelrahman on 18/11/17.
//

#ifndef LEXICAL_ANALYZER_CONSTRUCT_NFA_H
#define LEXICAL_ANALYZER_CONSTRUCT_NFA_H

#include "NFA.h"

NFA *regex_to_nfa(std::string regex);

NFA *language_to_nfa(vector<pair<string, string>> regexes);

#endif //LEXICAL_ANALYZER_CONSTRUCT_NFA_H
