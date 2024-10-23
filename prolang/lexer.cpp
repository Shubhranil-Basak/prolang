#include <iostream>
#include <string>
#include <cctype>
#include <vector>
#include <unordered_set>

using namespace std;

enum TokenType
{
    KEYWORD,
    IDENTIFIER,
    NUMBER,
    SYMBOL,
    END_OF_FILE
};

struct Token
{
    TokenType type;
    string value;

    Token(TokenType type, const string &value) : type(type), value(value) {}
};

class Lexer
{
private:
    string input;
    size_t pos;
    unordered_set<string> keywords;

    Token ParseIdentifierorKeyword()
    {
        size_t startPos = pos;
        while (pos < input.size() && isalnum(input[pos]))
        {
            ++pos;
        }
        string identifier = input.substr(startPos, pos - startPos);

        if (keywords.find(identifier) != keywords.end())
        {
            return Token(KEYWORD, identifier);
        }
        else
        {
            return Token(IDENTIFIER, identifier);
        }
    }

    Token ParseNumber()
    {
        size_t startPos = pos;
        while (pos < input.size() && isdigit(input[pos]))
        {
            ++pos;
        }
        string number = input.substr(startPos, pos - startPos);
        return Token(NUMBER, number);
    }

    Token ParseSymbol()
    {
        char symbol = input[pos++];
        return Token(SYMBOL, string(1, symbol));
    }

public:
    Lexer(const string &input) : input(input), pos(0)
    {
        keywords = {"def", "int", "if", "else", "return", "while", "elif"};
    }

    vector<Token> tokenize(){
        vector<Token> tokens;
        while(pos < input.size()){
            char currentChar = input[pos];

            if(isspace(currentChar)){
                ++pos;
            }
            else if(isalpha(currentChar)){
                tokens.push_back(ParseIdentifierorKeyword());
            } else if(isdigit(currentChar)){
                tokens.push_back(ParseNumber());
            } else if(ispunct(currentChar)){
                tokens.push_back(ParseSymbol());
            } else {
                ++pos;
            }
        }
        tokens.push_back(Token(END_OF_FILE, ""));
        return tokens;
    }
};

void printTokens(const vector<Token>& tokens) {
    for (const Token& token : tokens) {
        string typeName;
        switch (token.type) {
            case KEYWORD:    typeName = "KEYWORD";    break;
            case IDENTIFIER: typeName = "IDENTIFIER"; break;
            case NUMBER:     typeName = "NUMBER";     break;
            case SYMBOL:     typeName = "SYMBOL";     break;
            case END_OF_FILE:typeName = "END_OF_FILE";break;
        }
        std::cout << "Token: " << typeName << ", Value: " << token.value << std::endl;
    }
}

int main() {
    std::string input = "int x = 10 + 20; if (x > 10) return x;";
    Lexer lexer(input);
    std::vector<Token> tokens = lexer.tokenize();
    printTokens(tokens);
    return 0;
}