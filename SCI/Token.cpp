#pragma once

#include "SCI.h"
#include "Prototype.h"

TokenKind TkTbl[256];                     // 文字種表       
TokenInfo TKInfo;                         // トークン格納  
ifstream  Stream;                         // 入力ストリーム 
int       SrcLineNo;                      // ソース行番号
bool      EndFile_Flag;                   // ファイル終了
char      Buffer[LINE_SIZE + 5];          // ソース読込バッファ 
char      *TokenP;                        // プログラム一行の先頭文字
int       OneBackTokenCnt = 0;            // トークンの巻き戻し数
int       TwoBackTokenCnt = 0;            // トークンの巻き戻し数
int       ThreeBackTokenCnt = 0;          // トークンの巻き戻し数

struct KeyWord {                          // 字句           
	const char *KeyName;
	TokenKind KeyKind;                    // 種別           
};

// 制御分, 演算子表
KeyWord KeyWordTbl[] = {
	{ "for" ,   For },
	{ "while" , While },
	{ "do",     Do },
	{ "if" ,    If },
	{ "else",   Else },
	{ "else if", ElseIf },
	{ "printf", Printf },
	{ "scanf",  Scanf },
	{ "return", Return },
	{ "continue", Continue },
	{ "break",  Break },
	{ "sizeof", Sizeof },
	{ "(",      LParen },
	{ ")",      RParen },
	{ "+",      Plus },
	{ "-",      Minus },
	{ "*",      Multi },
	{ "%",      Mod },
	{ "&",      Addr },
	{ "!",      Not },
	{ "/",      Divi },
	{ "=",      Assign },
	{ ",",      Comma },
	{ ";",      SemiC },
	{ "==",     Equal },
	{ "!=",     NotEq },
	{ "+=",     PlusEq },
	{ "-=",     MinusEq },
	{ "++",     Inc },
	{ "--",     Dec },
	{ "*=",     MultiEq },
	{ "/=",     DiviEq },
	{ "%=",     ModEq },
	{ "<",      Less },
	{ "<=",     LessEq },
	{ ">",      Great },
	{ ">=",     GreatEq },
	{ "&&",     DblAnd },
	{ "||",     DblOr },
	{ "{",      LBlace },
	{ "}",      RBlace },
	{ "[",      LSBracket },
	{ "]",      RSBracket },
	{ "int",    Int },
	{ "double", Double },
	{ "char",   Char },
	{ "void",   Void },
	{ "//",		Coment },
	{ "#",      Sharp },
	{ "#include", Include },
	{ "",       END_list },
};

// トークン表初期化
void InitTkTable()
{
	for (int i = 0; i<256; ++i) { TkTbl[i] = Others; }
	for (int i = '0'; i <= '9'; ++i) { TkTbl[i] = Digit; }
	for (int i = 'A'; i <= 'Z'; ++i) { TkTbl[i] = Letter; }
	for (int i = 'a'; i <= 'z'; ++i) { TkTbl[i] = Letter; }
	TkTbl['('] = LParen;
	TkTbl[')'] = RParen;
	TkTbl['<'] = Less;
	TkTbl['>'] = Great;
	TkTbl['+'] = Plus;
	TkTbl['-'] = Minus;
	TkTbl['*'] = Multi;
	TkTbl['/'] = Divi;
	TkTbl['_'] = Letter;
	TkTbl['='] = Assign;
	TkTbl['&'] = Addr;
	TkTbl['!'] = Not;
	TkTbl['%'] = Mod;
	TkTbl[','] = Comma;
	TkTbl[';'] = SemiC;
	TkTbl['{'] = LBlace;
	TkTbl['}'] = RBlace;
	TkTbl['['] = LSBracket;
	TkTbl[']'] = RSBracket;
	TkTbl['"'] = DblQ;
	TkTbl['#'] = Sharp;
}

// 次のプログラムの一行を取得

void NextLine()
{
	if (EndFile_Flag) return;

	if (Stream.eof()) {
		Stream.clear();
		Stream.close();
		EndFile_Flag = 1;
		return;
	}

	Stream.getline(Buffer, LINE_SIZE + 5); // 一行を読み込む

	if (strlen(Buffer) > LINE_SIZE) {
		ErrExit("1行 ", LINE_SIZE, " 文字以内までです。");
	}
	if (++SrcLineNo > MAX_LINE_SIZE) {
		ErrExit("プログラムは ", MAX_LINE_SIZE, " 行までです。");
	}
	TokenP = Buffer;
}

// ヌル文字であればture

bool IsNullChar(char c_)
{
	return c_ == '\0';
}

// 二文字の演算子であればtrue

bool IsMultiOpe(int c1, int c2)
{
	if (IsNullChar(c1) || IsNullChar(c2)) return false;
	char s[] = { ' ', c1, c2, '\0' };
	return strstr(" && || ++ -- += -= *= /= %= <= >= == != // ", s) != NULL;
}

