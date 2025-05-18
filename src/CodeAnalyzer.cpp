#include "../include/CodeAnalyzer.h"
#include <iostream>

void CodeAnalyzer::analyze(const std::shared_ptr<ASTNode>& root) {
    if (!root) return;
    checkRedundantConditions(root);
    analyze(root->left);
    analyze(root->right);
}

void CodeAnalyzer::checkRedundantConditions(const std::shared_ptr<ASTNode>& node) {
    if (!node || node->type != ASTNodeType::BinaryOperation) return;

    if (node->left && node->right && node->left->value == node->right->value &&
        (node->value == "==" || node->value == "||" || node->value == "&&")) {
        report("Redundant condition: " + node->left->value + " " + node->value + " " + node->right->value);
    }

    if ((node->left && node->left->value == "true" && node->value == "||") ||
        (node->right && node->right->value == "true" && node->value == "||")) {
        report("Always true condition due to 'true' || something");
    }

    if ((node->left && node->left->value == "false" && node->value == "&&") ||
        (node->right && node->right->value == "false" && node->value == "&&")) {
        report("Always false condition due to 'false' && something");
    }
}

void CodeAnalyzer::report(const std::string& message) {
    std::cout << "[Analyzer] " << message << std::endl;
}
