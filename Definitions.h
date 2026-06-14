#pragma once
#include <string>
#include <vector>
#include <memory>

// --- TOKEN DEFINITIONS ---
enum class TokenType {
    LPAREN, RPAREN, LBRACE, RBRACE, PLUS, MINUS, STAR, SLASH, SEMICOLON,
    ASSIGN, EQUAL_EQUAL, LESS, IDENTIFIER, NUMBER,
    LET, PRINT, INPUT, IF, ELSE, WHILE, TRUE_KW, FALSE_KW, END_OF_FILE
};

struct Token {
    TokenType type;
    std::string lexeme;
    int line;
};

// --- AST DEFINITIONS ---
struct Expr { virtual ~Expr() = default; };

struct NumberExpr : Expr { int value; NumberExpr(int v) : value(v) {} };
struct BoolExpr : Expr { bool value; BoolExpr(bool v) : value(v) {} };
struct VarExpr : Expr { std::string name; VarExpr(std::string n) : name(n) {} };
struct InputExpr : Expr {};

struct BinaryExpr : Expr {
    std::string op;
    std::unique_ptr<Expr> left, right;
    BinaryExpr(std::string o, std::unique_ptr<Expr> l, std::unique_ptr<Expr> r)
        : op(std::move(o)), left(std::move(l)), right(std::move(r)) {}
};

struct Stmt { virtual ~Stmt() = default; };

struct PrintStmt : Stmt { std::unique_ptr<Expr> expr; PrintStmt(std::unique_ptr<Expr> e) : expr(std::move(e)) {} };
struct LetStmt : Stmt {
    std::string name; std::unique_ptr<Expr> initializer;
    LetStmt(std::string n, std::unique_ptr<Expr> init) : name(std::move(n)), initializer(std::move(init)) {}
};
struct BlockStmt : Stmt { std::vector<std::unique_ptr<Stmt>> statements; };
struct IfStmt : Stmt {
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> thenBranch, elseBranch;
    IfStmt(std::unique_ptr<Expr> c, std::unique_ptr<Stmt> t, std::unique_ptr<Stmt> e)
        : condition(std::move(c)), thenBranch(std::move(t)), elseBranch(std::move(e)) {}
};
struct WhileStmt : Stmt {
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> body;
    WhileStmt(std::unique_ptr<Expr> c, std::unique_ptr<Stmt> b)
        : condition(std::move(c)), body(std::move(b)) {}
};

// --- BYTECODE DEFINITIONS ---
enum OpCode : uint8_t {
    OP_CONSTANT, OP_LOAD_VAR, OP_STORE_VAR,
    OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_EQUAL, OP_LESS,
    OP_PRINT, OP_INPUT,
    OP_JUMP, OP_JUMP_IF_FALSE, OP_LOOP, 
    OP_HALT
};