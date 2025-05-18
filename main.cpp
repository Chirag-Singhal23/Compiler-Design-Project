#include <iostream>
#include "Tokenizer.h"
#include "Parser.h"
#include "CodeAnalyzer.h"

void printTokens(const std::vector<Token>& tokens) {
    std::cout << "Tokens:\n";
    for (const auto& token : tokens) {
        std::string typeStr;
        switch (token.type) {
            case TokenType::Identifier: typeStr = "Identifier"; break;
            case TokenType::Keyword: typeStr = "Keyword"; break;
            case TokenType::Operator: typeStr = "Operator"; break;
            case TokenType::Number: typeStr = "Number"; break;
            case TokenType::Separator: typeStr = "Separator"; break;
            case TokenType::Literal: typeStr = "Literal"; break;
            case TokenType::Unknown: typeStr = "Unknown"; break;
            default: typeStr = "Undefined"; break;
        }
        std::cout << "  Type: " << typeStr << ", Value: \"" << token.value << "\"\n";
    }
    std::cout << std::endl;
}

int main() {
    // Test with a code snippet that has redundant conditions
    std::string code = "int x = 5;\n"
                       "if (x == x) {\n"
                       "    int y = 10;\n"
                       "}\n"
                       "if (true || x > 0) {\n"
                       "    int z = 20;\n"
                       "}\n"
                       "if (false && x < 10) {\n"
                       "    int w = 30;\n"
                       "}";

    std::cout << "Analyzing code:\n" << code << "\n\n";

    Tokenizer tokenizer;
    auto tokens = tokenizer.tokenize(code);
    
    // Print tokens for debugging with better type names
    printTokens(tokens);

    Parser parser(tokens);
    auto ast = parser.parse();

    std::cout << "AST Tree:\n";
    printAST(ast);

    std::cout << "\nCode Analysis Results:\n";
    CodeAnalyzer analyzer;
    analyzer.analyze(ast);

    return 0;
}