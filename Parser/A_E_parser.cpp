// Ali Elbeyali
// ae526
// 280-011

#include "parser.h"

map<string, bool> defVar;
map<string, Token> SymTable;

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
static string lastVarName;
static bool emittedProgBodyForThisStmt = false;
static int anchor = -1;
static bool inBlock = false,exprMissingRParen = false,exprMissingOper = false, exprMissingUn = false,needOp = false, primaryAfterUn = false;
static int lastLine = -1, endLine = -1, tempLine = 0;

int ErrCount()
{
    return error_count;
}

void ParseError(int line, string msg)
{
	++error_count;
	cout << error_count << ". Line " << line << ": " << msg << endl;
}

bool Prog(istream& in, int& line){
    if(!StmtList(in,line)){
        if(emittedProgBodyForThisStmt){
            emittedProgBodyForThisStmt = false;
            return false;
        }
        while(true){
            LexItem tok = Parser::GetNextToken(in,line);
            Token tk = tok.GetToken();

            if(tk == DONE){
                break;
            }
            if(tk == IF || tk == PRINTLN || tk == IDENT){
                ParseError(line,"Syntactic error in Program Body");
                Parser::PushBackToken(tok);
                break;
            }
        }
        return false;
    }
    LexItem token = Parser::GetNextToken(in,line);
    if(token.GetToken() != DONE){
        ParseError(line, "Syntactic error in Program Body");
        return false;
    }

    cout << "Declared Variables:\n";
    bool first = true;
    for(const auto& kv:defVar){
        if(!kv.second){
            continue;
        }
        if(!first){
            cout << ", ";
        }
        cout << kv.first;
        first = false;
    }
    cout << "\n\nDONE" << endl;
    return true;
}

bool StmtList(istream& in, int& line){
    if(!Stmt(in,line)){
        return false;
    }

    int expectedLine = (endLine > 0 ? endLine:line);
    endLine = -1;
    LexItem token = Parser::GetNextToken(in,line);
    if(token.GetToken() != SEMICOL){
        ParseError(expectedLine-1, "Missing semicolon at end of Statement");
        anchor = expectedLine;
        return false;
    }

    if(inBlock)
    lastLine = token.GetLinenum();

    while(true){
        LexItem li = Parser::GetNextToken(in,line);
        Token tok = li.GetToken();

        if(tok==ELSE){
            if(inBlock){
                Parser::PushBackToken(li);
                break;
            }
            else{
                ParseError(li.GetLinenum(), "Illegal If Statement Else-Clause");
                emittedProgBodyForThisStmt = true;
                return false;
            }
        }

        if(tok == IF || tok == PRINTLN || tok == IDENT){
            Parser::PushBackToken(li);

            if(!Stmt(in,line)) return false;

            expectedLine = (endLine > 0 ? endLine:line);
            token = Parser::GetNextToken(in,line);
            if(token.GetToken() != SEMICOL){
                ParseError(expectedLine-1, "Missing semicolon at end of Statement");
                emittedProgBodyForThisStmt = true;
                return false;
            }
            if(inBlock)
            lastLine = token.GetLinenum();
            continue;
        }
        Parser::PushBackToken(li);
        break;
    }
    return true;
}

bool Stmt(istream& in, int& line){
    LexItem tok = Parser::GetNextToken(in,line);
    switch(tok.GetToken()){
        case IF:
            Parser::PushBackToken(tok); return IfStmt(in, line);
        case PRINTLN:
            Parser::PushBackToken(tok); return PrintLnStmt(in, line);
        case IDENT:
            Parser::PushBackToken(tok); return AssignStmt(in, line);
        case ERR:
            ParseError(line, "Lexical Error"); return false;
        default:
            Parser::PushBackToken(tok); 
            ParseError(line, "Syntactic error in Program Body"); 
            return false;
    }
}

bool PrintLnStmt(istream& in, int& line){
    LexItem tok = Parser::GetNextToken(in,line);
    if(tok.GetToken() != PRINTLN){
        ParseError(line, "Missing PRINTLN in PrintLnStmt");
        return false;
    }
    tok = Parser::GetNextToken(in,line);
    if(tok.GetToken() != LPAREN){
        ParseError(line, "Missing Left Parenthesis of PrintLn Statement");
        ParseError(line, "Incorrect PrintLn Statement");
        ParseError(line, "Syntactic error in Program Body");
        emittedProgBodyForThisStmt = true;
        return false;
    }
    if(!ExprList(in,line)){
        ParseError(line, "Missing expression after Left Parenthesis in PRINTLN");
        return false;
    }
    tok = Parser::GetNextToken(in,line);
    if(tok.GetToken() != RPAREN){
        ParseError(line, "Missing Parenthesis of PrintLn Statement");
        ParseError(line, "Incorrect   PrintLn Statement");
        return false;
    }
    endLine = tok.GetLinenum();
    return true;
}

