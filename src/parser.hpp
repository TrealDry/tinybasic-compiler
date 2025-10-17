#pragma once

#include <cstddef>
#include <optional>
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

struct NodeExpr;
struct NodeFactor {
    std::variant<NodeVar*, NodeNum*, NodeExpr*> body;
};

struct NodeFactorMult;
struct NodeFactorDiv {
    std::variant<NodeFactor*, NodeFactorMult*, NodeFactorDiv*> fact;
    NodeFactor* fact2;
};

struct NodeFactorMult {
    std::variant<NodeFactor*, NodeFactorMult*, NodeFactorDiv*> fact;
    NodeFactor* fact2;
};

struct NodeTerm {
    std::variant<NodeFactor*, NodeFactorMult*, NodeFactorDiv*> fact;
};

struct NodeTermNegative {
    NodeTerm* term;
};

struct NodeTermPositive {
    NodeTerm* term;
};

struct NodeTermMinus;
struct NodeTermPlus {
    std::variant<NodeTerm*, NodeTermPositive*, NodeTermNegative*, 
    NodeTermPlus*, NodeTermMinus*> term;
    std::variant<NodeTerm*> term2;
};

struct NodeTermMinus {
    std::variant<NodeTerm*, NodeTermPositive*,
    NodeTermNegative*, NodeTermPlus*, NodeTermMinus*> term;
    std::variant<NodeTerm*> term2;
};

struct NodeExpr {
    std::variant<NodeTerm*, NodeTermPositive*, 
    NodeTermNegative*, NodeTermPlus*, NodeTermMinus*> expr;
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

    NodeExpr* parse_expr();
    NodeStatLet* parse_stat_let();
    NodeLine* parse_line();
    NodeStat* parse_stat();

    MemoryPool m_mem_pool;
    std::vector<Token> m_tokens;
    size_t m_index = 0;
};