/* Implementation of Interpreter
 * for the Basic Perl-Like (BPL) Language
 * parserInt.cpp
 * Programming Assignment 3
 * Fall 2025
*/


#include "parserInt.h"

map<string, bool> defVar;
//map<string, Token> SymTable;
map<string, Value> TempsResults; //Container of temporary locations of Value objects for results of expressions, variables values and constants 
queue <Value> * ValQue; //declare a pointer variable to a queue of Value objects

namespace Parser {
	bool pushed_back = false;
	LexItem	pushed_token;

	static LexItem GetNextToken(istream& in, int& line) {
		if( pushed_back ) {
			pushed_back = false;
			return pushed_token;
		}
		return getNextToken(in, line);
	}

	static void PushBackToken(LexItem & t) {
		if( pushed_back ) {
			abort();
		}
		pushed_back = true;
		pushed_token = t;	
	}

}

static int error_count = 0;

int ErrCount()
{
    return error_count;
}

void ParseError(int line, string msg)
{
	++error_count;
	cout << error_count << ". Line " << line << ": " << msg << endl;
}

// Helper methods

static bool ValueTruth(const Value &v, int line) {
	if (v.IsBool()) return v.GetBool();
	if (v.IsNum()) return fabs(v.GetNum()) > 1e-12;
	if (v.IsString()) {
		string s = v.GetString();
		if (s.empty()) return false;
		if (s == "0") return false;
		return true;
	}
	throw string("Run-Time Error-Using Undefined Variable or Error Value");
}

bool Prog(istream& in, int& line){
	bool status = StmtList(in, line);

    if (!status) {
        return false;
    }

    LexItem tk = Parser::GetNextToken(in, line);
    if (tk != DONE) {
        ParseError(line, "Extra input after program end");
        return false;
    }
	cout << endl;
    cout << "\nDONE" << endl;
    return true;
}

bool StmtList(istream& in, int& line){
    while(true){
        LexItem tk = Parser::GetNextToken(in, line);

        if (tk == DONE || tk == RBRACES) {
            Parser::PushBackToken(tk);
            return true;
        }

        if (tk == ELSE) {
            Parser::PushBackToken(tk);
            return false;
        }

        Parser::PushBackToken(tk);

        bool status = Stmt(in, line);
        if (!status) {
            ParseError(line, "Syntactic error in Program Body.");
            ParseError(line, "Missing Program");
            return false;
        }

        tk = Parser::GetNextToken(in, line);

        if (tk == SEMICOL) {
            continue;
        }

        if (tk == IDENT || tk == PRINTLN || tk == IF ||
            tk == RBRACES || tk == DONE) 
        {
            Parser::PushBackToken(tk);
            continue;
        }

        ParseError(line, "Missing semicolon at end of Statement");
        return false;
    }
}

bool Stmt(istream& in, int& line){
	bool status = false;
	LexItem tk = Parser::GetNextToken(in,line);
	switch(tk.GetToken()){
		case IDENT:
			Parser::PushBackToken(tk);
			status=AssignStmt(in,line);
			if(!status){
				ParseError(line,"Incorrect Assignment Statement.");
			}
			return status;
		case PRINTLN:
			status=PrintLnStmt(in,line);
			if(!status){
				ParseError(line,"Incorrect PrintLn Statement");
			}
			return status;
		case IF:
			status = IfStmt(in,line);
			if(!status){
				ParseError(line, "Incorrect If-Statement.");
			}
			return status;
		case SEMICOL:
			return true;
		case ELSE:
			Parser::PushBackToken(tk);
			return false;
		case DONE:
			return true;
		default:
			Parser::GetNextToken(in,line);
			return true;
	}
	return status;
}

bool PrintLnStmt(istream& in, int& line){
	LexItem tk = Parser::GetNextToken(in,line);
	if(tk != LPAREN ) {
		ParseError(line, "Missing Left Parenthesis of PrintLn Statement");
		return false;
	}

	queue<Value> q;
	ValQue = &q;
	bool ex = ExprList(in,line);
	if(!ex){
		ParseError(line,"Missing expression list after PrintLn");
		return false;
	}

	tk=Parser::GetNextToken(in,line);
	if(tk != RPAREN){
		ParseError(line,"Missing Right Parenthesis of PrintLn Statement");
		return false;
	}

	while(!q.empty()){
		cout << q.front();
		q.pop();
	}
	cout<<endl;
	return true;
}

