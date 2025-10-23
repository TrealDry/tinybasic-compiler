#pragma once

#include <cstddef>
#include <sstream>
#include <string>
#include <unordered_map>

#include "parser.hpp"

class Generator {
public:
    explicit Generator(NodeProg& node_prog, size_t unique_let, bool no_new_line) 
        : m_node_prog(std::move(node_prog)), m_unique_let(unique_let), m_no_new_line(no_new_line) {}

    std::string gen_asm();
private:
    void remove_extra_zeros(std::string& str);

    void gen_fact_op(NodeFactorOp* fact_op);
    void gen_fact(NodeFactor* fact);

    void gen_term_op(NodeTermOp* term_op);
    void gen_term(NodeTerm* term);
    void gen_expr(NodeExpr* expr);
    
    void gen_stat(NodeStat* stat);
    void gen_line(NodeLine* line);

    inline std::string get_var_pointer(size_t stack_loc) {
        std::stringstream var_pointer;
        var_pointer << "[rbp-" << stack_loc * 8 << "]";
        return var_pointer.str();
    }

    inline std::string get_var_value(size_t stack_loc) {
        return "QWORD " + get_var_pointer(stack_loc);
    }

    int write_str_in_data(std::string& str);
    void print_number(bool last_print);
    void print_str(std::string& str, bool last_print);

    std::stringstream m_output;
    std::stringstream m_data;

    size_t m_data_counter = 1;
    size_t m_skip_counter = 1;

    size_t m_unique_let = 0;
    size_t m_free_var_ptr = 1;
    struct Var { size_t stack_loc; };
    std::unordered_map<std::string, Var> m_vars;

    bool m_no_new_line = false;

    NodeProg m_node_prog;

    void clear();
};