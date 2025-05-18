#include "Tokenizer.h"
#include <iostream>
#include <cctype>
#include <unordered_set>
#include <unordered_map>

std::vector<Token> Tokenizer::tokenize(const std::string& code) {
    std::vector<Token> tokens;
    std::unordered_set<std::string> keywords = {"int", "float", "if", "else", "while", "for", "return"};
    std::unordered_map<std::string, TokenType> literals = {
        {"true", TokenType::Literal},
        {"false", TokenType::Literal}
    };
    
    // Operators that can be combined
    std::unordered_map<std::string, TokenType> multiCharOps = {
        {"==", TokenType::Operator},
        {"!=", TokenType::Operator},
        {"<=", TokenType::Operator},
        {">=", TokenType::Operator},
        {"&&", TokenType::Operator},
        {"||", TokenType::Operator}
    };
    
    std::unordered_set<char> singleCharOps = {'+', '-', '*', '/', '=', '>', '<', '!'};
    std::unordered_set<char> separators = {';', ',', '(', ')', '{', '}'};

    size_t i = 0;
    while (i < code.length()) {
        // Skip whitespace
        if (isspace(code[i])) {
            ++i;
            continue;
        }

        // Handle identifiers and keywords
        if (isalpha(code[i]) || code[i] == '_') {
            std::string id;
            while (i < code.length() && (isalnum(code[i]) || code[i] == '_')) {
                id += code[i++];
            }
            
            // Check if it's a keyword, literal, or identifier
            if (keywords.count(id)) {
                tokens.push_back({TokenType::Keyword, id});
            } else if (literals.count(id)) {
                tokens.push_back({literals[id], id});
            } else {
                tokens.push_back({TokenType::Identifier, id});
            }
        }
        // Handle numbers
        else if (isdigit(code[i])) {
            std::string num;
            while (i < code.length() && isdigit(code[i])) {
                num += code[i++];
            }
            tokens.push_back({TokenType::Number, num});
        }
        // Handle operators
        else if (singleCharOps.count(code[i])) {
            // Try to match multi-character operators first
            if (i + 1 < code.length()) {
                std::string op = code.substr(i, 2);
                if (multiCharOps.count(op)) {
                    tokens.push_back({multiCharOps[op], op});
                    i += 2;
                    continue;
                }
            }
            
            // Fall back to single-character operator
            tokens.push_back({TokenType::Operator, std::string(1, code[i++])});
        }
        // Handle separators
        else if (separators.count(code[i])) {
            tokens.push_back({TokenType::Separator, std::string(1, code[i++])});
        }
        // Handle string literals
        else if (code[i] == '"' || code[i] == '\'') {
            char quote = code[i++]; // Consume opening quote
            std::string literal;
            
            while (i < code.length() && code[i] != quote) {
                // Handle escape sequences
                if (code[i] == '\\' && i + 1 < code.length()) {
                    literal += code[i++]; // Add the backslash
                }
                literal += code[i++];
            }
            
            if (i < code.length()) {
                ++i; // Consume closing quote
            }
            
            tokens.push_back({TokenType::Literal, literal});
        }
        // Handle unknown tokens
        else {
            tokens.push_back({TokenType::Unknown, std::string(1, code[i++])});
        }
    }

    return tokens;
}