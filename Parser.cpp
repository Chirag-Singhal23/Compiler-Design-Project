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
                // Chain statements to the right (simple linked list)
                auto current = programNode->left;
                while (current->right) current = current->right;
                current->right = stmt;
            }
        } else {
            // Skip unrecognized tokens
            std::cout << "Warning: Skipping unrecognized token: " << tokens[pos].value << std::endl;
            ++pos;
        }
    }
    return programNode;
}

std::shared_ptr<ASTNode> Parser::parseStatement() {
    // Try to parse declaration
    if (pos < tokens.size() && tokens[pos].type == TokenType::Keyword) {
        if (tokens[pos].value == "int" || tokens[pos].value == "float") {
            return parseDeclaration();
        }
    }
    
    // Try parse if statement
    if (pos < tokens.size() && tokens[pos].type == TokenType::Keyword && tokens[pos].value == "if") {
        return parseIfStatement();
    }

    // Try parse assignment or expression ending with ';'
    auto node = parseAssignment();
    if (!node) node = parseExpression();

    // Expect and consume ';'
    if (node && match(TokenType::Separator, ";")) {
        return node;
    }
    
    // Handle block (compound statement)
    if (pos < tokens.size() && tokens[pos].type == TokenType::Separator && tokens[pos].value == "{") {
        return parseBlock();
    }
    
    // If no valid statement found, return null
    return nullptr;
}

std::shared_ptr<ASTNode> Parser::parseDeclaration() {
    // Format: "type identifier [= expression] ;"
    std::string type = advance().value;
    
    if (pos < tokens.size() && tokens[pos].type == TokenType::Identifier) {
        std::string name = advance().value;
        
        auto declNode = std::make_shared<ASTNode>(ASTNodeType::Declaration, type);
        declNode->left = std::make_shared<ASTNode>(ASTNodeType::Identifier, name);
        
        // Check for initialization
        if (match(TokenType::Operator, "=")) {
            auto expr = parseExpression();
            declNode->right = expr;
        }
        
        // Consume the semicolon
        match(TokenType::Separator, ";");
        return declNode;
    }
    
    return nullptr;
}

std::shared_ptr<ASTNode> Parser::parseIfStatement() {
    // Format: if ( condition ) statement [else statement]
    advance(); // consume 'if'
    
    if (!match(TokenType::Separator, "(")) return nullptr;
    
    // Parse condition
    auto condition = parseExpression();
    
    if (!match(TokenType::Separator, ")")) return nullptr;
    
    // Parse 'then' statement
    auto thenStmt = parseStatement();
    
    auto ifNode = std::make_shared<ASTNode>(ASTNodeType::IfStatement, "if");
    ifNode->left = condition;
    ifNode->right = thenStmt;
    
    // Check for 'else'
    if (pos < tokens.size() && tokens[pos].type == TokenType::Keyword && tokens[pos].value == "else") {
        advance(); // consume 'else'
        auto elseStmt = parseStatement();
        
        // Create else node and link it to the if node
        auto elseNode = std::make_shared<ASTNode>(ASTNodeType::ElseStatement, "else");
        elseNode->left = elseStmt;
        
        // Replace the right branch with the else node
        ifNode->right = elseNode;
    }
    
    return ifNode;
}

std::shared_ptr<ASTNode> Parser::parseBlock() {
    // Format: { statement* }
    advance(); // consume '{'
    
    auto blockNode = std::make_shared<ASTNode>(ASTNodeType::Block, "block");
    
    std::shared_ptr<ASTNode> firstStmt = nullptr;
    std::shared_ptr<ASTNode> lastStmt = nullptr;
    
    // Parse statements until '}'
    while (pos < tokens.size() && tokens[pos].type != TokenType::Separator && tokens[pos].value != "}") {
        auto stmt = parseStatement();
        if (stmt) {
            if (!firstStmt) {
                firstStmt = stmt;
                lastStmt = stmt;
            } else {
                lastStmt->right = stmt;
                lastStmt = stmt;
            }
        } else {
            // Skip unrecognized tokens
            ++pos;
        }
    }
    
    // Consume '}'
    match(TokenType::Separator, "}");
    
    blockNode->left = firstStmt;
    return blockNode;
}

std::shared_ptr<ASTNode> Parser::parseAssignment() {
    // assignment: Identifier '=' expression
    if (peek().type == TokenType::Identifier) {
        size_t savedPos = pos;
        auto idToken = advance();
        if (match(TokenType::Operator, "=")) {
            auto expr = parseExpression();
            auto assignNode = std::make_shared<ASTNode>(ASTNodeType::Assignment, "=");
            assignNode->left = std::make_shared<ASTNode>(ASTNodeType::Identifier, idToken.value);
            assignNode->right = expr;
            return assignNode;
        } else {
            // not assignment, rollback position
            pos = savedPos;
        }
    }
    return nullptr;
}

std::shared_ptr<ASTNode> Parser::parseExpression() {
    return parseBooleanExpression();
}

