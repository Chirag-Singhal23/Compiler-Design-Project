#include "../include/Tokenizer.h"
#include "../include/Parser.h"
#include "../include/CodeAnalyzer.h"
#include "../include/CodeOptimizer.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

// Function to read content from file
std::string readFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Error opening file: " + filename);
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// Function to write content to file
void writeFile(const std::string& filename, const std::string& content) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Error opening file for writing: " + filename);
    }
    
    file << content;
    file.close();
}

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

int main(int argc, char* argv[]) {
    // Check if input and output file paths are provided
    if (argc < 3) {
        std::cout << "Usage: " << argv[0] << " <input_file> <output_file>" << std::endl;
        return 1;
    }
    
    std::string inputFile = argv[1];
    std::string outputFile = argv[2];
    
    try {
        // Read input code
        std::string code = readFile(inputFile);
        std::cout << "Processing file: " << inputFile << std::endl;
        
        // Tokenize
        Tokenizer tokenizer;
        auto tokens = tokenizer.tokenize(code);
        
        std::cout << "Tokenization complete. Found " << tokens.size() << " tokens." << std::endl;
        
        // Parse
        Parser parser(tokens);
        auto ast = parser.parse();
        
        std::cout << "Parsing complete. AST created." << std::endl;
        
        // Analyze
        std::cout << "\nRunning code analysis..." << std::endl;
        CodeAnalyzer analyzer;
        analyzer.analyze(ast);
        
        // Optimize
        std::cout << "\nOptimizing code..." << std::endl;
        CodeOptimizer optimizer;
        auto optimizedAst = optimizer.optimize(ast);
        
        // Generate optimized code
        std::string optimizedCode = optimizer.generateCode(optimizedAst);
        
        // Write output
        writeFile(outputFile, optimizedCode);
        
        std::cout << "Optimization complete. Optimized code written to: " << outputFile << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}