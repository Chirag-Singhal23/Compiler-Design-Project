#include "../include/CodeOptimizer.h"
#include <iostream>
#include <sstream>
#include <cctype>
#include <algorithm>
#include <cmath>

std::shared_ptr<ASTNode> CodeOptimizer::optimize(const std::shared_ptr<ASTNode>& root) {
    if (!root) return nullptr;
    
    // Clear constant values for each optimization run
    constantValues.clear();
    
    // Create a new node to avoid modifying the original
    auto newNode = std::make_shared<ASTNode>(root->type, root->value);
    
    // First optimize children recursively
    if (root->left) {
        newNode->left = optimize(root->left);
    }
    if (root->right) {
        newNode->right = optimize(root->right);
    }
    
    // Optimize children vector
    for (const auto& child : root->children) {
        if (child) {
            auto optimizedChild = optimize(child);
            if (optimizedChild) {
                newNode->children.push_back(optimizedChild);
            }
        }
    }
    
    // Apply different optimizations in the correct order
    auto optimizedNode = optimizeConstantFolding(newNode);
    optimizedNode = optimizeRedundantConditions(optimizedNode);
    optimizedNode = eliminateDeadCode(optimizedNode);
    optimizedNode = optimizeLoops(optimizedNode);
    
    return optimizedNode;
}

std::shared_ptr<ASTNode> CodeOptimizer::optimizeLoops(const std::shared_ptr<ASTNode>& node) {
    if (!node) return nullptr;
    
    // Optimize for loops with constant conditions
    if (node->type == ASTNodeType::ForStatement && node->children.size() >= 2) {
        auto condition = node->children[1]; // condition is at index 1
        
        // Check if condition is always false
        if (condition && condition->type == ASTNodeType::Literal && condition->value == "false") {
            std::cout << "[Optimizer] Eliminated for loop with false condition" << std::endl;
            return nullptr; // Remove the entire loop
        }
        
        // Check if condition is always true (infinite loop warning)
        if (condition && condition->type == ASTNodeType::Literal && condition->value == "true") {
            std::cout << "[Optimizer] Warning: For loop with always true condition (infinite loop)" << std::endl;
        }
    }
    
    // Optimize while loops with constant conditions
    if (node->type == ASTNodeType::WhileStatement && node->left) {
        auto condition = node->left;
        
        // Check if condition is always false
        if (condition->type == ASTNodeType::Literal && condition->value == "false") {
            std::cout << "[Optimizer] Eliminated while loop with false condition" << std::endl;
            return nullptr; // Remove the entire loop
        }
        
        // Check if condition is always true (infinite loop warning)
        if (condition->type == ASTNodeType::Literal && condition->value == "true") {
            std::cout << "[Optimizer] Warning: While loop with always true condition (infinite loop)" << std::endl;
        }
    }
    
    // Optimize do-while loops with constant conditions
    if (node->type == ASTNodeType::DoWhileStatement && node->right) {
        auto condition = node->right;
        
        // For do-while, the body executes at least once, but we can optimize the loop part
        if (condition->type == ASTNodeType::Literal && condition->value == "false") {
            std::cout << "[Optimizer] Simplified do-while loop with false condition to execute body once" << std::endl;
            // Return just the body since it executes once and then exits
            return node->left;
        }
        
        // Check for infinite loop
        if (condition->type == ASTNodeType::Literal && condition->value == "true") {
            std::cout << "[Optimizer] Warning: Do-while loop with always true condition (infinite loop)" << std::endl;
        }
    }
    
    return node;
}

