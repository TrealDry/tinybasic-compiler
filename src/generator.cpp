#include <stdexcept>
#include <format>
#include <string>
#include <variant>

#include "generator.hpp"
#include "parser.hpp"

void Generator::remove_extra_zeros(std::string& str) {
    while (true) {
        if (str.length() == 0) {
            str = '0';
            return;
        }

        if (str.at(0) == '0') {
            str.erase(0, 1);
        } else {
            return;
        }
    }
}

int Generator::write_str_in_data(std::string& str) {
    m_data << std::format(
        "\tstr{} db \'{}\', 0\n",
        m_data_counter, str
    );
    m_data << std::format(
        "\tlen{} equ $ - str{}\n",
        m_data_counter, m_data_counter
    );

    return m_data_counter++;
}

void Generator::print_number(bool last_print) {
    if (last_print) {
        m_output << "\tmov rdi, frmn\n";
    } else {
        m_output << "\tmov rdi, frm\n";
    }

    m_output << "\tpop rsi\n";
    m_output << "\txor eax, eax\n";
    m_output << "\tcall printf\n";
}

void Generator::print_str(std::string& str, bool last_print) {
    int str_index = write_str_in_data(str);
    
    m_output << "\tmov rdi, str" << str_index << "\n";
    m_output << "\txor eax, eax\n";
    m_output << "\tcall printf\n";

    if (last_print) {
        m_output << "\tmov rdi, 10\n";
        m_output << "\tcall putchar\n";
    }
}

void Generator::gen_fact(NodeFactor* fact) {
    struct FactorVisitor {
        Generator* gen;

        void operator()(NodeVar& var) {
            if (!gen->m_vars.contains(var.name))
                throw std::runtime_error("Var " + var.name + " has not been initialized!");

            auto& v = gen->m_vars.at(var.name);

            gen->m_output << "\tpush " << gen->get_var_value(v.stack_loc) << "\n";
        }

        void operator()(NodeNum& num) {
            gen->m_output << "\tmov rax, " << num.num << "\n";
            gen->m_output << "\tpush rax\n";
        }

        void operator()(NodeTerm* term) {
            gen->gen_term(term);
        }

        void operator()(NodeTermOp* term_op) {
            gen->gen_term_op(term_op);
        }
    };

    std::visit(FactorVisitor{.gen = this}, fact->body);
}

void Generator::gen_fact_op(NodeFactorOp* fact_op) {
    struct FactVisitor {
        Generator* gen;

        void operator()(NodeFactor* fact) {
            gen->gen_fact(fact);
        }

        void operator()(NodeFactorOp* fact_op) {
            gen->gen_fact_op(fact_op);
        }
    };

    FactVisitor f{.gen = this};
    std::visit(f, fact_op->fact);
    std::visit(f, fact_op->fact2);

    m_output << "\tpop rbx\n";
    m_output << "\tpop rax\n";

    if (fact_op->is_mul) {
        m_output << "\timul rbx\n";
    } else {
        m_output << "\tcqo\n";
        m_output << "\tidiv rbx\n";
    }

    m_output << "\tpush rax\n";
}

void Generator::gen_term(NodeTerm* term) {
    struct FactorVisitor {
        Generator* gen;

        void operator()(std::monostate& mono) {}

        void operator()(NodeFactor* fact) {
            gen->gen_fact(fact);
        }

        void operator()(NodeFactorOp* fact_op) {
            gen->gen_fact_op(fact_op);
        }
    };

    std::visit(FactorVisitor{.gen = this}, term->fact);
}

void Generator::gen_term_op(NodeTermOp* term_op) {
    struct TermVisitor {
        Generator* gen;

        void operator()(NodeTerm* term) {
            gen->gen_term(term);
        }

        void operator()(NodeTermOp* term_op) {
            gen->gen_term_op(term_op);
        }
    };

    TermVisitor t{.gen = this};
    std::visit(t, term_op->term);
    std::visit(t, term_op->term2);

    m_output << "\tpop rsi\n";
    m_output << "\tpop rdi\n";

    if (term_op->is_add) {
        m_output << "\tadd rdi, rsi\n";
    } else {
        m_output << "\tsub rdi, rsi\n";
    }

    m_output << "\tpush rdi\n";
}

