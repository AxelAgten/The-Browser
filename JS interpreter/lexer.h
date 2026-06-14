#ifndef JS_LEXER_H
#define JS_LEXER_H

#include <string>
#include <vector>

enum class TokenType
{
    Number,
    String,
    Plus,
    Minus,
    Star,
    Slash,
    Percent,
    LeftParen,
    RightParen,
    Equals,
    Identifier,
    Let,
    If,
    Function,
    Return,
    True,
    False,
    EqualsComp,
    And,
    Or,
    Not,
    SmallerThen,
    LargerThen,
    SmallerEquals,
    LargerEquals,
    NotEquals,
    Semicolon,
    NewLine,
    EndOfFile
};

struct Token
{
    TokenType type;
    std::string text;
    size_t line;
};

class Lexer
{
public:
    explicit Lexer(std::string source);

    std::vector<Token> Tokenize();

private:
    void ParseCharacter(std::vector<Token> &tokens, char character, std::string &number, size_t line, bool &inString);
    void FlushNumber(std::vector<Token>& tokens, std::string& number, size_t line);
    void FlushWord(std::vector<Token>& tokens, std::string& word, size_t line);
    bool IsRaraCharacter(char character);
    bool IsNumberWord(const std::string& word);
    bool IsBoolChar(char character);

    std::string validRaraChars = "+-*/%$_;()!\" ";
    std::string validBoolChars = "=&|<>!";
    std::string sourceCode;
    size_t position = 0;
};

#endif //JS_LEXER_H