std::shared_ptr<ASTNode> CodeOptimizer::optimizeRedundantConditions(const std::shared_ptr<ASTNode>& node) {
    if (!node) return nullptr;
    
    // Optimize x == x to true
    if (node->type == ASTNodeType::BinaryOperation && node->value == "==" && 
        node->left && node->right && 
        node->left->type == ASTNodeType::Identifier && 
        node->right->type == ASTNodeType::Identifier &&
        node->left->value == node->right->value) {
        std::cout << "[Optimizer] Optimized redundant equality check: " << node->left->value << " == " << node->right->value << " to true" << std::endl;
        return std::make_shared<ASTNode>(ASTNodeType::Literal, "true");
    }
    
    // Optimize constant == constant
    if (node->type == ASTNodeType::BinaryOperation && node->value == "==" && 
        node->left && node->right && 
        node->left->type == ASTNodeType::Literal && 
        node->right->type == ASTNodeType::Literal &&
        node->left->value == node->right->value) {
        std::cout << "[Optimizer] Optimized constant equality: " << node->left->value << " == " << node->right->value << " to true" << std::endl;
        return std::make_shared<ASTNode>(ASTNodeType::Literal, "true");
    }
    
    // Optimize true || x to true
    if (node->type == ASTNodeType::BinaryOperation && node->value == "||" &&
        ((node->left && node->left->value == "true") || 
         (node->right && node->right->value == "true"))) {
        std::cout << "[Optimizer] Optimized OR with true to always true" << std::endl;
        return std::make_shared<ASTNode>(ASTNodeType::Literal, "true");
    }
    
    // Optimize false && x to false
    if (node->type == ASTNodeType::BinaryOperation && node->value == "&&" &&
        ((node->left && node->left->value == "false") || 
         (node->right && node->right->value == "false"))) {
        std::cout << "[Optimizer] Optimized AND with false to always false" << std::endl;
        return std::make_shared<ASTNode>(ASTNodeType::Literal, "false");
    }
    
    // Optimize x || false to x
    if (node->type == ASTNodeType::BinaryOperation && node->value == "||") {
        if (node->left && node->left->value == "false" && node->right) {
            std::cout << "[Optimizer] Optimized false || x to x" << std::endl;
            return node->right;
        }
        if (node->right && node->right->value == "false" && node->left) {
            std::cout << "[Optimizer] Optimized x || false to x" << std::endl;
            return node->left;
        }
    }
    
    // Optimize x && true to x
    if (node->type == ASTNodeType::BinaryOperation && node->value == "&&") {
        if (node->left && node->left->value == "true" && node->right) {
            std::cout << "[Optimizer] Optimized true && x to x" << std::endl;
            return node->right;
        }
        if (node->right && node->right->value == "true" && node->left) {
            std::cout << "[Optimizer] Optimized x && true to x" << std::endl;
            return node->left;
        }
    }
    
    return node;
}

