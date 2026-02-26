#include "LexicalAnalyzerGenerator.h"
#include <sstream>
#include <iomanip>

// ==================== NFA Implementation ====================

NFA::NFA() : startState(0), stateCounter(0) {}

NFA NFA::fromSymbol(char symbol) {
    NFA nfa;
    nfa.addState(0);
    nfa.addState(1, true);
    nfa.addTransition(0, 1, symbol);
    nfa.setStartState(0);
    nfa.stateCounter = 2;
    return nfa;
}

NFA NFA::concatenate(const NFA& nfa1, const NFA& nfa2) {
    NFA result;
    int offset = nfa1.states.size();
    
    // Add states from nfa1
    for (const auto& state : nfa1.states) {
        result.addState(state.id, false);
    }
    
    // Add states from nfa2 with offset
    for (const auto& state : nfa2.states) {
        result.addState(state.id + offset, state.isAccepting);
    }
    
    // Add transitions from nfa1
    for (const auto& trans : nfa1.transitions) {
        result.addTransition(trans.fromState, trans.toState, trans.symbol);
    }
    
    // Connect accepting states of nfa1 to start of nfa2 with epsilon
    for (int acceptState : nfa1.acceptingStates) {
        result.addTransition(acceptState, nfa2.startState + offset, '\0');
    }
    
    // Add transitions from nfa2 with offset
    for (const auto& trans : nfa2.transitions) {
        result.addTransition(trans.fromState + offset, trans.toState + offset, trans.symbol);
    }
    
    result.setStartState(nfa1.startState);
    result.stateCounter = nfa1.states.size() + nfa2.states.size();
    
    return result;
}

NFA NFA::union_op(const NFA& nfa1, const NFA& nfa2) {
    NFA result;
    int newStart = 0;
    int offset1 = 1;
    int offset2 = offset1 + nfa1.states.size();
    int newAccept = offset2 + nfa2.states.size();
    
    // Add new start state
    result.addState(newStart);
    result.setStartState(newStart);
    
    // Add states from nfa1 with offset
    for (const auto& state : nfa1.states) {
        result.addState(state.id + offset1, false);
    }
    
    // Add states from nfa2 with offset
    for (const auto& state : nfa2.states) {
        result.addState(state.id + offset2, false);
    }
    
    // Add new accepting state
    result.addState(newAccept, true);
    
    // Connect new start to both NFAs with epsilon
    result.addTransition(newStart, nfa1.startState + offset1, '\0');
    result.addTransition(newStart, nfa2.startState + offset2, '\0');
    
    // Add transitions from nfa1 with offset
    for (const auto& trans : nfa1.transitions) {
        result.addTransition(trans.fromState + offset1, trans.toState + offset1, trans.symbol);
    }
    
    // Add transitions from nfa2 with offset
    for (const auto& trans : nfa2.transitions) {
        result.addTransition(trans.fromState + offset2, trans.toState + offset2, trans.symbol);
    }
    
    // Connect accepting states to new accept state
    for (int acceptState : nfa1.acceptingStates) {
        result.addTransition(acceptState + offset1, newAccept, '\0');
    }
    for (int acceptState : nfa2.acceptingStates) {
        result.addTransition(acceptState + offset2, newAccept, '\0');
    }
    
    result.stateCounter = newAccept + 1;
    
    return result;
}

NFA NFA::kleeneStar(const NFA& nfa) {
    NFA result;
    int newStart = 0;
    int offset = 1;
    int newAccept = offset + nfa.states.size();
    
    // Add new start state
    result.addState(newStart);
    result.setStartState(newStart);
    
    // Add states from nfa with offset
    for (const auto& state : nfa.states) {
        result.addState(state.id + offset, false);
    }
    
    // Add new accepting state
    result.addState(newAccept, true);
    
    // Connect new start to nfa start and new accept with epsilon
    result.addTransition(newStart, nfa.startState + offset, '\0');
    result.addTransition(newStart, newAccept, '\0');
    
    // Add transitions from nfa with offset
    for (const auto& trans : nfa.transitions) {
        result.addTransition(trans.fromState + offset, trans.toState + offset, trans.symbol);
    }
    
    // Connect accepting states back to nfa start and to new accept
    for (int acceptState : nfa.acceptingStates) {
        result.addTransition(acceptState + offset, nfa.startState + offset, '\0');
        result.addTransition(acceptState + offset, newAccept, '\0');
    }
    
    result.stateCounter = newAccept + 1;
    
    return result;
}