// トークンを返す
TokenKind GetTokenKind(string str_)
{
	for (int i = 0; KeyWordTbl[i].KeyKind != END_list; i++) {
		if (str_ == KeyWordTbl[i].KeyName) {
			return KeyWordTbl[i].KeyKind;
		}
	}
	if (TkTbl[str_[0]] == Letter) return Ident;
	if (TkTbl[str_[0]] == Digit)  return DblNum;
	return Others;
}

// ソースコードを開く

void FileOpen(string _srcdir)
{
	SrcLineNo = 0;
	EndFile_Flag = false;
	Stream.open(_srcdir.c_str());
	if (!Stream) {
		cout << _srcdir.c_str() << "を開くことが出来ませんでした\n" << endl;
		Stream.close();
		exit(1);
	}
}

// NextToken()を一つ巻き戻す
void OneBackNextToken()
{
	TokenP = TokenP - OneBackTokenCnt;
}

// NextToken()を2つ巻き戻す
void TwoBackNextToken()
{
	TokenP = TokenP - TwoBackTokenCnt;
}

// NextToken()を3つ巻き戻す
void ThreeBackNextToken()
{
	TokenP = TokenP - ThreeBackTokenCnt;
}

// トークンを取得
TokenInfo NextToken()
{
	TokenKind tk;
	TokenInfo chkelifTK; // else if 確認用
	static TokenKind oneBackTK = Others; // ひとつ前のトークン種類
	int savech, num = 0;
	string txt = "";
	ThreeBackTokenCnt = TwoBackTokenCnt;
	TwoBackTokenCnt = OneBackTokenCnt;
	OneBackTokenCnt = 0;

	if (EndFile_Flag) // ソースコードを全て読み終えたらEOF_Prgを返す 
		return TokenInfo(EOF_Prg);

	while (isspace(*TokenP)) {
		++TokenP;
		++OneBackTokenCnt;
		++TwoBackTokenCnt;
		++ThreeBackTokenCnt;
	}

	if (*TokenP == '\0') // 一行読み終えたらEOF_Lineを返す
		return TokenInfo(EOF_Line);

	switch (TkTbl[*TokenP]) {
		case Sharp:
			txt += *TokenP;
			++TokenP;
			++OneBackTokenCnt;
			++TwoBackTokenCnt;
			++ThreeBackTokenCnt;
		case Letter: // 文字の場合   
			while (TkTbl[*TokenP] == Letter || TkTbl[*TokenP] == Digit) {
				txt += *TokenP;
				++TokenP;
				++OneBackTokenCnt;
				++TwoBackTokenCnt;
				++ThreeBackTokenCnt;
			}
			break;

		case Digit:  // 数値の場合   
			tk = IntNum;
			oneBackTK = IntNum;
			while (TkTbl[*TokenP] == Digit) {
				txt += *TokenP;
				++TokenP;
				++OneBackTokenCnt;
				++TwoBackTokenCnt;
				++ThreeBackTokenCnt;
			}
			if (*TokenP == '.') {
				oneBackTK = tk = DblNum;
				txt += *TokenP;
				++TokenP;
				++OneBackTokenCnt;
				++TwoBackTokenCnt;
				++ThreeBackTokenCnt;
			}
			else {
				return TokenInfo(tk, txt, atof(txt.c_str()));
			}
			if (TkTbl[*TokenP] != Digit) {
				ErrExit("小数点の次が値になっていません");
			}
			while (TkTbl[*TokenP] == Digit) {
				txt += *TokenP;
				++TokenP;
				++OneBackTokenCnt;
				++TwoBackTokenCnt;
				++ThreeBackTokenCnt;
			}

			return TokenInfo(tk, txt, atof(txt.c_str()));

		case DblQ:   // ダブルクオートの場合  
			for (++TokenP, ++OneBackTokenCnt; *TokenP != EOF && *TokenP != '"'; ++TokenP, ++OneBackTokenCnt, ++TwoBackTokenCnt, ++ThreeBackTokenCnt) {
				if (*TokenP == (char)'\\') {
					char c = *++TokenP;
					if(c == (char)'n')
						txt += '\n';
					else if (c == (char)'a')
						txt += '\a';
					else if (c == (char)'t')
						txt += '\t';
					else if (c == (char)'a')
						txt += '\b';
					else if (c == (char)'\\')
						txt += '\\';
					else if (c == (char)'\'')
						txt += '\'';
					else if (c == (char)'\"')
						txt += '\"';
					else if (c == (char)'0')
						txt += '\0';
				}
				else {
					txt += *TokenP;
					continue;
				}

				++OneBackTokenCnt;
				++TwoBackTokenCnt;
				++ThreeBackTokenCnt;
			}
			if (*TokenP != '"') {
				cout << "文字列リテラルが閉じていません\n";
				exit(1);
			}
			++TokenP;
			++OneBackTokenCnt;
			++TwoBackTokenCnt;
			++ThreeBackTokenCnt;
			return TokenInfo(String, txt);

		case Addr: //アドレスの場合
			savech = *TokenP;
			txt += *TokenP;
			++TokenP;
			++OneBackTokenCnt;
			++TwoBackTokenCnt;
			++ThreeBackTokenCnt;
			if (TkTbl[*TokenP] == Addr) {
				txt += *TokenP;
				++TokenP;
				++OneBackTokenCnt;
				++TwoBackTokenCnt;
				++ThreeBackTokenCnt;
				oneBackTK = DblAnd;
				return TokenInfo(DblAnd, txt);
			}
			oneBackTK = Addr;
			return TokenInfo(Addr, txt);

		case Multi: //*の場合
			switch (oneBackTK) {
			case Letter:
			case Digit:
			case RParen:
			case RBlace:
				txt += *TokenP;
				++TokenP;
				++OneBackTokenCnt;
				++TwoBackTokenCnt;
				++ThreeBackTokenCnt;
				oneBackTK = Multi;
				return TokenInfo(Multi, txt);
			case Multi:
			case Int:
			case Char:
			case Void:
			case Double:
			case Assign:
			case Plus:
			case Minus:
			case Divi:
			case Mod:
			case LBlace:
			case LParen:
			case SemiC:
			case PlusEq:
			case MinusEq:
			case MultiEq:
			case DiviEq:
			case ModEq:
			case Great:
			case GreatEq:
			case Less:
			case LessEq:
			case DblAnd:
			case DblOr:
			case Comma:
				txt += *TokenP;
				++TokenP;
				++OneBackTokenCnt;
				++TwoBackTokenCnt;
				++ThreeBackTokenCnt;
				oneBackTK = Pointer;
				return TokenInfo(Pointer, txt);
			case Pointer:
				txt += *TokenP;
				++TokenP;
				++OneBackTokenCnt;
				++TwoBackTokenCnt;
				++ThreeBackTokenCnt;
				oneBackTK = Multi;
				return TokenInfo(Pointer, txt);
			}
			// フォールスルー

		default: // & * 以外の記号の場合  
			txt += *TokenP; // <
			++TokenP;  // =
			++OneBackTokenCnt;
			++TwoBackTokenCnt;
			++ThreeBackTokenCnt;
			if (IsMultiOpe(*(TokenP-1), *TokenP)) { // ひとつ前の演算子(<)と現在の演算子(=) 
				txt += *TokenP; //txt: <=
				++TokenP;
				++OneBackTokenCnt;
				++TwoBackTokenCnt;
				++ThreeBackTokenCnt;
			}
			break;
	}

	oneBackTK = tk = GetTokenKind(txt);

	if (tk == Others) {
		cout << "不正なトークンです: " << txt << endl;
		exit(1);
	}

	if (tk == Else) {
		if (NextToken().TK == If) {
			
			oneBackTK = tk = ElseIf;
			return TokenInfo(tk, txt += "if");
		}
		OneBackNextToken();
	}

	return TokenInfo(tk, txt);
}

