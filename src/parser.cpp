#include <stdexcept>

#include "parser.hpp"
#include "lexer.hpp"

NodeExpr* Parser::parse_expr() {
    auto expr = m_mem_pool.alloc<NodeExpr>();

    return expr;
}

NodeStatLet* Parser::parse_stat_let() {
    auto stat_let = m_mem_pool.alloc<NodeStatLet>();

    if (!peek().has_value() || peek().value().type != TokenType::var)
        throw std::runtime_error("Var name is empty!");

    std::string var_name = consume().var.value();
    if (var_name.length() != 1 || !std::isupper(var_name.at(0)))
        throw std::runtime_error("Invalid var name!");
    
    stat_let->var = NodeVar{.name = consume().var.value()};

    if (!peek().has_value() || peek().value().type != TokenType::eq)
        throw std::runtime_error("Operator '=' is missing!");

    consume();

    stat_let->expr = parse_expr();

    return stat_let;
}

NodeStat* Parser::parse_stat() {
    if (!peek().has_value()) {
        throw std::runtime_error("Command is empty!");
    }

    auto node_stat = m_mem_pool.alloc<NodeStat>();
    auto token = consume();

    switch (token.type) {
        case TokenType::print: break;
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

    auto token = consume();
    if (token.type == TokenType::num) {
        line->num = NodeNum{.num = token.var.value()};
        token = consume();
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
