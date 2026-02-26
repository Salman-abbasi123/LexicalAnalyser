#ifndef LEXICAL_ANALYZER_GENERATOR_H
#define LEXICAL_ANALYZER_GENERATOR_H

#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <stack>
#include <queue>
#include <algorithm>
#include <fstream>

using namespace std;

// Forward declarations
class NFA;
class DFA;

/**
 * @brief Represents a state in an NFA or DFA
 */
class State {
public:
    int id;
    bool isAccepting;
    string tokenType;
    
    State(int stateId = 0) : id(stateId), isAccepting(false), tokenType("") {}
    
    bool operator<(const State& other) const {
        return id < other.id;
    }
    
    bool operator==(const State& other) const {
        return id == other.id;
    }
};

/**
 * @brief Represents a transition in an automaton
 */
class Transition {
public:
    int fromState;
    int toState;
    char symbol;  // '\0' represents epsilon transition
    
    Transition(int from, int to, char sym) 
        : fromState(from), toState(to), symbol(sym) {}
};

/**
 * @brief Non-deterministic Finite Automaton implementation
 */
class NFA {
private:
    vector<State> states;
    vector<Transition> transitions;
    int startState;
    set<int> acceptingStates;
    int stateCounter;
    
public:
    NFA();
    
    // Thompson's Construction methods
    static NFA fromSymbol(char symbol);
    static NFA concatenate(const NFA& nfa1, const NFA& nfa2);
    static NFA union_op(const NFA& nfa1, const NFA& nfa2);
    static NFA kleeneStar(const NFA& nfa);
    static NFA fromRegex(const string& regex);
    
    // Helper methods
    void addState(int stateId, bool isAccepting = false);
    void addTransition(int from, int to, char symbol);
    void setStartState(int stateId);
    void addAcceptingState(int stateId);
    
    // Getters
    const vector<State>& getStates() const { return states; }
    const vector<Transition>& getTransitions() const { return transitions; }
    int getStartState() const { return startState; }
    const set<int>& getAcceptingStates() const { return acceptingStates; }
    
    // Utility methods
    set<int> epsilonClosure(int state) const;
    set<int> epsilonClosure(const set<int>& states) const;
    set<int> move(const set<int>& states, char symbol) const;
    
    void display() const;
};

/**
 * @brief Deterministic Finite Automaton implementation
 */
class DFA {
private:
    vector<State> states;
    map<pair<int, char>, int> transitions;  // (state, symbol) -> next state
    int startState;
    set<int> acceptingStates;
    set<char> alphabet;
    map<int, string> stateToTokenType;
    
public:
    DFA();
    
    // Subset construction from NFA to DFA
    static DFA fromNFA(const NFA& nfa);
    
    // Helper methods
    void addState(int stateId, bool isAccepting = false);
    void addTransition(int from, char symbol, int to);
    void setStartState(int stateId);
    void addAcceptingState(int stateId);
    void setTokenType(int stateId, const string& tokenType);
    
    // Getters
    const vector<State>& getStates() const { return states; }
    const map<pair<int, char>, int>& getTransitions() const { return transitions; }
    int getStartState() const { return startState; }
    const set<int>& getAcceptingStates() const { return acceptingStates; }
    const set<char>& getAlphabet() const { return alphabet; }
    
    // DFA operations
    int getNextState(int currentState, char symbol) const;
    bool accepts(const string& input) const;
    
    void display() const;
    
    // Code generation
    void generateCppCode(const string& filename, const map<string, string>& tokenPatterns) const;
};

/**
 * @brief Main Lexical Analyzer Generator class
 */
class LexicalAnalyzerGenerator {
private:
    map<string, string> tokenPatterns;  // tokenType -> regex pattern
    NFA combinedNFA;
    DFA finalDFA;
    
public:
    LexicalAnalyzerGenerator();
    
    // Add token pattern
    void addTokenPattern(const string& tokenType, const string& pattern);
    
    // Build the lexical analyzer
    void build();
    
    // Generate C++ code
    void generateCode(const string& outputFileName);
    
    // Display information
    void displayNFA() const;
    void displayDFA() const;
};

/**
 * @brief Utility class for regex parsing
 */
class RegexParser {
public:
    // Check if regex is valid
    static bool isValidRegex(const string& regex);
    
    // Convert infix regex to postfix for easier processing
    static string infixToPostfix(const string& regex);
    
    // Get operator precedence
    static int getPrecedence(char op);
    
    // Check if character is operator
    static bool isOperator(char c);
};

#endif // LEXICAL_ANALYZER_GENERATOR_H