NFA NFA::fromRegex(const string& regex) {
    string postfix = RegexParser::infixToPostfix(regex);
    stack<NFA> nfaStack;
    
    for (char c : postfix) {
        if (c == '*') {
            if (nfaStack.empty()) continue;
            NFA nfa = nfaStack.top();
            nfaStack.pop();
            nfaStack.push(kleeneStar(nfa));
        }
        else if (c == '|') {
            if (nfaStack.size() < 2) continue;
            NFA nfa2 = nfaStack.top(); nfaStack.pop();
            NFA nfa1 = nfaStack.top(); nfaStack.pop();
            nfaStack.push(union_op(nfa1, nfa2));
        }
        else if (c == '.') {
            if (nfaStack.size() < 2) continue;
            NFA nfa2 = nfaStack.top(); nfaStack.pop();
            NFA nfa1 = nfaStack.top(); nfaStack.pop();
            nfaStack.push(concatenate(nfa1, nfa2));
        }
        else {
            nfaStack.push(fromSymbol(c));
        }
    }
    
    return nfaStack.empty() ? NFA() : nfaStack.top();
}

void NFA::addState(int stateId, bool isAccepting) {
    State state(stateId);
    state.isAccepting = isAccepting;
    states.push_back(state);
    if (isAccepting) {
        acceptingStates.insert(stateId);
    }
}

void NFA::addTransition(int from, int to, char symbol) {
    transitions.push_back(Transition(from, to, symbol));
}

void NFA::setStartState(int stateId) {
    startState = stateId;
}

void NFA::addAcceptingState(int stateId) {
    acceptingStates.insert(stateId);
    for (auto& state : states) {
        if (state.id == stateId) {
            state.isAccepting = true;
            break;
        }
    }
}

set<int> NFA::epsilonClosure(int state) const {
    set<int> closure;
    stack<int> stateStack;
    
    closure.insert(state);
    stateStack.push(state);
    
    while (!stateStack.empty()) {
        int current = stateStack.top();
        stateStack.pop();
        
        for (const auto& trans : transitions) {
            if (trans.fromState == current && trans.symbol == '\0') {
                if (closure.find(trans.toState) == closure.end()) {
                    closure.insert(trans.toState);
                    stateStack.push(trans.toState);
                }
            }
        }
    }
    
    return closure;
}

set<int> NFA::epsilonClosure(const set<int>& states) const {
    set<int> closure;
    
    for (int state : states) {
        set<int> stateClosure = epsilonClosure(state);
        closure.insert(stateClosure.begin(), stateClosure.end());
    }
    
    return closure;
}

set<int> NFA::move(const set<int>& states, char symbol) const {
    set<int> result;
    
    for (int state : states) {
        for (const auto& trans : transitions) {
            if (trans.fromState == state && trans.symbol == symbol) {
                result.insert(trans.toState);
            }
        }
    }
    
    return result;
}

void NFA::display() const {
    cout << "\n========== NFA Structure ==========" << endl;
    cout << "Start State: " << startState << endl;
    cout << "Accepting States: ";
    for (int state : acceptingStates) {
        cout << state << " ";
    }
    cout << "\n\nStates: " << states.size() << endl;
    
    cout << "\nTransitions:" << endl;
    for (const auto& trans : transitions) {
        cout << "  State " << trans.fromState << " --";
        if (trans.symbol == '\0') {
            cout << "ε";
        } else {
            cout << trans.symbol;
        }
        cout << "--> State " << trans.toState << endl;
    }
    cout << "===================================" << endl;
}

