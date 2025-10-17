#pragma once

#include <string>
#include <vector>
#include <optional>

enum class TokenType {
    print, _if, then, _goto, input, let, 
    gosub, _return, clear, list, run, end,
    num, var, cr, open_paren, close_paren,

};

struct Token {
    TokenType type;
    std::optional<std::string> var;
};

class Lexer {
public:
    Lexer(std::string& code) : m_code(std::move(code)) {}

    std::vector<Token> gen_tokens();

private:
    std::optional<char> peek(int offset = 0);
    char consume();

    Token tokenize_alpha();
    Token tokenize_digit();

    size_t m_index = 0;
    std::string m_code;
};