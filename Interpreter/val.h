#ifndef VALUE_H
#define VALUE_H

#include <iostream>
#include <string>
#include <queue>
#include <map>
#include <iomanip>
#include <stdexcept>
#include <cmath>
#include <sstream>

using namespace std;

enum ValType { VNUM, VSTRING, VBOOL, VERR };

class Value {
    ValType	T;
    bool    Btemp;
    double   Ntemp;
    string	Stemp;
    //string ErrMsg;
    
       
public:
    Value() : T(VERR), Btemp(false), Ntemp(0.0), Stemp("") {}
    Value(bool vb) : T(VBOOL), Btemp(vb), Ntemp(0.0), Stemp("") {}
    
    Value(double vr) : T(VNUM), Btemp(false), Ntemp(vr), Stemp("") {}
    Value(string vs) : T(VSTRING), Btemp(false), Ntemp(0.0), Stemp(vs) {}
    
    
    ValType GetType() const { return T; }
    bool IsErr() const { return T == VERR; }
    bool IsString() const { return T == VSTRING; }
    bool IsNum() const {return T == VNUM;}
    bool IsBool() const {return T == VBOOL;}
    
    
    string GetString() const { if( IsString() ) return Stemp; throw "RUNTIME ERROR: Value not a string"; }
    
    double GetNum() const { if( IsNum() ) return Ntemp; throw "RUNTIME ERROR: Value not an integer"; }
    
    bool GetBool() const {if(IsBool()) return Btemp; throw "RUNTIME ERROR: Value not a boolean";}
    
    //string GetErrMsg () const {if(IsErr()) return ErrMsg; throw "RUNTIME ERROR: Value not an Error";}
    
    void SetType(ValType type)
    {
    	T = type;
	}
		
	void SetNum(double val)
    {
    	Ntemp = val;
	}
	
	void SetString(string val)
    {
    	Stemp = val;
	}
	
	void SetBool(bool val)
    {
    	Btemp = val;
	}
	
	/*void SetErrMsg(string msg)
	{
		ErrMsg = msg;
	}*/
    // numeric overloaded add op to this
    Value operator+(const Value& op) const;
    
    // numeric overloaded subtract op from this
    Value operator-(const Value& op) const;
    
    // numeric overloaded multiply this by op
    Value operator*(const Value& op) const;
    
    // numeric overloaded divide this by op
    Value operator/(const Value& op) const;
    //numeric remaider of integer division of this by op
    Value operator%(const Value& op) const;
    //numeric overloaded equality operator of this with op
    Value operator==(const Value& op) const;
	//numeric overloaded greater than operator of this with op
	Value operator>=(const Value& op) const;
	//numeric overloaded less than operator of this with op
	Value operator<(const Value& op) const;
	
	//Numeric exponentiation operation this raised to the power of op 
	Value Expon(const Value& oper) const;
	
	//string concatenation operation of this with op
	Value Catenate(const Value& oper) const;
	//string repetition operation of this with op
	Value Repeat(const Value& oper) const;
	//string equality (-eq) operator of this with op
	Value SEQ(const Value& oper) const;
	//string greater than (-gt) operator of this with op
	Value SGT(const Value& oper) const;
	//string less than operator of this with op
	Value SLE(const Value& oper) const;
	
	//Logic overloaded Anding of this and op
	Value operator&&(const Value& op) const;
	//Logic overloaded Oring of this and op
	Value operator||(const Value& op) const;
	//Logic overloaded Not of this 
	Value operator!() const;
	    
    friend ostream& operator<<(ostream& out, const Value& op) {
        //if( op.IsNum()) out << fixed << showpoint << setprecision(1) << op.Ntemp;
        if( op.IsNum()) out << fixed << setprecision(1) << op.Ntemp;
		else if( op.IsString() ) out <<  op.Stemp ;
        else if(op.IsBool()) out << (op.GetBool()? "true" : "false");
        else out << "ERROR";
        return out;
    }
};


#endif