std::shared_ptr<ASTNode> CodeOptimizer::optimizeConstantFolding(const std::shared_ptr<ASTNode>& node) {
    if (!node) return nullptr;
    
    // Handle function declarations - need to process their bodies for constant tracking
    if (node->type == ASTNodeType::FunctionDeclaration && node->left && node->left->type == ASTNodeType::Block) {
        // Process the function body to track constants
        optimizeConstantFolding(node->left);
        return node;
    }
    
    // Handle blocks - process children and maintain constants across statements
    if (node->type == ASTNodeType::Block) {
        for (auto& child : node->children) {
            if (child) {
                child = optimizeConstantFolding(child);
            }
        }
        return node;
    }
    
    // Handle declarations with constant values
    if (node->type == ASTNodeType::Declaration && 
        node->left && node->left->type == ASTNodeType::Identifier &&
        node->right && node->right->type == ASTNodeType::Literal) {
        constantValues[node->left->value] = node->right->value;
        std::cout << "[Optimizer] Saved constant value: " << node->left->value << " = " << node->right->value << std::endl;
        return node;
    }
    
    // Handle declarations with binary operations that can be folded
    if (node->type == ASTNodeType::Declaration && 
        node->left && node->left->type == ASTNodeType::Identifier &&
        node->right && node->right->type == ASTNodeType::BinaryOperation) {
        
        // First replace variables in the binary operation with their known values
        if (node->right->left && node->right->left->type == ASTNodeType::Identifier && 
            constantValues.count(node->right->left->value)) {
            std::cout << "[Optimizer] Replaced variable " << node->right->left->value << " with constant " << constantValues[node->right->left->value] << std::endl;
            node->right->left = std::make_shared<ASTNode>(ASTNodeType::Literal, constantValues[node->right->left->value]);
        }
        
        if (node->right->right && node->right->right->type == ASTNodeType::Identifier && 
            constantValues.count(node->right->right->value)) {
            std::cout << "[Optimizer] Replaced variable " << node->right->right->value << " with constant " << constantValues[node->right->right->value] << std::endl;
            node->right->right = std::make_shared<ASTNode>(ASTNodeType::Literal, constantValues[node->right->right->value]);
        }
        
        // Now try to fold the constants
        auto optimizedRight = optimizeConstantFolding(node->right);
        
        // If the result is now a literal, save it as a constant
        if (optimizedRight && optimizedRight->type == ASTNodeType::Literal) {
            constantValues[node->left->value] = optimizedRight->value;
            std::cout << "[Optimizer] Saved folded constant: " << node->left->value << " = " << optimizedRight->value << std::endl;
            node->right = optimizedRight;
        }
        
        return node;
    }
    
    // Handle assignments with constant propagation
    if (node->type == ASTNodeType::Assignment && 
        node->left && node->left->type == ASTNodeType::Identifier &&
        node->right) {
        
        // Replace variables in the right side with their known values
        if (node->right->type == ASTNodeType::Identifier && 
            constantValues.count(node->right->value)) {
            std::cout << "[Optimizer] Replaced variable " << node->right->value << " with constant " << constantValues[node->right->value] << std::endl;
            node->right = std::make_shared<ASTNode>(ASTNodeType::Literal, constantValues[node->right->value]);
        }
        
        // If right side is a binary operation, try to optimize it
        if (node->right->type == ASTNodeType::BinaryOperation) {
            node->right = optimizeConstantFolding(node->right);
        }
        
        // If the result is a literal, save it as a constant
        if (node->right && node->right->type == ASTNodeType::Literal) {
            constantValues[node->left->value] = node->right->value;
            std::cout << "[Optimizer] Updated constant value: " << node->left->value << " = " << node->right->value << std::endl;
        }
        
        return node;
    }
    
    // Replace variables with known constants in binary operations
    if (node->type == ASTNodeType::BinaryOperation) {
        // Replace left operand if it's a known constant
        if (node->left && node->left->type == ASTNodeType::Identifier && 
            constantValues.count(node->left->value)) {
            std::cout << "[Optimizer] Replaced variable " << node->left->value << " with constant " << constantValues[node->left->value] << std::endl;
            node->left = std::make_shared<ASTNode>(ASTNodeType::Literal, constantValues[node->left->value]);
        }
        
        // Replace right operand if it's a known constant
        if (node->right && node->right->type == ASTNodeType::Identifier && 
            constantValues.count(node->right->value)) {
            std::cout << "[Optimizer] Replaced variable " << node->right->value << " with constant " << constantValues[node->right->value] << std::endl;
            node->right = std::make_shared<ASTNode>(ASTNodeType::Literal, constantValues[node->right->value]);
        }
    }
    
    // Enhanced constant folding for complex expressions
    if (node->type == ASTNodeType::BinaryOperation && 
        node->left && node->right) {
        
        // Handle nested binary operations (like 5 * 10 + 20 / 4)
        auto leftResult = evaluateConstantExpression(node->left);
        auto rightResult = evaluateConstantExpression(node->right);
        
        if (leftResult.first && rightResult.first) {
            double result = 0;
            bool canOptimize = true;
            
            if (node->value == "+") {
                result = leftResult.second + rightResult.second;
            } else if (node->value == "-") {
                result = leftResult.second - rightResult.second;
            } else if (node->value == "*") {
                result = leftResult.second * rightResult.second;
            } else if (node->value == "/" && rightResult.second != 0) {
                result = leftResult.second / rightResult.second;
            } else {
                canOptimize = false;
            }
            
            if (canOptimize) {
                std::cout << "[Optimizer] Folded constant expression: " 
                         << leftResult.second << " " << node->value << " " 
                         << rightResult.second << " = " << result << std::endl;
                
                // Convert back to integer if result is a whole number
                int intResult = static_cast<int>(result);
                if (std::abs(result - intResult) < 1e-9) {
                    return std::make_shared<ASTNode>(ASTNodeType::Literal, std::to_string(intResult));
                } else {
                    return std::make_shared<ASTNode>(ASTNodeType::Literal, std::to_string(result));
                }
            }
        }
    }
    
    return node;
}