bool IfStmt(istream& in, int& line){
	LexItem tk = Parser::GetNextToken(in,line);
    if(tk != LPAREN){
        ParseError(line, "Missing Left Parenthesis of If condition");
        return false;
    }

    Value cond;
    if (!Expr(in, line, cond)) {
        ParseError(line, "Missing if statement Logic Expression");
        return false;
    }

    tk = Parser::GetNextToken(in, line);
    if(tk != RPAREN) {
        ParseError(line, "Missing Right Parenthesis of If condition");
        return false;
    }

    tk = Parser::GetNextToken(in, line);
    if(tk != LBRACES) {
        ParseError(line, "Missing left brace for If Statement Clause");
        return false;
    }

    bool condVal;
    try { 
        condVal = ValueTruth(cond, line); 
    } catch(...) {
        ParseError(line, "Run-Time Error evaluating if condition");
        return false;
    }

    if (condVal) {
        if (!StmtList(in, line)) return false;

        LexItem rb = Parser::GetNextToken(in, line);
        if(rb != RBRACES){
            ParseError(line, "Missing right brace for If Statement Clause");
            return false;
        }

        LexItem peek = Parser::GetNextToken(in,line);
        if(peek == ELSE) {
            LexItem lb = Parser::GetNextToken(in,line);
            if(lb != LBRACES){
                ParseError(line, "Missing left brace for Else-Clause");
                return false;
            }
            int braceCount = 1;
            while(braceCount > 0){
                LexItem tk = Parser::GetNextToken(in,line);
                if(tk == LBRACES) braceCount++;
                else if(tk == RBRACES) braceCount--;
                else if(tk == DONE){
                    ParseError(line, "Unexpected end of program in else-clause");
                    return false;
                }
            }
        } else {
            Parser::PushBackToken(peek);
        }
    } else {
        // Skip if-block
        int braceCount = 1;
        while(braceCount > 0){
            LexItem tk = Parser::GetNextToken(in,line);
            if(tk == LBRACES) braceCount++;
            else if(tk == RBRACES) braceCount--;
            else if(tk == DONE){
                ParseError(line, "Unexpected end of program in if-clause");
                return false;
            }
        }

        // Execute else-block if present
        LexItem peek = Parser::GetNextToken(in,line);
        if(peek == ELSE){
            LexItem lb = Parser::GetNextToken(in,line);
            if(lb != LBRACES){
                ParseError(line, "Missing left brace for Else-Clause");
                return false;
            }
            if(!StmtList(in,line)) return false;

            LexItem rb = Parser::GetNextToken(in,line);
            if(rb != RBRACES){
                ParseError(line, "Missing right brace for Else-Clause");
                return false;
            }
        } else {
            Parser::PushBackToken(peek);
        }
    }

    return true;
}

