#pragma once

#include "SCI.h"


// SCI_Token
TokenKind GetTokenKind(string);
TokenInfo NextToken();
TokenInfo NextLineToken();
TokenInfo CheckNextToken(const TokenInfo&, int);
string    KindToStr(int);
string	  KindToStr(int);
string    KindToStr(const CodeManager&);
void      InitTkTable();
void	  NextLine();
void      FileOpen(string);
void      run(string);
void	  OneBackNextToken();
void	  TwoBackNextToken();
void      ThreeBackNextToken();
bool      IsNullChar(char);
bool      IsMultiOpe(int, int);
int       GetLineNo();

// SCI_Pars
void Ini();
bool IsLocalScope();
void ConvToItrCode(const char*);
void Conv();
void NoCheckSetName(const char*);
void SetName(const char*);
void SetName();
void PushItrCd();
void ConvBlockCall();
void ConvBlockCall(bool&, bool&);
void ConvBlock();
void ConvRest();
void ConvRest(bool&, bool&);
void ConvExpr();
void VarDecl(TokenKind);
void FuncDecl(TokenKind);
void BackPatch(int, int);
void BackPatch2(int, int);
void SetCode(int);
void SkipSpaceLine();
int  SetCode(int, int);
void SetCodeRest();
void SetCodeEnd();
void SetCodeEof();
void VarNameCheck(TokenInfo tk_);
void SetAryLeng();

// SCI_EtcFuncs
string ErrMsg(const string&, const string&);
string DblToStr(double);
void   ErrExit(TypeObj fst1_ = "\1", TypeObj fst2_ = "\1", TypeObj fst3_ = "\1", TypeObj fst4_ = "\1");


// SCI_Code
void LookIntrCode();
CodeManager FirstCode(int);
CodeManager NextCode();
CodeManager NextLookCode();
int CheckNest(const CodeManager& cm_);
CodeManager CheckNextCode(const CodeManager&, int);
TokenKind LookCode(int);
int GetMemAddr(const CodeManager&);
int GetTopAddr(const CodeManager&);
int OpOrder(TokenKind);
void ForExpr3Skip(int);
int GetExpression(int kind1_ = 0, int kind2_ = 0);
void SyntaxCheck();
void StartPrgCntr(int);
void CheckEofLine();
int EndlineOfIf(int);
int EndlineOfIf_Etc(int);
int EndlineOfIf_Last(int);
void FuncCallSyntax(int);
void IfWhileSyntax();
void ForSyntax();
void Expression(int, int);
void Expression();
void Term(int);
void Factor();
void sysFuncExe(TokenKind);
void CheckDtType(const CodeManager&);
void FncCall(int);
void FncExec(int);
void SetDtType(const CodeManager&, char);
void Block();
void BinaryExpr(TokenKind);
void Statement();
void Execute();
int SetLITERAL(double);
int SetLITERAL(const string&);
void PrintfFunc(string, int*, int);

// SCI_Table
vector<SymbolTbl>::iterator TableP(const CodeManager&);
int  SearchName(const string&, int); 
bool IsLocalName(const string&, SymbolKind); 
void SetStartLTbl();
int  Enter(SymbolTbl&, SymbolKind);
