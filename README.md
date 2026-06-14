# CVM++ 🚀

**A custom, dynamically-typed scripting language, bytecode compiler, and stack-based Virtual Machine written entirely from scratch in C++17.**

CVM++ demystifies the abstraction layer between high-level programming languages and raw machine execution. It features a complete pipeline: from a custom Lexer and Recursive Descent Parser to an Abstract Syntax Tree (AST) generator, a Bytecode Compiler, and a highly-optimized Stack-Based Virtual Machine.

---

## ⚡ Features

* **Complete Compiler Pipeline:** Lexical Analysis → AST Generation → Bytecode Compilation → VM Execution.
* **Dynamic Typing:** Seamlessly handles Integers, Booleans, and Strings using a custom Tagged Union (`Value` struct) memory architecture.
* **Lexical Scoping & Control Flow:** Supports block statements `{ }`, `if/else` branching, and `while` loops utilizing compiler backpatching for forward/backward jumps.
* **Advanced Expression Parsing:** Custom Recursive Descent Parser enforces strict mathematical precedence (PEMDAS) and right-to-left unary operators.
* **Bytecode Disassembler:** Built-in tool to translate raw numeric opcodes into human-readable, assembly-like instructions (e.g., `OP_JUMP_IF_FALSE`) for debugging.
* **Robust Error Diagnostics:** Halts execution and pinpoints exact line numbers for syntax and runtime errors, preventing undefined behavior.

---

## 🏗️ System Architecture

CVM++ follows a strict, modular, unidirectional data flow simulating industry-standard compilers (like JVM or V8):

```text
Source Code (.cvm)
       │
       ▼
[ Lexical Analyzer ]   ➔ Tokenizes raw text (ignores whitespace/comments).
       │
       ▼
[ Recursive Parser ]   ➔ Enforces grammar and operator precedence.
       │
       ▼
[ Abstract Syntax Tree ]➔ Intermediate hierarchical representation.
       │
       ▼
[ Bytecode Compiler ]  ➔ Flattens AST into an 8-bit instruction set (Opcodes).
       │
       ▼
[ Virtual Machine ]    ➔ Stack-based dispatch loop executes the raw memory.