// Helper function to evaluate constant expressions recursively
std::pair<bool, double> CodeOptimizer::evaluateConstantExpression(const std::shared_ptr<ASTNode>& node) {
    if (!node) return {false, 0.0};
    
    if (node->type == ASTNodeType::Literal) {
        try {
            double val = std::stod(node->value);
            return {true, val};
        } catch (...) {
            return {false, 0.0};
        }
    }
    
    // Handle identifiers that are known constants
    if (node->type == ASTNodeType::Identifier && constantValues.count(node->value)) {
        try {
            double val = std::stod(constantValues[node->value]);
            return {true, val};
        } catch (...) {
            return {false, 0.0};
        }
    }
    
    if (node->type == ASTNodeType::BinaryOperation && node->left && node->right) {
        auto leftResult = evaluateConstantExpression(node->left);
        auto rightResult = evaluateConstantExpression(node->right);
        
        if (leftResult.first && rightResult.first) {
            if (node->value == "+") {
                return {true, leftResult.second + rightResult.second};
            } else if (node->value == "-") {
                return {true, leftResult.second - rightResult.second};
            } else if (node->value == "*") {
                return {true, leftResult.second * rightResult.second};
            } else if (node->value == "/" && rightResult.second != 0) {
                return {true, leftResult.second / rightResult.second};
            }
        }
    }
    
    return {false, 0.0};
}

std::shared_ptr<ASTNode> CodeOptimizer::eliminateDeadCode(const std::shared_ptr<ASTNode>& node) {
    if (!node) return nullptr;
    
    // Eliminate if statements with false conditions
    if (node->type == ASTNodeType::IfStatement && 
        node->left && node->left->type == ASTNodeType::Literal && 
        node->left->value == "false") {
        std::cout << "[Optimizer] Eliminated dead code: if (false) block" << std::endl;
        return nullptr; // Remove the entire if statement
    }
    
    // Convert if (true) statements to just execute the body directly
    if (node->type == ASTNodeType::IfStatement && 
        node->left && node->left->type == ASTNodeType::Literal && 
        node->left->value == "true") {
        std::cout << "[Optimizer] Simplified if (true) to just the body" << std::endl;
        
        // Return the body content directly
        return node->right;
    }
    
    return node;
}

std::string CodeOptimizer::generateCode(const std::shared_ptr<ASTNode>& root) {
    std::ostringstream code;
    code << "// Optimized C++ code\n";
    generateCodeForNode(root, code, 0);
    return code.str();
}