// ==================== DFA Implementation ====================

DFA::DFA() : startState(0) {}

DFA DFA::fromNFA(const NFA& nfa) {
    DFA dfa;
    
    // Get alphabet from NFA (excluding epsilon)
    set<char> alphabet;
    for (const auto& trans : nfa.getTransitions()) {
        if (trans.symbol != '\0') {
            alphabet.insert(trans.symbol);
        }
    }
    dfa.alphabet = alphabet;
    
    // Map from set of NFA states to DFA state ID
    map<set<int>, int> dfaStateMap;
    queue<set<int>> unmarkedStates;
    int dfaStateCounter = 0;
    
    // Start with epsilon closure of NFA start state
    set<int> startClosure = nfa.epsilonClosure(nfa.getStartState());
    dfaStateMap[startClosure] = dfaStateCounter++;
    unmarkedStates.push(startClosure);
    dfa.setStartState(0);
    dfa.addState(0);
    
    // Check if start state is accepting
    for (int nfaState : startClosure) {
        if (nfa.getAcceptingStates().count(nfaState)) {
            dfa.addAcceptingState(0);
            break;
        }
    }
    
    // Subset construction algorithm
    while (!unmarkedStates.empty()) {
        set<int> currentStates = unmarkedStates.front();
        unmarkedStates.pop();
        
        int currentDfaState = dfaStateMap[currentStates];
        
        // For each symbol in alphabet
        for (char symbol : alphabet) {
            set<int> moveResult = nfa.move(currentStates, symbol);
            set<int> newStates = nfa.epsilonClosure(moveResult);
            
            if (newStates.empty()) continue;
            
            // Check if this set of states already exists
            if (dfaStateMap.find(newStates) == dfaStateMap.end()) {
                int newDfaState = dfaStateCounter++;
                dfaStateMap[newStates] = newDfaState;
                unmarkedStates.push(newStates);
                dfa.addState(newDfaState);
                
                // Check if new state is accepting
                for (int nfaState : newStates) {
                    if (nfa.getAcceptingStates().count(nfaState)) {
                        dfa.addAcceptingState(newDfaState);
                        break;
                    }
                }
            }
            
            int nextDfaState = dfaStateMap[newStates];
            dfa.addTransition(currentDfaState, symbol, nextDfaState);
        }
    }
    
    return dfa;
}

void DFA::addState(int stateId, bool isAccepting) {
    State state(stateId);
    state.isAccepting = isAccepting;
    states.push_back(state);
    if (isAccepting) {
        acceptingStates.insert(stateId);
    }
}

void DFA::addTransition(int from, char symbol, int to) {
    transitions[{from, symbol}] = to;
    alphabet.insert(symbol);
}

void DFA::setStartState(int stateId) {
    startState = stateId;
}

void DFA::addAcceptingState(int stateId) {
    acceptingStates.insert(stateId);
    for (auto& state : states) {
        if (state.id == stateId) {
            state.isAccepting = true;
            break;
        }
    }
}

void DFA::setTokenType(int stateId, const string& tokenType) {
    stateToTokenType[stateId] = tokenType;
}

int DFA::getNextState(int currentState, char symbol) const {
    auto it = transitions.find({currentState, symbol});
    if (it != transitions.end()) {
        return it->second;
    }
    return -1;  // No transition found
}

bool DFA::accepts(const string& input) const {
    int currentState = startState;
    
    for (char c : input) {
        currentState = getNextState(currentState, c);
        if (currentState == -1) {
            return false;
        }
    }
    
    return acceptingStates.count(currentState) > 0;
}

