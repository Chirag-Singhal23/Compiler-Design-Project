#include "../include/Tokenizer.h"
#include <cctype>
#include <unordered_set>
#include <cstring>

std::vector<Token> Tokenizer::tokenize(const std::string& code) {
    std::vector<Token> tokens;
    std::unordered_set<std::string> keywords = {
        "int", "float", "if", "else", "while", "for", "do", "return", 
        "include", "iostream", "std", "cout", "cin", "endl", "main", "true", "false"
    };
    
    size_t i = 0;
    while (i < code.length()) {
        if (isspace(code[i])) {
            ++i;
            continue;
        }

        // Skip comments
        if (i < code.length() - 1 && code[i] == '/' && code[i+1] == '/') {
            while (i < code.length() && code[i] != '\n') {
                ++i;
            }
            continue;
        }

        // Handle preprocessor directives - capture the full line
        if (code[i] == '#') {
            std::string directive;
            while (i < code.length() && code[i] != '\n') {
                directive += code[i++];
            }
            tokens.push_back({TokenType::Keyword, directive});
            continue;
        }

        // Handle string literals
        if (code[i] == '"') {
            std::string str = "\"";
            ++i;
            while (i < code.length() && code[i] != '"') {
                if (code[i] == '\\' && i + 1 < code.length()) {
                    str += code[i++]; // escape character
                }
                str += code[i++];
            }
            if (i < code.length()) str += code[i++]; // closing quote
            tokens.push_back({TokenType::Literal, str});
            continue;
        }

        // Handle identifiers and keywords
        if (isalpha(code[i]) || code[i] == '_') {
            std::string id;
            while (i < code.length() && (isalnum(code[i]) || code[i] == '_')) {
                id += code[i++];
            }
            TokenType type = keywords.count(id) ? TokenType::Keyword : TokenType::Identifier;
            // Special handling for boolean literals
            if (id == "true" || id == "false") {
                type = TokenType::Literal;
            }
            tokens.push_back({type, id});
            continue;
        }

        // Handle numbers (including floating point)
        if (isdigit(code[i])) {
            std::string num;
            while (i < code.length() && (isdigit(code[i]) || code[i] == '.')) {
                num += code[i++];
            }
            tokens.push_back({TokenType::Number, num});
            continue;
        }

        // Handle multi-character operators (check longer ones first)
        if (i < code.length() - 1) {
            std::string twoChar = code.substr(i, 2);
            if (twoChar == "==" || twoChar == "!=" || twoChar == "<=" || twoChar == ">=" ||
                twoChar == "||" || twoChar == "&&" || twoChar == "::" || twoChar == "<<" ||
                twoChar == ">>" || twoChar == "++" || twoChar == "--" || twoChar == "+=" || 
                twoChar == "-=" || twoChar == "*=" || twoChar == "/=") {
                tokens.push_back({TokenType::Operator, twoChar});
                i += 2;
                continue;
            }
        }

        // Single character operators
        if (strchr("+-*/=<>!", code[i])) {
            tokens.push_back({TokenType::Operator, std::string(1, code[i++])});
            continue;
        }

        // Separators
        if (strchr(";,(){}[]", code[i])) {
            tokens.push_back({TokenType::Separator, std::string(1, code[i++])});
            continue;
        }

        // Unknown character
        tokens.push_back({TokenType::Unknown, std::string(1, code[i++])});
    }

    return tokens;
}