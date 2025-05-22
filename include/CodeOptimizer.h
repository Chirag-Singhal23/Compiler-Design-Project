#ifndef CODE_OPTIMIZER_H
#define CODE_OPTIMIZER_H

#include "Parser.h"
#include <string>
#include <unordered_map>
#include <sstream>

class CodeOptimizer {
public:
    // Takes the AST and performs optimizations, returning a new optimized AST
    std::shared_ptr<ASTNode> optimize(const std::shared_ptr<ASTNode>& root);
    
    // Convert the optimized AST back to code
    std::string generateCode(const std::shared_ptr<ASTNode>& root);

private:
    // Various optimization methods
    std::shared_ptr<ASTNode> optimizeRedundantConditions(const std::shared_ptr<ASTNode>& node);
    std::shared_ptr<ASTNode> optimizeConstantFolding(const std::shared_ptr<ASTNode>& node);
    std::shared_ptr<ASTNode> eliminateDeadCode(const std::shared_ptr<ASTNode>& node);
    
    // Helpers for code generation
    void generateCodeForNode(const std::shared_ptr<ASTNode>& node, std::ostringstream& code, int indent);
    void generateCodeForNode(const std::shared_ptr<ASTNode>& node, std::string& code);
    
    // Symbol table for constant propagation
    std::unordered_map<std::string, std::string> constantValues;
};

#endif