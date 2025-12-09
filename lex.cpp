#include <iostream>
#include <string>
#include <cctype>
#include <map>
#include <sstream>
#include "lex.h"

using namespace std;

static const map<Token, string> TokStr = {
    {PRINTLN, "PRINTLN" }, {IF, "IF" }, {ELSE, "ELSE" },
    {IDENT, "IDENT" },
    {ICONST, "ICONST" }, {FCONST, "FCONST" }, {SCONST, "SCONST" },
    {PLUS, "PLUS" }, {MINUS, "MINUS" }, {MULT, "MULT" }, {DIV, "DIV" }, {REM, "REM" },
    {EXPONENT, "EXPONENT" },
    {NEQ, "NEQ" }, {SEQ, "SEQ" },
    {NLT, "NLT" }, {NGTE, "NGTE" },
    {SLTE, "SLTE" }, {SGT, "SGT" },
    {CAT, "CAT" }, {SREPEAT, "SREPEAT" },
    {AND, "AND" }, {OR, "OR" }, {NOT, "NOT" },
    {ASSOP, "ASSOP" }, {CADDA, "CADDA" }, {CSUBA, "CSUBA" }, {CCATA, "CCATA" },
    {COMMA, "COMMA" }, {SEMICOL, "SEMICOL" },
    {LPAREN, "LPAREN" }, {RPAREN, "RPAREN" },
    {LBRACES, "LBRACES" }, {RBRACES, "RBRACES" },
    {ERR, "ERR" }, {DONE, "DONE" }
};

static string toLowerAscii(string s){
    for(char &c:s)
    c=static_cast<char>(tolower(static_cast<unsigned char>(c)));
    return s;
}

static bool isLetter(char c){
    return (c>= 'a'&& c <= 'z')||(c>='A'&&c<='Z');
}
static bool isDigit(char c){
    return (c>='0' && c<='9');
}

ostream& operator<<(ostream& out, const LexItem& tok){
    Token t = tok.GetToken();
    const string& lx = tok.GetLexeme();
    int linNum = tok.GetLinenum();
    if(t == ERR){
        out << "ERR: Error-Unrecognized Lexeme {" << lx << "} in line " << linNum << "\n";
        return out;
    }
    auto it = TokStr.find(t);
    string name = (it != TokStr.end() ? it->second: "UNKNOWN");

    if(t==ICONST ||  t==FCONST){
        out << name << ": [" << lx << "]\n";
    }
    else if(t==IDENT){
        out << name << ": (" << lx << ")\n";
    }
    else if(t==SCONST){
        out << name << ": <" << lx << ">\n";
    }
    else{
        if(!lx.empty())
        out << name << ": \"" << lx << "\"\n";
        else
        out << name << "\n";
    }
    return out;
    //return (out << tok.GetLexeme() << " : " << tok.GetToken() << " : " << tok.GetLinenum());
}

LexItem id_or_kw(const string& lexeme, int linenum){
    static const map<string, Token> kw = {
        // case sensitive keywords
        {"println",PRINTLN},
        {"if",IF},
        {"else", ELSE}
    };
    string lower = toLowerAscii(lexeme);
    auto it = kw.find(lower);
    if(it != kw.end()){
        return LexItem(it->second,lexeme,linenum);
    }
    return LexItem(IDENT, lexeme, linenum);
}


