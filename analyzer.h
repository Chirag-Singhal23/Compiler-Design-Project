#ifndef CODE_ANALYZER_H
#define CODE_ANALYZER_H

#include "Parser.h"

class CodeAnalyzer {
public:
    void analyze(const std::shared_ptr<ASTNode>& root);

private:
    void checkRedundantConditions(const std::shared_ptr<ASTNode>& node);
    void report(const std::string& message);
};

#endif // CODE_ANALYZER_H