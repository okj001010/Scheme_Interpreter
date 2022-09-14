#include <iostream>
#include <string>
#define NODE_LENGTH 100
#define HASH_LENGTH 100
#define TOKEN_LENGTH 100
using namespace std;

// use stack to save data of parameter in function
template <class T>
class Stack {
private:
	int top;
	T* stack;
	int capacity;
public:
	Stack(int stackCapacity = 10);
	bool IsEmpty() const;
	T& Top() const;
	void Push(const T& item);
	void Pop();
};

template <class T>
Stack<T>::Stack(int stackCapacity) : capacity(stackCapacity) {
	if (capacity < 1) throw "Stack capacity must be > 0";
	stack = new T[capacity];
	top = -1;
}

// if top is -1, it means stack is empty
template <class T>
bool Stack<T>::IsEmpty() const { return top == -1; }

template <class T>
T& Stack<T>::Top() const {
	if (IsEmpty()) throw "Stack is empty";
	return stack[top];
}

template <class T>
void Stack<T>::Push(const T& x) {
	if (top == capacity - 1) { // double the capacity
		T* temp = new T[2 * capacity];
		for (int i = 0; i < capacity; i++)
			temp[i] = stack[i];
		delete[] stack;
		stack = temp;
		capacity *= 2;
	}
	stack[++top] = x; // add 1 to top and save data
}

template <class T>
void Stack<T>::Pop() {
	if (IsEmpty()) throw "Stack is empty. Cannot delete.";
	stack[top--].~T();
}


// Hash Class is a node of Hash Table
// it contains symbol and link
// in GetHashValue function, it control the symbol of hash
// so symbol member variable need getter and setter
class Hash {
private:
	string symbol = "";
	int link = NULL;
public:
	string getSymbol() { return symbol; }
	void setSymbol(string str) { symbol = str; }
	int getLink() { return link; }
	void setLink(int pointer) { link = pointer; }
};

// Node Class is a node of Memory
// it contains lchild and rchild
// in Read function, it control lchild, rchild of every node
// so lchild and rchild need getter and setter
class Node {
private:
	int lchild = NULL, rchild = NULL;
public:
	int getLChild() { return lchild; }
	int getRChild() { return rchild; }
	void setLChild(int num) { lchild = num; }
	void setRChild(int num) { rchild = num; }
};

class Scheme {
private:
	Node Memory[NODE_LENGTH + 1];
	bool Memory_Used[NODE_LENGTH + 1] = {}; // array that indicate whether the node is used or not
	bool Current_Input[NODE_LENGTH + 1] = {}; // array that indicate the current input node
	Hash Hash_Table[HASH_LENGTH + 1];
	bool Variable_Defined[HASH_LENGTH + 1] = {}; // array that indicate whether the variable is defined or not
	string Tokens[TOKEN_LENGTH + 1]; // save every token of input
	int free_root; // root of free list
	int token_terms; // size of Tokens (use in InitializeTokenizer func)
	int current_token; // index of current token (use in GetNextToken, PutBack func)
	int current_node; // index of current node (use in Alloc func)
public:
	Scheme();
	string GetCommand();
	void InitializeTokenizer(string command);
	string Preprocessing();
	int Read();
	string GetNextToken() { return Tokens[current_token++]; }
	int GetHashValue(string str);
	int Alloc();
	void PutBack() { current_token--; }
	void PrintResult(int result);
	void Print(int result, bool tf);
	void Print_Memory_Hash(int root); // print Memory, Memory_Used, Hash_Table -> need in debugging
	float GetVal(int hash);
	bool isNumber(int hash);
	int Eval(int root);
	int CheckStructure(int left, int right);
	void Garbage_Collection(int root);
};

// initialize rchild of every node to next node index
// put simple arithmetic operation into Hash_Table
Scheme::Scheme() : free_root(1), token_terms(0), current_token(0), current_node(0) {
	for (int i = 1; i <= TOKEN_LENGTH; i++)
		Tokens[i] = "";
	for (int i = 1; i <= NODE_LENGTH; i++)
		Memory_Used[i] = false; // every node is not used at first
}

// initialize every variable to get another input
// if a letter is alphabet but upper case letter,
// then transform to lower case letter
string Scheme::GetCommand() {
	string command;
	token_terms = 0;
	current_token = 0;
	current_node = free_root - 1;
	for (int i = 1; i <= NODE_LENGTH; i++)
		Current_Input[i] = false;
	for (int i = 1; i <= TOKEN_LENGTH; i++)
		Tokens[i] = "";

	cout << "> ";
	getline(cin, command);
	for (unsigned int i = 0; i < command.length(); i++) {
		if (isalpha(command[i]))
			command[i] = tolower(command[i]);
	}
	return command;
}