void CodeOptimizer::generateCodeForNode(const std::shared_ptr<ASTNode>& node, std::ostringstream& code, int indent) {
    if (!node) return;
    
    std::string indentStr(indent, ' ');
    
    switch (node->type) {
        case ASTNodeType::Program:
            for (const auto& child : node->children) {
                generateCodeForNode(child, code, indent);
            }
            break;
            
        case ASTNodeType::Preprocessor:
            code << node->value << "\n\n";
            break;
            
        case ASTNodeType::FunctionDeclaration:
            code << indentStr << "int " << node->value << "() ";
            if (node->left) {
                generateCodeForNode(node->left, code, indent);
            }
            code << "\n";
            break;
            
        case ASTNodeType::Block:
            code << "{\n";
            for (const auto& child : node->children) {
                generateCodeForNode(child, code, indent + 4);
            }
            code << indentStr << "}\n";
            break;
            
        case ASTNodeType::Declaration:
            code << indentStr << node->value << " ";
            if (node->left) {
                generateCodeForNode(node->left, code, 0);
            }
            if (node->right) {
                code << " = ";
                generateCodeForNode(node->right, code, 0);
            }
            code << ";\n";
            break;
            
        case ASTNodeType::Assignment:
            code << indentStr;
            if (node->left) {
                generateCodeForNode(node->left, code, 0);
            }
            code << " = ";
            if (node->right) {
                generateCodeForNode(node->right, code, 0);
            }
            code << ";\n";
            break;
            
        case ASTNodeType::IfStatement:
            code << indentStr << "if (";
            if (node->left) {
                generateCodeForNode(node->left, code, 0);
            }
            code << ") ";
            if (node->right) {
                generateCodeForNode(node->right, code, indent);
            }
            break;
            
        case ASTNodeType::ForStatement:
            code << indentStr << "for (";
            // Generate initialization (index 0)
            if (node->children.size() > 0 && node->children[0]) {
                // For init statements, don't include indentation or newlines
                std::ostringstream initCode;
                generateCodeForNode(node->children[0], initCode, 0);
                std::string initStr = initCode.str();
                // Remove trailing newline and semicolon for inline generation
                if (!initStr.empty() && initStr.back() == '\n') {
                    initStr.pop_back();
                }
                if (!initStr.empty() && initStr.back() == ';') {
                    initStr.pop_back();
                }
                // Remove leading whitespace for inline generation
                size_t firstNonSpace = initStr.find_first_not_of(" \t");
                if (firstNonSpace != std::string::npos) {
                    initStr = initStr.substr(firstNonSpace);
                }
                code << initStr;
            }
            code << "; ";
            
            // Generate condition (index 1)
            if (node->children.size() > 1 && node->children[1]) {
                generateCodeForNode(node->children[1], code, 0);
            }
            code << "; ";
            
            // Generate increment (index 2)
            if (node->children.size() > 2 && node->children[2]) {
                generateCodeForNode(node->children[2], code, 0);
            }
            code << ") ";
            
            // Generate body (index 3)
            if (node->children.size() > 3 && node->children[3]) {
                generateCodeForNode(node->children[3], code, indent);
            }
            break;
            
        case ASTNodeType::WhileStatement:
            code << indentStr << "while (";
            if (node->left) {
                generateCodeForNode(node->left, code, 0);
            }
            code << ") ";
            if (node->right) {
                generateCodeForNode(node->right, code, indent);
            }
            break;
            
        case ASTNodeType::DoWhileStatement:
            code << indentStr << "do ";
            if (node->left) {
                generateCodeForNode(node->left, code, indent);
            }
            code << indentStr << "while (";
            if (node->right) {
                generateCodeForNode(node->right, code, 0);
            }
            code << ");\n";
            break;
            
        case ASTNodeType::PreIncrement:
            code << node->value;
            if (node->left) {
                generateCodeForNode(node->left, code, 0);
            }
            break;
            
        case ASTNodeType::PostIncrement:
            if (node->left) {
                generateCodeForNode(node->left, code, 0);
            }
            code << node->value;
            break;
            
        case ASTNodeType::CompoundAssignment:
            if (node->left) {
                generateCodeForNode(node->left, code, 0);
            }
            code << " " << node->value << " ";
            if (node->right) {
                generateCodeForNode(node->right, code, 0);
            }
            break;
            
        case ASTNodeType::PrintStatement:
            // Handle cout statements
            code << indentStr << "std::cout";
            for (size_t i = 0; i < node->children.size(); ++i) {
                code << " << ";
                const auto& child = node->children[i];
                if (child->type == ASTNodeType::Literal) {
                    // Check if it's std::endl
                    if (child->value == "std::endl") {
                        code << "std::endl";
                    } else {
                        // Regular string literal - preserve quotes if they exist
                        std::string cleanValue = child->value;
                        // Remove trailing spaces
                        while (!cleanValue.empty() && cleanValue.back() == ' ') {
                            cleanValue.pop_back();
                        }
                        code << cleanValue;
                    }
                } else {
                    generateCodeForNode(child, code, 0);
                }
            }
            code << ";\n";
            break;

        case ASTNodeType::InputStatement:
            // Handle cin statements
            code << indentStr << "std::cin";
            for (size_t i = 0; i < node->children.size(); ++i) {
                code << " >> ";
                const auto& child = node->children[i];
                generateCodeForNode(child, code, 0);
            }
            code << ";\n";
            break;
            
        case ASTNodeType::ReturnStatement:
            code << indentStr << "return";
            if (node->left) {
                code << " ";
                generateCodeForNode(node->left, code, 0);
            }
            code << ";\n";
            break;
            
        case ASTNodeType::ExpressionStatement:
            code << indentStr;
            if (node->left) {
                generateCodeForNode(node->left, code, 0);
            }
            code << ";\n";
            break;
            
        case ASTNodeType::BinaryOperation:
            if (node->left) {
                generateCodeForNode(node->left, code, 0);
            }
            code << " " << node->value << " ";
            if (node->right) {
                generateCodeForNode(node->right, code, 0);
            }
            break;
            
        case ASTNodeType::Literal:
            code << node->value;
            break;
            
        case ASTNodeType::Identifier:
            code << node->value;
            break;
            
        default:
            break;
    }
}