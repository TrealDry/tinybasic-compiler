#include "lexer.hpp"
#include "error.hpp"

#include <cctype>
#include <format>
#include <string>
#include <utility>
#include <vector>

Token Lexer::tokenize_alpha() {
    std::string buf;
    
    while (peek().has_value() && std::isalpha(peek().value())) {
        buf.push_back(consume());
    }

    if      (buf == "PRINT")  return {.type = TokenType::print, .line=m_line};
    else if (buf == "IF")     return {.type = TokenType::_if, .line=m_line};
    else if (buf == "THEN")   return {.type = TokenType::then, .line=m_line};
    else if (buf == "GOTO")   return {.type = TokenType::_goto, .line=m_line};
    else if (buf == "INPUT")  return {.type = TokenType::input, .line=m_line};
    else if (buf == "LET")    return {.type = TokenType::let, .line=m_line};
    else if (buf == "GOSUB")  return {.type = TokenType::gosub, .line=m_line};
    else if (buf == "RETURN") return {.type = TokenType::_return, .line=m_line};
    else if (buf == "CLEAR")  return {.type = TokenType::clear, .line=m_line};
    else if (buf == "LIST")   return {.type = TokenType::list, .line=m_line};
    else if (buf == "RUN")    return {.type = TokenType::run, .line=m_line};
    else if (buf == "END")    return {.type = TokenType::end, .line=m_line};
    else                      return {.type = TokenType::var, .line=m_line, .var = buf};
}

Token Lexer::tokenize_digit() {
    std::string buf;

    while (peek().has_value() && std::isdigit(peek().value())) {
        buf.push_back(consume());
    }

    return {.type = TokenType::num, .line=m_line, .var = buf};
}

Token Lexer::tokenize_str() {
    std::string buf;
    consume();  // skip first double quotes

    while (peek().has_value() && peek().value() != '"') {
        if (peek().value() == '\\' && peek(1).has_value()) {  // escape sequence
            switch (peek(1).value()) {
                case 'n':
                    buf.push_back('\n');
                    break;
                case 't':
                    buf.push_back('\t');
                    break;
                default:
                    buf.push_back(consume());
                    continue;
            }
            consume();
            consume();
        } else {
            buf.push_back(consume());
        }
    }

    return {.type = TokenType::str, .line=m_line, .var = buf};
}

void Lexer::remove_comments() {
    while (peek().has_value() && peek().value() != '\n') {
        consume();
    }

    m_index--;  // so that the lexer doesn't miss cr
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
                case '(': result.push_back({.type = TokenType::open_paren, .line=m_line}); break;
                case ')': result.push_back({.type = TokenType::close_paren, .line=m_line}); break;
                case '>': result.push_back({.type = TokenType::gt, .line=m_line}); break;
                case '<': result.push_back({.type = TokenType::lt, .line=m_line}); break;
                case '=': result.push_back({.type = TokenType::eq, .line=m_line}); break;
                case '+': result.push_back({.type = TokenType::plus, .line=m_line}); break;
                case '-': result.push_back({.type = TokenType::minus, .line=m_line}); break;
                case '*': result.push_back({.type = TokenType::mul, .line=m_line}); break;
                case '/': result.push_back({.type = TokenType::div, .line=m_line}); break;
                case ',': result.push_back({.type = TokenType::com, .line=m_line}); break;
                case '\n': result.push_back({.type = TokenType::cr, .line=m_line++}); break;
                case '"': result.push_back(tokenize_str()); break;
                case '\'': remove_comments();
                case ' ': break;
                default: Error::warning(m_line, "Non-standard character!"); break;
            }
            consume();
        }
    }
    
    if (result.size() != 0)
        result.push_back({.type=TokenType::cr, .line=m_line});

    m_line = 1;
    m_index = 0;
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
