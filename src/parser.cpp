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
    expr->term = std::monostate{};

    while (peek().has_value() && peek().value().type != TokenType::cr) {
        bool term_op_exits = false;
        NodeTerm* first_term;

        if (peek().value().type == TokenType::plus || peek().value().type == TokenType::minus) {
            term_op_exits = true;
        } else if (peek().value().type == TokenType::num || \
                   peek().value().type == TokenType::var || \
                   peek().value().type == TokenType::open_paren || \
                   peek().value().type == TokenType::mul || \
                   peek().value().type == TokenType::div) 
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
            term_op->term = std::get<1>(expr->term);
        } else {
            term_op->term = first_term;
        }
        term_op->term2 = second_term;

        expr->term = term_op;
    }

    return expr;
}

NodeRelop Parser::parse_relop() {
    if (!peek().has_value() && !peek(1).has_value())
        throw std::runtime_error("Relop is empty!");
    
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
            if (second_token.type == TokenType::gt) 
                relop.type = RelopType::ne;
            else if (second_token.type == TokenType::eq)
                relop.type = RelopType::lte;
            else
                relop.type = RelopType::lt;
            break;

        case TokenType::gt:
            if (second_token.type == TokenType::lt) 
                relop.type = RelopType::crazy;  // TODO print warn
            else if (second_token.type == TokenType::eq)
                relop.type = RelopType::gte;
            else
                relop.type = RelopType::gt;
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

    if (!peek().has_value())
        throw std::runtime_error("First expr is empty!");

    stat_if->expr  = parse_expr();
    stat_if->relop = parse_relop();
    stat_if->expr2 = parse_expr();

    if (!peek().has_value() || peek().value().type != TokenType::then)
        throw std::runtime_error("THEN keyword is missing!");

    consume();

    stat_if->then = parse_stat();

    return stat_if;
}

NodeStatGoto* Parser::parse_stat_goto() {
    auto stat_goto = m_mem_pool.alloc<NodeStatGoto>();

    if (!peek().has_value() || peek().value().type == TokenType::cr)
        throw std::runtime_error("Goto expr is empty!");

    stat_goto->expr = parse_expr();

    if (stat_goto->expr->term.index() == 2)
        throw std::runtime_error("Goto expr is not constant!");

    auto term = std::get<1>(stat_goto->expr->term);
    if (term->fact.index() == 1)
        throw std::runtime_error("Goto expr is not constant!");

    auto fact = std::get<0>(term->fact);

    if (fact->body.index() != 1)
        throw std::runtime_error("Goto expr is not constant!");

    return stat_goto;
}

NodeStatLet* Parser::parse_stat_let() {
    auto stat_let = m_mem_pool.alloc<NodeStatLet>();

    if (!peek().has_value() || peek().value().type != TokenType::var)
        throw std::runtime_error("Var name is empty!");

    std::string var_name = consume().var.value();
    if (var_name.length() != 1 || !std::isupper(var_name.at(0)))
        throw std::runtime_error("Invalid var name!");
    
    m_unique_let.insert(var_name.at(0));
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
        case TokenType::_if:
            node_stat->com = Parser::parse_stat_if();
            break;
        case TokenType::_goto:
            node_stat->com = Parser::parse_stat_goto();
            break;
        case TokenType::input: break;
        case TokenType::let: 
            node_stat->com = Parser::parse_stat_let();
            break;
        case TokenType::gosub: break;
        case TokenType::_return: break;
        case TokenType::clear: break;
        case TokenType::list: break;
        case TokenType::run: break;
        case TokenType::end:
            node_stat->com = m_mem_pool.alloc<NodeStatEnd>();
            break;
        case TokenType::cr: break;
        default: 
            throw std::runtime_error("Invalid command!");
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
    }

    line->stat = parse_stat();

    return line;
}

NodeProg Parser::gen_prog() {
    NodeProg prog;

    m_index = 0;
    m_unique_let.clear();

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
