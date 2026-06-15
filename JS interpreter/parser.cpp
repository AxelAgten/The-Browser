#include "parser.h"
#include <ostream>
#include <iostream>

std::ostream& operator<<(std::ostream& os, const Node& n) {
	return os << "Node(" << n.op << ")";
}

Parser::Parser(const std::vector<Token>& newTokens) : tokens(newTokens) {

}

Parser::~Parser() {
	if (!output.empty()) {
		for (auto node : output) {
			DeleteNode(node);
		}
	}
}

std::vector<Node*>& Parser::ParseToAst() {
	std::vector<Opperator> ops = {};
	std::vector<Node*> astOutput = {};
	bool inLet = false;
	bool expectingOperand = true;

	for (auto &token : tokens) {
		if (token.type == TokenType::Semicolon || token.type == TokenType::NewLine) {
			if (inLet) {
				throw std::runtime_error("Expected '=' in let statement");
			}
			while (!ops.empty()) {
				AddNode(ops.back(), astOutput);
				ops.pop_back();
			}
			if (!astOutput.empty()) {
				output.push_back(astOutput[0]);
			}
			astOutput = {};
			expectingOperand = true;
			continue;
		}
		if (token.type == TokenType::Number || token.type == TokenType::True || token.type == TokenType::False || token.type == TokenType::String) {
			astOutput.push_back(new Node(token.text, token.type));
			expectingOperand = false;
		} else if (token.type == TokenType::Identifier) {
			astOutput.push_back(new Node(token.text, token.type));
			expectingOperand = false;
		} else if (token.type == TokenType::Equals && inLet) {
			inLet = false;
			continue;
		} else if (token.type == TokenType::Let) {
			ops.push_back({token.text, token.type});
			inLet = true;
		} else if (token.type == TokenType::LeftParen) {
			ops.push_back({token.text, token.type});
		} else if (token.type == TokenType::RightParen) {
			while (!ops.empty() && ops.back().op != "(") {
				AddNode(ops.back(),astOutput);
				ops.pop_back();
			}
			if (!ops.empty() && ops.back().op == "(") {
				ops.pop_back();
			}
		} else if (IsOperator(token.text)) {
			// if there is an error here look in the map in the Parser.h file and look if all operators are present
			std::string op = token.text;

			if (op == "+" || op == "-") {
				if (expectingOperand) {
					op = "u" + op; // e.g. u+ or u-
				}
			}

			while (ops.size() > 0 && IsOperator(ops.back().op) && operators.at(ops.back().op) >= operators.at(op)) {
				AddNode(ops.back(), astOutput);
				ops.pop_back();
			}
			ops.push_back({op, token.type});
			expectingOperand = true;
		}
	}
	if (inLet) {
		throw std::runtime_error("Expected '=' in let statement");
	}
	while (!ops.empty()) {
		AddNode(ops.back(), astOutput);
		ops.pop_back();
	}
	if (!astOutput.empty()) {
		output.push_back(astOutput[0]);
	}

	return output;
}

bool Parser::IsOperator(std::string type) {
	for (auto &[oper, value] : operators) {
		if (type == oper) {
			return true;
		}
	}
	return false;
}

void Parser::AddNode(Opperator op, std::vector<Node*> &astOutput) {
	if (op.op == "u+" || op.op == "u-" || op.op == "!") {
		if (astOutput.size() < 1)
			throw std::runtime_error("Invalid unary expression");

		Node* operand = astOutput.back();
		astOutput.pop_back();

		astOutput.push_back(new Node(op.op, operand, nullptr, op.type));
		return;
	}

	if (astOutput.size() < 2) {
		throw std::runtime_error("Invalid expression");
	}
	Node *right = astOutput.back();
	astOutput.pop_back();
	Node *left = astOutput.back();
	astOutput.pop_back();

	astOutput.push_back(new Node(op.op, left, right, op.type));
}

void Parser::DeleteNode(Node *node) {
	if (node == nullptr) return;
	DeleteNode(node->left);
	DeleteNode(node->right);
	delete node;
}

void Parser::PrintNode(Node *node) {
	if (node == nullptr) return;
	PrintNode(node->left);
	PrintNode(node->right);
	std::cout << *node;
}