// from command, split every token and save into Tokens
void Scheme::InitializeTokenizer(string command) {
	// token starts from 'start variable' and ends in 'end variable'
	int start = 0, end = 0;
	token_terms = 0;
	current_token = 0;
	while (true) {
		if (isspace(command[end])) {
			// if letter is ' ', save token from start to end
			Tokens[token_terms++] = command.substr(start, end - start);
			end++;
			// there could be many space, so skip if there is another space
			while (isspace(command[end]))
				end++;
			start = end;
		}
		else if (command[end] == '(' || command[end] == ')' || command[end] == '\'') {
			// if letter is '(' or ')' or '\'', save token from start to end
			// and also save token '(' or ')' or '\''
			if (start != end)
				Tokens[token_terms++] = command.substr(start, end - start);
			Tokens[token_terms++] = command.substr(end, 1);
			end++;
			// there could be many space, so skip if there is another space
			while (isspace(command[end]))
				end++;
			start = end;
		}
		else if (command[end] == NULL) {
			// end of string
			Tokens[token_terms++] = command.substr(start, end - start);
			break;
		}
		else end++;
	}
}

// add lambda and quote appropriately
string Scheme::Preprocessing() {
	string newcommand = "";
	string token = GetNextToken();
	while (token != "") {
		if (token == "define") {
			newcommand += "define ";
			token = GetNextToken();
			if (token == "(") { // it means it's user define function
				token = GetNextToken();
				newcommand += token + "(lambda(" + Preprocessing() + ")";
			}
			else newcommand += token + " "; // it means it's declaration of variable
		}
		else if (token == "'") {
			newcommand += "(quote ";
			int number_of_left_paren = 0;
			do { // read all data in the parenthesis set
				token = GetNextToken();
				newcommand += " " + token;
				if (token == "(")
					number_of_left_paren++;
				else if (token == ")")
					number_of_left_paren--;
			} while (number_of_left_paren > 0);
			newcommand += ")";
		}
		else newcommand += token + " ";
		token = GetNextToken();
	}
	return newcommand;
}


// return the Hash Value of str if it is in Hash_Table
// put new str in Hash_Table and return if it is not in Hash_Table
// finish the source code if Hash_Table is full
int Scheme::GetHashValue(string str) {
	// Hash Function: sum of ASCII code of str[i] and get remainder + 1
	int ASCII_sum = 0;

	for (unsigned int i = 0; i < str.length(); i++)
		ASCII_sum += str[i];
	ASCII_sum = ASCII_sum % HASH_LENGTH + 1;

	int index = ASCII_sum;
	do {
		// if Hash_Table[index] is empty, it means the str is not in Hash_Table
		// so put str into Hash_Table and return
		if (Hash_Table[index].getSymbol() == "") {
			Hash_Table[index].setSymbol(str);
			return -index;
		}
		// if Hash_Table[index] is str, it means str is already in Hash_Table
		// so just return the index
		else if (Hash_Table[index].getSymbol() == str)
			return -index;

		// search for next index (if index goes over 30 then go back to 1)
		if (index == HASH_LENGTH)
			index = 1;
		else index++;
	} while (index != ASCII_sum);
	// if index == ASCII_sum, it means it searched for every Hash_Table
	// but it couldn't find str or empty space
	// therefore it means Hash_Table is full
	cout << endl << "Hash Table is FULL!" << endl;
	Print_Memory_Hash(0);
	exit(1);
}

int Scheme::Alloc() {
	// current_node will be the node that is not used
	for (int i = 1; i <= NODE_LENGTH; i++) {
		if (!Memory_Used[i]) {
			current_node = i;
			Memory_Used[i] = true;
			Current_Input[i] = true;
			return current_node;
		}
	}

	cout << "****** Garbage Collection *****" << endl;
	// if current_node is not returned in upper for loop, it means node is full and need garbage collecion
	// every Memory_Used is false except the current input
	// some node should not be removed so we will call Garbage_Collecion function to make some Memory_Used true
	for (int i = 1; i <= NODE_LENGTH; i++) {
		if (!Current_Input[i])
			Memory_Used[i] = false;
	}
	// if link of hash is positive, it means funcion defined
	// so we should save the tree that of root is lambda
	for (int i = 1; i <= HASH_LENGTH; i++) {
		if (Hash_Table[i].getLink() > 0)
			Garbage_Collection(Hash_Table[i].getLink());
	}
	// set unused node to zero 
	for (int i = 1; i <= NODE_LENGTH; i++) {
		if (!Memory_Used[i]) {
			Memory[i].setLChild(0);
			Memory[i].setRChild(0);
		}
	}

	// now nodes are garbage collected so return current_node like line 266
	for (int i = 1; i <= NODE_LENGTH; i++) {
		if (!Memory_Used[i] && i != current_node) {
			current_node = i;
			Memory_Used[i] = true;
			Current_Input[i] = true;
			return current_node;
		}
	}

	// if current_node is not returned despite the garbage collection,
	// it means the node is really full, so exit with error message
	cout << "NODE is FULL!" << endl;
	exit(1);
}