LexItem getNextToken(istream& in, int& linenum){
    char c;

    //skipping whitespaces
    while(true){
        int ch = in.peek();
        if(ch==EOF){
            return LexItem(DONE, "", linenum);
        }
        c = static_cast<char>(ch);
        
        if(c=='#'){
            //skip the rest (comment)
            in.get();
            while(true){
                int cc = in.get();
                if(cc==EOF){
                    return LexItem(DONE, "", linenum);
                }
                if(cc == '\n'){
                    linenum++;
                    break;
                }
            }
            continue;
        }
        else if(c==' '|| c=='\t'|| c== '\r'){
            in.get();
            continue;
        }
        else if(c=='\n'){
            in.get();
            linenum++;
            continue;
        }
        break;
    }
    c=static_cast<char>(in.get());

    //scan for longer lexeme
    if(c=='\'' || c=='"'){
        char q = c;
        string lex;
        while(true){
            int ch = in.get();
            if(ch==EOF){
                return LexItem(ERR, string(1,q)+lex,linenum);
            }
            char cc = static_cast<char>(ch);
            if(cc=='\n'){
                //linenum++;
                return LexItem(ERR, string(1,q)+lex, linenum);
            }
            if(cc==q){
                // end of string
                return LexItem(SCONST, lex, linenum);
            }
            lex += cc;
        }
    }

    if(isLetter(c)||c=='$'){
        string lex(1,c);
        while(true){
            int ch = in.peek();
            if(ch == EOF)
            break;
            char cc = static_cast<char>(ch);
            if(isLetter(cc)||isDigit(cc)||cc=='$'||cc=='_'){
                lex +=static_cast<char>(in.get());
            }
            else
            break;
        }
        return id_or_kw(lex, linenum);
    }

    if(isDigit(c)){
        string digits(1,c);

        while(isDigit(in.peek())){
            digits+=static_cast<char>(in.get());
        }
        if(in.peek()=='.'){
            in.get();
            int next = in.peek();
            if(next != EOF && isDigit(static_cast<char>(next))){
                string f;
                while(isDigit(in.peek())){
                    f += static_cast<char>(in.get());
                }
                string exp;
                if(in.peek() == 'e'||in.peek()=='E'){
                    exp += static_cast<char>(in.get());
                    if(in.peek()=='+'||in.peek()=='-'){
                        exp += static_cast<char>(in.get());
                    }
                    if(!isDigit(in.peek())){
                        return LexItem(ERR, digits+"."+f+exp,linenum);
                    }
                    while(isDigit(in.peek())){
                        exp += static_cast<char>(in.get());
                    }
                }
                string fl = digits + "." + f + exp;
                if(in.peek()=='.'){
                    in.get();
                    return LexItem(ERR, fl+".", linenum);
                }
                return LexItem(FCONST, digits + '.'+f+exp,linenum);
            }
            else{
                in.putback('.');
                return LexItem(FCONST, digits, linenum);
            }
        }
        else{
            return LexItem(ICONST, digits, linenum);
        }
    }

    if(c=='.'){
        int p1=in.peek();
        if(p1=='x'){
            in.get();
            if(in.peek()=='.'){
                in.get();
                return LexItem(SREPEAT, ".x.",linenum);
            }
            else{
                return LexItem(ERR, ".x",linenum);
            }
        }
        else if(p1== '='){
            in.get();
            return LexItem(CCATA, ".=", linenum);
        }
        else
        return LexItem(CAT,".",linenum);
    }

    if(c=='*'){
        if(in.peek()=='*'){
            in.get();
            return LexItem(EXPONENT,"**",linenum);
        }
        return LexItem(MULT,"*",linenum);
    }

    if(c=='='){
        if(in.peek() == '='){
            in.get();
            return LexItem(NEQ, "==", linenum);
        }
        return LexItem(ASSOP, "=",linenum);
    }

    if(c=='>'){
        if(in.peek()=='='){
            in.get();
            return LexItem(NGTE, ">=",linenum);
        }
        return LexItem(ERR, ">", linenum);
    }

    if(c=='<'){
        return LexItem(NLT,"<",linenum);
    }

    if(c=='&'){
        if(in.peek()=='&'){
            in.get();
            return LexItem(AND, "&&",linenum);
        }
        return LexItem(ERR, "&",linenum);
    }

    if(c=='|'){
        if(in.peek()=='|'){
            in.get();
            return LexItem(OR,"||",linenum);
        }
        return LexItem(ERR, "|",linenum);
    }

    if(c=='+'){
        if(in.peek()=='='){
            in.get();
            return LexItem(CADDA,"+=",linenum);
        }
        return LexItem(PLUS,"+",linenum);
    }

    if(c=='-'){
        if(in.peek()=='='){
            in.get();
            return LexItem(CSUBA, "-=",linenum);
        }
        return LexItem(MINUS, "-",linenum);
    }

    if(c=='/'){
        return LexItem(DIV,"/",linenum);
    }
    if(c=='%') 
    return LexItem(REM, "%",linenum);
    if(c=='!') 
    return LexItem(NOT, "!",linenum);

    if(c=='@'){
        int a=in.get();
        int b=in.get();
        string s = "@";
        if(a != EOF)
        s+=static_cast<char>(a);
        if(b != EOF)
        s+=static_cast<char>(b);
        if(a==EOF||b==EOF){
            return LexItem(ERR,s,linenum);
        }
        char a1 = static_cast<char>(tolower(static_cast<unsigned char>(a)));
        char b1 = static_cast<char>(tolower(static_cast<unsigned char>(b)));
        if(a1 == 'l' && b1 == 'e')
        return LexItem(SLTE, s,linenum);
        if(a1=='g'&& b1 == 't')
        return LexItem(SGT,s,linenum);
        if(a1 == 'e' && b1=='q')
        return LexItem(SEQ, s,linenum);

        return LexItem(ERR, "@", linenum);
    }

    //delimiters
    if(c==',')
    return LexItem(COMMA, ",",linenum);
    if(c==';')
    return LexItem(SEMICOL, ";",linenum);
    if(c=='(')
    return LexItem(LPAREN, "(",linenum);
    if(c==')')
    return LexItem(RPAREN, ")",linenum);
    if(c=='{')
    return LexItem(LBRACES,"{",linenum);
    if(c=='}')
    return LexItem(RBRACES, "}",linenum);

    return LexItem(ERR, string(1,c),linenum);
}