TokenInfo NextLineToken() // 次の行を読み次のトークンを返す 
{
	NextLine();
	return NextToken();
}

// 確認しトークンを取得
TokenInfo CheckNextToken(const TokenInfo& tk_, int kind_)
{
	if (tk_.TK != kind_) {
		ErrExit(ErrMsg(tk_.Text, KindToStr(kind_)));
		
	}
	return NextToken();
}

// 種類から文字列へ変換
string KindToStr(int kind_)
{
	for (int i = 0;; ++i) {
		if (KeyWordTbl[i].KeyKind == END_KeyList) break;
		if (KeyWordTbl[i].KeyKind == kind_) return KeyWordTbl[i].KeyName;
	}
	return "none";
}

string KindToStr(const CodeManager& cm_) 
{
	switch (cm_.Kind)
	{
	case LVar:
	case GVar:
	case FuncCall:
		return TableP(cm_)->Name;
	case IntNum:
	case DblNum:
		return DblToStr(cm_.IntVal);
	case String:
		return string("\"") + cm_.Text + string("\"");
	case EOF_Line:
		return "";
	}

	return KindToStr(cm_.Kind);
}

int GetLineNo() // 読込or実行中の行番号
{
	extern int PC;
	return (PC == -1) ? SrcLineNo : PC; 
}

void run(string str)
{
	FileOpen(str);

	cout << "文字列   種類 値\n";

	InitTkTable(); // トークン表を初期化

	TokenInfo tk;

	while (!EndFile_Flag) {
		NextLine();
		while ((tk = NextToken()).TK != EOF_Line && !EndFile_Flag) {
			cout << left << setw(10) << tk.Text
				<< right << setw(3) << tk.TK
				<< " " << tk.Num << endl;
		}
	}
}