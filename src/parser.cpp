#include "parser.hpp"
#include "lexer.hpp"

NodeProg Parser::gen_prog() {
    ;
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
