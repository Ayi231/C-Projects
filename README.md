Implementation of the parts of a compiler for a BPL-language in C++. Here are the grammar rules:
1. Prog ::= StmtList
2. StmtList ::= Stmt; { Stmt; }
3. Stmt ::= IfStmt | AssignStmt | PrintLnStmt
4. PrintLnStmt ::= PRINTLN (ExprList)
5. IfStmt ::= IF (Expr) ‘{‘ StmtList ‘}’ [ ELSE ‘{‘ StmtList ‘}’ ]
6. Var ::= IDENT
7. ExprList ::= Expr { , Expr }
8. AssignStmt ::= Var AssigOp Expr
9. Expr ::= OrExpr
10. AssigOp ::= ( = | += | -= | .= )
11. OrExpr ::= AndExpr { || AndExpr }
12. AndExpr ::= RelExpr { && RelExpr }
13. RelExpr ::= AddExpr [ ( @le | @gt | @eq | < | >= | == ) AddExpr ]
14. AddExpr :: MultExpr { ( + | - | . ) MultExpr }
15. MultExpr ::= UnaryExpr { ( * | / | % | .x. ) UnaryExpr }
16. UnaryExpr ::= [( - | + | ! )] ExponExpr
17. ExponExpr ::= PrimaryExpr { ** PrimaryExpr }
18. PrimaryExpr ::= IDENT | ICONST | FCONST | SCONST | (Expr)

This is the precedence level of operators (important for parser/interpreter):
<img width="629" height="289" alt="image" src="https://github.com/user-attachments/assets/8f58ab86-6032-46a9-9a44-e6c66f24ee59" />