void Generator::gen_expr(NodeExpr* expr) {
    struct ExprVisitor {
        Generator* gen;

        void operator()(std::monostate& mono) {}

        void operator()(NodeTerm* term) {
            gen->gen_term(term);
        }

        void operator()(NodeTermOp* term_op) {
            gen->gen_term_op(term_op);
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
                bool last_print;

                void operator()(NodeExpr* expr) {
                    gen->gen_expr(expr);
                    gen->print_number(last_print);
                }
                void operator()(std::string& str) {
                    gen->print_str(str, last_print);
                }
            };

            for (auto& var: stat_print->exprs->list) {
                bool last_print = false;

                if (&var == &stat_print->exprs->list.back()) {
                    last_print = true;
                }

                std::visit(
                    ExprListVisitor{.gen = gen, .last_print = last_print}, var
                );
            }
        }

        void operator()(NodeStatLet* stat_let) {
            auto ident = stat_let->var.name;

            if (gen->m_vars.contains(ident)) {
                auto var = gen->m_vars.at(ident);

                gen->gen_expr(stat_let->expr);
                
                gen->m_output << "\tpop rax\n";
                gen->m_output << "\tmov " \
                    << gen->get_var_value(var.stack_loc) \
                    << ", rax\n";
            } else {
                gen->m_vars.insert({ident, {.stack_loc = gen->m_free_var_ptr++}});
                gen->gen_expr(stat_let->expr);
                
                gen->m_output << "\tpop rax\n";
                gen->m_output << "\tmov " \
                    << gen->get_var_value(gen->m_free_var_ptr - 1) \
                    << ", rax\n";
            }
        }

        void operator()(const NodeStatIf* stat_if) {
            gen->gen_expr(stat_if->expr);
            gen->gen_expr(stat_if->expr2);

            std::string jump;  // jump for skip stat
            bool reverse_reg = false;

            switch (stat_if->relop.type) {
                case RelopType::eq:  jump = "jne"; break;
                case RelopType::ne:  jump = "je";  break;
                case RelopType::lt:  jump = "jle"; break;
                case RelopType::lte: jump = "jl";  break;

                case RelopType::gt:  jump = "jle"; reverse_reg = true; break;
                case RelopType::gte: jump = "jl";  reverse_reg = true; break;

                case RelopType::crazy: jump = "je"; break;  // plug
            }

            if (!reverse_reg) {
                gen->m_output << "\tpop rax\n";
                gen->m_output << "\tpop rbx\n";
            } else {
                gen->m_output << "\tpop rbx\n";
                gen->m_output << "\tpop rax\n";
            }

            gen->m_output << "\tcmp rax, rbx\n";
            gen->m_output << "\t" << jump << " skip" << gen->m_skip_counter << "\n";

            gen->gen_stat(stat_if->then);

            gen->m_output << "skip" << gen->m_skip_counter++ << ":\n";
        }

        void operator()(const NodeStatGoto* stat_goto) {
            std::string line_num = std::get<1>(
                std::get<1>(std::get<1>(stat_goto->expr->term)->fact)->body
            ).num;

            gen->remove_extra_zeros(line_num);
            gen->m_output << "\tjmp com" << line_num << "\n";
        }

        void operator()(const NodeStatInput* stat_input) {
            for (auto& var: stat_input->var_list.list) {
                if (!gen->m_vars.contains(var.name)) 
                    throw std::runtime_error("Var " + var.name + " does not exist!");
                
                gen->m_output << "\tmov rdi, frm\n";
                gen->m_output << "\tlea rsi, " \
                    << gen->get_var_pointer(gen->m_vars.at(var.name).stack_loc) \
                    << "\n";
                gen->m_output << "\tcall scanf\n";
            }
        }

        void operator()(const NodeStatGosub* stat_gosub) {
            std::string line_num = std::get<1>(
                std::get<1>(std::get<1>(stat_gosub->expr->term)->fact)->body
            ).num;

            gen->remove_extra_zeros(line_num);

            
            gen->m_output << "\tmov rdi, [cntr]\n";
            gen->m_output << "\tinc rdi\n";
            gen->m_output << "\tmov [cntr], rdi\n";
            gen->m_output << "\tcall com" << line_num << "\n";
        }

        void operator()(const NodeStatReturn* stat_return) {
            gen->m_output << "\tmov rdi, [cntr]\n";
            gen->m_output << "\ttest rdi, rdi\n";
            gen->m_output << "\tjz skip" << gen->m_skip_counter << "\n";
            gen->m_output << "\tdec rdi\n";
            gen->m_output << "\tmov [cntr], rdi\n";
            gen->m_output << "\tret\n";
            gen->m_output << "skip" << gen->m_skip_counter++ << ":\n";
        }

        void operator()(const NodeStatClear* stat_clear) {}
        void operator()(const NodeStatList* stat_list) {}
        void operator()(const NodeStatRun* stat_run) {}

        void operator()(const NodeStatEnd* stat_end) {
            gen->m_output << "\tjmp exit\n";
        }
    };

    std::visit(StatVisitor{.gen = this}, stat->com);
}

void Generator::gen_line(NodeLine* line) {
    if (line->num.has_value()) {
        std::string num = line->num.value().num;
        remove_extra_zeros(num);

        m_output << "com" << num << ":\n";
    }

    gen_stat(line->stat);
}

std::string Generator::gen_asm() {
    clear();

    m_output << "\tpush rbp\n";
    m_output << "\tmov rbp, rsp\n";
    if (m_unique_let)
        m_output << "\tsub rsp, " << (m_unique_let + 1) * 8 << "\n";

    // for print nums
    m_data << "\tfrm db '%li', 0\n";
    m_data << "\tfrmn db '%li', 10, 0\n";

    m_data << "\tcntr dq 0\n";

    for (auto line: m_node_prog.lines) {
        gen_line(line);
    }

    m_output << "exit:\n";

    m_output << "\tmov rsp, rbp\n";
    m_output << "\tpop rbp\n\n";

    m_output << "\tmov rax, 60\n";
    m_output << "\tmov rdi, 0\n";
    m_output << "\tsyscall\n";

    std::string result = std::format(
        "extern printf, putchar, scanf\n"
        "section .data\n{}\n"
        "section .text\n"
        "\tglobal _start\n\n"
        "_start:\n{}", 
        m_data.str(), m_output.str()
    );

    return result;
}

void Generator::clear() {
    m_output.clear();
    m_data.clear();
    m_vars.clear();

    m_data_counter = 1;
    m_skip_counter = 1;
    m_free_var_ptr = 1;
}
