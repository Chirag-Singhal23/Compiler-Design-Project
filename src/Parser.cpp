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
    
    // Handle for loops
    if (check(TokenType::Keyword, "for")) {
        return parseForStatement();
    }
    
    // Handle while loops
    if (check(TokenType::Keyword, "while")) {
        return parseWhileStatement();
    }
    
    // Handle do-while loops
    if (check(TokenType::Keyword, "do")) {
        return parseDoWhileStatement();
    }
    
    // Handle input/output statements (std::cout or std::cin) - FIXED
    if (check(TokenType::Keyword, "std")) {
        // Look ahead to see if it's cout or cin
        if (pos + 2 < tokens.size() && tokens[pos + 1].value == "::") {
            if (tokens[pos + 2].value == "cout") {
                return parsePrintStatement();
            } else if (tokens[pos + 2].value == "cin") {
                return parseInputStatement();
            }
        }
    }
    
    // Handle blocks
    if (check(TokenType::Separator, "{")) {
        return parseBlock();
    }
    
    // Handle increment/decrement statements (i++, ++i, i--, --i)
    if (check(TokenType::Identifier)) {
        // Look ahead to see if this is an increment/decrement statement
        if (pos + 1 < tokens.size() && 
            (tokens[pos + 1].value == "++" || tokens[pos + 1].value == "--" || 
             tokens[pos + 1].value == "+=" || tokens[pos + 1].value == "-=" ||
             tokens[pos + 1].value == "*=" || tokens[pos + 1].value == "/=")) {
            auto stmt = parseIncrementExpression();
            if (stmt && match(TokenType::Separator, ";")) {
                auto exprStmt = std::make_shared<ASTNode>(ASTNodeType::ExpressionStatement, "ExpressionStatement");
                exprStmt->left = stmt;
                return exprStmt;
            }
        }
        // Handle assignments (identifier = expression)
        else if (pos + 1 < tokens.size() && 
                 tokens[pos + 1].type == TokenType::Operator && tokens[pos + 1].value == "=") {
            return parseAssignment();
        }
    }
    
    // Handle pre-increment/decrement statements (++i, --i)
    if (check(TokenType::Operator, "++") || check(TokenType::Operator, "--")) {
        auto stmt = parseIncrementExpression();
        if (stmt && match(TokenType::Separator, ";")) {
            auto exprStmt = std::make_shared<ASTNode>(ASTNodeType::ExpressionStatement, "ExpressionStatement");
            exprStmt->left = stmt;
            return exprStmt;
        }
    }
    
    return nullptr;
}

std::shared_ptr<ASTNode> Parser::parseInputStatement() {
    if (match(TokenType::Keyword, "std") && match(TokenType::Operator, "::") && match(TokenType::Keyword, "cin")) {
        auto inputNode = std::make_shared<ASTNode>(ASTNodeType::InputStatement, "cin");
        
        // Parse each part of the cin statement
        while (!check(TokenType::Separator, ";") && pos < tokens.size()) {
            if (check(TokenType::Operator, ">>")) {
                advance(); // skip >>
            } else if (check(TokenType::Identifier)) {
                // Handle variables in cin
                auto varNode = std::make_shared<ASTNode>(ASTNodeType::Identifier, advance().value);
                inputNode->children.push_back(varNode);
            } else {
                // Skip other tokens we don't handle
                advance();
            }
        }
        
        match(TokenType::Separator, ";");
        return inputNode;
    }
    return nullptr;
}

std::shared_ptr<ASTNode> Parser::parseForStatement() {
    if (match(TokenType::Keyword, "for")) {
        match(TokenType::Separator, "(");
        
        auto forNode = std::make_shared<ASTNode>(ASTNodeType::ForStatement, "for");
        
        // Parse initialization (e.g., int i = 0)
        auto init = parseStatement();
        if (init) {
            forNode->children.push_back(init);
        } else {
            // Handle empty initialization
            match(TokenType::Separator, ";");
            forNode->children.push_back(nullptr);
        }
        
        // Parse condition (e.g., i < 10)
        auto condition = parseExpression();
        forNode->children.push_back(condition);
        match(TokenType::Separator, ";");
        
        // Parse increment (e.g., i++)
        auto increment = parseIncrementExpression();
        forNode->children.push_back(increment);
        match(TokenType::Separator, ")");
        
        // Parse body
        auto body = parseStatement();
        if (!body && check(TokenType::Separator, "{")) {
            body = parseBlock();
        }
        forNode->children.push_back(body);
        
        return forNode;
    }
    return nullptr;
}

std::shared_ptr<ASTNode> Parser::parseWhileStatement() {
    if (match(TokenType::Keyword, "while")) {
        match(TokenType::Separator, "(");
        
        auto whileNode = std::make_shared<ASTNode>(ASTNodeType::WhileStatement, "while");
        
        // Parse condition
        auto condition = parseExpression();
        whileNode->left = condition;
        match(TokenType::Separator, ")");
        
        // Parse body
        auto body = parseStatement();
        if (!body && check(TokenType::Separator, "{")) {
            body = parseBlock();
        }
        whileNode->right = body;
        
        return whileNode;
    }
    return nullptr;
}

