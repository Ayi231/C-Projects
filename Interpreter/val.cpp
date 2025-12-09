// val.cpp
#include "val.h"

#include <cmath>
#include <sstream>
#include <iomanip>

using namespace std;

static double StringToDoubleStrict(const string &s) {
    // convert string to double; throw on failure
    try {
        size_t idx = 0;
        double v = stod(s, &idx);
        if (idx != s.size()) throw string("bad");
        return v;
    } catch(...) {
        throw string("Illegal opernad type for the operation.");
    }
}

static double ToNumStrict(const Value &v) {
    if (v.IsNum()) return v.GetNum();
    if (v.IsString()) return StringToDoubleStrict(v.GetString());
    throw string("Illegal Relational operation.");
}

static string NumToString(double d) {
    if (std::isfinite(d) && floor(d) == d) {
        // integer
        long long iv = (long long) d;
        return to_string(iv);
    } else {
        ostringstream ss;
        ss << fixed << setprecision(1) << d;
        return ss.str();
    }
}

static string ToStringLenient(const Value &v) {
    if (v.IsString()) return v.GetString();
    if (v.IsNum()) return NumToString(v.GetNum());
    if (v.IsBool()) return v.GetBool() ? string("true") : string("false");
    throw string("Illegal Relational operation.");
}

static bool Truthiness(const Value &v) {
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

/* -----------------------
   Numeric operators
   ----------------------- */

Value Value::operator+(const Value& op) const {
    if (this->IsBool() || op.IsBool()) throw string("Run-Time Error-Illegal Mixed Type Operands");
    double a = ToNumStrict(*this);
    double b = ToNumStrict(op);
    return Value(a + b);
}

Value Value::operator-(const Value& op) const {
    if (this->IsBool() || op.IsBool()) throw string("Run-Time Error-Illegal Mixed Type Operands");
    double a = ToNumStrict(*this);
    double b = ToNumStrict(op);
    return Value(a - b);
}

Value Value::operator*(const Value& op) const {
    if (this->IsBool() || op.IsBool()) throw string("Run-Time Error-Illegal Mixed Type Operands");
    double a = ToNumStrict(*this);
    double b = ToNumStrict(op);
    return Value(a * b);
}

Value Value::operator/(const Value& op) const {
    if (this->IsBool() || op.IsBool()) throw string("Run-Time Error-Illegal Mixed Type Operands");
    double a = ToNumStrict(*this);
    double b = ToNumStrict(op);
    if (fabs(b) < 1e-12) throw string("Illegal operand type or value for the division operation.");
    return Value(a / b);
}

Value Value::operator%(const Value& op) const {
    if (IsBool() || op.IsBool())
        throw string("Illegal operand type for the operation.");

    double leftNum = ToNumStrict(*this);
    double rightNum = ToNumStrict(op);
    int a = (int)leftNum;
    int b = (int)rightNum;

    if (b == 0)
        throw string("Illegal operand type or value for the division operation.");

    // Special-case
    if (IsString())
        return Value((double)a);

    int r = a % b;
    return Value((double)r);
}

/* -----------------------
   Numeric relational / equality (operate on numeric types)
   If operands are strings, they are converted to numeric.
   ----------------------- */

Value Value::operator==(const Value& op) const {
    if (this->IsBool() || op.IsBool()) throw string("Illegal Relational operation.");
    double a = ToNumStrict(*this);
    double b = ToNumStrict(op);
    return Value(fabs(a - b) < 1e-12);
}

Value Value::operator>=(const Value& op) const {
    if (this->IsBool() || op.IsBool()) throw string("Illegal Relational operation.");
    double a = ToNumStrict(*this);
    double b = ToNumStrict(op);
    return Value(a >= b);
}

Value Value::operator<(const Value& op) const {
    if (this->IsBool() || op.IsBool()) throw string("Illegal Relational operation.");
    double a = ToNumStrict(*this);
    double b = ToNumStrict(op);
    return Value(a < b);
}

/* -----------------------
   Exponentiation (numeric)
   ----------------------- */

Value Value::Expon(const Value& oper) const {
    if (this->IsBool() || oper.IsBool()) throw string("Illegal Relational operation.");
    double base = ToNumStrict(*this);
    double exp = ToNumStrict(oper);
    double res = pow(base, exp);
    return Value(res);
}

/* -----------------------
   String operations
   ----------------------- */

Value Value::Catenate(const Value& oper) const {
    if (this->IsBool() || oper.IsBool()) throw string("Run-Time Error-Illegal Mixed Type Operands");
    string a = ToStringLenient(*this);
    string b = ToStringLenient(oper);
    return Value(a + b);
}

Value Value::Repeat(const Value& oper) const {
    if (this->IsBool() || oper.IsBool()) throw string("Run-Time Error-Illegal Mixed Type Operands");
    if (!this->IsString()) {
        throw string("Illegal operand type for the string repetition operation.");
    }
    double times_d;
    if (oper.IsNum()) times_d = oper.GetNum();
    else if (oper.IsString()) {
        times_d = StringToDoubleStrict(oper.GetString());
    } else {
        throw string("Illegal Relational operation.");
    }
    int times = (int) times_d;
    if (times < 0) throw string("Run-Time Error-Illegal String Repetition Operand");
    string out;
    for (int i = 0; i < times; ++i) out += this->GetString();
    return Value(out);
}

/* -----------------------
   String relational operators
   ----------------------- */

Value Value::SEQ(const Value& oper) const {
    if (!this->IsString() || !oper.IsString()) {
        throw string("Illegal Relational operation.");
    }
    return Value(this->GetString() == oper.GetString());
}

Value Value::SGT(const Value& oper) const {
    if (!this->IsString() || !oper.IsString()) throw string("Illegal Relational operation.");
    return Value(this->GetString() > oper.GetString());
}

Value Value::SLE(const Value& oper) const {
    if (!this->IsString() || !oper.IsString()) throw string("Illegal Relational operation.");
    return Value(this->GetString() < oper.GetString());
}

/* -----------------------
   Logical operators
    return boolean Values.
   ----------------------- */

Value Value::operator&&(const Value& op) const {
    bool a = Truthiness(*this);
    bool b = Truthiness(op);
    return Value(a && b);
}

Value Value::operator||(const Value& op) const {
    bool a = Truthiness(*this);
    bool b = Truthiness(op);
    return Value(a || b);
}

Value Value::operator!() const {
    bool a = Truthiness(*this);
    return Value(!a);
}
