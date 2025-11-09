#include <format>
#include <optional>
#include <stdexcept>
#include <utility>
#include <variant>

#include "error.hpp"
#include "parser.hpp"
#include "lexer.hpp"

NodeFactor* Parser::parse_factor() {
    if (!peek().has_value())
        Error::critical(m_line, "Expression is empty!");

    auto fact = m_mem_pool.alloc<NodeFactor>();

    switch (peek().value().type) {
        case TokenType::var:
            fact->body = NodeVar{.name = consume().var.value()};
            break;
        case TokenType::num:
            fact->body = NodeNum{.num = consume().var.value()};
            break;
        case TokenType::open_paren:
            consume();

            struct FactorVisitor {
                NodeFactor* fact;

                void operator()(std::monostate& mono) {}

                void operator()(NodeTerm* term) { 
                    fact->body = term; 
                }

                void operator()(NodeTermOp* term_op) { 
                    fact->body = term_op; 
                }
            };

            std::visit(FactorVisitor{.fact=fact}, parse_expr()->term);

            if (!peek().has_value() || peek().value().type != TokenType::close_paren)
                Error::critical(m_line, "Expression was not closed by close paren!");

            consume();
            break;
        default:
            Error::critical(m_line, "Invalid expression!");
    }

    return fact;
}

NodeTerm* Parser::parse_term() {
    if (!peek().has_value()) 
        Error::critical(m_line, "Expression is empty!");

    auto term = m_mem_pool.alloc<NodeTerm>();
    term->fact = std::monostate{};

    while (peek().has_value() && peek().value().type != TokenType::cr) {
        bool fact_op_exits = false;
        NodeFactor* first_fact;

        if (peek().value().type == TokenType::mul || peek().value().type == TokenType::div) {
            fact_op_exits = true;
        } else if (peek().value().type == TokenType::num || \
                   peek().value().type == TokenType::var || \
                   peek().value().type == TokenType::open_paren) 
        {
            first_fact = parse_factor();

            if (!peek().has_value() || peek().value().type == TokenType::cr) {
                term->fact = first_fact;
                return term;
            }

            if (peek().value().type != TokenType::mul && peek().value().type != TokenType::div) {
                if (term->fact.index() == 0)
                    term->fact = first_fact;

                return term;
            }
        } else {
            return term;
        }

        auto fact_op = m_mem_pool.alloc<NodeFactorOp>();
        auto op = consume();

        NodeFactor* second_fact = parse_factor();

        fact_op->is_mul = op.type == TokenType::mul;
        if (fact_op_exits) {
            fact_op->fact = std::get<2>(term->fact);
        } else {
            fact_op->fact = first_fact;
        }
        fact_op->fact2 = second_fact;

        term->fact = fact_op;
    }

    if (term->fact.index() == 0)
        Error::critical(m_line, "Expression is empty!");

    if (term->fact.index() == 2) {
        if (auto fact_op = std::get<2>(term->fact); fact_op->is_mul) {
            return term;
        } else if (fact_op->fact2.index() == 1) {
            if (auto body = std::get<0>(fact_op->fact2)->body; body.index() == 1) {
                if (std::get<1>(body).num == "0")
                    Error::critical(m_line, "Division by zero!");
            }
        }
    }
    
    return term;
}

NodeExpr* Parser::parse_expr() {
    if (!peek().has_value()) 
        Error::critical(m_line, "Expression is empty!");

    auto expr = m_mem_pool.alloc<NodeExpr>();
    expr->term = std::monostate{};

    while (peek().has_value() && peek().value().type != TokenType::cr) {
        bool term_op_exits = false;
        NodeTerm* first_term;

        if (peek().value().type == TokenType::plus || peek().value().type == TokenType::minus) {
            if (expr->term.index() != 0) {
                term_op_exits = true;
            } else {  // unary minus or plus
                auto op = consume();
                auto term = parse_term();

                if (op.type == TokenType::minus) {
                    term->is_negative = true;
                }

                expr->term = term;
                
                continue;
            }
        } else if (peek().value().type == TokenType::num || \
                   peek().value().type == TokenType::var || \
                   peek().value().type == TokenType::open_paren) 
        {
            first_term = parse_term();

            if (!peek().has_value() || peek().value().type == TokenType::cr) {
                expr->term = first_term;
                return expr;
            }

            if (peek().value().type != TokenType::plus && peek().value().type != TokenType::minus) {
                if (expr->term.index() == 0)
                    expr->term = first_term;

                return expr;
            }
        } else {
            return expr;
        }

        auto term_op = m_mem_pool.alloc<NodeTermOp>();
        auto op = consume();

        NodeTerm* second_term = parse_term();

        term_op->is_add = op.type == TokenType::plus;
        if (term_op_exits) {
            term_op->term = expr->term;
        } else {
            term_op->term = first_term;
        }
        term_op->term2 = second_term;

        expr->term = term_op;
    }

    if (expr->term.index() == 0)
        Error::critical(m_line, "Expression is empty!");

    return expr;
}

