#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <queue>

using namespace std;

struct Node
{
	int key;		// Key
	int h;			// Height attribute. EVERY function must leave h correct for all nodes.
	Node* left;		// Left child
	Node* right;	// Right child
	Node* p;		// Parent

	Node(int k)		// Constructor to create a new node with key k
	{
		key = k;
		clear();
	}

	void clear()    // Reset attributes, detach from any tree
	{
		left = right = p = NULL;
		h = -1;
	}
};

struct Tree
{
	Node* root;		// Root node. Has NIL parent
	Node* NIL;		// NIL is the sentinel

	Tree()			// Constructor to create an empty tree, with a newly allocated NIL node
	{
		NIL = new Node(0);	// Create a fresh node and designate it to be the NIL sentinel.
		root = NIL;
		root->p = root->left = root->right = NIL;
	}
	
	// Helper functions (you may wish to add more)
	void LEFT_ROTATE(Node*);		// Rotate left
	void RIGHT_ROTATE(Node*);		// Rotate right
	void HEIGHT_UP(Node*);			// Helper function for ROTATE. We assume some leaf has been adjusted, and the heights have been recalculated up to x. We must correct the height of x and all its ancestors.
	void BALANCE_UP(Node*);			// Given a node with correct height and balance, walk up to the root, correcting the height and performing BALANCE at each level.
	Node* GET_MIN(Node*);			// Find minimum (left most) vertex in non-empty subtree
	void REPLACE(Node*, Node*, bool);	
	
	// Assignment functions
	void BALANCE(Node*);			// Balance subtree after insert or delete
	void AVL_INSERT(Node*, Node*);	// Insert node into subtree. If tree is empty, give NIL as first parameter.
	void AVL_DELETE(Node*, Node*);	// Delete node from tree
	Node* SEARCH(Node*, int);		// Search for a key in subtree, return node if found, else NIL

	// IO functions
	string toString();				// Produces a string, representing your tree in the array format described in (d)
	void write(ofstream&, Node*);	// Writes tree in special DOT format required for use of GraphViz
	void display(string);			// Produce image of tree, using Graphviz (need to install it first)

	// Overloaded public functions
	Node* SEARCH(int k)
	{
		return SEARCH(root, k);
	}
	void AVL_INSERT(int k) 
	{
		Node* z = new Node(k);
		AVL_INSERT(root, z);
	}
	void AVL_DELETE(int k)
	{
		Node* z = SEARCH(root, k);
		if(z != NIL)
			AVL_DELETE(root, z);
	}
};

Node* Tree::SEARCH(Node* x, int k)
{
	if(x == NIL)
		return NIL; // k not in tree
	if(x->key == k)
		return x;
	else if(k < x->key)
		return SEARCH(x->left, k);
	else
		return SEARCH(x->right, k);
}

void Tree::HEIGHT_UP(Node* x) // corrects height upwards
{
	if(x != NIL) // haven't gone past the root yet
	{
		if(x->h == max(x->left->h, x->right->h)+1)
			return; // x is already correct. Given the context of this function, we can safely assume then that all ancestors are correct. So don't waste extra time.
		x->h = max(x->left->h, x->right->h)+1;
		HEIGHT_UP(x->p);
	}
}

void Tree::BALANCE_UP(Node* x) // performs balance on each ancestor, upwards
{
	if(x != NIL) // haven't gone past the root yet
	{
		Node* parent = x->p;
		BALANCE(x); // rebalance subtree rooted at x
		BALANCE_UP(parent); // continue up the ladder
	}
}

void Tree::LEFT_ROTATE(Node* x)
{
	Node* y = x->right;
	x->right = y->left;
	if(y->left != NIL)
		y->left->p = x;
	y->p = x->p;
	if(x->p == NIL)
		root = y;
	else if(x == x->p->left)
		x->p->left = y;
	else
		x->p->right = y;
	y->left = x;
	x->p = y;
	// But because we moved x and y, the h attribute for x and y might now be wrong! Let's recalculate them to make sure
	HEIGHT_UP(x);
}