// if lchild or rchild is pointing another node,
// call Garbage_Collection function with parameter lchild or rchild recursively
// then the Memory_Used of whole node in the tree will be true
void Scheme::Garbage_Collection(int root) {
	Memory_Used[root] = true;
	if (Memory[root].getLChild() > 0)
		Garbage_Collection(Memory[root].getLChild());
	if (Memory[root].getRChild() > 0)
		Garbage_Collection(Memory[root].getRChild());
}

// read Tokens and make parse_tree recursively
int Scheme::Read() {
	int root = 0, temp = 0;
	bool first = true;
	int token_hash_value = GetHashValue(GetNextToken());
	if (token_hash_value == GetHashValue("(")) {
		// if left parenthesis start, read until right parenthesis
		while (true) {
			token_hash_value = GetHashValue(GetNextToken());
			if (token_hash_value == GetHashValue(")"))
				break;

			if (first) {
				// if first is true, then it means it is root
				root = temp = Alloc();
				first = false;
			}
			else {
				Memory[temp].setRChild(Alloc());
				temp = Memory[temp].getRChild();
			}
			if (token_hash_value == GetHashValue("(")) {
				// PutBack so that when Read function is recursively called, it can go into while loop
				PutBack();
				Memory[temp].setLChild(Read());
				// after finishing the recursive Read function LChild will be root
			}
			else Memory[temp].setLChild(token_hash_value);

			if (!first)
				Memory[temp].setRChild(NULL);
		}
		return root;
	}
	else return token_hash_value;
}

// final printing function
void Scheme::PrintResult(int result) {
	free_root = current_node + 1; // next command's node data will go into right after the last used node
	if (result != 0) {
		cout << "] ";
		Print(result, true);
		cout << endl << endl;
	}
	else
		cout << endl; // print nothing if result is zero
}

void Scheme::Print(int root, bool startList) {
	if (root == NULL)
		cout << "() ";
	else if (root < 0)
		cout << Hash_Table[-root].getSymbol() << " ";
	else if (root > 0) {
		if (startList)
			cout << "(";
		// at first, go to deepest LChild recursively
		// then print LChild
		Print(Memory[root].getLChild(), true);
		if (Memory[root].getRChild() != NULL)
			// then print RChild and go out recursive function one level
			Print(Memory[root].getRChild(), false);
		else cout << '\b' << ")" << " ";
	}
}

void Scheme::Print_Memory_Hash(int root) {
	// node 1 from current node is used, so free_root will be current_node + 1
	cout << "] FreeList's root: " << free_root << endl;
	cout << "List's root: " << root << endl << endl;
	// print every node
	cout << "Memory table =" << endl;
	for (int i = 1; i <= NODE_LENGTH; i++)
		cout << "Node " << i << ": " << Memory[i].getLChild() << ", " << Memory[i].getRChild() << ", Used: " << Memory_Used[i] << endl;
	cout << endl;
	// print nonempty hash_table
	cout << "Hash table = " << endl;
	for (int i = 1; i <= HASH_LENGTH; i++) {
		if (Hash_Table[i].getSymbol() != "")
			cout << "Hash value " << -i << ": " << Hash_Table[i].getSymbol() << ", " << Hash_Table[i].getLink() << endl;
	}
	cout << endl;
}

// get value in hash table
float Scheme::GetVal(int hash) {
	return stof(Hash_Table[-hash].getSymbol());
}

// if s is number string, atoi will return the number so can check whether it's number or not
// if s is 0 string, it is zero number but atoi will return 0
// so use || and check for 0 string
bool Scheme::isNumber(int hash) {
	string s = Hash_Table[-hash].getSymbol();
	return atoi(s.c_str()) != 0 || s.compare("0") == 0;
}

