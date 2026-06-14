#pragma once
#include "Definitions.h"
#include <map>
#include <iostream>
#include <iomanip>

class Compiler {
    int emitJump(uint8_t instruction) {
        emitByte(instruction); emitByte(0xff); 
        return bytecode.size() - 1;
    }

    void patchJump(int offset) {
        int jump = bytecode.size() - offset - 1;
        bytecode[offset] = static_cast<uint8_t>(jump);
    }

    void emitLoop(int loopStart) {
        emitByte(OP_LOOP);
        int offset = bytecode.size() - loopStart + 1;
        emitByte(static_cast<uint8_t>(offset));
    }

public:
    std::vector<uint8_t> bytecode;
    std::vector<int> constants;
    std::map<std::string, uint8_t> varMap;
    uint8_t varCount = 0;

    void emitByte(uint8_t byte) { bytecode.push_back(byte); }
    
    void compileExpr(Expr* expr) {
        if (auto num = dynamic_cast<NumberExpr*>(expr)) {
            constants.push_back(num->value);
            emitByte(OP_CONSTANT); emitByte(constants.size() - 1);
        } else if (auto bl = dynamic_cast<BoolExpr*>(expr)) {
            constants.push_back(bl->value ? 1 : 0);
            emitByte(OP_CONSTANT); emitByte(constants.size() - 1);
        } else if (auto var = dynamic_cast<VarExpr*>(expr)) {
            emitByte(OP_LOAD_VAR); emitByte(varMap[var->name]);
        } else if (dynamic_cast<InputExpr*>(expr)) {
            emitByte(OP_INPUT);
        } else if (auto bin = dynamic_cast<BinaryExpr*>(expr)) {
            compileExpr(bin->left.get());
            compileExpr(bin->right.get());
            if (bin->op == "+") emitByte(OP_ADD);
            if (bin->op == "-") emitByte(OP_SUB);
            if (bin->op == "*") emitByte(OP_MUL);
            if (bin->op == "/") emitByte(OP_DIV);
            if (bin->op == "==") emitByte(OP_EQUAL);
            if (bin->op == "<") emitByte(OP_LESS);
        }
    }

    void compileStmt(Stmt* stmt) {
        if (auto let = dynamic_cast<LetStmt*>(stmt)) {
            compileExpr(let->initializer.get());
            if (!varMap.count(let->name)) varMap[let->name] = varCount++;
            emitByte(OP_STORE_VAR); emitByte(varMap[let->name]);
        } else if (auto print = dynamic_cast<PrintStmt*>(stmt)) {
            compileExpr(print->expr.get());
            emitByte(OP_PRINT);
        } else if (auto block = dynamic_cast<BlockStmt*>(stmt)) {
            for (auto& s : block->statements) compileStmt(s.get());
        } else if (auto ifStmt = dynamic_cast<IfStmt*>(stmt)) {
            compileExpr(ifStmt->condition.get());
            int thenJump = emitJump(OP_JUMP_IF_FALSE);
            compileStmt(ifStmt->thenBranch.get());
            if (ifStmt->elseBranch) {
                int elseJump = emitJump(OP_JUMP);
                patchJump(thenJump);
                compileStmt(ifStmt->elseBranch.get());
                patchJump(elseJump);
            } else { patchJump(thenJump); }
        } else if (auto whileStmt = dynamic_cast<WhileStmt*>(stmt)) {
            int loopStart = bytecode.size();
            compileExpr(whileStmt->condition.get());
            int exitJump = emitJump(OP_JUMP_IF_FALSE);
            compileStmt(whileStmt->body.get());
            emitLoop(loopStart);
            patchJump(exitJump);
        }
    }

    void compile(const std::vector<std::unique_ptr<Stmt>>& stmts) {
        for (const auto& stmt : stmts) compileStmt(stmt.get());
        emitByte(OP_HALT);
    }

    // --- BONUS: BYTECODE DISASSEMBLER ---
    void disassemble() {
        std::cout << "--- CVM++ Bytecode ---\n";
        for (int i = 0; i < bytecode.size(); ) {
            std::cout << std::setfill('0') << std::setw(4) << i << " ";
            uint8_t instruction = bytecode[i++];
            switch (instruction) {
                case OP_CONSTANT: std::cout << "OP_CONSTANT " << (int)bytecode[i++] << "\n"; break;
                case OP_LOAD_VAR: std::cout << "OP_LOAD_VAR " << (int)bytecode[i++] << "\n"; break;
                case OP_STORE_VAR: std::cout << "OP_STORE_VAR " << (int)bytecode[i++] << "\n"; break;
                case OP_ADD: std::cout << "OP_ADD\n"; break;
                case OP_SUB: std::cout << "OP_SUB\n"; break;
                case OP_MUL: std::cout << "OP_MUL\n"; break;
                case OP_DIV: std::cout << "OP_DIV\n"; break;
                case OP_LESS: std::cout << "OP_LESS\n"; break;
                case OP_EQUAL: std::cout << "OP_EQUAL\n"; break;
                case OP_JUMP_IF_FALSE: std::cout << "OP_JUMP_IF_FALSE " << (int)bytecode[i++] << "\n"; break;
                case OP_JUMP: std::cout << "OP_JUMP " << (int)bytecode[i++] << "\n"; break;
                case OP_LOOP: std::cout << "OP_LOOP " << (int)bytecode[i++] << "\n"; break;
                case OP_PRINT: std::cout << "OP_PRINT\n"; break;
                case OP_HALT: std::cout << "OP_HALT\n"; break;
                default: std::cout << "UNKNOWN\n"; break;
            }
        }
        std::cout << "----------------------\n";
    }
};

class VM {
    std::vector<uint8_t> bytecode;
    std::vector<int> constants;
    std::vector<int> stack;
    int variables[256] = {0};
    int ip = 0;

    void push(int val) { stack.push_back(val); }
    int pop() { int val = stack.back(); stack.pop_back(); return val; }
public:
    VM(std::vector<uint8_t> code, std::vector<int> consts)
        : bytecode(std::move(code)), constants(std::move(consts)) {}

    void run() {
        while (true) {
            switch (bytecode[ip++]) {
                case OP_CONSTANT: push(constants[bytecode[ip++]]); break;
                case OP_LOAD_VAR: push(variables[bytecode[ip++]]); break;
                case OP_STORE_VAR: variables[bytecode[ip++]] = pop(); break;
                case OP_ADD: { int b = pop(); int a = pop(); push(a + b); break; }
                case OP_SUB: { int b = pop(); int a = pop(); push(a - b); break; }
                case OP_MUL: { int b = pop(); int a = pop(); push(a * b); break; }
                case OP_DIV: { int b = pop(); int a = pop(); push(a / b); break; }
                case OP_EQUAL: { int b = pop(); int a = pop(); push(a == b ? 1 : 0); break; }
                case OP_LESS: { int b = pop(); int a = pop(); push(a < b ? 1 : 0); break; }
                case OP_INPUT: { int val; std::cin >> val; push(val); break; }
                case OP_JUMP: { uint8_t offset = bytecode[ip++]; ip += offset; break; }
                case OP_JUMP_IF_FALSE: { uint8_t offset = bytecode[ip++]; if (pop() == 0) ip += offset; break; }
                case OP_LOOP: { uint8_t offset = bytecode[ip++]; ip -= offset; break; }
                case OP_PRINT: std::cout << ">> " << pop() << "\n"; break;
                case OP_HALT: return;
            }
        }
    }
};