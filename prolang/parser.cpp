#include <iostream>
#include <vector>
#include <memory>
#include <string>

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

class ASTNode
{
public:
    virtual void print() const = 0;
};

class NumberNode : public ASTNode
{
    int value;

public:
    NumberNode(int value) : value(value) {}
    void print() const override
    {
        cout << "Number: " << value << endl;
    }
};

class IdentifierNode : public ASTNode
{
    string name;

public:
    IdentifierNode(const string &name) : name(name) {}
    void print() const override
    {
        cout << "Identifier: " << name << endl;
    }
};

class BinaryOpNode : public ASTNode
{
    string op;
    shared_ptr<ASTNode> left, right;

public:
    BinaryOpNode(const string &op, shared_ptr<ASTNode> left, shared_ptr<ASTNode> right)
        : op(op), left(left), right(right) {}

    void print() const override
    {
        left->print();
        cout << "Operator: " << op << endl;
        right->print();
    }
};

class AssignmentNode : public ASTNode
{
    shared_ptr<IdentifierNode> left;
    shared_ptr<ASTNode> right;

public:
    AssignmentNode(shared_ptr<IdentifierNode> left, shared_ptr<ASTNode> right)
        : left(left), right(right) {}

    void print() const override
    {
        left->print();
        cout << "Assignment: " << endl;
        right->print();
    }
};

class BlockNode : public ASTNode
{
    vector<shared_ptr<ASTNode>> statements;

public:
    void addStatement(shared_ptr<ASTNode> statement)
    {
        statements.push_back(statement);
    }

    void print() const override
    {
        for (const auto &stmt : statements)
        {
            stmt->print();
        }
    }
};

class FunctionNode : public ASTNode
{
    string name;
    vector<string> parameters;
    shared_ptr<BlockNode> body;

public:
    FunctionNode(const string &name, const vector<string> &parameters, shared_ptr<BlockNode> body)
        : name(name), parameters(parameters), body(body) {}

    void print() const override
    {
        cout << "Function: " << name << " with parameters: ";
        for (const auto &param : parameters)
        {
            cout << param << " ";
        }
        cout << endl;
        cout << "Body:" << endl;
        body->print();
    }
};

class FunctionCallNode : public ASTNode
{
    string functionName;
    vector<shared_ptr<ASTNode>> arguments;

public:
    FunctionCallNode(const string &functionName, const vector<shared_ptr<ASTNode>> &arguments)
        : functionName(functionName), arguments(arguments) {}

    void print() const override
    {
        cout << "Function Call: " << functionName << " with arguments: ";
        for (const auto &arg : arguments)
        {
            arg->print();
        }
    }
};

class Parser
{
public:
    Parser(const vector<Token> &tokens) : tokens(tokens), pos(0) {}

    shared_ptr<ASTNode> parseStatements()
    {
        auto block = make_shared<BlockNode>(); // Use BlockNode to hold multiple statements
        while (pos < tokens.size() && tokens[pos].type != END_OF_FILE)
        {
            block->addStatement(parseStatement()); // Keep adding statements until EOF
        }
        return block;
    }

    shared_ptr<ASTNode> parseStatement()
    {
        if (tokens[pos].value == "def")
        {
            return parseFunction(); // Parse function if the "def" keyword is found
        }
        auto expr = parseExpression();
        expectSymbol(";");
        return expr;
    }

private:
    vector<Token> tokens;
    size_t pos;

    shared_ptr<ASTNode> parseExpression()
    {
        auto left = parsePrimary();
        return parseBinaryOp(0, left);
    }