bool IfStmt(istream& in, int& line){
    LexItem tok = Parser::GetNextToken(in,line);
    if(tok.GetToken() != IF){
        ParseError(line, "Syntactic error in Program Body");
        return false;
    }
    tok = Parser::GetNextToken(in,line);
    if(tok.GetToken() != LPAREN){
        ParseError(line, "Missing Left Parenthesis after IF");
        ParseError(line, "Incorrect If-Statement");
        return false;
    }
    if(!Expr(in,line)){
        ParseError(line, "Incorrect If-Statement");
        return false;
    }
    tok = Parser::GetNextToken(in,line);
    if(tok.GetToken() != RPAREN){
        ParseError(line, "Missing Right Parenthesis of If condition");
        ParseError(line, "Incorrect If-Statement");
        ParseError(line, "Syntactic error in Program Body");
        emittedProgBodyForThisStmt = true;
        return false;
    }

    tok = Parser::GetNextToken(in,line);
    if(tok.GetToken() != LBRACES){
        int err = tok.GetLinenum();
        ParseError(err, "Missing left brace for If Statement Clause");
        ParseError(err, "Incorrect If-Statement");
        ParseError(err, "Syntactic error in Program Body");
        emittedProgBodyForThisStmt = true;
        return false;
    }

    inBlock = true;
    lastLine = -1;
    if(!StmtList(in,line)){
        inBlock = false;
        ParseError(line, "Incorrect If-Statement");
        return false;
    }
    inBlock = false;

    tok = Parser::GetNextToken(in,line);
    if(tok.GetToken() != RBRACES){
        int err = (tok.GetToken() == DONE ? lastLine + 1: tok.GetLinenum());
        ParseError(err, "Illegal If Statement Else-Clause");
        ParseError(err, "Missing Statement for If Statement Clause");
        ParseError(err, "Incorrect If-Statement");
        ParseError(err, "Syntactic error in Program Body");
        emittedProgBodyForThisStmt = true;
        return false;
    }
    endLine = tok.GetLinenum();

    tok = Parser::GetNextToken(in,line);
    if(tok.GetToken() == ELSE){
        tok = Parser::GetNextToken(in,line);
        if(tok.GetToken() != LBRACES){
            int err = tok.GetLinenum();
            ParseError(err, "Missing left brace for If Statement Clause");
            ParseError(err, "Incorrect If-Statement");
            ParseError(err, "Syntactic error in Program Body");
            emittedProgBodyForThisStmt = true;
            return false;
        }

        inBlock = true;
        lastLine = -1;
        if(!StmtList(in,line)){
            inBlock = false;
            int err = (anchor > 0 ? anchor : line);
            anchor = -1;
            ParseError(err, "Missing Statement for Else-Clause");
            ParseError(err, "Incorrect If-Statement");
            ParseError(err, "Syntactic error in Program Body");
            emittedProgBodyForThisStmt = true;
            return false;
        }
        inBlock = false;
        
        tok = Parser::GetNextToken(in,line);
        if(tok.GetToken() != RBRACES){
            int err = (tok.GetToken() == DONE ? lastLine : tok.GetLinenum());
            ParseError(err, "Missing right brace for an Else-Clause");
            ParseError(err, "Incorrect If-Statement");
            ParseError(err, "Syntactic error in Program Body");
            emittedProgBodyForThisStmt = true;
            return false;
        }
        endLine = tok.GetLinenum();
        return true;
    }

    Parser::PushBackToken(tok);
    return true;
}

bool Var(istream& in, int& line){
    LexItem tok = Parser::GetNextToken(in,line);
    if(tok.GetToken() == IDENT){
        lastVarName = tok.GetLexeme();
        return true;
    }
    if(tok.GetToken() == ERR){
        ParseError(line,"Lexical Error");
        return false;
    }
    Parser::PushBackToken(tok);
    ParseError(line,"Missing Identifier");
    return false;
}

bool ExprList(istream& in, int& line){
    if(!Expr(in,line))
    return false;

    while(true){
        LexItem tok = Parser::GetNextToken(in,line);
        if(tok.GetToken() == COMMA){
            if(!Expr(in,line)){
                ParseError(line,"Missing expression after comma");
                return false;
            }
        }
        else{
            Parser::PushBackToken(tok);
            break;
        }
    }
    return true;
}

