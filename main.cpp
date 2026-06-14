#include "Frontend.h"
#include "Backend.h"
#include <iostream>
#include <fstream>
#include <sstream>

void runPipeline(const std::string& source, bool debugMode) {
    // 1. Lexer
    Lexer lexer(source);
    auto tokens = lexer.scanTokens();

    // 2. Parser & AST Generation
    Parser parser(tokens);
    auto ast = parser.parse();

    if (debugMode) {
        std::cout << "\n--- Abstract Syntax Tree (AST) ---\n";
        for (const auto& stmt : ast) {
            printAST(stmt.get());
        }
        std::cout << "----------------------------------\n\n";
    }

    // 3. Compiler & Bytecode Generation
    Compiler compiler;
    compiler.compile(ast);

    if (debugMode) {
        compiler.disassemble();
        std::cout << "\n";
    }

    // 4. VM Execution
    std::cout << "--- Execution Output ---\n";
    VM vm(compiler.bytecode, compiler.constants);
    vm.run();
    std::cout << "------------------------\n";
}

int main(int argc, char* argv[]) {
    // Default file to run
    std::string filename = "script.cvm";
    
    // If you pass a filename in the terminal (e.g., ./cvm_pipeline custom.cvm)
    if (argc > 1) {
        filename = argv[1];
    }

    std::ifstream file(filename);
    
    // Failsafe: Auto-generate a test file if it doesn't exist
    if (!file.is_open()) {
        std::cout << "Warning: " << filename << " not found. Auto-generating a test script...\n";
        std::ofstream newFile(filename);
        newFile << "let n = 5;\n"
                << "let factorial = 1;\n"
                << "while (0 < n) {\n"
                << "    factorial = factorial * n;\n"
                << "    n = n - 1;\n"
                << "}\n"
                << "print factorial;\n";
        newFile.close();
        file.open(filename);
    }

    // Read the file into a string
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string program = buffer.str();

    std::cout << "====================================\n";
    std::cout << "      CVM++ Compiler & VM Exec      \n";
    std::cout << "====================================\n";
    std::cout << "Target File: " << filename << "\n";
    std::cout << "Source Code:\n\n" << program << "\n";
    std::cout << "====================================\n";
    
    // Set to true to show the AST and Bytecode!
    bool debugMode = true; 
    
    // Execute the full compilation and execution pipeline
    runPipeline(program, debugMode);
    
    return 0;
    
}