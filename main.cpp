#include <iostream>
#include "Tokenizer.h"
#include "Parser.h"
#include "CodeAnalyzer.h"

int main() {
    std::string code = "int x = 5 + 5;";

    Tokenizer tokenizer;
    auto tokens = tokenizer.tokenize(code);

    Parser parser(tokens);
    auto ast = parser.parse();

    std::cout << "\nAST Tree:\n";
    printAST(ast);

    CodeAnalyzer analyzer;
    analyzer.analyze(ast);

    return 0;
}