void DFA::display() const {
    cout << "\n========== DFA Structure ==========" << endl;
    cout << "Start State: " << startState << endl;
    cout << "Accepting States: ";
    for (int state : acceptingStates) {
        cout << state << " ";
    }
    cout << "\n\nAlphabet: { ";
    for (char c : alphabet) {
        cout << "'" << c << "' ";
    }
    cout << "}" << endl;
    
    cout << "\nStates: " << states.size() << endl;
    
    cout << "\nTransition Table:" << endl;
    cout << setw(10) << "State";
    for (char c : alphabet) {
        cout << setw(10) << c;
    }
    cout << endl;
    cout << string(10 + alphabet.size() * 10, '-') << endl;
    
    for (const auto& state : states) {
        cout << setw(10) << state.id;
        for (char c : alphabet) {
            int next = getNextState(state.id, c);
            if (next != -1) {
                cout << setw(10) << next;
            } else {
                cout << setw(10) << "-";
            }
        }
        cout << endl;
    }
    cout << "===================================" << endl;
}

void DFA::generateCppCode(const string& filename, const map<string, string>& tokenPatterns) const {
    ofstream outFile(filename);
    
    if (!outFile.is_open()) {
        cerr << "Error: Could not open file " << filename << " for writing." << endl;
        return;
    }
    
    // Write header and includes
    outFile << "// Auto-generated Lexical Analyzer" << endl;
    outFile << "// Generated on: " << __DATE__ << " " << __TIME__ << endl;
    outFile << "\n#include <iostream>" << endl;
    outFile << "#include <string>" << endl;
    outFile << "#include <vector>" << endl;
    outFile << "#include <map>" << endl;
    outFile << "using namespace std;" << endl;
    outFile << "\n// Token structure" << endl;
    outFile << "struct Token {" << endl;
    outFile << "    string type;" << endl;
    outFile << "    string lexeme;" << endl;
    outFile << "    int line;" << endl;
    outFile << "    int column;" << endl;
    outFile << "};" << endl;
    
    // Write DFA transition table
    outFile << "\n// DFA Transition Table" << endl;
    outFile << "class LexicalAnalyzer {" << endl;
    outFile << "private:" << endl;
    outFile << "    static const int START_STATE = " << startState << ";" << endl;
    outFile << "    map<pair<int, char>, int> transitionTable;" << endl;
    outFile << "    map<int, bool> acceptingStates;" << endl;
    outFile << "    map<int, string> stateToToken;" << endl;
    outFile << "    \n    void initializeTransitionTable() {" << endl;
    
    // Add transitions
    for (const auto& trans : transitions) {
        outFile << "        transitionTable[{" << trans.first.first << ", '" 
                << trans.first.second << "'}] = " << trans.second << ";" << endl;
    }
    
    outFile << "    }" << endl;
    outFile << "    \n    void initializeAcceptingStates() {" << endl;
    
    // Add accepting states
    for (int state : acceptingStates) {
        outFile << "        acceptingStates[" << state << "] = true;" << endl;
        auto it = stateToTokenType.find(state);
        if (it != stateToTokenType.end()) {
            outFile << "        stateToToken[" << state << "] = \"" << it->second << "\";" << endl;
        }
    }
    
    outFile << "    }" << endl;
    
    // Write getNextState method
    outFile << "\n    int getNextState(int currentState, char symbol) {" << endl;
    outFile << "        auto it = transitionTable.find({currentState, symbol});" << endl;
    outFile << "        if (it != transitionTable.end()) {" << endl;
    outFile << "            return it->second;" << endl;
    outFile << "        }" << endl;
    outFile << "        return -1;" << endl;
    outFile << "    }" << endl;
    
    // Write tokenize method
    outFile << "\npublic:" << endl;
    outFile << "    LexicalAnalyzer() {" << endl;
    outFile << "        initializeTransitionTable();" << endl;
    outFile << "        initializeAcceptingStates();" << endl;
    outFile << "    }" << endl;
    
    outFile << "\n    vector<Token> tokenize(const string& input) {" << endl;
    outFile << "        vector<Token> tokens;" << endl;
    outFile << "        int currentState = START_STATE;" << endl;
    outFile << "        string currentLexeme = \"\";" << endl;
    outFile << "        int line = 1, column = 1;" << endl;
    outFile << "        int lastAcceptState = -1;" << endl;
    outFile << "        int lastAcceptPos = -1;" << endl;
    outFile << "        \n        for (size_t i = 0; i < input.length(); i++) {" << endl;
    outFile << "            char c = input[i];" << endl;
    outFile << "            int nextState = getNextState(currentState, c);" << endl;
    outFile << "            \n            if (nextState != -1) {" << endl;
    outFile << "                currentState = nextState;" << endl;
    outFile << "                currentLexeme += c;" << endl;
    outFile << "                \n                if (acceptingStates[currentState]) {" << endl;
    outFile << "                    lastAcceptState = currentState;" << endl;
    outFile << "                    lastAcceptPos = i;" << endl;
    outFile << "                }" << endl;
    outFile << "            } else {" << endl;
    outFile << "                if (lastAcceptState != -1) {" << endl;
    outFile << "                    Token token;" << endl;
    outFile << "                    token.type = stateToToken[lastAcceptState];" << endl;
    outFile << "                    token.lexeme = currentLexeme.substr(0, lastAcceptPos + 1);" << endl;
    outFile << "                    token.line = line;" << endl;
    outFile << "                    token.column = column;" << endl;
    outFile << "                    tokens.push_back(token);" << endl;
    outFile << "                    \n                    i = lastAcceptPos;" << endl;
    outFile << "                    currentState = START_STATE;" << endl;
    outFile << "                    currentLexeme = \"\";" << endl;
    outFile << "                    lastAcceptState = -1;" << endl;
    outFile << "                    lastAcceptPos = -1;" << endl;
    outFile << "                } else {" << endl;
    outFile << "                    // Error: no valid token" << endl;
    outFile << "                    if (c != ' ' && c != '\\t' && c != '\\n') {" << endl;
    outFile << "                        cerr << \"Lexical error at line \" << line << \", column \" << column << endl;" << endl;
    outFile << "                    }" << endl;
    outFile << "                    currentState = START_STATE;" << endl;
    outFile << "                    currentLexeme = \"\";" << endl;
    outFile << "                }" << endl;
    outFile << "            }" << endl;
    outFile << "            \n            if (c == '\\n') {" << endl;
    outFile << "                line++;" << endl;
    outFile << "                column = 1;" << endl;
    outFile << "            } else {" << endl;
    outFile << "                column++;" << endl;
    outFile << "            }" << endl;
    outFile << "        }" << endl;
    outFile << "        \n        // Handle last token" << endl;
    outFile << "        if (lastAcceptState != -1) {" << endl;
    outFile << "            Token token;" << endl;
    outFile << "            token.type = stateToToken[lastAcceptState];" << endl;
    outFile << "            token.lexeme = currentLexeme;" << endl;
    outFile << "            token.line = line;" << endl;
    outFile << "            token.column = column;" << endl;
    outFile << "            tokens.push_back(token);" << endl;
    outFile << "        }" << endl;
    outFile << "        \n        return tokens;" << endl;
    outFile << "    }" << endl;
    outFile << "};" << endl;
    
    // Write main function for testing
    outFile << "\nint main() {" << endl;
    outFile << "    LexicalAnalyzer analyzer;" << endl;
    outFile << "    \n    cout << \"Enter input to tokenize (Ctrl+D to end):\" << endl;" << endl;
    outFile << "    string input, line;" << endl;
    outFile << "    while (getline(cin, line)) {" << endl;
    outFile << "        input += line + \"\\n\";" << endl;
    outFile << "    }" << endl;
    outFile << "    \n    vector<Token> tokens = analyzer.tokenize(input);" << endl;
    outFile << "    \n    cout << \"\\n========== TOKENS ==========\" << endl;" << endl;
    outFile << "    for (const auto& token : tokens) {" << endl;
    outFile << "        cout << \"<\" << token.type << \", \" << token.lexeme << \">\" << endl;" << endl;
    outFile << "    }" << endl;
    outFile << "    \n    return 0;" << endl;
    outFile << "}" << endl;
    
    outFile.close();
    cout << "\nC++ code generated successfully: " << filename << endl;
}