std::shared_ptr<ASTNode> Parser::parseDoWhileStatement() {
    if (match(TokenType::Keyword, "do")) {
        auto doWhileNode = std::make_shared<ASTNode>(ASTNodeType::DoWhileStatement, "do-while");
        
        // Parse body first
        auto body = parseStatement();
        if (!body && check(TokenType::Separator, "{")) {
            body = parseBlock();
        }
        doWhileNode->left = body;
        
        // Parse while condition
        match(TokenType::Keyword, "while");
        match(TokenType::Separator, "(");
        auto condition = parseExpression();
        doWhileNode->right = condition;
        match(TokenType::Separator, ")");
        match(TokenType::Separator, ";");
        
        return doWhileNode;
    }
    return nullptr;
}

std::shared_ptr<ASTNode> Parser::parseIncrementExpression() {
    // Handle i++, ++i, i--, --i, i += 1, i -= 1, etc.
    if (check(TokenType::Identifier)) {
        auto id = advance();
        
        // Post-increment/decrement (i++, i--)
        if (check(TokenType::Operator, "++") || check(TokenType::Operator, "--")) {
            auto op = advance();
            auto incNode = std::make_shared<ASTNode>(ASTNodeType::PostIncrement, op.value);
            incNode->left = std::make_shared<ASTNode>(ASTNodeType::Identifier, id.value);
            return incNode;
        }
        
        // Compound assignment (i += 1, i -= 1, etc.)
        if (check(TokenType::Operator, "+=") || check(TokenType::Operator, "-=") || 
            check(TokenType::Operator, "*=") || check(TokenType::Operator, "/=")) {
            auto op = advance();
            auto expr = parseExpression();
            auto compoundNode = std::make_shared<ASTNode>(ASTNodeType::CompoundAssignment, op.value);
            compoundNode->left = std::make_shared<ASTNode>(ASTNodeType::Identifier, id.value);
            compoundNode->right = expr;
            return compoundNode;
        }
        
        // Regular assignment (i = i + 1)
        if (check(TokenType::Operator, "=")) {
            advance(); // skip =
            auto expr = parseExpression();
            auto assignNode = std::make_shared<ASTNode>(ASTNodeType::Assignment, "=");
            assignNode->left = std::make_shared<ASTNode>(ASTNodeType::Identifier, id.value);
            assignNode->right = expr;
            return assignNode;
        }
        
        // Just the identifier (in case of empty increment)
        return std::make_shared<ASTNode>(ASTNodeType::Identifier, id.value);
    }
    
    // Pre-increment/decrement (++i, --i)
    if (check(TokenType::Operator, "++") || check(TokenType::Operator, "--")) {
        auto op = advance();
        if (check(TokenType::Identifier)) {
            auto id = advance();
            auto preIncNode = std::make_shared<ASTNode>(ASTNodeType::PreIncrement, op.value);
            preIncNode->left = std::make_shared<ASTNode>(ASTNodeType::Identifier, id.value);
            return preIncNode;
        }
    }
    
    return nullptr;
}

std::shared_ptr<ASTNode> Parser::parseAssignment() {
    if (check(TokenType::Identifier)) {
        auto id = advance();
        if (match(TokenType::Operator, "=")) {
            auto assignNode = std::make_shared<ASTNode>(ASTNodeType::Assignment, "=");
            assignNode->left = std::make_shared<ASTNode>(ASTNodeType::Identifier, id.value);
            assignNode->right = parseExpression();
            match(TokenType::Separator, ";");
            return assignNode;
        }
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
        
        // Parse each part of the cout statement separately
        while (!check(TokenType::Separator, ";") && pos < tokens.size()) {
            if (check(TokenType::Operator, "<<")) {
                advance(); // skip <<
            } else if (check(TokenType::Literal)) {
                // Handle string literals
                auto outputNode = std::make_shared<ASTNode>(ASTNodeType::Literal, advance().value);
                printNode->children.push_back(outputNode);
            } else if (check(TokenType::Keyword, "std") && pos + 2 < tokens.size() && 
                      tokens[pos + 1].value == "::" && tokens[pos + 2].value == "endl") {
                // Handle std::endl properly
                advance(); // std
                advance(); // ::
                advance(); // endl
                auto endlNode = std::make_shared<ASTNode>(ASTNodeType::Literal, "std::endl");
                printNode->children.push_back(endlNode);
            } else if (check(TokenType::Identifier)) {
                // Handle variables in cout
                auto varNode = std::make_shared<ASTNode>(ASTNodeType::Identifier, advance().value);
                printNode->children.push_back(varNode);
            } else {
                // Skip other tokens we don't handle
                advance();
            }
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