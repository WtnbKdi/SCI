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
#define MAX_LINE_SIZE 2000 // �v���O�����̍ő�s���e�l
#define LINE_SIZE 255      // �v���O������s�̍ő勖�e�l

using namespace std;

// �g�[�N���̎�� + 25
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
	FuncDec_INT, // �֐���`
	FuncDec_DBL,
	FuncDec_CHR,
	FuncCall,   // �֐��Ăяo��
	For,        // for��
	Do,         // dowhile�� Do
	While,      // while��
	If,         // if��
	Else,       // else��
	ElseIf,     // else-if��
	Return,     // Return��
	Break,      // Break��
	Continue,   // Continue��
	GVar,       // �O���[�o���ϐ�
	LVar,       // ���[�J���ϐ�
	VarDec_INT, // �ϐ��錾
	VarDec_DBL,
	VarDec_CHR,
	Sharp,	    // #
	Include,    // #include (��������)
	Ident,      // �g�[�N������
	IntNum,     // int�^�̒l
	DblNum,     // double�^�̒l
	String,     // ������
	Letter,     // a~z, _�̕���
	Digit,		// 0~9
	Int,        // int�^
	Char,       // char�^
	Double,     // double�^
	Void,       // void�^
	EofTkn,     // �g�[�N���I�[
	Others,     // �Ή����Ă��Ȃ��g�[�N��
	END_list,   // �I�[
	EOF_Prg,    // �v���O�����I��
	EOF_Line,   // �v���O������s�I��
	END_KeyList,// �L�[���[�h���X�g�I��
};

// �g�[�N�����
struct TokenInfo {
	TokenKind   TK;	     // �g�[�N���̎�� 
	string      Text;	 // �g�[�N�������� 
	int      Num;	 // �萔�l
	TokenInfo() : TK(Others), Text(""), Num(0) {};
	TokenInfo(TokenKind tk_, const string& str_ = "", double num_ = 0.0)
		: TK(tk_), Text(str_), Num(num_) {};
};


// �^��
enum DtType {
	NON_Type,
	INT_Type,
	DBL_Type,
	CHR_Type,
	VOD_Type
};

// �L���\�̓o�^���̎��
enum SymbolKind {
	NoneID,
	VarID,
	FuncID,
	ParaID,
};

// �L���\
struct SymbolTbl {
	string  Name;               // �ϐ���֐��̖��O  
	SymbolKind SymbolKind;      // ���                 
	char    VarType;            // �ϐ��^  
	int     AryLeng;            // �z��T�C�Y 0�͒P���ϐ�   
	short   Args;               // �֐��̈����T�C�Y       
	int     Addr;               // �ϐ��Ɗ֐��̔Ԓn      
	int     FrameSize;          // �֐��p�̃t���[���T�C�Y 
	int     ScopeCntr;          // ���ڂ̃X�R�[�v�ɑ������ϐ����̒l

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

// �R�[�h�Ǘ�
struct CodeManager {
	TokenKind Kind;              // ���                      
	const char *Text;			 // �����񃊃e�����̂Ƃ��̈ʒu
	int IntVal;               // int�^�̐��l�萔�̂Ƃ��̒l
	int    SymbolNum;            // �L���\�̓Y���ʒu           
	int    JmpAddr;              // ���Ԓn   
	int	   ScopeCntr;            // ���ڂ̃X�R�[�v�ɑ����Ă��邩

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

// �^���t��obj 
struct TypeObj {
	char Type; // �i�[�^ 'd':double 's':string '-':����
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


// ������
class Mymemory {
private:
	vector<int> memo;
public:
	void Resize(int n) {                 // �Ċm�ۉ񐔗}���̂��ߑ��߂Ɋm�� 
		if (n >= (int)memo.size()) {
			n = (n / 256 + 1) * 256;
			memo.resize(n);
		}
	}
	void clear() { memo.clear(); };
	void set(int adrs, int dt) { memo[adrs] = dt; }            // ���������� 
	void add(int adrs, int dt) { memo[adrs] += dt; }            // ���������Z 
	double get(int adrs) { return memo[adrs]; }                // �������Ǐo 
	int size() { return (int)memo.size(); }          // �i�[�T�C�Y 
	void resize(int n) { memo.resize(n); }                // �T�C�Y�m�� 
};