bool AssignStmt(istream& in, int& line){
	LexItem idtok;
	bool status = Var(in,line,idtok);
	if(!status){
		ParseError(line, "Missing Left-Hand Side Variable in Assignment statement");
		return false;
	}

	LexItem tk = Parser::GetNextToken(in,line);
	if(!(tk == ASSOP || tk == CADDA || tk == CSUBA || tk == CCATA)) {
		if(tk.GetToken() == ERR) {
			ParseError(line, "Unrecognized Input Pattern");
			cout << "(" << tk.GetLexeme() << ")" << endl;
			return false;
		}
		ParseError(line, "Missing Assignment Operator");
		return false;
	}
	
	Value rhs;
	bool ex = Expr(in,line,rhs);
	if(!ex){
		ParseError(line, "Missing Expression in Assignment Statement");
		return false;
	}

	string name = idtok.GetLexeme();

	try{
		if(tk == ASSOP) {
			if(rhs.IsBool()) {
				ParseError(line, "Illegal Assignment of a boolean value to a variable.");
				return false;
			}
			TempsResults[name] = rhs;
			defVar[name] = true;
			return true;
		}
		else if(tk==CADDA){
			if(TempsResults.find(name) == TempsResults.end()) {
				ParseError(line, "Run-Time Error-Using Undefined Variable in compound assignment");
				return false;
			}
			Value left = TempsResults[name];
			Value res = left + rhs;
			if(res.IsBool()) {
				ParseError(line, "Run-Time Error-Illegal Assignment Operation");
				return false;
			}
			TempsResults[name] = res;
			return true;
		}
		else if(tk==CSUBA){
			if(TempsResults.find(name) == TempsResults.end()) {
				ParseError(line, "Run-Time Error-Using Undefined Variable in compound assignment");
				return false;
			}
			Value left = TempsResults[name];
			Value res = left - rhs;
			if(res.IsBool()) {
				ParseError(line, "Run-Time2 Error-Illegal Assignment Operation");
				return false;
			}
			TempsResults[name] = res;
			return true;
		}
		else if(tk == CCATA){
			if(TempsResults.find(name) == TempsResults.end()) {
				ParseError(line, "Run-Time Error-Using Undefined Variable in compound assignment");
				return false;
			}
			Value left = TempsResults[name];
			Value res = left.Catenate(rhs);
			TempsResults[name] = res;
			return true;
		}
	}
	catch(string &s){
		ParseError(line, s);
		return false;
	}
	catch(...){
		ParseError(line, "R6un-Time Error-Illegal Assignment Operation");
		return false;
	}
	
	return true;
}

bool Var(istream& in, int& line, LexItem & idtok){
	LexItem tk = Parser::GetNextToken(in,line);

	if (tk == IDENT ){
		idtok = tk;
		string identstr = tk.GetLexeme();
		if (defVar.find(identstr) == defVar.end() || !defVar[identstr]) {
			defVar[identstr] = true;
		}
		return true;
	}
	else if(tk.GetToken() == ERR){
		ParseError(line, "Unrecognized Input Pattern");
		return false;
	}

	return false;
}

bool ExprList(istream& in, int& line){
	Value v;
	bool status = Expr(in, line, v);
	if(!status){
		ParseError(line, "Missing Expression");
		return false;
	}

	if(ValQue != nullptr) {
		ValQue->push(v);
	}

	LexItem tk = Parser::GetNextToken(in, line);
	if (tk == COMMA) {
		status = ExprList(in, line);
	} else if(tk.GetToken() == ERR){
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tk.GetLexeme() << ")" << endl;
		return false;
	} else {
		Parser::PushBackToken(tk);
		return true;
	}
	return status;
}

bool Expr(istream& in, int& line, Value & retVal){
	bool ok = OrExpr(in, line, retVal);
	return ok;
}

bool OrExpr(istream& in, int& line, Value & retVal){
	Value left;
	bool ok = AndExpr(in, line, left);
	if(!ok) return false;

	LexItem tk = Parser::GetNextToken(in, line);
	while (tk == OR ) {
		// short-circuit: if left is true, result is true
		bool leftTruth;
		try {
			leftTruth = ValueTruth(left, line);
		} catch(string &e) {
			ParseError(line, string("Run-Time Error-") + e);
			return false;
		} catch(...) {
			ParseError(line, "Run-Time Error-Illegal operand for OR");
			return false;
		}

		if(leftTruth) {
			Value rhs;
			if(!AndExpr(in, line, rhs)) {
				ParseError(line, "Missing operand after OR operator");
				return false;
			}
			// result is true
			retVal = Value(true);
		} else {
			// need to evaluate rhs to determine result
			Value rhs;
			if(!AndExpr(in, line, rhs)) {
				ParseError(line, "Missing operand after OR operator");
				return false;
			}
			bool rhsTruth;
			try {
				rhsTruth = ValueTruth(rhs, line);
			} catch(...) {
				if(rhs.IsString()) rhsTruth = !rhs.GetString().empty();
        		else throw;
			}
			retVal = Value(leftTruth || rhsTruth);
		}

		tk = Parser::GetNextToken(in, line);
	}

	Parser::PushBackToken(tk);
	if(retVal.GetType() == VERR) retVal = left;
	return true;
}

