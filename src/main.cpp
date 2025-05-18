#include "../include/Tokenizer.h"
#include "../include/Parser.h"
#include "../include/CodeAnalyzer.h"
#include <iostream>


void printTokens(const std::vector<Token>& tokens) {
    for (const auto& token : tokens) {
        std::cout << "Type: ";
        switch (token.type) {
            case TokenType::Identifier: std::cout << "Identifier"; break;
            case TokenType::Keyword:    std::cout << "Keyword"; break;
            case TokenType::Operator:   std::cout << "Operator"; break;
            case TokenType::Number:     std::cout << "Number"; break;
            case TokenType::Separator:  std::cout << "Separator"; break;
            case TokenType::Literal:    std::cout << "Literal"; break;
            default:                   std::cout << "Unknown"; break;
        }
        std::cout << ", Value: '" << token.value << "'\n";
    }
}





int main() {
    std::string code = "a = 2; b = 5; c = a + b;";

    Tokenizer tokenizer;
    auto tokens = tokenizer.tokenize(code);

    std::cout << "Tokens:\n";
    printTokens(tokens);

    for (auto& t : tokens) {
        std::cout << static_cast<int>(t.type) << " : " << t.value << std::endl;
    }

    Parser parser(tokens);
    auto ast = parser.parse();

    std::cout << "\nAST Tree:\n";
    printAST(ast);

    CodeAnalyzer analyzer;
    analyzer.analyze(ast);

    return 0;
}
