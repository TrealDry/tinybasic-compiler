#include "lexer.hpp"
#include <cctype>
#include <string>
#include <vector>

Token Lexer::tokenize_alpha() {
    std::string buf;
    
    while (peek().has_value() && std::isalpha(peek().value())) {
        buf.push_back(consume());
    }

    if      (buf == "PRINT")  return {.type = TokenType::print};
    else if (buf == "IF")     return {.type = TokenType::_if};
    else if (buf == "THEN")   return {.type = TokenType::then};
    else if (buf == "GOTO")   return {.type = TokenType::_goto};
    else if (buf == "INPUT")  return {.type = TokenType::input};
    else if (buf == "LET")    return {.type = TokenType::let};
    else if (buf == "GOSUB")  return {.type = TokenType::gosub};
    else if (buf == "RETURN") return {.type = TokenType::_return};
    else if (buf == "CLEAR")  return {.type = TokenType::clear};
    else if (buf == "LIST")   return {.type = TokenType::list};
    else if (buf == "RUN")    return {.type = TokenType::run};
    else if (buf == "END")    return {.type = TokenType::end};
    else                      return {.type = TokenType::var, .var = buf};
}

Token Lexer::tokenize_digit() {
    std::string buf;

    while (peek().has_value() && std::isdigit(peek().value())) {
        buf.push_back(consume());
    }

    return {.type = TokenType::num, .var = buf};
}

std::vector<Token> Lexer::gen_tokens() {
    std::vector<Token> result;
    std::string buf;

    while (true) {
        auto c = peek();
        if (!c.has_value()) break;

        if (std::isalpha(c.value())) {
            result.push_back(tokenize_alpha());
        } else if (std::isdigit(c.value())) {
            result.push_back(tokenize_digit());
        } else {
            switch (c.value()) {
                case '(': result.push_back({.type = TokenType::open_paren}); break;
                case ')': result.push_back({.type = TokenType::close_paren}); break;
                case '>': break;
                case '<': break;
                case '=': break;
                case '"': break;
                case '+': break;
                case '-': break;
                case '*': break;
                case '/': break;
                case ',': break;
                case ' ': break;
                case '\n': result.push_back({.type = TokenType::cr}); break;
            }
            consume();
        }
    }
    
    return result;
}

std::optional<char> Lexer::peek(int offset) {
    if (m_index + offset >= m_code.length()) {
        return {};
    } else {
        return m_code.at(m_index + offset);
    }
}

char Lexer::consume() {
    return m_code.at(m_index++);
}
