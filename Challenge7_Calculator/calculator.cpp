#include <iostream>
#include <vector>
#include <string>
#include <unordered_set>
#include <stdexcept>

std::unordered_set<char> operations;
enum opTypes { EMPTY, PLUS, MINUS, MULT, DIV, NUM, OPEN_PAREN, CLOSING_PAREN };

opTypes charToOp(char c) {
	if (c == '+')
		return PLUS;
	if (c == '-')
		return MINUS;
	if (c == '*')
		return MULT;
	if (c == '/')
		return DIV;
}

bool inOperations(char c){
	return operations.find(c) != operations.end();
}

bool inOperations(opTypes op) {
	return (op == MINUS || op == MULT || op == PLUS || op == DIV);
}

bool isDigit(char c) {
	return (c >= '0' && c <= '9');
}

std::string makeNum(int &i, std::string expression) {
	std::string num = "";
	bool decimalPlace = false;
	while (i < expression.size()) {
		char c = expression[i];
		if (isDigit(c)) {
			num += c;
		}
		else if (c == '.') {
			if (!decimalPlace) {
				decimalPlace = true;
				num += c;
			} else {
				throw std::invalid_argument("There is 2 decimal places\n");
				return 0;
			}
		} else if (c == ' ') {

			// end here
			// don't increment i
			break;
		}	
		else {
			throw std::invalid_argument("The random character cannot be part of a number!\n");
			return 0;
		}
		i++;
	}

	return num;
}

std::string performOp(std::string num1, std::string num2, char op) {
	float numb1 = stof(num1);
	float numb2 = stof(num2);
	float total = 0;
	if (op == '+') {
		total = numb2 + numb1;	
	} else if (op == '-') {
		total = numb2 - numb1;	
	} else if (op == '*') {
		total = numb2 * numb1;	
	} else if (op == '/') {
		total = numb2 / numb1;
	}
	printf("num1: %f, num2: %f inside performOp func\n", numb1, numb2);

	return std::to_string(total);
}

void eval(std::vector<std::string> &stack, std::vector<char>& opStack) {
	std::string num1;
	std::string num2;
	char op;
	printf("Called eval!\n");
	while (true) {
		
		if (stack.size() < 2) {
			break;
		}
		num1 = stack.back();
		stack.pop_back();
		printf("Num1 inside of eval %s\n", num1.c_str());
		if (num1 == "(") {
			break;
		}
		printf("Num1 break test %s\n", num1.c_str());

		num2 = stack.back();
		stack.pop_back();
		
		if (num2 == "(") {
			printf("Puch back num1 = %s\n", num1.c_str());
			stack.push_back(num1);
			break;
		}

		op = opStack.back();
		opStack.pop_back();

		if (!opStack.size()) {
			break;
		}

		printf("NUM1: %s, NUM2: %s\n", num1.c_str(), num2.c_str());
		std::string total = performOp(num1, num2, op);
		stack.push_back(total);
		printf("Stack size before operation %d\n", stack.size());
		printf("Total that will be added %s\n", total.c_str());
	}
	printf("Stack size after operation %d\n", stack.size());
}

int main() {
	std::vector<std::string> stack;
	std::vector<char> opStack;

	operations.insert('+');
	operations.insert('-');
	operations.insert('/');
	operations.insert('*');

	std::string expression;
	printf("Enter expression:\n");
	std::getline(std::cin, expression);
	opTypes prev = EMPTY;

	for (int i = 0; i < expression.size(); i++) {
		char c = expression[i];
		bool isOpenParen = (true ? c == '(' : false);
		bool digit = isDigit(c);
		int vars = 0;
		std::string number = "";
		if (digit) {
			try {
				number = makeNum(i, expression);
			} catch (const std::invalid_argument& e) {
				std::cout << (e.what());
			}

		}
		if (c == ' ') {
			continue;
		}

		if (stack.empty()) {
			if (isOpenParen) {
				stack.push_back(std::string(1, c));
				prev = OPEN_PAREN;
			} else if (inOperations(c))  {
				printf("Error! Operation was added before a number\n");
				return 0;
			} else if (digit) {
				stack.push_back(number);		
				prev = NUM;
				vars++;
			} else {
				printf("Random character is invalid character!\n");
				return 0;
			}
		} 
		else {
			if (inOperations(prev)) {
				if (inOperations(c)) {
					printf("Error! Operation was added directly after another operation!\n");
					return 0;
				}
				else if (isOpenParen) {
					stack.push_back(std::string(1, c));
					printf("Added %c\n", c);
					prev = OPEN_PAREN;
					continue;
				} else if (digit) {
					prev = NUM;
					stack.push_back(number);
					printf("Number %s added\n", number.c_str());
					if (!opStack.empty()) {
						printf("reached this point\n");
						if (opStack.back() == MULT || opStack.back() == DIV) {
							if (stack.size() < 2) {
								printf("Cannot do operation on less than 2 numbers!\n");
								return 0;
							}
							std::string num1 = stack.back();
							stack.pop_back();
							std::string num2 = stack.back();
							stack.pop_back();
							char op = opStack.back();
							opStack.pop_back();
							std::string output = performOp(num1, num2, op);
							printf("Output in mainloop %s\n", output.c_str());
							stack.push_back(output);
						}

					}
					continue;
				}
			} else if (prev == OPEN_PAREN) {
				if (inOperations(c)) {
					printf("Error! Cannot have operation directly after opening parenthesis!\n");
					return 0;
				} else if (digit) {
					stack.push_back(number);
					printf("Pushed back number\n");
					prev = NUM;
				}
				else if (c == ')') {
					stack.pop_back();
				}
			} else if (prev == NUM) {
				if (isDigit(c)) {
					printf("Error! Cannot have a number after another number!\n");
					return 0;
				} else if (inOperations(c)) {
					opStack.push_back(c);
					printf("Added op to op stack\n");
					prev = charToOp(c);
				} else if (c == ')') {	
					printf("Evaluating stack\n");
					eval(stack, opStack);
					prev = NUM;
				}
			}
			else if (c == ')') {
				printf("Evaluating stack\n");
				eval(stack, opStack);
				prev = charToOp(opStack.back());
			}
			else if (inOperations(c)) {
				opStack.push_back(c);
				prev = charToOp(c);
			}
		}

	}

		printf("STACK SIZE: %d\n", stack.size());
		for (int i = 0; i < stack.size(); i++) {
			printf("[%s] ", stack[i].c_str());
		}
}