bool AndExpr(istream& in, int& line, Value & retVal){
	Value left;
	bool ok = RelExpr(in, line, left);
	if(!ok) return false;

	LexItem tk = Parser::GetNextToken(in, line);
	while (tk == AND ) {
		bool leftTruth;
		try {
			leftTruth = ValueTruth(left, line);
		} catch(string &e) {
			ParseError(line, string("Run-Time Error-") + e);
			return false;
		} catch(...) {
			ParseError(line, "Run-Time Error-Illegal operand for AND");
			return false;
		}

		if(!leftTruth) {
			Value rhs;
			if(!RelExpr(in, line, rhs)) {
				ParseError(line, "Missing operand after AND operator");
				return false;
			}
			retVal = Value(false);
		} else {
			// evaluate rhs
			Value rhs;
			if(!RelExpr(in, line, rhs)) {
				ParseError(line, "Missing operand after AND operator");
				return false;
			}
			bool rhsTruth;
			try {
				rhsTruth = ValueTruth(rhs, line);
			} catch(...) {
				if(rhs.IsString()) rhsTruth = !rhs.GetString().empty();
        		else throw;
			}
			retVal = Value(leftTruth && rhsTruth);
		}

		tk = Parser::GetNextToken(in, line);
	}

	Parser::PushBackToken(tk);
	if(retVal.GetType() == VERR) 
	retVal = left;

	return true;
}

bool RelExpr(istream& in, int& line, Value & retVal){
	Value left;
	bool ok = AddExpr(in, line, left);
	if(!ok) return false;

	LexItem tk = Parser::GetNextToken(in, line);
	if(tk.GetToken() == ERR){
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tk.GetLexeme() << ")" << endl;
		return false;
	}

	if (tk == NGTE || tk == NLT || tk == NEQ || tk == SLTE || tk == SGT || tk == SEQ){
		Value right;
		if(!AddExpr(in, line, right)) {
			ParseError(line, "Missing operand after a relational operator");
			return false;
		}
		bool result = false;

		try {
			switch (tk.GetToken()) {
                case NLT:   // numeric <
                    if (!left.IsNum() || !right.IsNum()) {
                        ParseError(line, "Illegal numeric relational operation.");
                        return false;
                    }
                    result = left.GetNum() < right.GetNum();
                    break;

                case NGTE:
                    if (!left.IsNum() || !right.IsNum()) {
                        ParseError(line, "Illegal numeric relational operation.");
                        return false;
                    }
                    result = left.GetNum() >= right.GetNum();
                    break;

                case NEQ:
                    if (left.IsNum() && right.IsNum()) {
                        result = left.GetNum() == right.GetNum();
                    }
                    else if (left.IsString() && right.IsString()) {
                        result = (left.GetString() == right.GetString());
                    }
                    else {
                        ParseError(line, "Illegal relational operation between mixed types.");
                        return false;
                    }
                    break;

                case SLTE:
                    if (!left.IsString() || !right.IsString()) {
                        ParseError(line, "Illegal string relational operation.");
                        return false;
                    }
                    result = left.GetString() <= right.GetString();
                    break;

                case SGT:
                    if (!left.IsString() || !right.IsString()) {
                        ParseError(line, "Illegal string relational operation.");
                        return false;
                    }
                    result = left.GetString() > right.GetString();
                    break;

                case SEQ:
                    if (!left.IsString() || !right.IsString()) {
                        ParseError(line, "Illegal string relational operation.");
                        return false;
                    }
                    result = left.GetString() == right.GetString();
                    break;

                default:
                    ParseError(line, "Unknown relational operator");
                    return false;
            }
		} catch(...) {
			ParseError(line, "Illegal Relational operation.");
			return false;
		}

		left = Value(result);
        tk = Parser::GetNextToken(in, line);
	}

	Parser::PushBackToken(tk);
	retVal = left;
	return true;
}