// left, right is eval of two inputs of "equal?"
// if left and right is both positive, it means both are pointing another node
// so check structure of both lchild and rchild of left and right recursively
// if left and right are not both positive, it is only true when left equals right
int Scheme::CheckStructure(int left, int right) {
	if (left > 0 && right > 0) {
		if (CheckStructure(Memory[left].getLChild(), Memory[right].getLChild()) == GetHashValue("#t") &&
			CheckStructure(Memory[left].getRChild(), Memory[right].getRChild()) == GetHashValue("#t"))
			return GetHashValue("#t");
		else return GetHashValue("#f");
	}
	else {
		if (left == right) return GetHashValue("#t");
		else return GetHashValue("#f");
	}
}

int Scheme::Eval(int root) {
	int token_index = 0;
	// if root is negative, it means it's in hash table
	// if the data's link in hash table is not zero, it means it has linked value so return that
	// if the data's link in hash is zero, it's not defined variable so just return root
	if (root < 0) {
		if (Hash_Table[-root].getLink()) return Hash_Table[-root].getLink();
		else {
			if (isNumber(root))
				return root;
			else if(!Variable_Defined[-root]) {
				cout << "Undefined Variable Used: " << Hash_Table[-root].getSymbol() << endl;
				exit(1);
			}
			else return 0;
		}
	}

	// if root is positive, it has children
	// so check lchild and proceed with contextual work
	else if (root > 0) {
		token_index = Memory[root].getLChild();

		// fundameental operator will operate value of rchild's lchild and rchild's rchild
		if (token_index == GetHashValue("+"))
			return GetHashValue(to_string(GetVal(Eval(Memory[Memory[root].getRChild()].getLChild())) + GetVal(Eval(Memory[Memory[Memory[root].getRChild()].getRChild()].getLChild()))));
		else if (token_index == GetHashValue("-"))
			return GetHashValue(to_string(GetVal(Eval(Memory[Memory[root].getRChild()].getLChild())) - GetVal(Eval(Memory[Memory[Memory[root].getRChild()].getRChild()].getLChild()))));
		else if (token_index == GetHashValue("*"))
			return GetHashValue(to_string(GetVal(Eval(Memory[Memory[root].getRChild()].getLChild())) * GetVal(Eval(Memory[Memory[Memory[root].getRChild()].getRChild()].getLChild()))));
		else if (token_index == GetHashValue("=")) {
			if (GetVal(Eval(Memory[Memory[root].getRChild()].getLChild())) == GetVal(Eval(Memory[Memory[Memory[root].getRChild()].getRChild()].getLChild())))
				return GetHashValue("#t");
			else return GetHashValue("#f");
		}
		else if (token_index == GetHashValue(">")) {
			if (GetVal(Eval(Memory[Memory[root].getRChild()].getLChild())) > GetVal(Eval(Memory[Memory[Memory[root].getRChild()].getRChild()].getLChild())))
				return GetHashValue("#t");
			else return GetHashValue("#f");
		}
		else if (token_index == GetHashValue("<")) {
			if (GetVal(Eval(Memory[Memory[root].getRChild()].getLChild())) < GetVal(Eval(Memory[Memory[Memory[root].getRChild()].getRChild()].getLChild())))
				return GetHashValue("#t");
			else return GetHashValue("#f");
		}

		// if eval of two inputs are same, return true
		else if (token_index == GetHashValue("eq?")) {
			if (Eval(Memory[Memory[root].getRChild()].getLChild()) == Eval(Memory[Memory[Memory[root].getRChild()].getRChild()].getLChild()))
				return GetHashValue("#t");
			else return GetHashValue("#f");
		}

		// call CheckStructure function with two parameter (eval of two inputs)
		// CheckStructure function will check whether the structure is same or not recursively
		else if (token_index == GetHashValue("equal?")) {
			return CheckStructure(Eval(Memory[Memory[root].getRChild()].getLChild()), Eval(Memory[Memory[Memory[root].getRChild()].getRChild()].getLChild()));
		}

		// check with isNumber function
		else if (token_index == GetHashValue("number?")) {
			if (isNumber(Eval(Memory[Memory[root].getRChild()].getLChild())))
				return GetHashValue("#t");
			else return GetHashValue("#f");
		}
		// if eval value is not zero and not number, it's symbol
		else if (token_index == GetHashValue("symbol?")) {
			int result = Eval(Memory[Memory[root].getRChild()].getLChild());
			if (result < 0 && !isNumber(result))
				return GetHashValue("#t");
			else return GetHashValue("#f");
		}
		// if rchild or eval of rchild is zero, it's null
		else if (token_index == GetHashValue("null?")) {
			if (!Memory[root].getRChild() || !Eval(Memory[root].getRChild()))
				return GetHashValue("#t");
			else return GetHashValue("#f");
		}

		// alloc newmemory and set lchild, rchild
		else if (token_index == GetHashValue("cons")) {
			int newmemory = Alloc();
			Memory[newmemory].setLChild(Eval(Memory[Memory[root].getRChild()].getLChild()));
			Memory[newmemory].setRChild(Eval(Memory[Memory[Memory[root].getRChild()].getRChild()].getLChild()));
			return newmemory;
		}

		else if (token_index == GetHashValue("cond")) {
			while (Memory[Memory[root].getRChild()].getRChild()) { // checking every condition before 'else'
				root = Memory[root].getRChild();
				if (Eval(Memory[Memory[root].getLChild()].getLChild()) == GetHashValue("#t")) // if condition is true, proceed the expression
					return Eval(Memory[Memory[root].getLChild()].getRChild());
			}
			if (Memory[Memory[Memory[root].getRChild()].getLChild()].getLChild() != GetHashValue("else")) { // it means there's no 'else' in cond statement so error
				cout << endl << "You must write ELSE!" << endl;
				exit(1);
			}
			return Eval(Memory[Memory[Memory[Memory[root].getRChild()].getLChild()].getRChild()].getLChild());
		}

		else if (token_index == GetHashValue("car"))
			return Memory[Eval(Memory[Memory[root].getRChild()].getLChild())].getLChild();
		else if (token_index == GetHashValue("cdr"))
			return Memory[Eval(Memory[Memory[root].getRChild()].getLChild())].getRChild();

		// need to set link in hash table appropriately
		else if (token_index == GetHashValue("define")) {
			if (Memory[Memory[Memory[Memory[root].getRChild()].getRChild()].getLChild()].getLChild() == GetHashValue("lambda"))
				Hash_Table[-Memory[Memory[root].getRChild()].getLChild()].setLink(Memory[Memory[Memory[root].getRChild()].getRChild()].getLChild());
			else {
				Hash_Table[-Memory[Memory[root].getRChild()].getLChild()].setLink(Eval(Memory[Memory[Memory[root].getRChild()].getRChild()].getLChild()));
				Variable_Defined[-Memory[Memory[root].getRChild()].getLChild()] = true;
			}
			return 0;
		}

		// token_index is in hash table and it has linked value
		// it means it's user defined function or user defined variable
		else if (token_index < 0 && Hash_Table[-token_index].getLink() != 0) {
			if (Memory[Hash_Table[-token_index].getLink()].getLChild() == GetHashValue("lambda")) { // user defined function
				// we should put data into parameter's of function and put original data into stack
				// param_root is node of parameter's of function
				// current_root is node of data which will be save into parameter
				Stack<int> hash;
				Stack<int> link;
				Stack<bool> predefined;
				int param_root = Memory[Memory[Hash_Table[-token_index].getLink()].getRChild()].getLChild();
				int current_root = root;
				while (1) {
					// push original data into stack
					hash.Push(Memory[param_root].getLChild());
					link.Push(Hash_Table[-Memory[param_root].getLChild()].getLink());
					predefined.Push(Variable_Defined[-Memory[param_root].getLChild()]);
					// change parameter into input data
					Hash_Table[-Memory[param_root].getLChild()].setLink(Eval(Memory[current_root].getRChild()));
					// variable should be temporarily defined
					Variable_Defined[-Memory[param_root].getLChild()] = true;
					// if there is no more parameter, break
					// but if there is more parameter, param_root and current_root will be rchild of itself and iterate while loop once again
					if (!Memory[param_root].getRChild())
						break;
					param_root = Memory[param_root].getRChild();
					current_root = Memory[current_root].getRChild();
				}

				int result = Eval(Memory[Memory[Memory[Hash_Table[-Memory[root].getLChild()].getLink()].getRChild()].getRChild()].getLChild());
				// return the parameter data into original data
				// return whether the variable was defined or not into original state
				while (!hash.IsEmpty()) {
					Hash_Table[-hash.Top()].setLink(link.Top());
					Variable_Defined[-hash.Top()] = predefined.Top();
					hash.Pop();
					link.Pop();
					predefined.Pop();
				}
				return result;
			}
			else return Hash_Table[-token_index].getLink(); // user defined variable, list
		}
		else if (token_index == GetHashValue("quote"))
			return Memory[Memory[root].getRChild()].getLChild();
		else Eval(token_index); // it means lchild is pointing another node
	}
	else return 0; // root is zero so return 0
}

int main() {
	Scheme scheme;
	while (true) {
		string command = scheme.GetCommand();
		scheme.InitializeTokenizer(command);
		string newcommand = scheme.Preprocessing();
		scheme.InitializeTokenizer(newcommand);

		int root = scheme.Read();
		int result = scheme.Eval(root);
		scheme.PrintResult(result);
	}
}