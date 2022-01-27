#pragma once

#include "SCI.h"
#include "Prototype.h"

TokenKind TkTbl[256];                     // ������\       
TokenInfo TKInfo;                         // �g�[�N���i�[  
ifstream  Stream;                         // ���̓X�g���[�� 
int       SrcLineNo;                      // �\�[�X�s�ԍ�
bool      EndFile_Flag;                   // �t�@�C���I��
char      Buffer[LINE_SIZE + 5];          // �\�[�X�Ǎ��o�b�t�@ 
char      *TokenP;                        // �v���O������s�̐擪����
int       OneBackTokenCnt = 0;            // �g�[�N���̊����߂���
int       TwoBackTokenCnt = 0;            // �g�[�N���̊����߂���
int       ThreeBackTokenCnt = 0;          // �g�[�N���̊����߂���

struct KeyWord {                          // ����           
	const char *KeyName;
	TokenKind KeyKind;                    // ���           
};

// ���䕪, ���Z�q�\
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

// �g�[�N���\������
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

// ���̃v���O�����̈�s���擾

void NextLine()
{
	if (EndFile_Flag) return;

	if (Stream.eof()) {
		Stream.clear();
		Stream.close();
		EndFile_Flag = 1;
		return;
	}

	Stream.getline(Buffer, LINE_SIZE + 5); // ��s��ǂݍ���

	if (strlen(Buffer) > LINE_SIZE) {
		ErrExit("1�s ", LINE_SIZE, " �����ȓ��܂łł��B");
	}
	if (++SrcLineNo > MAX_LINE_SIZE) {
		ErrExit("�v���O������ ", MAX_LINE_SIZE, " �s�܂łł��B");
	}
	TokenP = Buffer;
}

// �k�������ł����ture

bool IsNullChar(char c_)
{
	return c_ == '\0';
}

// �񕶎��̉��Z�q�ł����true

bool IsMultiOpe(int c1, int c2)
{
	if (IsNullChar(c1) || IsNullChar(c2)) return false;
	char s[] = { ' ', c1, c2, '\0' };
	return strstr(" && || ++ -- += -= *= /= %= <= >= == != // ", s) != NULL;
}

// �g�[�N����Ԃ�
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

// �\�[�X�R�[�h���J��

void FileOpen(string _srcdir)
{
	SrcLineNo = 0;
	EndFile_Flag = false;
	Stream.open(_srcdir.c_str());
	if (!Stream) {
		cout << _srcdir.c_str() << "���J�����Ƃ��o���܂���ł���\n" << endl;
		Stream.close();
		exit(1);
	}
}

// NextToken()��������߂�
void OneBackNextToken()
{
	TokenP = TokenP - OneBackTokenCnt;
}

// NextToken()��2�����߂�
void TwoBackNextToken()
{
	TokenP = TokenP - TwoBackTokenCnt;
}

// NextToken()��3�����߂�
void ThreeBackNextToken()
{
	TokenP = TokenP - ThreeBackTokenCnt;
}

// �g�[�N�����擾
TokenInfo NextToken()
{
	TokenKind tk;
	TokenInfo chkelifTK; // else if �m�F�p
	static TokenKind oneBackTK = Others; // �ЂƂO�̃g�[�N�����
	int savech, num = 0;
	string txt = "";
	ThreeBackTokenCnt = TwoBackTokenCnt;
	TwoBackTokenCnt = OneBackTokenCnt;
	OneBackTokenCnt = 0;

	if (EndFile_Flag) // �\�[�X�R�[�h��S�ēǂݏI������EOF_Prg��Ԃ� 
		return TokenInfo(EOF_Prg);

	while (isspace(*TokenP)) {
		++TokenP;
		++OneBackTokenCnt;
		++TwoBackTokenCnt;
		++ThreeBackTokenCnt;
	}

	if (*TokenP == '\0') // ��s�ǂݏI������EOF_Line��Ԃ�
		return TokenInfo(EOF_Line);

	switch (TkTbl[*TokenP]) {
		case Sharp:
			txt += *TokenP;
			++TokenP;
			++OneBackTokenCnt;
			++TwoBackTokenCnt;
			++ThreeBackTokenCnt;
		case Letter: // �����̏ꍇ   
			while (TkTbl[*TokenP] == Letter || TkTbl[*TokenP] == Digit) {
				txt += *TokenP;
				++TokenP;
				++OneBackTokenCnt;
				++TwoBackTokenCnt;
				++ThreeBackTokenCnt;
			}
			break;

		case Digit:  // ���l�̏ꍇ   
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
				ErrExit("�����_�̎����l�ɂȂ��Ă��܂���");
			}
			while (TkTbl[*TokenP] == Digit) {
				txt += *TokenP;
				++TokenP;
				++OneBackTokenCnt;
				++TwoBackTokenCnt;
				++ThreeBackTokenCnt;
			}

			return TokenInfo(tk, txt, atof(txt.c_str()));

		case DblQ:   // �_�u���N�I�[�g�̏ꍇ  
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
				cout << "�����񃊃e���������Ă��܂���\n";
				exit(1);
			}
			++TokenP;
			++OneBackTokenCnt;
			++TwoBackTokenCnt;
			++ThreeBackTokenCnt;
			return TokenInfo(String, txt);

		case Addr: //�A�h���X�̏ꍇ
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

		case Multi: //*�̏ꍇ
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
			// �t�H�[���X���[

		default: // & * �ȊO�̋L���̏ꍇ  
			txt += *TokenP; // <
			++TokenP;  // =
			++OneBackTokenCnt;
			++TwoBackTokenCnt;
			++ThreeBackTokenCnt;
			if (IsMultiOpe(*(TokenP-1), *TokenP)) { // �ЂƂO�̉��Z�q(<)�ƌ��݂̉��Z�q(=) 
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
		cout << "�s���ȃg�[�N���ł�: " << txt << endl;
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

TokenInfo NextLineToken() // ���̍s��ǂݎ��̃g�[�N����Ԃ� 
{
	NextLine();
	return NextToken();
}

// �m�F���g�[�N�����擾
TokenInfo CheckNextToken(const TokenInfo& tk_, int kind_)
{
	if (tk_.TK != kind_) {
		ErrExit(ErrMsg(tk_.Text, KindToStr(kind_)));
		
	}
	return NextToken();
}

// ��ނ��當����֕ϊ�
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

int GetLineNo() // �Ǎ�or���s���̍s�ԍ�
{
	extern int PC;
	return (PC == -1) ? SrcLineNo : PC; 
}

void run(string str)
{
	FileOpen(str);

	cout << "������   ��� �l\n";

	InitTkTable(); // �g�[�N���\��������

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