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

bool Parser::check(TokenType type, const std::string& val) {
    if (pos >= tokens.size()) return false;
    if (tokens[pos].type != type) return false;
    if (!val.empty() && tokens[pos].value != val) return false;
    return true;
}

void Parser::skipTo(const std::string& target) {
    while (pos < tokens.size() && tokens[pos].value != target) {
        ++pos;
    }
}

std::shared_ptr<ASTNode> Parser::parse() {
    auto programNode = std::make_shared<ASTNode>(ASTNodeType::Program, "Program");
    
    while (pos < tokens.size()) {
        auto stmt = parseStatement();
        if (stmt) {
            programNode->children.push_back(stmt);
        } else {
            // Skip unknown tokens
            if (pos < tokens.size()) {
                ++pos;
            }
        }
    }
    return programNode;
}

std::shared_ptr<ASTNode> Parser::parseStatement() {
    // Handle preprocessor directives
    if (check(TokenType::Keyword) && !peek().value.empty() && peek().value[0] == '#') {
        return parsePreprocessor();
    }
    
    // Handle function declarations
    if (check(TokenType::Keyword, "int") && pos + 1 < tokens.size() && tokens[pos + 1].value == "main") {
        return parseFunctionDeclaration();
    }
    
    // Handle return statements
    if (check(TokenType::Keyword, "return")) {
        return parseReturnStatement();
    }
    
    // Handle variable declarations
    if (check(TokenType::Keyword, "int") || check(TokenType::Keyword, "float")) {
        return parseDeclaration();
    }
    
    // Handle if statements
    if (check(TokenType::Keyword, "if")) {
        return parseIfStatement();
    }
    
    // Handle print statements (std::cout)
    if (check(TokenType::Keyword, "std")) {
        return parsePrintStatement();
    }
    
    // Handle blocks
    if (check(TokenType::Separator, "{")) {
        return parseBlock();
    }
    
    return nullptr;
}

std::shared_ptr<ASTNode> Parser::parsePreprocessor() {
    if (check(TokenType::Keyword) && !peek().value.empty() && peek().value[0] == '#') {
        auto preprocessor = std::make_shared<ASTNode>(ASTNodeType::Preprocessor, advance().value);
        return preprocessor;
    }
    return nullptr;
}

std::shared_ptr<ASTNode> Parser::parseFunctionDeclaration() {
    if (match(TokenType::Keyword, "int") && match(TokenType::Keyword, "main")) {
        match(TokenType::Separator, "(");
        match(TokenType::Separator, ")");
        
        auto funcNode = std::make_shared<ASTNode>(ASTNodeType::FunctionDeclaration, "main");
        
        if (check(TokenType::Separator, "{")) {
            funcNode->left = parseBlock();
        }
        
        return funcNode;
    }
    return nullptr;
}

std::shared_ptr<ASTNode> Parser::parseReturnStatement() {
    if (match(TokenType::Keyword, "return")) {
        auto returnNode = std::make_shared<ASTNode>(ASTNodeType::ReturnStatement, "return");
        
        if (!check(TokenType::Separator, ";")) {
            returnNode->left = parseExpression();
        }
        
        match(TokenType::Separator, ";");
        return returnNode;
    }
    return nullptr;
}

std::shared_ptr<ASTNode> Parser::parseDeclaration() {
    if (check(TokenType::Keyword, "int") || check(TokenType::Keyword, "float")) {
        auto typeToken = advance();
        
        if (check(TokenType::Identifier)) {
            auto idToken = advance();
            auto declNode = std::make_shared<ASTNode>(ASTNodeType::Declaration, typeToken.value);
            declNode->left = std::make_shared<ASTNode>(ASTNodeType::Identifier, idToken.value);
            
            if (match(TokenType::Operator, "=")) {
                declNode->right = parseExpression();
            }
            
            match(TokenType::Separator, ";");
            return declNode;
        }
    }
    return nullptr;
}

std::shared_ptr<ASTNode> Parser::parseIfStatement() {
    if (match(TokenType::Keyword, "if")) {
        match(TokenType::Separator, "(");
        auto condition = parseLogicalExpression();
        match(TokenType::Separator, ")");
        
        auto ifNode = std::make_shared<ASTNode>(ASTNodeType::IfStatement, "if");
        ifNode->left = condition;
        
        if (check(TokenType::Separator, "{")) {
            ifNode->right = parseBlock();
        }
        
        return ifNode;
    }
    return nullptr;
}