void Tree::RIGHT_ROTATE(Node* x)
{
	Node* y = x->left;
	x->left = y->right;
	if(y->right != NIL)
		y->right->p = x;
	y->p = x->p;
	if(x->p == NIL)
		root = y;
	else if(x == x->p->left)
		x->p->left = y;
	else
		x->p->right = y;
	y->right = x;
	x->p = y;
	// But because we moved x and y, the h attribute for x, y and ancestors might now be wrong! Let's recalculate them to make sure
	HEIGHT_UP(x);
}

Node* Tree::GET_MIN(Node* x)
{
	if(x->left == NIL)
		return x;
	else
		return GET_MIN(x->left); // recurse down to the left until there is no left child
}

void Tree::REPLACE(Node* x, Node* z, bool keepkids) // set keepkids to true to let z keep its children
{
	if(z != NIL)
		z->p = x->p;

	if(!keepkids) 
	{
		// copy attributes from x to z
		z->left = x->left;
		z->right = x->right;
		z->h = x->h;
	
		// give x's children to z
		if(x->left != NIL)
			x->left->p = z;
		if(x->right != NIL)
			x->right->p = z;
	}

	// direct x's parent to z
	if(x->p == NIL) // x is root
		root = z;
	else // x has a parent
	{
		if(x->p->right == x)	// x is a right child
			x->p->right = z;
		else				// x is a left child
			x->p->left = z;
		if(keepkids)
			HEIGHT_UP(x->p);	// recalculate heights above z
	}

	// clear x's attributes
	x->clear();
}

void Tree::BALANCE(Node* x)
{
	if(x->left->h > x->right->h + 1)
	{
		if(x->left->right->h > x->left->left->h)
			LEFT_ROTATE(x->left);
		RIGHT_ROTATE(x);
	}
	if(x->right->h > x->left->h + 1)
	{
		if(x->right->left->h > x->right->right->h)
			RIGHT_ROTATE(x->right);
		LEFT_ROTATE(x);
	}
	HEIGHT_UP(x);
}

void Tree::AVL_INSERT(Node* x, Node* z)
{
	if(root == NIL)
	{
		root = z;
		z->p = NIL;
		z->left = NIL;
		z->right = NIL;
		z->h = 0;
		return;
	}
	if(z->key < x->key) // insert on the left branch
	{
		if(x->left == NIL) // found an empty space
		{
			x->left = z;
			z->left = NIL;
			z->right = NIL;
			z->h = 0;
			z->p = x;
		}
		else
			AVL_INSERT(x->left, z); // recurse on left branch
	}
	else // insert on the right branch
	{
		if(x->right == NIL) // found an empty space
		{
			x->right = z;
			z->left = NIL;
			z->right = NIL;
			z->h = 0;
			z->p = x;
		}
		else
			AVL_INSERT(x->right, z); // recurse on right branch
	}
	
	x->h = max(x->left->h, x->right->h) + 1; // correct height
	BALANCE(x); // balance at this ancestor
}

void Tree::AVL_DELETE(Node* x, Node* z)
{
	if(z->left == NIL && z->right == NIL) // z is a leaf
	{
		Node* parent = z->p;
		REPLACE(z, NIL, true); // replace z with NIL
		BALANCE_UP(parent);
	}
	else if(z->left == NIL)	// z has no left child
	{
		Node* parent = z->p;
		REPLACE(z, z->right, true); // replace z with its child subtree
		BALANCE_UP(parent);
	}
	else if(z->right == NIL) // z has no right child
	{
		Node* parent = z->p;
		REPLACE(z, z->left, true); // replace z with its child subtree
		BALANCE_UP(parent);
	}
	else
	{
		Node* successor = GET_MIN(z->right); // find successor in right branch
		Node* dummy = new Node(successor->key); // make a clone of the successor
		//dummy->data = successor->data; // copy data to clone (only if you are using data)
		REPLACE(successor, dummy, false); // replace successor with clone
		REPLACE(z, successor, false); // replace z with successor
		AVL_DELETE(x, dummy); // finally delete the clone
	}
}