// ==================== RegexParser Implementation ====================

bool RegexParser::isValidRegex(const string& regex) {
    int parenCount = 0;
    for (char c : regex) {
        if (c == '(') parenCount++;
        if (c == ')') parenCount--;
        if (parenCount < 0) return false;
    }
    return parenCount == 0;
}

string RegexParser::infixToPostfix(const string& regex) {
    string result = "";
    stack<char> opStack;
    string processed = "";
    
    // Add explicit concatenation operator
    for (size_t i = 0; i < regex.length(); i++) {
        char c = regex[i];
        processed += c;
        
        if (i + 1 < regex.length()) {
            char next = regex[i + 1];
            if ((c != '(' && c != '|') && (next != ')' && next != '|' && next != '*')) {
                processed += '.';
            }
        }
    }
    
    // Convert to postfix
    for (char c : processed) {
        if (!isOperator(c) && c != '(' && c != ')') {
            result += c;
        }
        else if (c == '(') {
            opStack.push(c);
        }
        else if (c == ')') {
            while (!opStack.empty() && opStack.top() != '(') {
                result += opStack.top();
                opStack.pop();
            }
            if (!opStack.empty()) opStack.pop();  // Remove '('
        }
        else {
            while (!opStack.empty() && opStack.top() != '(' && 
                   getPrecedence(opStack.top()) >= getPrecedence(c)) {
                result += opStack.top();
                opStack.pop();
            }
            opStack.push(c);
        }
    }
    
    while (!opStack.empty()) {
        result += opStack.top();
        opStack.pop();
    }
    
    return result;
}

