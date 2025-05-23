#include "../include/CodeAnalyzer.h"
#include <iostream>

void CodeAnalyzer::analyze(const std::shared_ptr<ASTNode>& root) {
    if (!root) return;
    
    checkRedundantConditions(root);
    
    // Analyze children
    if (root->left) analyze(root->left);
    if (root->right) analyze(root->right);
    
    // Analyze children vector
    for (const auto& child : root->children) {
        analyze(child);
    }
}

void CodeAnalyzer::checkRedundantConditions(const std::shared_ptr<ASTNode>& node) {
    if (!node || node->type != ASTNodeType::BinaryOperation) return;

    // Check for x == x
    if (node->left && node->right && node->left->value == node->right->value &&
        (node->value == "==" || node->value == "||" || node->value == "&&")) {
        report("Redundant condition: " + node->left->value + " " + node->value + " " + node->right->value);
    }

    // Check for true || something
    if ((node->left && node->left->value == "true" && node->value == "||") ||
        (node->right && node->right->value == "true" && node->value == "||")) {
        report("Always true condition due to 'true' || something");
    }

    // Check for false && something
    if ((node->left && node->left->value == "false" && node->value == "&&") ||
        (node->right && node->right->value == "false" && node->value == "&&")) {
        report("Always false condition due to 'false' && something");
    }
    
    // Check for constant folding opportunities
    if (node->left && node->right && 
        node->left->type == ASTNodeType::Literal && 
        node->right->type == ASTNodeType::Literal &&
        (node->value == "+" || node->value == "-" || node->value == "*" || node->value == "/")) {
        report("Constant folding opportunity: " + node->left->value + " " + node->value + " " + node->right->value);
    }
}

void CodeAnalyzer::report(const std::string& message) {
    std::cout << "[Analyzer] " << message << std::endl;
}