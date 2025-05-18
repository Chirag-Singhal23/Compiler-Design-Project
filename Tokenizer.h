#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <string>
#include <vector>

enum class TokenType {
    Identifier,
    Keyword,
    Operator,
    Number,
    Separator,
    Literal,
    Unknown
};

struct Token {
    TokenType type;
    std::string value;
};

class Tokenizer {
public:
    std::vector<Token> tokenize(const std::string& code);
};

#endif // TOKENIZER_H
