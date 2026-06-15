#ifndef JS_PARSER_H
#define JS_PARSER_H

#include "map"

#include "lexer.h"

struct Node {
	std::string op;
	TokenType type;
	struct Node *left;
	struct Node *right;

	Node(const std::string& oper, Node *leftNode, Node *rightNode) { op = oper; left = leftNode; right = rightNode; };
	Node(const std::string& oper, Node *leftNode, Node *rightNode, TokenType itemType) { op = oper; left = leftNode; right = rightNode; type = itemType; };
	Node(std::string oper) { op = oper; left = nullptr; right = nullptr; };
	Node(std::string oper, TokenType itemType) { op = oper; left = nullptr; right = nullptr; type = itemType; };
};

struct Opperator {
	std::string op;
	TokenType type;
};

class Parser {
public:
	Parser(const std::vector<Token> &newTokens);
	~Parser();

	std::vector<Node*>& ParseToAst();

private:
	bool IsOperator(std::string type);
	void AddNode(Opperator op, std::vector<Node*>& astOutput);
	void DeleteNode(Node *node);
	void PrintNode(Node *node);

	std::map<std::string, int> operators = {{"let", -1},{"=", 0},{"||", 1}, {"&&", 2}, {"<", 3}, {"<=", 3},{">", 3}, {">=", 3}, {"==", 3}, {"!=", 3},{"+", 4}, {"-", 4}, {"*", 5}, {"/", 5}, {"%", 5}, {"**", 6}, {"u+", 7}, {"u-", 7}, {"!", 7}};
	std::vector<Token> tokens;
	std::vector<Node*> output;
};


#endif //JS_PARSER_H