#include <fstream>
#include <iostream>
#include <sstream>

#include "lexer.h"

#include "parser.h"

std::ostream& operator<<(std::ostream& os, TokenType t) {
    switch (t) {
    case TokenType::Number: return os << "Number";
    case TokenType::String: return os << "String";
    case TokenType::Plus: return os << "Plus";
    case TokenType::Minus: return os << "Minus";
    case TokenType::Star: return os << "Star";
    case TokenType::Slash: return os << "Slash";
    case TokenType::Percent: return os << "Percent";
    case TokenType::ExclamationMark: return os << "ExclamationMark";
    case TokenType::StarStar: return os << "StarStar";
    case TokenType::LeftParen: return os << "LeftParen";
    case TokenType::RightParen: return os << "RightParen";
    case TokenType::Equals: return os << "Equals";
    case TokenType::Identifier: return os << "Identifier";
    case TokenType::Let: return os << "Let";
    case TokenType::If: return os << "If";
    case TokenType::Function: return os << "Function";
    case TokenType::Return: return os << "Return";
    case TokenType::True: return os << "True";
    case TokenType::False: return os << "False";
    case TokenType::And: return os << "And";
    case TokenType::Or: return os << "Or";
    case TokenType::EqualsComp: os << "EqualsComp";
    case TokenType::LargerEquals: return os << "LargerEquals";
    case TokenType::LargerThen: return os << "LargerThen";
    case TokenType::SmallerEquals: return os << "SmallerEquals";
    case TokenType::SmallerThen: return os << "SmallerThen";
    case TokenType::Not: return os << "Not";
    case TokenType::NotEquals: return os << "NotEquals";
    case TokenType::Semicolon: return os << "Semicolon";
    case TokenType::NewLine: return os << "NewLine";
    case TokenType::EndOfFile: return os << "EndOfFile";
    }
    return os << "Unknown";
}

std::ostream& operator<<(std::ostream& os, const Token& t) {
    return os << "type: " << t.type << ", contents: " << t.text << ", line: " << t.line;
}

Lexer::Lexer(std::string source)
{
    std::ifstream sourceFile(source);

    if (!sourceFile) {
        throw std::runtime_error("File failed to open");
    }

    std::stringstream buffer;
    buffer << sourceFile.rdbuf();

    sourceFile.close();

    sourceCode = buffer.str();
}

std::vector<Token> Lexer::Tokenize() {
    std::vector<Token> tokens;
    char character = 'a';
    std::string word = "";
    size_t line = 1;
    bool insString = false;
    bool escaped = false;
    while (position != sourceCode.length()) {
        character = sourceCode[position];

        if (character == '\n') {
            if (insString) {
                throw std::runtime_error(std::string("string not closed on line: ") + std::to_string(line));
            }
            if (!word.empty()) {
                if (IsNumberWord(word)){
                    FlushNumber(tokens, word, line);
                } else {
                    FlushWord(tokens, word, line);
                }
            }
            Token token;
            token.type = TokenType::NewLine;
            token.text = "\\n";
            token.line = line;
            tokens.push_back(token);
            line++;
            position++;
            continue;
        }

        if (std::isspace(character)) {
            if (! insString) {
                if (!word.empty()) {
                    if (IsNumberWord(word)){
                        FlushNumber(tokens, word, line);
                    } else {
                        FlushWord(tokens, word, line);
                    }
                }
                position++;
                continue;
            }
        }

        /*bool valid = false;
        if (std::isdigit(character) || std::isalpha(character)) {
            valid = true;
        }

        if (!valid)
        {
            for (auto &c : validRaraChars + "." + validBoolChars) {
                if (character == c) {
                    valid = true;
                    break;
                }
            }
        }

        if (!valid) {
            throw std::runtime_error(std::string("Unknown character ") + character + std::string(" on line ") + std::to_string(line));
        }*/

        if (character == '.' && word.find('.') != std::string::npos) {
            throw std::runtime_error(std::string("You cant have a decimal in a decimal on line") + std::to_string(line));
        }

        if (!word.empty() && !insString) {
            if (IsRaraCharacter(character)) {
                if (IsNumberWord(word)){
                    FlushNumber(tokens, word, line);
                } else {
                    FlushWord(tokens, word, line);
                }
            }
            if (IsBoolChar(character) && !IsBoolChar(word[0]) || !IsBoolChar(character) && IsBoolChar(word[0])) {
                FlushWord(tokens, word, line);
            }
        }
        ParseCharacter(tokens, sourceCode, position, word, line, insString, escaped);
        position++;
    }
    if (!word.empty() && !insString) {
        if (IsNumberWord(word)){
            FlushNumber(tokens, word, line);
        } else {
            FlushWord(tokens, word, line);
        }
    }

    Token token;
    token.type = TokenType::EndOfFile;
    token.text = "";
    token.line = line;
    tokens.push_back(token);

    for (auto &token : tokens) {
        std::cout << token << std::endl;
    }

    return tokens;
}