bool AssignStmt(istream& in, int& line){
    if(!Var(in,line)){
        ParseError(line,"Incorrect Assignment Statement");
        return false;
    }
    if(!AssigOp(in,line)){
        ParseError(line,"Incorrect Assignment Statement");
        ParseError(line, "Syntactic error in Program Body");
        emittedProgBodyForThisStmt = true;
        return false;
    }

    if(!Expr(in,line)){
        ParseError(line,"Missing Expression in Assignment Statement");
        ParseError(line,"Incorrect Assignment Statement");

        tempLine = line;
        if((!inBlock && (exprMissingRParen || exprMissingOper))||exprMissingUn){
            ParseError(line, "Syntactic error in Program Body");
            emittedProgBodyForThisStmt = true;
        }
        exprMissingRParen = false;
        exprMissingOper = false;
        exprMissingUn=false;
        needOp = false;
        return false;
    }

    endLine = line;
    if(!lastVarName.empty())
    defVar[lastVarName] = true;
    return true;
}

bool Expr(istream& in, int& line){
    needOp=false;
    exprMissingOper=false;
    exprMissingRParen=false;
    exprMissingUn=false;
    return OrExpr(in,line);
}

bool AssigOp(istream& in, int& line){
    LexItem tok = Parser::GetNextToken(in,line);
    switch(tok.GetToken()){
        case ASSOP:
        case CADDA:
        case CSUBA:
        case CCATA:
            return true;
        case ERR:
            ParseError(line,"Lexical Error");
            return false;
        default:
            Parser::PushBackToken(tok);
            ParseError(line,"Missing Assignment Operator");
            return false;
    }
    
}

bool OrExpr(istream& in, int& line){
    if(!AndExpr(in,line)){
        return false;
    }

    while(true){
        LexItem tok = Parser::GetNextToken(in,line);
        if(tok.GetToken() == OR){
            if(!AndExpr(in,line)){
                return false;
            }
        }
        else if(tok.GetToken() == ERR){
            ParseError(line, "Lexical Error");
            return false;
        }
        else{
            Parser::PushBackToken(tok);
            break;
        }
    }
    return true;
}

bool AndExpr(istream& in, int& line){
    if(!RelExpr(in,line)){
        return false;
    }
    while(true){
        LexItem tok = Parser::GetNextToken(in,line);
        if(tok.GetToken() == AND){
            if(!RelExpr(in,line)){
                return false;
            }
        }
        else if(tok.GetToken() == ERR){
            ParseError(line,"Lexical Error");
            return false;
        }
        else{
            Parser::PushBackToken(tok);
            break;
        }
    }
    return true;
}

bool RelExpr(istream& in, int& line){
    if(!AddExpr(in,line)){
        return false;
    }

    LexItem tok = Parser::GetNextToken(in,line);
    switch(tok.GetToken()){
        case SLTE:
        case SGT:
        case SEQ:
        case NLT:
        case NGTE:
        case NEQ:
            if(!AddExpr(in,line))
            return false;
            return true;
        case ERR:
            ParseError(line,"Lexical Error");
            return false;
        default:
            Parser::PushBackToken(tok);
            return true;

    }
}

bool AddExpr(istream& in, int& line){
    if(!MultExpr(in,line))
    return false;

    while(true){
        LexItem tok = Parser::GetNextToken(in,line);
        Token tk = tok.GetToken();

        if(tk == PLUS || tk == MINUS || tk == CAT){
            int op = tok.GetLinenum();

            LexItem li = Parser::GetNextToken(in,line);
            Token lt = li.GetToken();

            if(lt == IDENT){
                const string& name = li.GetLexeme();
                if(!defVar[name]){
                    ParseError(li.GetLinenum(), string("Using Undefined Variable: " + name));
                    ParseError(op, "Missing operand for an operator");
                    ParseError(op, "Missing operand after operator");
                    Parser::PushBackToken(li);
                    return false;
                }
            }

            if(!(lt == IDENT || lt == ICONST || lt == FCONST || lt == SCONST || lt == LPAREN || lt == PLUS || lt == MINUS || lt == NOT)){
                ParseError(op,"Missing operand for an operator");
                ParseError(op, "Missing operand after operator");
                Parser::PushBackToken(li);
                return false;
            }

            Parser::PushBackToken(li);
            if(!MultExpr(in,line)){
                if(needOp){
                    ParseError(line, "Missing operand for an operator");
                    ParseError(line, "Missing operand after operator");
                    needOp=false;
                }
                return false;
            }
            continue;
        }
        if(tk == ERR){
            ParseError(line,"Lexical Error");
            return false;
        }
        Parser::PushBackToken(tok);
        break;
    }
    return true;
}

