// AVL_Tree implementation
// Liam Mencel, November 2014
// Information sources: CS160 lecture slides 7-8, http://en.wikipedia.org/wiki/AVL_tree

#include <iostream>
#include <stdlib.h>
#include <algorithm>
#include <fstream>
#include <string>

using namespace std;

struct Node
{
	int key;		// Key. We will assume for simplicity that each node has a UNIQUE key.
	//int data;		// If you want to store data in your tree, uncomment this and pick a type.
	int h;			// Height attribute. EVERY function must leave h correct for all nodes.
	Node* left;		// Left child
	Node* right;	// Right child
	Node* p;		// Parent. Root has NIL parent

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

struct Tree // (5 marks for Tree and Node structures)
{
	Node* root;		// Root node
	Node* NIL;		// Not always necessary to make NIL a node rather than a NULL pointer, but here it will make things easier to allow NIL to have the h attribute.

	Tree()			// Constructor to create an empty tree, with a newly allocated NIL node
	{
		NIL = new Node(0);	// Create a fresh node and designate it to be the NIL node.
		root = NIL;
		root->p = root->left = root->right = NIL;
	}
	
	// Helper functions
	Node* SEARCH(Node*, int);			// Search for a key in subtree, return node if found, else NIL
	void LEFT_ROTATE(Node*);			// Rotate left
	void RIGHT_ROTATE(Node*);			// Rotate right
	void HEIGHT_UP(Node*);				// Helper function for ROTATE. We assume some leaf has been adjusted, and the heights have been recalculated up to x. We must correct the height of x and all its ancestors.
	void BALANCE_UP(Node*);				// Given a node with correct height and balance, walk up to the root, correcting the height and performing BALANCE at each level.
	Node* GET_MIN(Node*);				// Find minimum (left most) vertex in non-empty subtree
	void REPLACE(Node*, Node*, bool);	// Replace first node with second node, dislodging first node from the tree. bool is used to keep second node's children, if required.
	void write(ofstream&, Node*);		// Write tree to text file
	void print(string);					// Produce image of tree, using Graphviz
	
	// Assignment functions
	bool IS_AVL(Node*);				// Verify that subtree is an AVL tree (5 marks)
	void BALANCE(Node*);			// Balance subtree after insert or delete (5 marks)
	void AVL_INSERT(Node*, Node*);	// Insert node into subtree. If tree is empty, give NIL as first parameter. (10 marks)
	void AVL_DELETE(Node*);			// Delete node from tree (10 marks) * Note that question asked us to supply two parameters. In my implementation, the x parameter is redundant, so I omit it.

	// Overloaded functions - Same functions as above, but user-friendly
	bool IS_AVL() {return IS_AVL(root);}
	void AVL_INSERT(int k) 
	{
		Node* z = new Node(k);
		AVL_INSERT(root, z);
	}
	void AVL_DELETE(int k)
	{
		Node* z = SEARCH(root, k);
		if(z != NIL)
			AVL_DELETE(z);
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

void Tree::LEFT_ROTATE(Node* x) // from lectures
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

void Tree::RIGHT_ROTATE(Node* x) // from lectures
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

void Tree::write(ofstream &filestream, Node* x)
{
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

void Tree::print(string filename = "tree")
{
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

bool Tree::IS_AVL(Node* x) // from question 1
{
	if(x == NIL)
		return true;
	if(abs(x->left->h - x->right->h) <= 1 && IS_AVL(x->left) && IS_AVL(x->right))
		return true;
	return false;
}

void Tree::BALANCE(Node* x)	// from question 2
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
}

void Tree::AVL_INSERT(Node* x, Node* z) // insert z to x (recursively) and balance x
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

void Tree::AVL_DELETE(Node* z)
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
		AVL_DELETE(dummy); // finally delete the clone
	}
}


int main() // (5 marks for rest of program; main(), testing, etc)
{
	Tree* T;
	int i;
	string notavl = "Error. Tree is not AVL!\n";

	// Test 1 - Insert

	T = new Tree();
	string testname1 = "inserta";

	int keys1[10] = {20, 4, 26, 3, 9, 21, 30, 2, 7, 11};
	for(i=0; i<10; i++)
	{
		T->AVL_INSERT(keys1[i]);
		T->print(testname1 + to_string(i+1));
		if(!T->IS_AVL())
			cout << notavl;
	}

	T->AVL_INSERT(15);

	T->print(testname1 + to_string(i+1));
	if(!T->IS_AVL())
		cout << notavl;

	// Test 2 - Insert with balancing high up

	T = new Tree();
	string testname2 = "insertb";

	int keys2[9] = {3, 2, 6, 1, 5, 8, 4, 7, 9};
	for(i=0; i<9; i++)
	{
		T->AVL_INSERT(keys2[i]);
		T->print(testname2 + to_string(i+1));
		if(!T->IS_AVL())
			cout << notavl;
	}

	T->AVL_INSERT(10);

	T->print(testname2 + to_string(i+1));
	if(!T->IS_AVL())
		cout << notavl;

	// Test 3 - Delete

	T = new Tree();
	string testname3 = "deletea";

	int keys3[12] = {5, 2, 8, 1, 3, 7, 15, 4, 6, 9, 18, 20};
	for(i=0; i<12; i++)
	{
		T->AVL_INSERT(keys3[i]);
		T->print(testname3 + to_string(i+1));
		if(!T->IS_AVL())
			cout << notavl;
	}

	T->AVL_DELETE(1);

	T->print(testname3 + to_string(i+1));
	if(!T->IS_AVL())
		cout << notavl;

	cout << "Program terminated. Press enter to exit...";
	cin.get();
	return 0;
}