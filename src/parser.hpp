#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <unordered_set>
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
    std::variant<NodeFactor*> fact2;
    bool is_mul;  // false - div, true - mul
};

struct NodeTerm {
    std::variant<std::monostate, NodeFactor*, NodeFactorOp*> fact;
    std::optional<bool> is_negative;
};

struct NodeTermOp {
    std::variant<NodeTerm*, NodeTermOp*> term;
    std::variant<NodeTerm*> term2;
    bool is_add;  // false - sub, true - add
};

struct NodeExpr {
    std::variant<std::monostate, NodeTerm*, NodeTermOp*> term;
};

struct NodeExprList {
    std::vector< std::variant<NodeExpr*, std::string> > list;
};

struct NodeVarList {
    std::vector<NodeVar> list;
};

enum class RelopType {
    eq, ne, gt, gte, lt, lte, crazy
};
struct NodeRelop {
    RelopType type;
};

struct NodeStat;
struct NodeStatPrint {
    NodeExprList* exprs;
};
struct NodeStatIf {
    NodeExpr* expr;
    NodeExpr* expr2;
    NodeRelop relop;
    NodeStat* then;
};
struct NodeStatGoto {
    NodeExpr* expr;
};
struct NodeStatInput {
    NodeVarList var_list;
};
struct NodeStatLet {
    NodeVar var;
    NodeExpr* expr;
};
struct NodeStatGosub {
    NodeExpr* expr;
};
struct NodeStatReturn {};
struct NodeStatEnd {};

// outsiders
struct NodeStatClear {};
struct NodeStatList {};
struct NodeStatRun {};

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
    inline size_t get_unique_let() { return m_unique_let.size(); }

private:
    void clear();

    std::optional<Token> peek(int offset = 0);
    Token consume();

    NodeFactor* parse_factor();
    NodeTerm* parse_term();
    NodeExpr* parse_expr();
    NodeRelop parse_relop();
    NodeVarList parse_var_list();
    NodeExpr* parse_const_expr();
    
    NodeStatPrint* parse_stat_print();
    NodeStatInput* parse_stat_input();
    NodeStatGosub* parse_stat_gosub();
    NodeStatGoto* parse_stat_goto();
    NodeStatLet* parse_stat_let();
    NodeStatIf* parse_stat_if();
    NodeStat* parse_stat();

    NodeLine* parse_line();

    MemoryPool m_mem_pool;
    std::vector<Token> m_tokens;
    size_t m_index = 0;

    std::unordered_set<char> m_unique_let;
};