std::shared_ptr<ASTNode> Parser::parseBooleanExpression() {
    auto left = parseComparisonExpression();
    
    while (pos < tokens.size() && tokens[pos].type == TokenType::Unknown) {
        // Handle boolean operators
        if (tokens[pos].value == "&" && pos + 1 < tokens.size() && tokens[pos + 1].value == "&") {
            advance(); // consume first '&'
            advance(); // consume second '&'
            
            auto right = parseComparisonExpression();
            auto opNode = std::make_shared<ASTNode>(ASTNodeType::BinaryOperation, "&&");
            opNode->left = left;
            opNode->right = right;
            left = opNode;
        } else if (tokens[pos].value == "|" && pos + 1 < tokens.size() && tokens[pos + 1].value == "|") {
            advance(); // consume first '|'
            advance(); // consume second '|'
            
            auto right = parseComparisonExpression();
            auto opNode = std::make_shared<ASTNode>(ASTNodeType::BinaryOperation, "||");
            opNode->left = left;
            opNode->right = right;
            left = opNode;
        } else {
            break;
        }
    }
    
    return left;
}

std::shared_ptr<ASTNode> Parser::parseComparisonExpression() {
    auto left = parseArithmeticExpression();
    
    if (pos < tokens.size() && tokens[pos].type == TokenType::Operator) {
        std::string op = tokens[pos].value;
        if (op == "==" || op == "!=" || op == "<" || op == "<=" || op == ">" || op == ">=") {
            advance(); // consume operator
            
            auto right = parseArithmeticExpression();
            auto opNode = std::make_shared<ASTNode>(ASTNodeType::BinaryOperation, op);
            opNode->left = left;
            opNode->right = right;
            return opNode;
        }
    }
    
    return left;
}

std::shared_ptr<ASTNode> Parser::parseArithmeticExpression() {
    auto left = parseTerm();
    
    while (pos < tokens.size() && tokens[pos].type == TokenType::Operator) {
        std::string op = tokens[pos].value;
        if (op == "+" || op == "-") {
            advance(); // consume operator
            
            auto right = parseTerm();
            auto opNode = std::make_shared<ASTNode>(ASTNodeType::BinaryOperation, op);
            opNode->left = left;
            opNode->right = right;
            left = opNode;
        } else {
            break;
        }
    }
    
    return left;
}

std::shared_ptr<ASTNode> Parser::parseTerm() {
    auto left = parseFactor();
    
    while (pos < tokens.size() && tokens[pos].type == TokenType::Operator) {
        std::string op = tokens[pos].value;
        if (op == "*" || op == "/") {
            advance(); // consume operator
            
            auto right = parseFactor();
            auto opNode = std::make_shared<ASTNode>(ASTNodeType::BinaryOperation, op);
            opNode->left = left;
            opNode->right = right;
            left = opNode;
        } else {
            break;
        }
    }
    
    return left;
}

std::shared_ptr<ASTNode> Parser::parseFactor() {
    // Handle literals and identifiers
    if (pos < tokens.size()) {
        Token token = peek();
        
        if (token.type == TokenType::Number) {
            advance(); // consume number
            return std::make_shared<ASTNode>(ASTNodeType::Literal, token.value);
        } else if (token.type == TokenType::Identifier) {
            advance(); // consume identifier
            
            // Handle boolean literals
            if (token.value == "true" || token.value == "false") {
                return std::make_shared<ASTNode>(ASTNodeType::Literal, token.value);
            }
            
            return std::make_shared<ASTNode>(ASTNodeType::Identifier, token.value);
        } else if (token.type == TokenType::Separator && token.value == "(") {
            advance(); // consume '('
            auto expr = parseExpression();
            match(TokenType::Separator, ")"); // consume ')'
            return expr;
        }
    }
    
    // Return a default node if nothing matches
    return std::make_shared<ASTNode>(ASTNodeType::Literal, "0");
}

void printAST(const std::shared_ptr<ASTNode>& node, int indent) {
    if (!node) return;
    std::string pad(indent, ' ');
    
    // Get node type as string
    std::string typeStr;
    switch (node->type) {
        case ASTNodeType::Program: typeStr = "Program"; break;
        case ASTNodeType::Declaration: typeStr = "Declaration"; break;
        case ASTNodeType::Assignment: typeStr = "Assignment"; break;
        case ASTNodeType::BinaryOperation: typeStr = "BinaryOperation"; break;
        case ASTNodeType::Literal: typeStr = "Literal"; break;
        case ASTNodeType::Identifier: typeStr = "Identifier"; break;
        case ASTNodeType::IfStatement: typeStr = "IfStatement"; break;
        case ASTNodeType::ElseStatement: typeStr = "ElseStatement"; break;
        case ASTNodeType::Block: typeStr = "Block"; break;
        default: typeStr = "Unknown"; break;
    }
    
    std::cout << pad << node->value << " (" << typeStr << ")\n";
    printAST(node->left, indent + 2);
    printAST(node->right, indent + 2);
}