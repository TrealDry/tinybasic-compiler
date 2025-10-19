#pragma once

#include <cstddef>
#include <sstream>
#include <string>

#include "parser.hpp"

class Generator {
public:
    explicit Generator(NodeProg& node_prog) 
        : m_node_prog(std::move(node_prog)) {}

    std::string gen_asm();
private:
    void gen_stat(NodeStat* stat);
    void gen_line(NodeLine* line);

    int write_str_in_data(std::string str);

    std::stringstream m_output;
    std::stringstream m_data;

    size_t m_data_counter = 1;

    NodeProg m_node_prog;
};