bool AddExpr(istream& in, int& line, Value & retVal){
	Value left;
	bool ok = MultExpr(in, line, left);
	if(!ok) return false;

	LexItem tk = Parser::GetNextToken(in, line);
	if(tk.GetToken() == ERR){
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tk.GetLexeme() << ")" << endl;
		return false;
	}

	while (tk == PLUS || tk == MINUS || tk == CAT) {
		Value right;
		if(!MultExpr(in, line, right)) {
			ParseError(line, "Missing operand after operator");
			return false;
		}
		try {
			if(tk == PLUS) {
				left = left + right;
			} else if(tk == MINUS) {
				left = left - right;
			} else if(tk == CAT) {
				left = left.Catenate(right);
			}
		} catch(string &s) {
			ParseError(line, s);
			return false;
		} catch(...) {
			ParseError(line, "Illegal Relational operation.");
			return false;
		}

		tk = Parser::GetNextToken(in, line);
		if(tk.GetToken() == ERR){
			ParseError(line, "Unrecognized Input Pattern");
			cout << "(" << tk.GetLexeme() << ")" << endl;
			return false;
		}
	}
	Parser::PushBackToken(tk);
	retVal = left;
	return true;
}

bool MultExpr(istream& in, int& line, Value & retVal){
	Value left;
	bool ok = UnaryExpr(in, line, left);
	if(!ok) return false;

	LexItem tk	= Parser::GetNextToken(in, line);
	if(tk.GetToken() == ERR){
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tk.GetLexeme() << ")" << endl;
		return false;
	}

	while (tk == MULT || tk == DIV  || tk == SREPEAT || tk==REM) {
		Value right;
		if(!UnaryExpr(in, line, right)) {
			ParseError(line, "Missing operand after operator");
			return false;
		}
		try {
			if(tk == MULT) {
				left = left * right;
			} else if(tk == DIV) {
				left = left / right;
			}
			else if(tk == REM){
				left = left%right;
			}
			else if(tk == SREPEAT) { // .x. string repeat
				Value lhsStr;
				if(left.IsString()) {
					lhsStr = left;
				} else if(left.IsNum()) {
					std::ostringstream oss;
					oss << std::fixed << std::setprecision(12) << left.GetNum();
					std::string s = oss.str();
					// remove trailing zeros
					s.erase(s.find_last_not_of('0') + 1, std::string::npos);
					if(!s.empty() && s.back() == '.') s.pop_back(); // remove trailing dot
					lhsStr = Value(s);
				} else {
					ParseError(line, "Illegal operand type for the string repetition operation.");
					return false;
				}

				if(!right.IsNum()) {
					ParseError(line, "Illegal operand type for the string repetition operation.");
					return false;
				}
				left = lhsStr.Repeat(right);
			}
		} catch(string &s) {
			ParseError(line, s);
			return false;
		} catch(...) {
			ParseError(line, "Illegal Relational operation.");
			return false;
		}

		tk = Parser::GetNextToken(in, line);
		if(tk.GetToken() == ERR){
			ParseError(line, "Unrecognized Input Pattern");
			cout << "(" << tk.GetLexeme() << ")" << endl;
			return false;
		}
	}
	Parser::PushBackToken(tk);
	retVal = left;
	return true;
}

bool UnaryExpr(istream& in, int& line, Value & retVal){
	LexItem t = Parser::GetNextToken(in, line);
	int sign = 0;
	if(t == MINUS )
	sign = -1; 
	else if(t == PLUS)
	sign = 1;
	else if(t == NOT)
	sign = 2;
	else 
	Parser::PushBackToken(t);

	bool status = ExponExpr(in, line, sign, retVal);
	if(!status) {
		ParseError(line, "Missing operand for an operator");
		return false;
	}
	return true;
}

bool ExponExpr(istream& in, int& line, int sign, Value & retVal){
	if(!PrimaryExpr(in, line, sign, retVal)) return false;

    LexItem tk = Parser::GetNextToken(in, line);
    if(tk == EXPONENT) {
        Value rhs;
        // recursive call for right-hand side (right-associative)
        if(!ExponExpr(in, line, 0, rhs)) {
            ParseError(line, "Missing exponent operand after exponentiation");
            return false;
        }

		if (!retVal.IsNum() || !rhs.IsNum()) {
        ParseError(line, "Illegal exponentiation operation.");
        return false;
		}

        // perform exponentiation
        try {
            retVal = retVal.Expon(rhs);
        } catch(string &s) {
            ParseError(line, s);
            return false;
        } catch(...) {
            ParseError(line, "Run-Time Error-Illegal exponent operation");
            return false;
        }

        return true;
    } else if(tk.GetToken() == ERR){
        ParseError(line, "Unrecognized Input Pattern");
        cout << "(" << tk.GetLexeme() << ")" << endl;
        return false;
    }

    Parser::PushBackToken(tk);
    return true;
}