int RegexParser::getPrecedence(char op) {
    if (op == '*') return 3;
    if (op == '.') return 2;
    if (op == '|') return 1;
    return 0;
}

bool RegexParser::isOperator(char c) {
    return c == '*' || c == '|' || c == '.';
}

// ==================== LexicalAnalyzerGenerator Implementation ====================

LexicalAnalyzerGenerator::LexicalAnalyzerGenerator() {}

void LexicalAnalyzerGenerator::addTokenPattern(const string& tokenType, const string& pattern) {
    tokenPatterns[tokenType] = pattern;
}

void LexicalAnalyzerGenerator::build() {
    vector<NFA> nfas;
    
    cout << "\nBuilding NFAs from regex patterns..." << endl;
    
    // Create NFA for each token pattern
    for (const auto& pattern : tokenPatterns) {
        cout << "  Processing: " << pattern.first << " -> " << pattern.second << endl;
        NFA nfa = NFA::fromRegex(pattern.second);
        nfas.push_back(nfa);
    }
    
    // Combine all NFAs using union
    if (nfas.empty()) {
        cerr << "Error: No token patterns defined!" << endl;
        return;
    }
    
    combinedNFA = nfas[0];
    for (size_t i = 1; i < nfas.size(); i++) {
        combinedNFA = NFA::union_op(combinedNFA, nfas[i]);
    }
    
    cout << "\nConverting NFA to DFA..." << endl;
    finalDFA = DFA::fromNFA(combinedNFA);
    
    // Set token types for accepting states
    int tokenIndex = 0;
    for (const auto& pattern : tokenPatterns) {
        for (int state : finalDFA.getAcceptingStates()) {
            finalDFA.setTokenType(state, pattern.first);
        }
        tokenIndex++;
    }
    
    cout << "Build complete!" << endl;
}

void LexicalAnalyzerGenerator::generateCode(const string& outputFileName) {
    finalDFA.generateCppCode(outputFileName, tokenPatterns);
}

void LexicalAnalyzerGenerator::displayNFA() const {
    combinedNFA.display();
}

void LexicalAnalyzerGenerator::displayDFA() const {
    finalDFA.display();
}
