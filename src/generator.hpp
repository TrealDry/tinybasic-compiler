#pragma once

#include <cstddef>
#include <sstream>
#include <string>
#include <unordered_map>

#include "parser.hpp"

class Generator {
public:
    explicit Generator(NodeProg& node_prog) 
        : m_node_prog(std::move(node_prog)) {}

    std::string gen_asm();

    bool used_printf() { return extern_printf; }
private:
    void gen_fact(NodeFactor* fact);
    void gen_term_op(NodeTermOp* term_op);
    void gen_term(NodeTerm* term);
    void gen_expr(NodeExpr* expr);
    void gen_stat(NodeStat* stat);
    void gen_line(NodeLine* line);

    inline void push(const std::string& reg) {
        m_output << "\tpush " << reg << "\n";
        m_stack_size++;
    }
    inline void pop(const std::string& reg) {
        m_output << "\tpop " << reg << "\n";
        m_stack_size--;
    }

    int write_str_in_data(std::string& str);
    void print_number(bool last_print);
    bool extern_printf = false;

    std::stringstream m_output;
    std::stringstream m_data;

    size_t m_data_counter = 1;

    struct Var { size_t stack_loc; };
    size_t m_stack_size = 0;
    std::unordered_map<std::string, Var> m_vars;

    NodeProg m_node_prog;

    void clear();
};