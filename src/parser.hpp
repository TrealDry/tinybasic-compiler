#pragma once

#include <cstddef>
#include <optional>
#include <variant>
#include <vector>

#include "lexer.hpp"

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
    NodeFactor* fact;
    std::variant<NodeFactor*, NodeFactorMult*, NodeFactorDiv*> fact2;
};

struct NodeFactorMult {
    NodeFactor* fact;
    std::variant<NodeFactor*, NodeFactorMult*, NodeFactorDiv*> fact2;
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
    std::variant<NodeTerm*, NodeTermPositive*, NodeTermNegative*> term;
    std::variant<NodeTerm*, NodeTermPlus*, NodeTermMinus*> term2;
};

struct NodeTermMinus {
    std::variant<NodeTerm*, NodeTermPositive*, NodeTermNegative*> term;
    std::variant<NodeTerm*, NodeTermPlus*, NodeTermMinus*> term2;
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
struct NodeStatLet {};
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
    std::optional<int> num;
    NodeStat* stat;
};

struct NodeProg {
    std::vector<NodeLine> lines;
};

class Parser {
public:
    Parser(std::vector<Token>& tokens) : m_tokens(std::move(tokens)) {}

    NodeProg gen_prog();

private:
    std::optional<Token> peek(int offset = 0);
    Token consume();

    std::vector<Token> m_tokens;
    size_t m_index = 0;
};