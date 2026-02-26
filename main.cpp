#include "LexicalAnalyzerGenerator.h"

void displayMenu() {
    cout << "\n========================================" << endl;
    cout << "   LEXICAL ANALYZER GENERATOR" << endl;
    cout << "========================================" << endl;
    cout << "1. Add Token Pattern" << endl;
    cout << "2. Build Lexical Analyzer" << endl;
    cout << "3. Display NFA" << endl;
    cout << "4. Display DFA" << endl;
    cout << "5. Generate C++ Code" << endl;
    cout << "6. Load Predefined Patterns (C-like Language)" << endl;
    cout << "7. Exit" << endl;
    cout << "========================================" << endl;
    cout << "Enter your choice: ";
}

void loadPredefinedPatterns(LexicalAnalyzerGenerator& generator) {
    cout << "\nLoading predefined patterns for C-like language..." << endl;
    
    // Keywords
    generator.addTokenPattern("KEYWORD_IF", "if");
    generator.addTokenPattern("KEYWORD_ELSE", "else");
    generator.addTokenPattern("KEYWORD_WHILE", "while");
    generator.addTokenPattern("KEYWORD_FOR", "for");
    generator.addTokenPattern("KEYWORD_INT", "int");
    generator.addTokenPattern("KEYWORD_FLOAT", "float");
    generator.addTokenPattern("KEYWORD_RETURN", "return");
    
    // Identifiers (simplified: letters followed by letters/digits)
    generator.addTokenPattern("IDENTIFIER", "(a|b|c|d|e|f|g|h|i|j|k|l|m|n|o|p|q|r|s|t|u|v|w|x|y|z|A|B|C|D|E|F|G|H|I|J|K|L|M|N|O|P|Q|R|S|T|U|V|W|X|Y|Z)(a|b|c|d|e|f|g|h|i|j|k|l|m|n|o|p|q|r|s|t|u|v|w|x|y|z|A|B|C|D|E|F|G|H|I|J|K|L|M|N|O|P|Q|R|S|T|U|V|W|X|Y|Z|0|1|2|3|4|5|6|7|8|9)*");
    
    // Numbers (simplified: one or more digits)
    generator.addTokenPattern("NUMBER", "(0|1|2|3|4|5|6|7|8|9)(0|1|2|3|4|5|6|7|8|9)*");
    
    // Operators
    generator.addTokenPattern("PLUS", "+");
    generator.addTokenPattern("MINUS", "-");
    generator.addTokenPattern("MULTIPLY", "*");
    generator.addTokenPattern("DIVIDE", "/");
    generator.addTokenPattern("ASSIGN", "=");
    
    // Relational operators
    generator.addTokenPattern("LESS_THAN", "<");
    generator.addTokenPattern("GREATER_THAN", ">");
    
    // Delimiters
    generator.addTokenPattern("SEMICOLON", ";");
    generator.addTokenPattern("LPAREN", "(");
    generator.addTokenPattern("RPAREN", ")");
    generator.addTokenPattern("LBRACE", "{");
    generator.addTokenPattern("RBRACE", "}");
    
    cout << "Predefined patterns loaded successfully!" << endl;
    cout << "\nSupported tokens:" << endl;
    cout << "  - Keywords: if, else, while, for, int, float, return" << endl;
    cout << "  - Identifiers: variable names (e.g., x, var1, myVar)" << endl;
    cout << "  - Numbers: integer literals (e.g., 0, 123, 456)" << endl;
    cout << "  - Operators: +, -, *, /, =" << endl;
    cout << "  - Relational: <, >" << endl;
    cout << "  - Delimiters: ; ( ) { }" << endl;
}

void printRegexHelp() {
    cout << "\n========== REGEX SYNTAX HELP ==========" << endl;
    cout << "Supported operators:" << endl;
    cout << "  |  - Union (OR)" << endl;
    cout << "  *  - Kleene star (zero or more)" << endl;
    cout << "  () - Grouping" << endl;
    cout << "  .  - Concatenation (added automatically)" << endl;
    cout << "\nExamples:" << endl;
    cout << "  a|b       - matches 'a' or 'b'" << endl;
    cout << "  ab        - matches 'ab'" << endl;
    cout << "  a*        - matches '', 'a', 'aa', 'aaa', ..." << endl;
    cout << "  (a|b)*    - matches '', 'a', 'b', 'ab', 'ba', ..." << endl;
    cout << "  a(b|c)    - matches 'ab' or 'ac'" << endl;
    cout << "=======================================" << endl;
}

int main() {
    LexicalAnalyzerGenerator generator;
    int choice;
    bool built = false;
    
    cout << "\n╔════════════════════════════════════════════════╗" << endl;
    cout << "║   LEXICAL ANALYZER GENERATOR IN C++           ║" << endl;
    cout << "║   Using Thompson's Construction & Subset      ║" << endl;
    cout << "║   Construction Algorithms                     ║" << endl;
    cout << "╚════════════════════════════════════════════════╝" << endl;
    
    printRegexHelp();
    
    while (true) {
        displayMenu();
        cin >> choice;
        cin.ignore();  // Clear newline
        
        switch (choice) {
            case 1: {
                string tokenType, pattern;
                cout << "\nEnter token type (e.g., IDENTIFIER, NUMBER): ";
                getline(cin, tokenType);
                cout << "Enter regex pattern: ";
                getline(cin, pattern);
                
                if (RegexParser::isValidRegex(pattern)) {
                    generator.addTokenPattern(tokenType, pattern);
                    cout << "Pattern added successfully!" << endl;
                } else {
                    cout << "Invalid regex pattern! Please check parentheses." << endl;
                }
                break;
            }
            
            case 2: {
                cout << "\nBuilding lexical analyzer..." << endl;
                generator.build();
                built = true;
                cout << "\nLexical analyzer built successfully!" << endl;
                break;
            }
            
            case 3: {
                if (!built) {
                    cout << "\nPlease build the analyzer first (option 2)!" << endl;
                } else {
                    generator.displayNFA();
                }
                break;
            }
            
            case 4: {
                if (!built) {
                    cout << "\nPlease build the analyzer first (option 2)!" << endl;
                } else {
                    generator.displayDFA();
                }
                break;
            }
            
            case 5: {
                if (!built) {
                    cout << "\nPlease build the analyzer first (option 2)!" << endl;
                } else {
                    string filename;
                    cout << "\nEnter output filename (e.g., lexer.cpp): ";
                    getline(cin, filename);
                    generator.generateCode(filename);
                    cout << "\nYou can now compile and run the generated file:" << endl;
                    cout << "  g++ -o lexer " << filename << endl;
                    cout << "  ./lexer" << endl;
                }
                break;
            }
            
            case 6: {
                loadPredefinedPatterns(generator);
                break;
            }
            
            case 7: {
                cout << "\nThank you for using Lexical Analyzer Generator!" << endl;
                cout << "Project by: Anees Asad, Hasham Ahmed, Zohaib Hassan" << endl;
                return 0;
            }
            
            default: {
                cout << "\nInvalid choice! Please try again." << endl;
                break;
            }
        }
    }
    
    return 0;
}
