#include <stdexcept>
#include <format>
#include <string>
#include <variant>

#include "generator.hpp"
#include "parser.hpp"

int Generator::write_str_in_data(std::string str) {
    m_data << std::format(
        "\tstr{} db \'{}\', 10\n",
        m_data_counter, str
    );
    m_data << std::format(
        "\tlen{} equ $ - str{}\n",
        m_data_counter, m_data_counter
    );

    return m_data_counter++;
}

void Generator::gen_stat(NodeStat* stat) {
    struct StatVisitor {
        Generator* gen;

        void operator()(const NodeStatPrint* stat_print) {
            if (stat_print->exprs->list.size() == 0)
                throw std::runtime_error("Print expression list is empty!");

            int str_index = gen->write_str_in_data(
                std::get<1>(stat_print->exprs->list.at(0))
            );  // first string in vector

            gen->m_output << "\tmov rax, 1\n";
            gen->m_output << "\tmov rdi, 1\n";
            gen->m_output << "\tmov rsi, str" << str_index << "\n";
            gen->m_output << "\tmov rdx, len" << str_index << "\n";
            gen->m_output << "\tsyscall\n";
        }

        void operator()(const NodeStatLet* stat_let) {
            ;
        }

        void operator()(const NodeStatIf* stat_if) {}
        void operator()(const NodeStatGoto* stat_goto) {}
        void operator()(const NodeStatInput* stat_input) {}
        void operator()(const NodeStatGosub* stat_gosub) {}
        void operator()(const NodeStatReturn* stat_return) {}
        void operator()(const NodeStatClear* stat_clear) {}
        void operator()(const NodeStatList* stat_list) {}
        void operator()(const NodeStatRun* stat_run) {}
        void operator()(const NodeStatEnd* stat_end) {}
    };

    std::visit(StatVisitor{.gen = this}, stat->com);
}

void Generator::gen_line(NodeLine* line) {
    // TODO line->num
    gen_stat(line->stat);
}

std::string Generator::gen_asm() {
    m_output.clear();
    m_data.clear();

    for (auto line: m_node_prog.lines) {
        gen_line(line);
    }

    m_output << "\tmov rax, 60\n";
    m_output << "\tmov rdi, 0\n";
    m_output << "\tsyscall\n";

    return std::format(
        "section .data\n{}\nsection .text\n\tglobal _start\n\n_start:\n{}", 
        m_data.str(), m_output.str()
    );
}
