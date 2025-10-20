#include <optional>
#include <stdexcept>
#include <variant>

#include "parser.hpp"
#include "lexer.hpp"

NodeFactor* Parser::parse_factor() {
    if (!peek().has_value())
        throw std::runtime_error("Factor is empty!");

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
            fact->body = std::get<1>(parse_expr()->term);

            if (!peek().has_value() || \
                peek().value().type != TokenType::close_paren)
                throw std::runtime_error("Doesnt exist close paren after expr!");
            consume();
            
            break;
        default:
            throw std::runtime_error("Wrong keyword for factor!");
    }

    return fact;
}

NodeTerm* Parser::parse_term() {
    if (!peek().has_value())
        throw std::runtime_error("Term is empty!");

    auto term = m_mem_pool.alloc<NodeTerm>();

    term->fact = parse_factor();
    
    return term;
}

NodeExpr* Parser::parse_expr() {
    if (!peek().has_value()) 
        throw std::runtime_error("Expression (tokens) is empty!");

    auto expr = m_mem_pool.alloc<NodeExpr>();

    while (peek().has_value() && peek().value().type != TokenType::cr) {
        bool term_op_exits = false;
        NodeTerm* first_term;

        if (peek().value().type == TokenType::plus || peek().value().type == TokenType::minus) {
            term_op_exits = true;
        } else if (peek().value().type == TokenType::close_paren) {
            if (expr) {
                return expr;
            } else {
                throw std::runtime_error("Expression is empty!");
            }
        } else {
            first_term = parse_term();

            if (!peek().has_value() || peek().value().type == TokenType::cr) {
                consume();
                expr->term = first_term;
                return expr;
            }

            if (peek().value().type != TokenType::plus && peek().value().type != TokenType::minus) {
                throw std::runtime_error("Invalid expression!");
            }
        }

        auto term_op = m_mem_pool.alloc<NodeTermOp>();
        auto op = consume();

        NodeTerm* second_term = parse_term();

        term_op->is_add = op.type == TokenType::plus;
        if (term_op_exits) {
            term_op->term = std::get<1>(expr->term);
        } else {
            term_op->term = first_term;
        }
        term_op->term2 = second_term;

        expr->term = term_op;
    }

    return expr;
}

NodeStatLet* Parser::parse_stat_let() {
    auto stat_let = m_mem_pool.alloc<NodeStatLet>();

    if (!peek().has_value() || peek().value().type != TokenType::var)
        throw std::runtime_error("Var name is empty!");

    std::string var_name = consume().var.value();
    if (var_name.length() != 1 || !std::isupper(var_name.at(0)))
        throw std::runtime_error("Invalid var name!");
    
    stat_let->var = NodeVar{.name = var_name};

    if (!peek().has_value() || peek().value().type != TokenType::eq)
        throw std::runtime_error("Operator '=' is missing!");

    consume();

    stat_let->expr = parse_expr();

    return stat_let;
}

NodeStatPrint* Parser::parse_stat_print() {
    auto stat_print = m_mem_pool.alloc<NodeStatPrint>();
    auto expr_list = m_mem_pool.alloc<NodeExprList>();

    stat_print->exprs = expr_list;

    if (!peek().has_value())
        throw std::runtime_error("Print is empty!");

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
            throw std::runtime_error("Wrong token in print!");
        }
    }

    return stat_print;
}

NodeStat* Parser::parse_stat() {
    if (!peek().has_value()) 
        throw std::runtime_error("Command is empty!");

    auto node_stat = m_mem_pool.alloc<NodeStat>();
    auto token = consume();

    switch (token.type) {
        case TokenType::print:
            node_stat->com = Parser::parse_stat_print();
            break;
        case TokenType::_if: break;
        case TokenType::_goto: break;
        case TokenType::input: break;
        case TokenType::let: 
            node_stat->com = Parser::parse_stat_let();
            break;
        case TokenType::gosub: break;
        case TokenType::_return: break;
        case TokenType::clear: break;
        case TokenType::list: break;
        case TokenType::run: break;
        case TokenType::end: break;
        default: 
            throw std::runtime_error("Invalid command!");
    }

    return node_stat;
}

NodeLine* Parser::parse_line() {
    auto line = m_mem_pool.alloc<NodeLine>();

    if (peek().value().type == TokenType::num) {
        auto token = consume();
        line->num = NodeNum{.num = token.var.value()};
    }

    line->stat = parse_stat();

    return line;
}

NodeProg Parser::gen_prog() {
    NodeProg prog;

    while (peek().has_value()) {
        prog.lines.push_back(parse_line());
    }

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