NodeRelop Parser::parse_relop() {
    if (!peek().has_value() && !peek(1).has_value())
        Error::critical(m_line, "Relop is empty!");
    
    NodeRelop relop;

    Token first_token = consume();
    Token second_token;

    if (peek().value().type == TokenType::eq || \
        peek().value().type == TokenType::gt || \
        peek().value().type == TokenType::lt)
    {
        second_token = consume();
    } else {
        second_token = Token{.type=TokenType::none};
    }

    switch (first_token.type) {
        case TokenType::lt:
            if (second_token.type == TokenType::gt) {
                relop.type = RelopType::ne;
            }
            else if (second_token.type == TokenType::eq) {
                relop.type = RelopType::lte;
            }
            else {
                relop.type = RelopType::lt;
            }
            break;

        case TokenType::gt:
            if (second_token.type == TokenType::lt) {
                relop.type = RelopType::crazy;
                Error::warning(m_line, "`><` relop (crazy) is not implemented!");
            }
            else if (second_token.type == TokenType::eq) {
                relop.type = RelopType::gte;
            }
            else {
                relop.type = RelopType::gt;
            }
            break;

        case TokenType::eq:
            relop.type = RelopType::eq;
            break;

        default:
            throw std::runtime_error("Wrong token for relop!");
    }

    return relop;
}

NodeStatIf* Parser::parse_stat_if() {
    auto stat_if = m_mem_pool.alloc<NodeStatIf>();

    stat_if->expr  = parse_expr();
    stat_if->relop = parse_relop();
    stat_if->expr2 = parse_expr();

    if (!peek().has_value() || peek().value().type != TokenType::then)
        Error::critical(m_line, "Keyword `THEN` is missing!");

    consume();

    auto stat = parse_stat();

    if (stat == nullptr || stat->com.index() == 0)
        Error::critical(m_line, "There is no command after `THEN`!");

    stat_if->then = stat;

    return stat_if;
}

NodeVarList Parser::parse_var_list() {  // var list used only command INPUT
    NodeVarList var_list;

    while (peek().has_value() && peek().value().type != TokenType::cr) {
        if (peek().value().type == TokenType::var) {
            var_list.list.emplace_back(consume().var.value());
        } else if (peek().value().type == TokenType::com) {
            consume();
        } else {
            Error::critical(m_line, "Command `INPUT` accepts only vars!");
        }
    }

    if (var_list.list.size() == 0)
        Error::critical(m_line, "Command `INPUT` has no arguments!");

    return var_list;
}

NodeStatInput* Parser::parse_stat_input() {
    auto stat_input = m_mem_pool.alloc<NodeStatInput>();

    stat_input->var_list = parse_var_list();

    return stat_input;
}

NodeExpr* Parser::parse_const_expr() {
    auto expr = parse_expr();

    if (expr->term.index() == 2)
        Error::critical(m_line, "Expression (for `GOTO` and `GOSUB`) is not constant!");

    auto term = std::get<1>(expr->term);
    if (term->fact.index() != 1)
        Error::critical(m_line, "Expression (for `GOTO` and `GOSUB`) is not constant!");

    auto fact = std::get<1>(term->fact);

    if (fact->body.index() != 1)
        Error::critical(m_line, "Expression (for `GOTO` and `GOSUB`) is not constant!");

    long long num = std::stoi(std::get<1>(fact->body).num);
    m_goto_num.insert({num, m_line});

    if (peek().has_value() && peek().value().type != TokenType::cr)
        Error::critical(m_line, "Chars after expression!");

    return expr;
}

NodeStatGoto* Parser::parse_stat_goto() {
    auto stat_goto = m_mem_pool.alloc<NodeStatGoto>();

    stat_goto->expr = parse_const_expr();

    return stat_goto;
}

NodeStatGosub* Parser::parse_stat_gosub() {
    auto stat_gosub = m_mem_pool.alloc<NodeStatGosub>();

    stat_gosub->expr = parse_const_expr();

    return stat_gosub;
}