string Tree::toString()
{
	// Your code here
	// An empty tree should just return the string "NIL"
	if (root == NIL)
		return "NIL";
	int size = (1 << (root->h + 1))-1;
	string result;
	queue<Node*> q;
	q.push(root);

	// Expand nodes in tree, row by row, including NIL nodes
	for(int i = 0; i < size; i++)
	{
		Node* node = q.front();
		q.pop();
		if(node == NIL)
			result += "NIL ";
		else
			result += (to_string(node->key) + " ");
		q.push(node->left);
		q.push(node->right);
	}

	// Delete extra whitespace char at end of string
	return result.substr(0, result.length()-1);
};

void Tree::write(ofstream &filestream, Node* x)
{
	// Write the tree in DOT language to filestream, required for Graphviz
	if(x != NIL)
	{
		if(x->left != NIL)
			filestream << x->key << " -> " << x->left->key << '\n';
		else
			filestream << "null" << x->key << "l [shape=point];\n" << x->key << " -> " << "null" << x->key << "l\n";
		if(x->right != NIL)
			filestream << x->key << " -> " << x->right->key << '\n';
		else
			filestream << "null" << x->key << "r [shape=point];\n" << x->key << " -> " << "null" << x->key << "r\n";
		write(filestream, x->left);
		write(filestream, x->right);
	}
}

void Tree::display(string filename = "tree")
{
	// Creates a PNG diagram of the tree
	return; // install Graphviz on your machine from http://www.graphviz.org/, then comment out this line

	ofstream myfile(filename + ".txt");
	if (myfile.is_open())
	{
		myfile << "digraph G {\n";
		myfile << "graph [ordering=\"out\"]\n";
		write(myfile, root);
		myfile << "}\n";
		myfile.close();
	}
	else
		cout << "Unable to open file";
	string s1("\"\"C:\\Program Files (x86)\\Graphviz2.38\\bin\\dot.exe\" -Tpng -o \"C:\\Users\\Mencella\\Documents\\DS Assignment 2\\AVL_tree\\AVL_tree\\"); // change these filepaths to match your graphviz installation location, and your C++ project location
	string s2(".png\" ");
	string s3(".txt\"");
	string args = s1 + filename + s2 + filename + s3;
	system(args.c_str()); 
}


bool IS_AVL(Node* x)
{
	if(x->h == -1) // i.e. x is NIL
		return true;
	if(abs(x->left->h - x->right->h) <= 1 && IS_AVL(x->left) && IS_AVL(x->right))
		return true;
	return false;
}

int main()
{
	Tree* T = new Tree();

	ifstream input("input.txt", ios::in);
	ofstream output("output.txt", ios::out);
	string line;

	while(getline(input, line)) 
	{
		// Extract the pair (command, key) from the line. E.g. "I 7"
		istringstream iss(line);
		string command;
		iss >> command;
		int key;
		iss >> key;

		// Carry out Insert or Delete command
		if(command == "I")
			T->AVL_INSERT(key);
		else if(command == "D")
			T->AVL_DELETE(key);

		// Verify AVL property
		if(!IS_AVL(T->root))
		{
			cout << "Error. Tree is not AVL!\n";
			cout << "Program terminated due to error. Press enter to exit...";
			cin.get();
			return 0;
		}

		// Convert tree to array format, and write to output
		output << T->toString() << endl;
	}
	// Close files
	input.close();
	output.close();

	// Display tree as png file (only if GraphViz is installed)
	T->display();

	cout << "Program terminated. Press enter to exit...";
	cin.get();
	return 0;
}
