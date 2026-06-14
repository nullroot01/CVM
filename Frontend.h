#pragma once
#include "Definitions.h"
#include <iostream>
#include <unordered_map>
#include <cctype>

class Lexer {
    std::string source;
    std::vector<Token> tokens;
    int start = 0; int current = 0; int line = 1;
    std::unordered_map<std::string, TokenType> keywords = {
        {"let", TokenType::LET}, {"print", TokenType::PRINT}, {"input", TokenType::INPUT},
        {"if", TokenType::IF}, {"else", TokenType::ELSE}, {"while", TokenType::WHILE},
        {"true", TokenType::TRUE_KW}, {"false", TokenType::FALSE_KW}
    };

    bool isAtEnd() { return current >= source.length(); }
    char advance() { return source[current++]; }
    char peek() { return isAtEnd() ? '\0' : source[current]; }
    bool match(char expected) {
        if (isAtEnd() || source[current] != expected) return false;
        current++; return true;
    }
public:
    Lexer(std::string src) : source(std::move(src)) {}
    std::vector<Token> scanTokens() {
        while (!isAtEnd()) {
            start = current;
            char c = advance();
            switch (c) {
                case '(': tokens.push_back({TokenType::LPAREN, "(", line}); break;
                case ')': tokens.push_back({TokenType::RPAREN, ")", line}); break;
                case '{': tokens.push_back({TokenType::LBRACE, "{", line}); break;
                case '}': tokens.push_back({TokenType::RBRACE, "}", line}); break;
                case '+': tokens.push_back({TokenType::PLUS, "+", line}); break;
                case '-': tokens.push_back({TokenType::MINUS, "-", line}); break;
                case '*': tokens.push_back({TokenType::STAR, "*", line}); break;
                case '/': tokens.push_back({TokenType::SLASH, "/", line}); break;
                case ';': tokens.push_back({TokenType::SEMICOLON, ";", line}); break;
                case '<': tokens.push_back({TokenType::LESS, "<", line}); break;
                case '=': tokens.push_back({match('=') ? TokenType::EQUAL_EQUAL : TokenType::ASSIGN, match('=') ? "==" : "=", line}); break;
                case ' ': case '\r': case '\t': break;
                case '\n': line++; break;
                default:
                    if (std::isdigit(c)) {
                        while (std::isdigit(peek())) advance();
                        tokens.push_back({TokenType::NUMBER, source.substr(start, current - start), line});
                    } else if (std::isalpha(c) || c == '_') {
                        while (std::isalnum(peek()) || peek() == '_') advance();
                        std::string text = source.substr(start, current - start);
                        TokenType type = keywords.count(text) ? keywords[text] : TokenType::IDENTIFIER;
                        tokens.push_back({type, text, line});
                    } break;
            }
        }
        tokens.push_back({TokenType::END_OF_FILE, "", line});
        return tokens;
    }
};

class Parser {
    std::vector<Token> tokens;
    int current = 0;
    bool isAtEnd() { return tokens[current].type == TokenType::END_OF_FILE; }
    Token advance() { return tokens[current++]; }
    Token peek() { return tokens[current]; }
    bool match(TokenType type) { if (isAtEnd() || peek().type != type) return false; advance(); return true; }

    std::unique_ptr<Expr> parsePrimary() {
        if (match(TokenType::TRUE_KW)) return std::make_unique<BoolExpr>(true);
        if (match(TokenType::FALSE_KW)) return std::make_unique<BoolExpr>(false);
        if (match(TokenType::INPUT)) return std::make_unique<InputExpr>();
        if (match(TokenType::NUMBER)) return std::make_unique<NumberExpr>(std::stoi(tokens[current - 1].lexeme));
        if (match(TokenType::IDENTIFIER)) return std::make_unique<VarExpr>(tokens[current - 1].lexeme);
        return nullptr;
    }

    std::unique_ptr<Expr> parseMultiplicative() {
        auto expr = parsePrimary();
        while (peek().type == TokenType::STAR || peek().type == TokenType::SLASH) {
            Token op = advance();
            auto right = parsePrimary();
            expr = std::make_unique<BinaryExpr>(op.lexeme, std::move(expr), std::move(right));
        }
        return expr;
    }