std::shared_ptr<ASTNode> Parser::parseBlock() {
    if (match(TokenType::Separator, "{")) {
        auto blockNode = std::make_shared<ASTNode>(ASTNodeType::Block, "Block");
        
        while (!check(TokenType::Separator, "}") && pos < tokens.size()) {
            auto stmt = parseStatement();
            if (stmt) {
                blockNode->children.push_back(stmt);
            } else {
                // Skip single tokens that we can't parse
                if (pos < tokens.size()) {
                    ++pos;
                }
            }
        }
        
        match(TokenType::Separator, "}");
        return blockNode;
    }
    return nullptr;
}

std::shared_ptr<ASTNode> Parser::parsePrintStatement() {
    if (match(TokenType::Keyword, "std") && match(TokenType::Operator, "::") && match(TokenType::Keyword, "cout")) {
        auto printNode = std::make_shared<ASTNode>(ASTNodeType::PrintStatement, "cout");
        
        // Simplified: just consume until semicolon and store as one string
        std::string output;
        while (!check(TokenType::Separator, ";") && pos < tokens.size()) {
            if (check(TokenType::Operator, "<<")) {
                advance(); // skip <<
            } else if (check(TokenType::Literal)) {
                output += advance().value + " ";
            } else if (check(TokenType::Keyword, "endl")) {
                output += "std::endl ";
                advance();
            } else {
                advance(); // skip other tokens
            }
        }
        
        if (!output.empty()) {
            auto outputNode = std::make_shared<ASTNode>(ASTNodeType::Literal, output);
            printNode->children.push_back(outputNode);
        }
        
        match(TokenType::Separator, ";");
        return printNode;
    }
    return nullptr;
}

std::shared_ptr<ASTNode> Parser::parseExpression() {
    return parseLogicalExpression();
}

std::shared_ptr<ASTNode> Parser::parseLogicalExpression() {
    auto left = parseComparisonExpression();
    
    while (check(TokenType::Operator, "||") || check(TokenType::Operator, "&&")) {
        auto op = advance().value;
        auto right = parseComparisonExpression();
        
        auto opNode = std::make_shared<ASTNode>(ASTNodeType::BinaryOperation, op);
        opNode->left = left;
        opNode->right = right;
        left = opNode;
    }
    
    return left;
}

std::shared_ptr<ASTNode> Parser::parseComparisonExpression() {
    auto left = parseArithmeticExpression();
    
    while (check(TokenType::Operator, "==") || check(TokenType::Operator, "!=") || 
           check(TokenType::Operator, "<") || check(TokenType::Operator, ">") ||
           check(TokenType::Operator, "<=") || check(TokenType::Operator, ">=")) {
        auto op = advance().value;
        auto right = parseArithmeticExpression();
        
        auto opNode = std::make_shared<ASTNode>(ASTNodeType::BinaryOperation, op);
        opNode->left = left;
        opNode->right = right;
        left = opNode;
    }
    
    return left;
}

std::shared_ptr<ASTNode> Parser::parseArithmeticExpression() {
    auto left = parseMultiplicativeExpression();
    
    while (check(TokenType::Operator, "+") || check(TokenType::Operator, "-")) {
        auto op = advance().value;
        auto right = parseMultiplicativeExpression();
        
        auto opNode = std::make_shared<ASTNode>(ASTNodeType::BinaryOperation, op);
        opNode->left = left;
        opNode->right = right;
        left = opNode;
    }
    
    return left;
}

std::shared_ptr<ASTNode> Parser::parseMultiplicativeExpression() {
    auto left = parsePrimary();
    
    while (check(TokenType::Operator, "*") || check(TokenType::Operator, "/")) {
        auto op = advance().value;
        auto right = parsePrimary();
        
        auto opNode = std::make_shared<ASTNode>(ASTNodeType::BinaryOperation, op);
        opNode->left = left;
        opNode->right = right;
        left = opNode;
    }
    
    return left;
}

std::shared_ptr<ASTNode> Parser::parsePrimary() {
    if (check(TokenType::Number)) {
        return std::make_shared<ASTNode>(ASTNodeType::Literal, advance().value);
    }
    
    if (check(TokenType::Literal)) {
        return std::make_shared<ASTNode>(ASTNodeType::Literal, advance().value);
    }
    
    if (check(TokenType::Identifier)) {
        return std::make_shared<ASTNode>(ASTNodeType::Identifier, advance().value);
    }
    
    if (match(TokenType::Separator, "(")) {
        auto expr = parseExpression();
        match(TokenType::Separator, ")");
        return expr;
    }
    
    return nullptr;
}

void printAST(const std::shared_ptr<ASTNode>& node, int indent) {
    if (!node) return;
    std::string pad(indent, ' ');
    std::cout << pad << node->value << " (" << static_cast<int>(node->type) << ")\n";
    
    if (node->left) printAST(node->left, indent + 2);
    if (node->right) printAST(node->right, indent + 2);
    
    for (const auto& child : node->children) {
        printAST(child, indent + 2);
    }
}