#pragma once

#include <cstddef>
#include <sstream>
#include <string>
#include <unordered_map>

#include "parser.hpp"

class Generator {
public:
    explicit Generator(NodeProg& node_prog, size_t unique_let) 
        : m_node_prog(std::move(node_prog)), m_unique_let(unique_let) {}

    std::string gen_asm();

    bool used_printf() { return extern_printf; }
private:
    void remove_extra_zeros(std::string& str);

    void gen_fact(NodeFactor* fact);
    void gen_term_op(NodeTermOp* term_op);
    void gen_term(NodeTerm* term);
    void gen_expr(NodeExpr* expr);
    void gen_stat(NodeStat* stat);
    void gen_line(NodeLine* line);

    inline std::string get_var_pointer(size_t stack_loc) {
        std::stringstream var_pointer;
        var_pointer << "QWORD [rbp-" << stack_loc * 8 << "]";
        return var_pointer.str();
    }

    int write_str_in_data(std::string& str);
    void print_number(bool last_print);
    bool extern_printf = false;

    std::stringstream m_output;
    std::stringstream m_data;

    size_t m_data_counter = 1;
    size_t m_skip_counter = 1;

    size_t m_unique_let = 0;
    size_t m_free_var_ptr = 0;
    struct Var { size_t stack_loc; };
    std::unordered_map<std::string, Var> m_vars;

    NodeProg m_node_prog;

    void clear();
};