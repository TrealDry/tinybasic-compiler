#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "lexer.hpp"
#include "mem_pool.hpp"

struct NodeVar {
    std::string name;
};

struct NodeNum {
    std::string num;
};

struct NodeTerm;
struct NodeTermOp;
struct NodeFactor {
    std::variant<NodeVar, NodeNum, NodeTerm*, NodeTermOp*> body;
};

struct NodeFactorOp {
    std::variant<NodeFactor*, NodeFactorOp*> fact;
    NodeFactor* fact2;
    bool is_mul;  // false - div, true - mul
};

struct NodeTerm {
    std::variant<NodeFactor*, NodeFactorOp*> fact;
    std::optional<bool> is_negative;
};

struct NodeTermOp {
    std::variant<NodeTerm*, NodeTermOp*> term;
    std::variant<NodeTerm*> term2;
    bool is_add;  // false - sub, true - add
};

struct NodeExpr {
    std::variant<NodeTerm*, NodeTermOp*> term;
};

struct NodeExprList {
    std::vector< std::variant<NodeExpr*, std::string> > list;
};

struct NodeVarList {
    std::vector<NodeVar*> list;
};

enum class RelopType {
    eq, ne, gt, gte, lt, lte, crazy
};
struct NodeRelop {
    RelopType type;
};

struct NodeStatPrint {
    NodeExprList* exprs;
};
struct NodeStatIf {};
struct NodeStatGoto {};
struct NodeStatInput {};
struct NodeStatLet {
    NodeVar var;
    NodeExpr* expr;
};
struct NodeStatGosub {};
struct NodeStatReturn {};
struct NodeStatClear {};
struct NodeStatList {};
struct NodeStatRun {};
struct NodeStatEnd {};

struct NodeStat {
    std::variant<NodeStatPrint*, NodeStatIf*, \
    NodeStatGoto*, NodeStatInput*, \
    NodeStatLet*, NodeStatGosub*, \
    NodeStatReturn*, NodeStatClear*, \
    NodeStatList*, NodeStatRun*, \
    NodeStatEnd*> com;
};

struct NodeLine {
    std::optional<NodeNum> num;
    NodeStat* stat;
};

struct NodeProg {
    std::vector<NodeLine*> lines;
};

class Parser {
public:
    Parser(std::vector<Token>& tokens) : m_tokens(std::move(tokens)), m_mem_pool() {}

    NodeProg gen_prog();

private:
    std::optional<Token> peek(int offset = 0);
    Token consume();

    NodeFactor* parse_factor();
    NodeTerm* parse_term();
    NodeExpr* parse_expr();
    
    NodeStatLet* parse_stat_let();
    NodeStatPrint* parse_stat_print();
    NodeStat* parse_stat();

    NodeLine* parse_line();

    MemoryPool m_mem_pool;
    std::vector<Token> m_tokens;
    size_t m_index = 0;
};