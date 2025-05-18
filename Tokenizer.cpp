#include "Tokenizer.h"
#include <cctype>
#include <unordered_set>

std::vector<Token> Tokenizer::tokenize(const std::string& code) {
    std::vector<Token> tokens;
    std::unordered_set<std::string> keywords = {"int", "float", "if", "else", "while", "for", "return"};
    std::unordered_set<char> operators = {'+', '-', '*', '/', '=', '>', '<', '!'};
    std::unordered_set<char> separators = {';', ',', '(', ')', '{', '}'};

    size_t i = 0;
    while (i < code.length()) {
        if (isspace(code[i])) {
            ++i;
            continue;
        }

        if (isalpha(code[i]) || code[i] == '_') {
            std::string id;
            while (isalnum(code[i]) || code[i] == '_') {
                id += code[i++];
            }
            tokens.push_back({keywords.count(id) ? TokenType::Keyword : TokenType::Identifier, id});
        }
        else if (isdigit(code[i])) {
            std::string num;
            while (isdigit(code[i])) {
                num += code[i++];
            }
            tokens.push_back({TokenType::Number, num});
        }
        else if (operators.count(code[i])) {
            std::string op;
            op += code[i++];
            if (i < code.length() && code[i] == '=') {
                op += code[i++];
            }
            tokens.push_back({TokenType::Operator, op});
        }
        else if (separators.count(code[i])) {
            tokens.push_back({TokenType::Separator, std::string(1, code[i++])});
        }
        else {
            tokens.push_back({TokenType::Unknown, std::string(1, code[i++])});
        }
    }

    return tokens;
}
