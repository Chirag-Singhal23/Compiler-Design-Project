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
    Identifier,
    IfStatement,
    Block,
    ExpressionStatement,
    PrintStatement,
    FunctionDeclaration,
    ReturnStatement,
    Preprocessor
};

struct ASTNode {
    ASTNodeType type;
    std::string value;
    std::shared_ptr<ASTNode> left;
    std::shared_ptr<ASTNode> right;
    std::vector<std::shared_ptr<ASTNode>> children; // For statements that need multiple children

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
    std::shared_ptr<ASTNode> parseDeclaration();
    std::shared_ptr<ASTNode> parseIfStatement();
    std::shared_ptr<ASTNode> parseBlock();
    std::shared_ptr<ASTNode> parsePrintStatement();
    std::shared_ptr<ASTNode> parseFunctionDeclaration();
    std::shared_ptr<ASTNode> parseReturnStatement();
    std::shared_ptr<ASTNode> parsePreprocessor();
    std::shared_ptr<ASTNode> parseLogicalExpression();
    std::shared_ptr<ASTNode> parseComparisonExpression();
    std::shared_ptr<ASTNode> parseArithmeticExpression();
    std::shared_ptr<ASTNode> parseMultiplicativeExpression(); // Added this missing function
    std::shared_ptr<ASTNode> parsePrimary();

    Token peek();
    Token advance();
    bool match(TokenType type, const std::string& val = "");
    bool check(TokenType type, const std::string& val = "");
    void skipTo(const std::string& target);
};

void printAST(const std::shared_ptr<ASTNode>& node, int indent = 0);

#endif // PARSER_H