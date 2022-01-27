#pragma once

#include <iostream>
#include <fstream>    
#include <sstream>    
#include <string>
#include <vector>
#include <stack>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <cctype>
#include <iomanip>
#include <algorithm>
#include <stdarg.h>
#include <algorithm>
#define MAX_LINE_SIZE 2000 // プログラムの最大行許容値
#define LINE_SIZE 255      // プログラム一行の最大許容値

using namespace std;

// トークンの種類 + 25
enum TokenKind {
	LParen = '(', 
	RParen = ')', 
	Plus   = '+', 
	Minus  = '-', 
	Multi  = '*', 
	Addr   = '&',
	Divi   = '/', 
	Mod    = '%', 
	Assign = '=', 
	Not    = '!', 
	Comma  = ',', 
	SemiC  = ';', 
	DblQ   = '"',
	Less   = '<',
	Great  = '>',
	LBlace = '{',
	RBlace = '}',
	LSBracket = '[',
	RSBracket = ']',
	Equal = 200,
	NotEq,
	LessEq, 
	GreatEq,
	PlusEq,
	MinusEq,
	MultiEq,
	DiviEq,
	ModEq,
	Inc,
	Dec,
	DblAnd,
	DblOr,   
	Pointer,
	Sizeof,     // sizeof
	Printf,
	Scanf,
	Coment,     // //
	FuncDec_INT, // 関数定義
	FuncDec_DBL,
	FuncDec_CHR,
	FuncCall,   // 関数呼び出し
	For,        // for文
	Do,         // dowhile文 Do
	While,      // while文
	If,         // if文
	Else,       // else文
	ElseIf,     // else-if文
	Return,     // Return文
	Break,      // Break文
	Continue,   // Continue文
	GVar,       // グローバル変数
	LVar,       // ローカル変数
	VarDec_INT, // 変数宣言
	VarDec_DBL,
	VarDec_CHR,
	Sharp,	    // #
	Include,    // #include (無視する)
	Ident,      // トークン文字
	IntNum,     // int型の値
	DblNum,     // double型の値
	String,     // 文字列
	Letter,     // a~z, _の文字
	Digit,		// 0~9
	Int,        // int型
	Char,       // char型
	Double,     // double型
	Void,       // void型
	EofTkn,     // トークン終端
	Others,     // 対応していないトークン
	END_list,   // 終端
	EOF_Prg,    // プログラム終了
	EOF_Line,   // プログラム一行終了
	END_KeyList,// キーワードリスト終了
};

// トークン情報
struct TokenInfo {
	TokenKind   TK;	     // トークンの種類 
	string      Text;	 // トークン文字列 
	int      Num;	 // 定数値
	TokenInfo() : TK(Others), Text(""), Num(0) {};
	TokenInfo(TokenKind tk_, const string& str_ = "", double num_ = 0.0)
		: TK(tk_), Text(str_), Num(num_) {};
};


// 型名
enum DtType {
	NON_Type,
	INT_Type,
	DBL_Type,
	CHR_Type,
	VOD_Type
};

// 記号表の登録名の種類
enum SymbolKind {
	NoneID,
	VarID,
	FuncID,
	ParaID,
};

// 記号表
struct SymbolTbl {
	string  Name;               // 変数や関数の名前  
	SymbolKind SymbolKind;      // 種類                 
	char    VarType;            // 変数型  
	int     AryLeng;            // 配列サイズ 0は単純変数   
	short   Args;               // 関数の引数サイズ       
	int     Addr;               // 変数と関数の番地      
	int     FrameSize;          // 関数用のフレームサイズ 
	int     ScopeCntr;          // 何個目のスコープに属した変数かの値

	SymbolTbl()
	{
		Clear();
	}

	void Clear()
	{
		Name = "";
		SymbolKind = NoneID;
		VarType = NON_Type;
		AryLeng = 0;
		Args = 0;
		Addr = 0;
		FrameSize = 0;
	}
};

// コード管理
struct CodeManager {
	TokenKind Kind;              // 種類                      
	const char *Text;			 // 文字列リテラルのときの位置
	int IntVal;               // int型の数値定数のときの値
	int    SymbolNum;            // 記号表の添字位置           
	int    JmpAddr;              // 飛先番地   
	int	   ScopeCntr;            // 何個目のスコープに属しているか

	CodeManager() { clear(); }

	CodeManager(TokenKind tk_) 
	{ 
		clear(); 
		Kind = tk_;  
	}
	CodeManager(TokenKind tk_, int i_)
	{
		clear(); 
		Kind = tk_;
		IntVal = i_;
	}
	CodeManager(TokenKind tk_, const char *str_)
	{
		clear();
		Kind = tk_; 
		Text = str_;
	}
	CodeManager(TokenKind tk_, int symbol_, int jmp_)
	{
		clear();
		Kind = tk_; 
		SymbolNum = symbol_; 
		JmpAddr = jmp_;
	}
	void clear()
	{
		Kind = Others;
		Text = "";
		IntVal = 0.0;
		JmpAddr = 0;
		SymbolNum = -1;
	}
};

// 型情報付きobj 
struct TypeObj {
	char Type; // 格納型 'd':double 's':string '-':未定
	double Dbl;
	string Str;
	TypeObj()
	{
		Type = '-';
		Dbl = 0.0;
		Str = "";
	}

	TypeObj(double dbl_) : Type('d'), Dbl(dbl_), Str("") {}
	TypeObj(const string& str_) : Type('s'), Dbl(0.0), Str(str_) {}
	TypeObj(const char *str_) :Type('s'), Dbl(0.0), Str(str_) {}
};


// メモリ
class Mymemory {
private:
	vector<int> memo;
public:
	void Resize(int n) {                 // 再確保回数抑制のため多めに確保 
		if (n >= (int)memo.size()) {
			n = (n / 256 + 1) * 256;
			memo.resize(n);
		}
	}
	void clear() { memo.clear(); };
	void set(int adrs, int dt) { memo[adrs] = dt; }            // メモリ書込 
	void add(int adrs, int dt) { memo[adrs] += dt; }            // メモリ加算 
	double get(int adrs) { return memo[adrs]; }                // メモリ読出 
	int size() { return (int)memo.size(); }          // 格納サイズ 
	void resize(int n) { memo.resize(n); }                // サイズ確保 
};