void Lexer::ParseCharacter(std::vector<Token> &tokens, const std::string &input, size_t &index, std::string &word, size_t line, bool &inString, bool &escaped) {
    char character = input[index];
    if (inString) {
        if (character == '\\' && !escaped) {
            escaped = true;
            return;
        }
        if (character != '\"' || escaped) {
            if (escaped) {
                if (character == 'n') character = '\n';
                else if (character == 'r') character = '\r';
                else if (character == 't') character = '\t';
                else if (character == 'b') character = '\b';
                else if (character == 'f') character = '\f';
                else if (character == 'v') character = '\v';
                else if (character == '0') character = '\0';
                else if (character == 'x') {
                    if (input.size() - index - 2 < 0) throw std::runtime_error(std::string("SyntaxError: Invalid hexadecimal escape sequence on line: ") + std::to_string(line));
                    std::string hex;
                    hex += input[++index];
                    hex += input[++index];
                    if (!(IsHexChar(hex[0]) && IsHexChar(hex[1]))) throw std::runtime_error(std::string("SyntaxError: Invalid hexadecimal escape sequence on line: ") + std::to_string(line));
                    character = (char) std::stoi(hex, nullptr, 16);
                }
                else if (character == 'u') {
                    if (input.size() - index - 4 < 0) throw std::runtime_error(std::string("Invalid Unicode escape sequence on line: ") + std::to_string(line));
                    std::string hex;
                    for (int i = 0; i < 4; i++) {
                        hex += input[++index];
                        if (!IsHexChar(hex[i])) throw std::runtime_error(std::string("Invalid Unicode escape sequence on line: ") + std::to_string(line));
                    }
                    character = (char) std::stoi(hex, nullptr, 16);
                }
                escaped = false;
            }
            word += character;
            return;
        }
    }

    bool isAlpha = std::isalpha(character) || character == '_' || character == '$' || character == '=' || character == '&' || character == '|' || character == '!';
    if (isAlpha) {
        if (IsNumberWord(word)) {
            FlushNumber(tokens, word, line);
        }
        word += character;
        return;
    }

    bool isDigit = std::isdigit(character) || character == '.';
    if (isDigit) {
        if (!word.empty() && !IsNumberWord(word)) {
            FlushWord(tokens, word, line);
        }
        word += character;
        return;
    }

    if (IsBoolChar(character)) {
        if (!word.empty() && IsNumberWord(word)) {
            FlushNumber(tokens, word, line);
        } else if (!word.empty() && !IsBoolChar(word[0])) {
            FlushWord(tokens, word, line);
        }
        word += character;
        return;
    }

    Token token;
    token.text = character;
    token.line = line;

    if (character == '+') {
        token.type = TokenType::Plus;
    } else if (character == '*') {
        if (index + 1 <= input.size() && input[index + 1] == '*') {
            tokens.push_back({TokenType::StarStar, "**", line});
            index++;
            return;
        }
        token.type = TokenType::Star;
    } else if (character == '-') {
        token.type = TokenType::Minus;
    } else if (character == '/') {
        token.type = TokenType::Slash;
    } else if (character == '%') {
        token.type = TokenType::Percent;
    } else if (character == '!') {
        token.type = TokenType::ExclamationMark;
    } else if (character == '(') {
        token.type = TokenType::LeftParen;
    } else if (character == ')') {
        token.type = TokenType::RightParen;
    } else if (character == ';') {
        token.type = TokenType::Semicolon;
    } else if (character == '\"') {
        if (inString) {
            token.text = word;
            token.type = TokenType::String;
            word = "";
        } else {
            if (!word.empty()) {
                if (IsNumberWord(word)){
                    FlushNumber(tokens, word, line);
                } else {
                    FlushWord(tokens, word, line);
                }
            }
            inString = !inString;
            return;
        }
        inString = !inString;
    } else {
        word += character;
        return;
    }

    tokens.push_back(token);
}

void Lexer::FlushNumber(std::vector<Token>& tokens, std::string& number, size_t line) {
    if (number.empty()) return;

    Token token;
    token.text = number;
    token.line = line;
    token.type = TokenType::Number;
    tokens.push_back(token);
    number.clear();
}

void Lexer::FlushWord(std::vector<Token>& tokens, std::string& word, size_t line) {
    if (word.empty()) return;

    Token token;
    token.text = word;
    token.line = line;

    if (word == "let") {
        token.type = TokenType::Let;
    } else if (word == "if") {
        token.type = TokenType::If;
    } else if (word == "function") {
        token.type = TokenType::Function;
    } else if (word == "return") {
        token.type = TokenType::Return;
    } else if (word == "true") {
        token.type = TokenType::True;
    } else if (word == "false") {
        token.type = TokenType::False;
    } else if (word == "==") {
        token.type = TokenType::EqualsComp;
    } else if (word == "=") {
        token.type = TokenType::Equals;
    } else if (word == "!") {
        token.type = TokenType::Not;
    } else if (word == "&&") {
        token.type = TokenType::And;
    } else if (word == "||") {
        token.type = TokenType::Or;
    } else if (word == "<=") {
        token.type = TokenType::SmallerEquals;
    } else if (word == "<") {
        token.type = TokenType::SmallerEquals;
    } else if (word == ">=") {
        token.type = TokenType::LargerEquals;
    } else if (word == ">") {
        token.type = TokenType::LargerThen;
    } else if (word == "!=") {
        token.type = TokenType::NotEquals;
    } else {
        token.type = TokenType::Identifier;
    }

    tokens.push_back(token);
    word.clear();
}

bool Lexer::IsRaraCharacter(char character) {
    for (auto &c : validRaraChars) {
        if (character == c) {
            return true;
        }
    }
    return false;
}

bool Lexer::IsNumberWord(const std::string& word) {
    return !word.empty() && (std::isdigit(word[0]) || word[0] == '.');
}

bool Lexer::IsBoolChar(char character) {
    for (auto &c : validBoolChars) {
        if (character == c) {
            return true;
        }
    }
    return false;
}

bool Lexer::IsHexChar(char character) {
    if (character >= '0' && character <= '9') return true;
    if (character >= 'A' && character <= 'Z') return true;
    if (character >= 'a' && character <= 'z') return true;
    return false;
}