    std::unique_ptr<Expr> parseAdditive() {
        auto expr = parseMultiplicative();
        while (peek().type == TokenType::PLUS || peek().type == TokenType::MINUS) {
            Token op = advance();
            auto right = parseMultiplicative();
            expr = std::make_unique<BinaryExpr>(op.lexeme, std::move(expr), std::move(right));
        }
        return expr;
    }

    std::unique_ptr<Expr> parseComparison() {
        auto expr = parseAdditive();
        while (peek().type == TokenType::LESS || peek().type == TokenType::EQUAL_EQUAL) {
            Token op = advance();
            auto right = parseAdditive();
            expr = std::make_unique<BinaryExpr>(op.lexeme, std::move(expr), std::move(right));
        }
        return expr;
    }

    std::unique_ptr<Stmt> parseStatement() {
        if (match(TokenType::LET)) {
            std::string name = advance().lexeme;
            match(TokenType::ASSIGN);
            auto expr = parseComparison();
            match(TokenType::SEMICOLON);
            return std::make_unique<LetStmt>(name, std::move(expr));
        }
        if (match(TokenType::PRINT)) {
            auto expr = parseComparison();
            match(TokenType::SEMICOLON);
            return std::make_unique<PrintStmt>(std::move(expr));
        }
        if (match(TokenType::IF)) {
            match(TokenType::LPAREN);
            auto cond = parseComparison();
            match(TokenType::RPAREN);
            auto thenBranch = parseStatement();
            std::unique_ptr<Stmt> elseBranch = nullptr;
            if (match(TokenType::ELSE)) { elseBranch = parseStatement(); }
            return std::make_unique<IfStmt>(std::move(cond), std::move(thenBranch), std::move(elseBranch));
        }
        if (match(TokenType::WHILE)) {
            match(TokenType::LPAREN);
            auto cond = parseComparison();
            match(TokenType::RPAREN);
            auto body = parseStatement();
            return std::make_unique<WhileStmt>(std::move(cond), std::move(body));
        }
        if (match(TokenType::LBRACE)) {
            auto block = std::make_unique<BlockStmt>();
            while (!isAtEnd() && peek().type != TokenType::RBRACE) {
                block->statements.push_back(parseStatement());
            }
            match(TokenType::RBRACE);
            return block;
        }
        if (peek().type == TokenType::IDENTIFIER) {
            std::string name = advance().lexeme;
            match(TokenType::ASSIGN);
            auto expr = parseComparison();
            match(TokenType::SEMICOLON);
            return std::make_unique<LetStmt>(name, std::move(expr));
        }
        return nullptr;
    }

public:
    Parser(std::vector<Token> t) : tokens(std::move(t)) {}
    std::vector<std::unique_ptr<Stmt>> parse() {
        std::vector<std::unique_ptr<Stmt>> statements;
        while (!isAtEnd()) { statements.push_back(parseStatement()); }
        return statements;
    }
};

// --- BONUS: AST VISUALIZER ---
void printExpr(Expr* expr) {
    if (auto num = dynamic_cast<NumberExpr*>(expr)) std::cout << num->value;
    else if (auto var = dynamic_cast<VarExpr*>(expr)) std::cout << var->name;
    else if (auto bin = dynamic_cast<BinaryExpr*>(expr)) {
        std::cout << "("; printExpr(bin->left.get()); std::cout << " " << bin->op << " "; printExpr(bin->right.get()); std::cout << ")";
    }
}

void printAST(Stmt* stmt, int indent = 0) {
    std::string ind(indent, ' ');
    if (auto let = dynamic_cast<LetStmt*>(stmt)) {
        std::cout << ind << "LetStmt(" << let->name << " = "; printExpr(let->initializer.get()); std::cout << ")\n";
    } else if (auto print = dynamic_cast<PrintStmt*>(stmt)) {
        std::cout << ind << "PrintStmt("; printExpr(print->expr.get()); std::cout << ")\n";
    } else if (auto whileStmt = dynamic_cast<WhileStmt*>(stmt)) {
        std::cout << ind << "WhileStmt(Cond: "; printExpr(whileStmt->condition.get()); std::cout << ")\n";
        printAST(whileStmt->body.get(), indent + 4);
    } else if (auto block = dynamic_cast<BlockStmt*>(stmt)) {
        std::cout << ind << "Block:\n";
        for (auto& s : block->statements) printAST(s.get(), indent + 4);
    }
}