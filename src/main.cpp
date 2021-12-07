/*
 *  UCF COP3330 Fall 2021 Assignment 6 Solution
 *  Copyright 2021 Jose Malave
 */
//here are the solutions to exercises 1 through 3 in chapter 7

#include "std_lib_facilities.h"


//here is the struct for the tokens
struct Token {
	char kind;
	double value;
	string name;
	Token(char ch) :kind(ch), value(0) { } 
	Token(char ch, double val) :kind(ch), value(val) { } 
	Token(char ch, string n) :kind(ch), name(n) { } 
};

//class for the token_stream
class Token_stream {
	bool full;
	Token buffer; 
public:
	Token_stream() :full(0), buffer(0) { }

	Token get();
	void unget(Token t) { buffer=t; full=true; } 

	void ignore(char); 
};

//all the const chars we are going to need
const char let = 'L'; 
const char constant = 'C';
const char quit = 'E'; 
const char print = ';'; 
const char number = '8';
const char name = 'a'; 
const char sqrts = 's'; 
const char pows = 'p';

// Here we are reading all the char
Token Token_stream::get() 
{
	if (full) {
     full=false; return buffer;
    } 
    
	char ch; 
	cin >> ch; 
	switch (ch) {
	case '(':
	case ')':
	case '+':
	case '-':
	case '*':
	case '/':
	case '%':
	case ';':
	case '=':
	case ',':
		return Token(ch); 
	case '.':
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	{	
		cin.unget(); 
		double val;
		cin >> val; 
		return Token(number,val);
	}
  
	default:
		if (isalpha(ch)) {
			string s;
			s += ch; 
			while(cin.get(ch) && (isalpha(ch) || isdigit(ch) || ch == '_')) s+=ch;
			cin.unget();

      if (s == "let") return Token(constant);
      if (s == "const") return Token(constant);
			if (s == "sqrt") return Token(sqrts);
			if (s == "pow") return Token(pows);
			if (s == "exit") return Token(quit); 
			return Token(name,s);
		}
		error("Bad token");
	}
}

//Here we want to ignore any characters that come before a symbol
void Token_stream::ignore(char c)
{
	if (full && c==buffer.kind) { 
		full = false;
		return;
	}
	full = false; 
	char ch;
	while (cin>>ch)
		if (ch==c) return; 
}

//here is our struct for all the variables that we are going to be using 
struct Variable { 
  //here is also where we determine if the variable is a constant
  int constant;
	string name;
	double value;
	Variable(int c, string n, double v) :constant(c), name(n), value(v) { }
};

vector<Variable> names;

double get_value(string s) {
	for (Variable i: names) {
		if (i.name == s) return i.value;
	}
	error("get: undefined name ",s);
}

void set_value(string s, double d) 
{
  //if variable exists and is not a constant, alter data
	for (int i = 0; i<=names.size(); ++i)
		if (names[i].name == s) {
      if(names[i].constant != 1)
			  names[i].value = d;
			return;
		}
	error("set: undefined name ",s);
}

//is there already an existing variable
bool is_declared(string s) 
{
	for (int i = 0; i<names.size(); ++i)
		if (names[i].name == s) return true;
	return false;
}

Token_stream ts; 

double expression();

//here are the parenthases and the pow sqrts functions
double primary()
{
	Token t = ts.get();
	switch (t.kind) {
	case '(':
	{
    double d = expression();
    t = ts.get();
    if (t.kind != ')') error("'(' expected");
	}

  case sqrts: 
	{
		double d = primary();
		if (d < 0) error("square root of negative number was detected");
		return sqrt(d);
	}

	case pows:
	{
		t = ts.get();
		if (t.kind == '(') {
			double lval = primary();
			int rval = 0;
			t = ts.get();
			if(t.kind == ',') rval = narrow_cast<int>(primary());
			else error("Second argument is not provided");
			double result = 1;
			for(double i = 0; i < rval; i++) {
				result*=lval;
			}
			t = ts.get();
			if (t.kind != ')') error("')' expected");
			return result;
		}
		else error("'(' expected");
	}

	case '-':
		return -primary();
	case number:
		return t.value;
	case name:
		return get_value(t.name);
	default:
		error("primary expected");
	}
}

//here are the operations that include the * / and %
double term()
{
	double left = primary();
	while (true) {
		Token t = ts.get();
		switch (t.kind) {
		case '*':
			left *= primary();
			break;

		case '/':
		{	
      double d = primary();
		  if (d == 0) error("divide by zero");
		  left /= d;
		  break;
		}
   case '%':
   {
    double d = primary();
    if (d == 0)
     error("%:divide by zero");
    left = fmod(left, d);
    break;
   }

		default:
			ts.unget(t);
			return left;
		}
	}
}

//basic operations
double expression()
{
	double left = term();
	while (true) {
		Token t = ts.get();
		switch (t.kind) {
		case '+':
			left += term();
			break;
		case '-':
			left -= term();
			break;
		default:
			ts.unget(t);
			return left;
		}
	}
}


//here is where we determine if the values exist and if we want to change them.
double declaration(int c) 
{
  int flag = 0;
	Token t = ts.get(); 
	if (t.kind != 'a') error ("name expected in declaration"); 
	string name = t.name; 
  
	if (is_declared(name)) flag = 1; 
	Token t2 = ts.get(); 

	if (t2.kind != '=') error("= missing in declaration of " ,name); 

  double d = expression();
  if (flag == 1) set_value(name,d); 
	else 
  {
    if (c == 0) names.push_back(Variable(c,name,d));
    else if(c == 1) names.push_back(Variable(c,name,d));
  }
    
	return d; // Return the value of new variable
}

//here is where we determine if we are declaring a regular variable or a constant one
double statement() 
{
	Token t = ts.get();
	switch(t.kind) {
	case let:
		return declaration(0);
  case constant:
    return declaration(1);
	default: 
		ts.unget(t);
		return expression();
	}
}

void clean_up_mess() 
{
	ts.ignore(print);
}

//this is what will always be outputed on each line
//one for inputes, and one for outputs
const string prompt = "> ";
const string result = "= ";

//here is where everyting starts to come together
void calculate() 
{
	while(true) try {
		cout << prompt; 
		Token t = ts.get(); 
		while (t.kind == print) t=ts.get(); 
		if (t.kind == quit) return; 
		ts.unget(t); 
		cout << result << statement() << endl; 
    }
	catch(runtime_error& e) {
		cerr << e.what() << endl; 
		clean_up_mess(); 
	}
}


int main()
	try {
		names.push_back(Variable(0,"k",1000)); 
		calculate(); 
		return 0;
	}
	catch (exception& e) { 
		cerr << "exception: " << e.what() << endl;
		char c;
		while (cin >>c&& c!=';');
		return 1;
	}
	catch (...) { 
		cerr << "exception\n";
		char c;
		while (cin>>c && c!=';');
		return 2;
	}