bool MultExpr(istream& in, int& line){
    if(!UnaryExpr(in,line)){
        return false;
    }
    while(true){
        LexItem tok = Parser::GetNextToken(in,line);
        Token tk = tok.GetToken();
        if(tk == MULT || tk==DIV || tk==REM || tk==SREPEAT){
            int op = tok.GetLinenum();

            LexItem li = Parser::GetNextToken(in,line);
            Token lt = li.GetToken();

            if(lt == IDENT){
                const string& name = li.GetLexeme();
                if(!defVar[name]){
                    ParseError(li.GetLinenum(), string("Using Undefined Variable: " + name));
                    ParseError(op, "Missing operand for an operator");
                    ParseError(op, "Missing operand after operator");
                    Parser::PushBackToken(li);
                    return false;
                }
            }

            if(!(lt == IDENT || lt == ICONST || lt == FCONST || lt == SCONST || lt == LPAREN || lt == PLUS || lt == MINUS || lt == NOT)){
                ParseError(op,"Missing operand for an operator");
                ParseError(op, "Missing operand after operator");
                Parser::PushBackToken(li);
                return false;
            }

            Parser::PushBackToken(li);
            if(!UnaryExpr(in,line)){
                if(needOp){
                    ParseError(line, "Missing operand after operator");
                    needOp=false;
                }
                return false;
            }
            continue;
        }
        if(tk == ERR){
            ParseError(line, "Lexical Error");
            return false;
        }
        Parser::PushBackToken(tok);
        break;
    }
    return true;
}

bool UnaryExpr(istream& in, int& line){
    LexItem tok = Parser::GetNextToken(in,line);
    Token tk = tok.GetToken();

    if(tk == MINUS || tk == PLUS || tk == NOT){
        primaryAfterUn = true;
        bool ok = ExponExpr(in,line,(tk==MINUS)?-1:(tk==PLUS ? +1: 2));
        primaryAfterUn = false;
        return ok;
    }
    Parser::PushBackToken(tok);
    return ExponExpr(in,line,0);
}

bool ExponExpr(istream& in, int& line, int /*sign*/){
    if(!PrimaryExpr(in,line,0))
    return false;
    
    while(true){
        LexItem tok = Parser::GetNextToken(in,line);
        if(tok.GetToken() == EXPONENT){
            LexItem rhs = Parser::GetNextToken(in,line);
            Token rt = rhs.GetToken();

            if(!(rt==IDENT || rt == ICONST || rt == FCONST || rt == SCONST || rt == LPAREN)){
                ParseError(line, "Missing exponent operand after exponentiation");
                ParseError(line, "Missing operand for an operator");
                ParseError(line, "Missing operand after operator");
                exprMissingOper=true;

                Parser::PushBackToken(tok);
                return false;
            }

            Parser::PushBackToken(rhs);
            if(!PrimaryExpr(in,line,0))
            return false;
            continue;
        }
        
        if(tok.GetToken() == ERR){
            ParseError(line, "Lexical Error");
            return false;
        }
        Parser::PushBackToken(tok);
        break;
    }
    return true;
}

bool PrimaryExpr(istream& in, int& line, int /*sign*/){
    LexItem tok = Parser::GetNextToken(in,line);
    Token t = tok.GetToken();
    if(primaryAfterUn && (t == MINUS || t == PLUS || t == NOT)){
        ParseError(line, "Missing operand for an operator");
        exprMissingUn = true;
        Parser::PushBackToken(tok);
        return false;
    }
    switch(t){
        case IDENT:
        case ICONST:
        case FCONST:
        case SCONST:
            return true;

        case LPAREN:
            if(!Expr(in,line)){
                ParseError(line,"Missing expression after Left Parenthesis");
                ParseError(line, "Missing operand for an operator");
                needOp = true;
                return false;
            }
            tok = Parser::GetNextToken(in,line);
            if(tok.GetToken() != RPAREN){
                ParseError(line, "Missing right Parenthesis after expression");
                ParseError(line, "Missing operand for an operator");
                exprMissingRParen = true;
                return false;
            }
            needOp=false;
            return true;
        case ERR:
            ParseError(line,"Lexical Error");
            return false;
        default:
            Parser::PushBackToken(tok);
            ParseError(line,"Missing operand for an operator");
            return false;
    }
}