    std::shared_ptr<ASTNode> parsePrimary()
    {
        if (tokens[pos].type == IDENTIFIER)
        {
            string identifier = tokens[pos].value;
            if (tokens[pos + 1].value == "(")
            {
                return parsseFuntionCall(identifier);
            }
            if (pos + 1 < tokens.size() && tokens[pos + 1].value == "=")
            {
                return parseAssignment(); // Assignment if identifier is followed by '='
            }
            else
            {
                return std::make_shared<IdentifierNode>(tokens[pos++].value);
            }
        }
        else if (tokens[pos].type == NUMBER)
        {
            int value = std::stoi(tokens[pos++].value);
            return std::make_shared<NumberNode>(value);
        }
        else if (tokens[pos].type == KEYWORD)
        {
            if (tokens[pos].value == "int")
            {
                ++pos;
                auto indetifier = make_shared<IdentifierNode>(tokens[pos++].value);
                expectSymbol("=");
                auto value = parseExpression();
                return make_shared<AssignmentNode>(indetifier, value);
            }
        }
        cout << "Error: Unexpected token: " << tokens[pos].value << endl;
        return nullptr;
    }

    shared_ptr<ASTNode> parseAssignment()
    {
        auto identifier = make_shared<IdentifierNode>(tokens[pos++].value);
        expectSymbol("=");
        auto value = parseExpression();
        return make_shared<AssignmentNode>(identifier, value);
    }

    shared_ptr<ASTNode> parseBinaryOp(int precedence, shared_ptr<ASTNode> left)
    {
        while (true)
        {
            if (pos >= tokens.size())
                return left;
            string op = tokens[pos].value;

            if (op == "+" || op == "-" || op == "*" || op == "/" || op == "<<" || op == ">>")
            {
                ++pos;
                auto right = parsePrimary();
                left = make_shared<BinaryOpNode>(op, left, right);
            }
            else
            {
                return left;
            }
        }
    }

    shared_ptr<ASTNode> parseFunction()
    {
        expectSymbol("def");                       // Expecting the keyword for defining functions
        string functionName = tokens[pos++].value; // Function name
        expectSymbol("(");

        vector<string> parameters;
        while (tokens[pos].value != ")")
        {
            parameters.push_back(tokens[pos++].value);
            if (tokens[pos].value == ",")
            {
                ++pos; // Skip the comma
            }
        }
        expectSymbol(")");

        expectSymbol("{");
        auto body = make_shared<BlockNode>();
        while (tokens[pos].value != "}")
        {
            body->addStatement(parseStatement());
        }
        expectSymbol("}");

        return make_shared<FunctionNode>(functionName, parameters, body);
    }

    shared_ptr<ASTNode> parsseFuntionCall(const string &functionName)
    {
        pos++;
        expectSymbol("(");
        vector<shared_ptr<ASTNode>> arguments;

        while (tokens[pos].value != ")")
        {
            arguments.push_back(parseExpression());
            if (tokens[pos].value == ",")
            {
                ++pos;
            }
        }

        expectSymbol(")");
        return make_shared<FunctionCallNode>(functionName, arguments);
    }

    void expectSymbol(const string &symbol)
    {
        if (tokens[pos].value == symbol)
        {
            ++pos;
        }
        else
        {
            throw runtime_error("Expected symbol: " + symbol);
        }
    }
};

int main()
{
    vector<Token> tokens = {
        Token(KEYWORD, "def"),       // function definition keyword
        Token(IDENTIFIER, "myFunc"), // function name
        Token(SYMBOL, "("),          // open parenthesis for parameters
        Token(IDENTIFIER, "a"),      // first parameter
        Token(SYMBOL, ","),          // comma between parameters
        Token(IDENTIFIER, "b"),      // second parameter
        Token(SYMBOL, ")"),          // close parenthesis
        Token(SYMBOL, "{"),          // open brace for function body
        Token(IDENTIFIER, "x"),      // assignment statement inside function
        Token(SYMBOL, "="),
        Token(NUMBER, "10"),
        Token(SYMBOL, ";"),
        Token(SYMBOL, "}"), // close brace for function body

        // Function call
        Token(IDENTIFIER, "myFunc"), // function name for call
        Token(SYMBOL, "("),          // open parenthesis for arguments
        Token(NUMBER, "5"),          // first argument
        Token(SYMBOL, ","),          // comma between arguments
        Token(NUMBER, "15"),         // second argument
        Token(SYMBOL, ")"),          // close parenthesis
        Token(SYMBOL, ";"),          // semicolon to end function call statement
    };

    Parser parser(tokens);
    shared_ptr<ASTNode> root = parser.parseStatements();
    root->print();

    return 0;
}
