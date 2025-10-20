#include <stdexcept>
#include <format>
#include <string>
#include <variant>

#include "generator.hpp"
#include "parser.hpp"

int Generator::write_str_in_data(std::string& str) {
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

void Generator::print_number() {
    if (!extern_printf) {
        extern_printf = true;
        m_data << "\tfrm db '%d', 10, 0\n";
    }

    m_output << "\tmov rdi, frm\n";
    pop("rsi");
    m_output << "\txor eax, eax\n";
    m_output << "\tcall printf\n";
}

void Generator::gen_fact(NodeFactor* fact) {
    struct FactorVisitor {
        Generator* gen;

        void operator()(NodeVar& var) {
            auto& v = gen->m_vars.at(var.name);

            std::stringstream reg;
            reg << "QWORD [rsp+" << (gen->m_stack_size - v.stack_loc - 1) * 8 << "]";

            gen->push(reg.str());
        }

        void operator()(NodeNum& num) {
            gen->m_output << "\tmov rax, " << num.num << "\n";
            gen->push("rax");
        }

        void operator()(NodeTerm* term) {
            ;
        }

        void operator()(NodeTermOp* term_op) {
            ;
        }
    };

    std::visit(FactorVisitor{.gen = this}, fact->body);
}

void Generator::gen_term(NodeTerm* term) {
    struct FactorVisitor {
        Generator* gen;

        void operator()(NodeFactor* fact) {
            gen->gen_fact(fact);
        }

        void operator()(NodeFactorOp* fact_op) {
            ;
        }
    };

    std::visit(FactorVisitor{.gen = this}, term->fact);
}

void Generator::gen_expr(NodeExpr* expr) {
    struct ExprVisitor {
        Generator* gen;

        void operator()(NodeTerm* term) {
            gen->gen_term(term);
        }

        void operator()(NodeTermOp* term_op) {
            ;
        }
    };

    std::visit(ExprVisitor{.gen = this}, expr->term);
}

void Generator::gen_stat(NodeStat* stat) {
    struct StatVisitor {
        Generator* gen;

        void operator()(NodeStatPrint* stat_print) {
            if (stat_print->exprs->list.size() == 0)
                throw std::runtime_error("Print expression list is empty!");

            struct ExprListVisitor {
                Generator* gen;
                bool simple_print_initialized = false;

                void operator()(NodeExpr* expr) {
                    simple_print_initialized = false;
                    gen->gen_expr(expr);
                    gen->print_number();
                }
                void operator()(std::string& str) {
                    int str_index = gen->write_str_in_data(str);

                    if (!simple_print_initialized) {
                        gen->m_output << "\tmov rax, 1\n";
                        gen->m_output << "\tmov rdi, 1\n";
                        simple_print_initialized = true;
                    }
                    gen->m_output << "\tmov rsi, str" << str_index << "\n";
                    gen->m_output << "\tmov rdx, len" << str_index << "\n";
                    gen->m_output << "\tsyscall\n";
                }
            };

            for (auto& var: stat_print->exprs->list) {
                std::visit(ExprListVisitor{.gen = gen}, var);
            }
        }

        void operator()(NodeStatLet* stat_let) {
            auto ident = stat_let->var.name;

            if (gen->m_vars.contains(ident)) {

            } else {
                gen->m_vars.insert({ident, {.stack_loc = gen->m_stack_size}});
                gen->gen_expr(stat_let->expr);
            }
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
    clear();

    for (auto line: m_node_prog.lines) {
        gen_line(line);
    }

    m_output << "\tmov rax, 60\n";
    m_output << "\tmov rdi, 0\n";
    m_output << "\tsyscall\n";

    std::string result = std::format(
        "section .data\n{}\nsection .text\n\tglobal _start\n\n_start:\n{}", 
        m_data.str(), m_output.str()
    );

    if (extern_printf) {
        result = "extern printf\n" + result;
    }

    return result;
}

void Generator::clear() {
    m_output.clear();
    m_data.clear();
    m_vars.clear();

    extern_printf = false;
    m_data_counter = 1;
    m_stack_size = 0;
}
