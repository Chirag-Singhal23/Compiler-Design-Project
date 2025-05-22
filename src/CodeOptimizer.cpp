#include "../include/CodeOptimizer.h"
#include <iostream>
#include <sstream>
#include <cctype>
#include <algorithm>

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
    
    return optimizedNode;
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
    
    return node;
}

std::shared_ptr<ASTNode> CodeOptimizer::optimizeConstantFolding(const std::shared_ptr<ASTNode>& node) {
    if (!node) return nullptr;
    
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
    
    // Constant folding for binary operations
    if (node->type == ASTNodeType::BinaryOperation && 
        node->left && node->left->type == ASTNodeType::Literal &&
        node->right && node->right->type == ASTNodeType::Literal) {
        
        // Parse numeric literals - handle both integers and check for valid numbers
        try {
            double leftVal = std::stod(node->left->value);
            double rightVal = std::stod(node->right->value);
            double result = 0;
            
            if (node->value == "+") {
                result = leftVal + rightVal;
                std::cout << "[Optimizer] Folded constant expression: " << leftVal << " + " << rightVal << " = " << result << std::endl;
            } else if (node->value == "-") {
                result = leftVal - rightVal;
                std::cout << "[Optimizer] Folded constant expression: " << leftVal << " - " << rightVal << " = " << result << std::endl;
            } else if (node->value == "*") {
                result = leftVal * rightVal;
                std::cout << "[Optimizer] Folded constant expression: " << leftVal << " * " << rightVal << " = " << result << std::endl;
            } else if (node->value == "/" && rightVal != 0) {
                result = leftVal / rightVal;
                std::cout << "[Optimizer] Folded constant expression: " << leftVal << " / " << rightVal << " = " << result << std::endl;
            } else {
                return node;
            }
            
            // Convert back to integer if result is a whole number
            int intResult = static_cast<int>(result);
            if (result == intResult) {
                return std::make_shared<ASTNode>(ASTNodeType::Literal, std::to_string(intResult));
            } else {
                return std::make_shared<ASTNode>(ASTNodeType::Literal, std::to_string(result));
            }
        } catch (const std::exception& e) {
            // If conversion fails, return original node
            return node;
        }
    }
    
    return node;
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
        
        // Return the body content directly, but preserve it as a proper block
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
            
        case ASTNodeType::PrintStatement:
            code << indentStr << "std::cout";
            for (const auto& child : node->children) {
                code << " << ";
                // Clean up the output string
                std::string cleanValue = child->value;
                // Remove extra spaces
                while (cleanValue.back() == ' ') {
                    cleanValue.pop_back();
                }
                code << cleanValue;
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

void CodeOptimizer::generateCodeForNode(const std::shared_ptr<ASTNode>& node, std::string& code) {
    std::ostringstream oss;
    generateCodeForNode(node, oss, 0);
    code = oss.str();
}