NodeStatLet* Parser::parse_stat_let() {
    auto stat_let = m_mem_pool.alloc<NodeStatLet>();

    if (!peek().has_value() || peek().value().type != TokenType::var)
        Error::critical(m_line, "Var name must be a single english capital letter!");

    std::string var_name = consume().var.value();
    if (var_name.length() != 1 || !std::isupper(var_name.at(0)))
        Error::critical(m_line, "Var name must be a single english capital letter!");
    
    m_unique_let.insert(var_name.at(0));
    stat_let->var = NodeVar{.name = var_name};

    if (!peek().has_value() || peek().value().type != TokenType::eq)
        Error::critical(m_line, "Operator `=` is missing!");

    consume();

    stat_let->expr = parse_expr();

    if (peek().has_value() && peek().value().type != TokenType::cr)
        Error::critical(m_line, "Chars after expression!");

    return stat_let;
}

NodeStatPrint* Parser::parse_stat_print() {
    auto stat_print = m_mem_pool.alloc<NodeStatPrint>();
    auto expr_list = m_mem_pool.alloc<NodeExprList>();

    stat_print->exprs = expr_list;

    while (peek().has_value() && peek().value().type != TokenType::cr) {
        if (peek().value().type == TokenType::num || \
            peek().value().type == TokenType::var || \
            peek().value().type == TokenType::open_paren) 
        {
            expr_list->list.push_back(parse_expr());
        }
        else if (peek().value().type == TokenType::str) {
            expr_list->list.push_back(consume().var.value());
        }
        else if (peek().value().type == TokenType::com) {
            consume();
        }
        else {
            Error::critical(m_line, "Command `PRINT` accepts only: vars, expressions, and strings!");
        }
    }

    if (stat_print->exprs->list.size() == 0)
        Error::critical(m_line, "Command `PRINT` has no arguments!");
    
    return stat_print;
}

NodeStat* Parser::parse_stat() {
    if (!peek().has_value()) 
        Error::critical(m_line, "Command is empty!");

    auto node_stat = m_mem_pool.alloc<NodeStat>();
    auto token = consume();

    m_line = token.line;

    switch (token.type) {
        case TokenType::print:   node_stat->com = Parser::parse_stat_print(); break;
        case TokenType::_if:     node_stat->com = Parser::parse_stat_if(); break;
        case TokenType::_goto:   node_stat->com = Parser::parse_stat_goto(); break;
        case TokenType::input:   node_stat->com = Parser::parse_stat_input(); break;
        case TokenType::let:     node_stat->com = Parser::parse_stat_let(); break;
        case TokenType::gosub:   node_stat->com = Parser::parse_stat_gosub(); break;
        case TokenType::_return: node_stat->com = m_mem_pool.alloc<NodeStatReturn>(); break;
        case TokenType::end:     node_stat->com = m_mem_pool.alloc<NodeStatEnd>(); break;
        case TokenType::clear:   
            node_stat->com = m_mem_pool.alloc<NodeStatClear>();
            Error::warning(m_line, "command `CLEAR` is not implemented!");
            break;
        case TokenType::list:    
            node_stat->com = m_mem_pool.alloc<NodeStatList>(); 
            Error::warning(m_line, "command `LIST` is not implemented!");
            break;
        case TokenType::run:     
            node_stat->com = m_mem_pool.alloc<NodeStatRun>(); 
            Error::warning(m_line, "command `RUN` is not implemented!");
            break;
        case TokenType::cr:      
            return nullptr;
        default:                 
            Error::critical(m_line, "Invalid command!");
    }

    if (peek().has_value() && peek()->type == TokenType::cr)
        consume();

    return node_stat;
}

NodeLine* Parser::parse_line() {
    auto line = m_mem_pool.alloc<NodeLine>();

    if (peek().value().type == TokenType::num) {
        auto token = consume();
        line->num = NodeNum{.num = token.var.value()};

        int old_size = m_unique_str_num.size();
        m_unique_str_num.insert(std::stoi(line->num->num));

        if (m_unique_str_num.size() == old_size) {
            Error::critical(m_line, "Row number is not unique!");
        }
    } else if (peek().value().type == TokenType::cr) {
        return nullptr;
    }

    if (auto stat = parse_stat()) {
        line->stat = stat;
        line->line = m_line;
        return line;
    } else {
        return nullptr;
    }
}

void Parser::check_correct_goto() {
    for (auto [num, line]: m_goto_num) {
        if (!m_unique_str_num.contains(num))
            Error::critical(line, std::format("Line number {} does not exist!", num).c_str());
    }
}

NodeProg Parser::gen_prog() {
    NodeProg prog;

    m_index = 0;
    m_unique_let.clear();

    while (peek().has_value()) {
        if (auto line = parse_line()) 
            prog.lines.push_back(line);
        else if (peek().has_value())
            consume();
    }

    check_correct_goto();

    return prog;
}

std::optional<Token> Parser::peek(int offset) {
    if (m_index + offset >= m_tokens.size()) {
        return {};
    } else {
        return m_tokens.at(m_index + offset);
    }
}

Token Parser::consume() {
    return m_tokens.at(m_index++);
}