bool PrimaryExpr(istream& in, int& line, int sign,Value & retVal){
	LexItem tk = Parser::GetNextToken(in, line);

	if(tk == IDENT ) {
		string lexeme = tk.GetLexeme();

		// if variable was never declared/seen its an error
		if (defVar.find(lexeme) == defVar.end() || !defVar[lexeme]) {
			ParseError(line, "Using Undefined Variable: "+ lexeme);
			Parser::PushBackToken(tk);
			return false;
		}

		auto it = TempsResults.find(lexeme);
		if(it == TempsResults.end()) {
			ParseError(line, "Run-Time Error-Using Undefined Variable: " + lexeme);
			Parser::PushBackToken(tk);
			return false;
		}

		retVal = it->second;

		// apply unary sign if present
		if(sign == 2) { // !
			bool val;
			try { val = ValueTruth(retVal, line); }
			catch(string &e) { ParseError(line, string("Run-Time Error-") + e); return false; }
			retVal = Value(!val);
		} else if(sign == -1) {
			// unary minus requires numeric
			if(!retVal.IsNum()) {
				ParseError(line, "Illegal Operand Type for Sign Operator");
				return false;
			}
			retVal.SetNum(- retVal.GetNum());
		} else if(sign == 1) {
			// unary plus requires numeric (no-op)
			if(!retVal.IsNum()) {
				ParseError(line, "Illegal Operand Type for Sign Operator");
				return false;
			}
		}
		return true;
	}
	else if(tk == ICONST ) {
		double val = 0.0;
		try {
			val = stod(tk.GetLexeme());
		} catch(...) {
			ParseError(line, "Run-Time Error-Illegal integer constant");
			return false;
		}
		retVal = Value(val);
		if(sign == 2) {
			bool b = ValueTruth(retVal, line);
			retVal = Value(!b);
		} else if(sign == -1) {
			retVal.SetNum(- retVal.GetNum());
		}
		return true;
	}
	else if(tk == FCONST ) {
		double val = 0.0;
		try {
			val = stod(tk.GetLexeme());
		} catch(...) {
			ParseError(line, "Run-Time Error-Illegal float constant");
			return false;
		}
		retVal = Value(val);
		if(sign == 2) {
			bool b = ValueTruth(retVal, line);
			retVal = Value(!b);
		} else if(sign == -1) {
			retVal.SetNum(- retVal.GetNum());
		}
		return true;
	}
	else if(tk == SCONST ) {
		retVal = Value(tk.GetLexeme());
		if(sign == 2) {
			bool b;
			try { b = ValueTruth(retVal, line); }
			catch(string &e) { ParseError(line, string("Run-Time Error-") + e); return false; }
			retVal = Value(!b);
		} else if(sign == -1) {
			ParseError(line, "Illegal Operand Type for Sign Operator");
			return false;
		}
		return true;
	}
	else if(tk == LPAREN ) {
		Value inner;
		bool ex = Expr(in, line, inner);
		if(!ex) {
			ParseError(line, "Missing expression after Left Parenthesis");
			return false;
		}
		LexItem rb = Parser::GetNextToken(in, line);
		if(rb != RPAREN) {
			Parser::PushBackToken(tk);
			ParseError(line, "Missing right Parenthesis after expression");
			return false;
		}
		if(sign == 2) {
			bool b;
			try { b = ValueTruth(inner, line); }
			catch(string &e) { ParseError(line, string("Run-Time Error-") + e); return false; }
			retVal = Value(!b);
		} else if(sign == -1) {
			if(!inner.IsNum()) {
				ParseError(line, "Illegal Operand Type for Sign Operator");
				return false;
			}
			retVal = Value(- inner.GetNum());
		} else {
			retVal = inner;
		}
		return true;
	}
	else if(tk.GetToken() == ERR){
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tk.GetLexeme() << ")" << endl;
		return false;
	}

	return false;
}