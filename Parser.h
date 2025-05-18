#ifndef PARSER_H
#define PARSER_H

#include "Tokenizer.h"
#include <memory>
#include <vector>

enum class ASTNodeType {
    Program,
    Declaration,
    Assignment,
    BinaryOperation,
    Literal,
    Identifier
};

struct ASTNode {
    ASTNodeType type;
    std::string value;
    std::shared_ptr<ASTNode> left;
    std::shared_ptr<ASTNode> right;

    ASTNode(ASTNodeType t, const std::string& val) : type(t), value(val), left(nullptr), right(nullptr) {}
};

class Parser {
public:
    Parser(const std::vector<Token>& tokens);
    std::shared_ptr<ASTNode> parse();

private:
    std::vector<Token> tokens;
    size_t pos;

    std::shared_ptr<ASTNode> parseStatement();
    std::shared_ptr<ASTNode> parseExpression();
    std::shared_ptr<ASTNode> parseAssignment();

    Token peek();
    Token advance();
    bool match(TokenType type, const std::string& val = "");
};

void printAST(const std::shared_ptr<ASTNode>& node, int indent = 0);

#endif // PARSER_H
