#include "Parser.h"
#include <iostream>

Parser::Parser(const std::vector<Token>& tokens) : tokens(tokens), pos(0) {}

Token Parser::peek() {
    return pos < tokens.size() ? tokens[pos] : Token{TokenType::Unknown, ""};
}

Token Parser::advance() {
    return pos < tokens.size() ? tokens[pos++] : Token{TokenType::Unknown, ""};
}

bool Parser::match(TokenType type, const std::string& val) {
    if (pos >= tokens.size()) return false;
    if (tokens[pos].type != type) return false;
    if (!val.empty() && tokens[pos].value != val) return false;
    ++pos;
    return true;
}

std::shared_ptr<ASTNode> Parser::parse() {
    auto programNode = std::make_shared<ASTNode>(ASTNodeType::Program, "Program");
    while (pos < tokens.size()) {
        auto stmt = parseStatement();
        if (stmt) {
            if (!programNode->left) {
                programNode->left = stmt;
            } else {
                // chain statements to the right (simple linked list)
                auto current = programNode->left;
                while (current->right) current = current->right;
                current->right = stmt;
            }
        } else {
            break; // stop if parseStatement fails
        }
    }
    return programNode;
}

std::shared_ptr<ASTNode> Parser::parseStatement() {
    // Try parse assignment or expression ending with ';'
    auto node = parseAssignment();
    if (!node) node = parseExpression();

    // Expect and consume ';'
    if (match(TokenType::Separator, ";")) {
        return node;
    }
    // If no semicolon, parse failed for this statement
    return nullptr;
}

std::shared_ptr<ASTNode> Parser::parseAssignment() {
    // assignment: Identifier '=' expression
    if (peek().type == TokenType::Identifier) {
        auto idToken = advance();
        if (match(TokenType::Operator, "=")) {
            auto expr = parseExpression();
            auto assignNode = std::make_shared<ASTNode>(ASTNodeType::Assignment, "=");
            assignNode->left = std::make_shared<ASTNode>(ASTNodeType::Identifier, idToken.value);
            assignNode->right = expr;
            return assignNode;
        }
         else {
            // not assignment, rollback position
            --pos;
        }
    }
    return nullptr;
}

std::shared_ptr<ASTNode> Parser::parseExpression() {
    // simple expression: Literal or Identifier [ Operator Literal or Identifier ]
    Token leftToken = advance();
    ASTNodeType leftType = (leftToken.type == TokenType::Number || leftToken.type == TokenType::Literal) 
                           ? ASTNodeType::Literal : ASTNodeType::Identifier;
    auto left = std::make_shared<ASTNode>(leftType, leftToken.value);

    if (peek().type == TokenType::Operator) {
        std::string op = advance().value;
        Token rightToken = advance();
        ASTNodeType rightType = (rightToken.type == TokenType::Number || rightToken.type == TokenType::Literal) 
                                ? ASTNodeType::Literal : ASTNodeType::Identifier;
        auto right = std::make_shared<ASTNode>(rightType, rightToken.value);

        auto opNode = std::make_shared<ASTNode>(ASTNodeType::BinaryOperation, op);
        opNode->left = left;
        opNode->right = right;
        return opNode;
    }
    return left;
}

void printAST(const std::shared_ptr<ASTNode>& node, int indent) {
    if (!node) return;
    std::string pad(indent, ' ');
    std::cout << pad << node->value << " (" << static_cast<int>(node->type) << ")\n";
    printAST(node->left, indent + 2);
    printAST(node->right, indent + 2);
}
