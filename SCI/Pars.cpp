#pragma once

#include "SCI.h"
#include "Prototype.h"

#define NO_DEFINE_ADDR 0        // �Ԓn����

TokenInfo CurTk;                // �������̃g�[�N��
SymbolTbl TmpSyTbl;             // �ꎞ�I�i�[�L���\
int BlockNest_Num;              // �u���b�N�[�x
int Local_Addr;                 // ���[�J���ϐ��A�h���X
int MainTbl_Num;                // ���C���֐��L���\�ʒu
int LoopNest_Num;		        // ���[�v�l�X�g
bool InFunc_Flag;               // �֐����ŏ������ł��邩
char CodeBuffer[LINE_SIZE + 1]; // �����R�[�h�����o�b�t�@
char *CodeBuffer_Ptr;			// �����R�[�h�����o�b�t�@�̃|�C���^
extern vector<char*> InterCode;             // �ϊ�������R�[�h
vector<int> GVarIni;
char *Token_Ptr = nullptr;
bool FuncLBlace_Flag;           // �֐��� ( ) �R�R �� { �����邩
extern vector<SymbolTbl> GTbl;

// ������
void Ini()
{
	GTbl.clear();
	Token_Ptr = nullptr;
	memset(CodeBuffer, 0, LINE_SIZE + 1);
	InterCode.clear();
	GVarIni.clear();
	GVarIni.resize(1);
	InitTkTable(); // ������ޕ\��������
	MainTbl_Num = -1;
	BlockNest_Num = LoopNest_Num = 0;
	InFunc_Flag = false;
	FuncLBlace_Flag = false;
	CodeBuffer_Ptr = CodeBuffer;
}

// ���[�J���X�R�[�v���ł���->ture
bool IsLocalScope()
{
	return InFunc_Flag;
}

// �󔒂̍s���X�L�b�v
void SkipSpaceLine()
{
	while (CurTk.TK == EOF_Line) {
		PushItrCd();
		CurTk = NextLineToken();
	}
}

// �R�[�h��ϊ�
void ConvToItrCode(const char* _filename)
{
	string fName; // �֐������i�[
	Ini(); // ������
	FileOpen(_filename); // �\�[�X�t�@�C�����J��
	// ��Ɋ֐������L���\�ɓo�^���Ă���
	while (CurTk = NextLineToken(), CurTk.TK != EOF_Prg) {
		switch (CurTk.TK) { // [int] func ()
			case Int:
			case Char:
			case Double:
			case Void:
				CurTk = NextToken();
				if (CurTk.TK != Ident)  // int [func] ()
					continue;
				fName = CurTk.Text;
				CurTk = NextToken();
				if (CurTk.TK != LParen)  // int func [(])
					continue;
				NoCheckSetName(fName.c_str()); // TmpSymTbl�ɖ��O�����ۑ�
				Enter(TmpSyTbl, FuncID); // �L���\�ɓo�^
				break;
		}
	}
	
	// �����R�[�h�֕ϊ�
	PushItrCd();
	FileOpen(_filename);
	// ���̈�s�̐擪�̃g�[�N�����擾
	CurTk = NextLineToken();
	while (CurTk.TK != EOF_Prg) Conv();
	// main�֐�������Ƃ��̐ݒ菈��
	if (MainTbl_Num != -1) {
		StartPrgCntr(InterCode.size());
		SetCode(FuncCall, MainTbl_Num);
		SetCode('(');
		SetCode(')');
		PushItrCd();
	}
	else {
		StartPrgCntr(1);
	}
}

// �擪�̃R�[�h������, �c��̕�����ConvRest�ŏ���

void Conv()
{
	TokenKind tmpTK;
	bool lblaceFlg = false, semicFlg = false;
	// �ϐ���`, �֐���`�ł��邩���ׂ�
	switch (CurTk.TK) {
		case Int:
		case Double:
		case Char:
			tmpTK = CurTk.TK; // �^���
			if ((CurTk = NextToken()).TK != Ident) break;
			if ( ((CurTk = NextToken()).TK == SemiC) 
				|| (CurTk.TK == LSBracket) 
				|| (CurTk.TK == Comma)
				|| (CurTk.TK == Assign)) {
				TwoBackNextToken(); // NextToken���O�Ɋ����߂�
				// int a [;] -> TwoBackNextToken() -> NextToken() -> int [a] ;
				// int a [,] -> int [a] ,
				// int a [[] -> int [a] [
				VarDecl(tmpTK);
				break;
			}
			else if (CurTk.TK == LParen) { // �֐���` int a [(] -> int [a] (
				ThreeBackNextToken(); // NextToken���O�Ɋ����߂�
				FuncDecl(tmpTK);
				break;
			}
			break;

		case Do:
			LoopNest_Num += 1;
			ConvBlockCall();
			if (CurTk.TK != RBlace) ErrExit(CurTk.Text, "} ���K�v�ł��B");
			SetCode(RBlace);
			LoopNest_Num -= 1;
			CurTk = NextToken(); // while������͂�
			SkipSpaceLine();
			if (CurTk.TK != While) {
				ErrExit(CurTk.Text, "while ���K�v�ł��B");
			}
			SetCode(While);
			CurTk = NextToken();
			SetCode(LParen); // ?
			SetCode(LParen); // ?
			ConvRest();
			break;

		case While:
		case For:
			LoopNest_Num += 1;
			ConvBlockCall(lblaceFlg, semicFlg);
			if(lblaceFlg) SetCodeEnd();
			LoopNest_Num -= 1;
			break;

		case If:
			ConvBlockCall();
			if (CurTk.TK != RBlace) ErrExit(CurTk.Text, "} ���K�v�ł��B");
			SetCode(RBlace);
			CurTk = NextToken(); // } �̎�
			SkipSpaceLine();
			while (CurTk.TK == ElseIf) {
				ConvBlockCall();
				if (CurTk.TK != RBlace) ErrExit(CurTk.Text, "} ���K�v�ł��B");
				SetCode(RBlace);
				CurTk = NextToken();
				SkipSpaceLine();
			}
			if (CurTk.TK == Else) {
				ConvBlockCall();
				if (CurTk.TK != RBlace) ErrExit(CurTk.Text, "} ���K�v�ł��B");
				SetCode(RBlace);
				CurTk = NextToken();
				SkipSpaceLine();
			}
			break;
		case Break:
		case Continue:
			if (LoopNest_Num <= 0) ErrExit("break�̈ʒu������������܂���B");
			SetCode(CurTk.TK);
			CurTk = NextToken();
			ConvRest();
			break;
		case Return:
			if (!InFunc_Flag) ErrExit("return�̈ʒu������������܂���B");
			SetCode(CurTk.TK);
			CurTk = NextToken();
			ConvRest();
			break;
		case Printf:
			SetCode(CurTk.TK);
			CurTk = NextToken();
			ConvRest();
			break;

		case Scanf:
			SetCode(CurTk.TK);
			CurTk = NextToken();
			ConvRest();
			break;

		case RBlace:
			ErrExit("�s���� } �ł��B");
			break;
		// #include ~ �͖���
	    // // ~ �͖���
		case Include:
		case Coment:
			CurTk.TK = EOF_Line;
			SetCode(EOF_Line);
			break;
		default:
			ConvRest();
			break;
	}

}

// �ϐ����m�F
void VarNameCheck(TokenInfo tk_)
{
	if (tk_.TK != Ident) ErrExit(ErrMsg(tk_.Text, "���ʎq"));
	if (SearchName(tk_.Text, 'V') != -1)
		ErrExit("���ʎq���d�����Ă��܂�: ", tk_.Text);
}

// �ϐ����m�F�Ȃ��Őݒ�
void NoCheckSetName(const char* str_)
{
	TmpSyTbl.Clear();
	TmpSyTbl.Name = str_;
	CurTk = NextToken();
}

// ���O��ݒ�
void SetName(const char* str_)
{
	if (CurTk.TK != Ident) ErrExit("���ʎq������܂���B: ", CurTk.Text);
	TmpSyTbl.Clear();
	TmpSyTbl.Name = str_;
	CurTk = NextToken();
}

void SetName()
{
	if (CurTk.TK != Ident) ErrExit("���ʎq������܂���B->", CurTk.Text);
	TmpSyTbl.Clear();
	TmpSyTbl.Name = CurTk.Text;
	CurTk = NextToken();
}

void SetAryLeng()
{
	TmpSyTbl.Args = 0;
	if (CurTk.TK != '[') return;

	CurTk = NextToken();
	if (CurTk.TK != IntNum) {
		ErrExit("�z�񒷂͐��̐����Ŏw�肵�Ă��������B :", CurTk.Text);
	}
	TmpSyTbl.AryLeng = (int)CurTk.Num;
	CurTk = CheckNextToken(NextToken(), ']');
	if (CurTk.TK == '[') ErrExit("���̌���͑������z��ɑΉ����Ă��܂���B");
}

// �ϊ��ςݓ����R�[�h���i�[
void PushItrCd()
{
	int siz = 0;
	char *pushcode = nullptr;
	*CodeBuffer_Ptr++ = '\0';
	if ((siz = CodeBuffer_Ptr - CodeBuffer) >= LINE_SIZE)
		ErrExit("��s�̕������������l�𒴂��܂����B�R�[�h��Z�����Ă��������B");
	try {
		pushcode = new char[siz];
		memcpy(pushcode, CodeBuffer, siz);
		InterCode.push_back(pushcode);
	}
	catch (bad_alloc) { ErrExit("�������̊m�ۂɎ��s���܂����B"); }
	CodeBuffer_Ptr = CodeBuffer;
}

// �u���b�N����
void ConvBlockCall()
{
	bool lblaceFlg = false, semicFlg = false;
	TokenKind tmpTk = CurTk.TK; // �\���(If, Else...)������͂�
	int patchline = SetCode(CurTk.TK, NO_DEFINE_ADDR);
	CurTk = NextToken();
	ConvRest();
	ConvBlock();

	if ((TokenKind)(unsigned char)*InterCode[patchline] != tmpTk)
		BackPatch2(patchline, GetLineNo());
	else
		BackPatch(patchline, GetLineNo());
}

void ConvBlockCall(bool& _lblaceFlg, bool& _semicFlg)
{
	bool lblaceFlg = false, semicFlg = false;
	unsigned lineNum = GetLineNo();
	TokenKind tmpTk = CurTk.TK; // �\���(If, Else...)������͂�
	int patchline = SetCode(CurTk.TK, NO_DEFINE_ADDR);
	CurTk = NextToken();
	ConvRest(lblaceFlg, semicFlg); 
	_lblaceFlg = lblaceFlg;
	_semicFlg = semicFlg;
	if (!lblaceFlg && semicFlg) { // while (������) ��;
		BackPatch(patchline, lineNum);
	}
	else if (!lblaceFlg && !semicFlg) { // while (������) 
		Conv();                                         //     ��;
		BackPatch(patchline, lineNum + 1);
	}
	else {
		ConvBlock();
		if ((TokenKind)(unsigned char)*InterCode[patchline] != tmpTk)
			BackPatch2(patchline, GetLineNo());
		else BackPatch(patchline, GetLineNo());
	}
}

// �u���b�N����
void ConvBlock()
{
	BlockNest_Num += 1;
	switch (CurTk.TK)
	{
		case EOF_Line:
			CurTk = NextLineToken();
			SkipSpaceLine();
			break;
		case RBlace:
			BlockNest_Num -= 1;
			return;
	}
	
	while (CurTk.TK != Else
		&& CurTk.TK != ElseIf
		&& CurTk.TK != RBlace
		&& CurTk.TK != SemiC
		&& CurTk.TK != EOF_Prg) {
		Conv();
	}

	BlockNest_Num -= 1;
}

// �c��̕�������
void ConvRest()
{
	int tblNum = -1;
	while (CurTk.TK != EOF_Line) {
		switch (CurTk.TK) {
		case Int: case Char: case Double: case Void:
			CurTk = NextToken();
			if (CurTk.TK != Ident) continue;
			CurTk = NextToken();
			if (CurTk.TK != LParen) continue;
			ErrExit("��������������܂���B", CurTk.Text);
			break;
		case If: case ElseIf: case Else: case For: case While: case Break:
		case FuncID: case Return: case VarID: case Printf:
			ErrExit("��������������܂���B", CurTk.Text);
			break;
		case Ident:
			SetName();
			if ((tblNum = SearchName(TmpSyTbl.Name, 'F')) != -1) {
				if (TmpSyTbl.Name == "main") ErrExit("main�֐����Ăяo�����Ƃ͂ł��܂���B");
				SetCode(FuncCall, tblNum);
				continue;
			}
			if ((tblNum = SearchName(TmpSyTbl.Name, 'V')) == -1) {
				ErrExit("�ϐ��錾���K�v�ł��B:", TmpSyTbl.Name);
				continue;
			}
			if (IsLocalName(TmpSyTbl.Name, VarID)) {
				SetCode(LVar, tblNum);
			}
			else {
				SetCode(GVar, tblNum);
			}
			continue;

		case LBlace:

			break;

		case IntNum:
		case DblNum:
			SetCode(CurTk.TK, SetLITERAL(CurTk.Num));
			break;
		case String:
			SetCode(CurTk.TK, SetLITERAL(CurTk.Text));
			break;
		case Addr:
			break;
		default:
			SetCode(CurTk.TK);
			break;
		}
		CurTk = NextToken();
	}
	SetCode(EOF_Line);
	PushItrCd();
	CurTk = NextLineToken();
	SkipSpaceLine();
}


void ConvRest(bool& _lblaceFlg, bool& _semicFlg)
{
	int tblNum = -1;
	while (CurTk.TK != EOF_Line) {
		switch (CurTk.TK) {
			case While:
				SetCode(While, GetLineNo());
				break;

			case For:
				SetCode(For, GetLineNo());
				break;

			case Do:
				SetCode(Do, GetLineNo());
				break;

			case If:
				SetCode(If, GetLineNo());
				break;


			case Ident:
				SetName();
				if ((tblNum = SearchName(TmpSyTbl.Name, 'F')) != -1) {
					if (TmpSyTbl.Name == "main") ErrExit("main�֐����Ăяo�����Ƃ͂ł��܂���B");
					SetCode(FuncCall, tblNum);
					continue;
				}
				if ((tblNum = SearchName(TmpSyTbl.Name, 'V')) == -1) {
					ErrExit("�ϐ��錾���K�v�ł��B:", TmpSyTbl.Name);
					continue;
				}
				if (IsLocalName(TmpSyTbl.Name, VarID)) {
					SetCode(LVar, tblNum);
				}
				else {
					SetCode(GVar, tblNum);
				}
				continue;

			case LBlace:
				SetCode(LBlace);
				_lblaceFlg = true;
				break;

			case SemiC:
				SetCode(SemiC);
				_semicFlg = true;
				break;

			case IntNum:
			case DblNum:
				SetCode(CurTk.TK, SetLITERAL(CurTk.Num));
				break;
			case String:
				SetCode(CurTk.TK, SetLITERAL(CurTk.Text));
				break;
			case Addr:
				break;
			default:
				SetCode(CurTk.TK);
				break;
		}
		CurTk = NextToken();
	}
	SetCode(EOF_Line);
	PushItrCd();
	CurTk = NextLineToken();
	SkipSpaceLine();
}
// �֐���`����
void FuncDecl(TokenKind tk_)
{	
	TokenInfo oneBackTK;
	int tblNum, patchLine, funcTblNum;
	if (BlockNest_Num > 0) ErrExit("�֐���`�̈ʒu������������܂���B");
	InFunc_Flag = true;
	Local_Addr = 0;
	SetStartLTbl();
	switch (tk_) {
		case Int:
			patchLine = SetCode(FuncDec_INT, NO_DEFINE_ADDR);
			break;
		case Double:
			patchLine = SetCode(FuncDec_DBL, NO_DEFINE_ADDR);
			break;
		case Char:
			patchLine = SetCode(FuncDec_CHR, NO_DEFINE_ADDR);
			break;
	}
	CurTk = NextToken();
	oneBackTK = CurTk; // oneBackTK�ɂ͌^���͂���͂�
	CurTk = NextToken(); // CurTk�ɂ͎��ʎq���͂���͂�
	if (CurTk.TK == LSBracket) {
		ErrExit("�֐��̖߂�l�^�ɔz��͑Ή����Ă��܂���B");
	}

	funcTblNum = SearchName(CurTk.Text, 'F');

	// �߂�l�̌^��ݒ�
	switch (tk_)
	{
	case Int:
		GTbl[funcTblNum].VarType = INT_Type;
		break;
	case Double:
		GTbl[funcTblNum].VarType = DBL_Type;
		break;
	case Char:
		GTbl[funcTblNum].VarType = CHR_Type;
		break;
	}

	// �������̉��
	CurTk = CheckNextToken(NextToken(), '('); // ( �̂͂�
	SetCode('(');
	if (CurTk.TK != ')') { // �֐���`�̈������� // int a, int b
		switch (CurTk.TK)
		{
			case Double:
				SetCode(VarDec_DBL);
				TmpSyTbl.VarType = CurTk.TK;
				CurTk = NextToken();
				break;
			case Int:
				SetCode(VarDec_INT);
				TmpSyTbl.VarType = CurTk.TK;
				CurTk = NextToken();
				break;
			case Char:
				SetCode(VarDec_CHR);
				TmpSyTbl.VarType = CurTk.TK;
				CurTk = NextToken();
				break;
			default:
				ErrExit("�����̌^������������܂���");
				break;
		}
		for (;;) {
			SetName();
			tblNum = Enter(TmpSyTbl, ParaID);
			SetCode(LVar, tblNum);
			++GTbl[funcTblNum].Args;
			if (CurTk.TK != ',') break;
			SetCode(',');
			CurTk = NextToken();
			switch (CurTk.TK)
			{
				case Double:
					SetCode(VarDec_DBL);
					TmpSyTbl.VarType = CurTk.TK;
					CurTk = NextToken();
					break;
				case Int:
					SetCode(VarDec_INT);
					TmpSyTbl.VarType = CurTk.TK;
					CurTk = NextToken();
					break;
				case Char:
					SetCode(VarDec_CHR);
					TmpSyTbl.VarType = CurTk.TK;
					CurTk = NextToken();
					break;
				default:
					ErrExit("�����̌^������������܂���");
					break;
			}
		}
	}

	CurTk = CheckNextToken(CurTk, ')');
	SetCode(')');

	switch (CurTk.TK) {
		case LBlace: // �֐����̎��� { ��������
			FuncLBlace_Flag = true;
			CurTk = NextToken();
			SetCode(LBlace);
			if(CurTk.TK == EOF_Line)
				PushItrCd();
			break;
		case EOF_Line: // �֐����̎���{�Ȃ��ŉ��s����Ă�����
			PushItrCd(); // �����R�[�h�֕ϊ�
			CurTk = NextLineToken(); // ���̍s
			SkipSpaceLine();
			if (!FuncLBlace_Flag) {
				if (CurTk.TK != LBlace) {
					ErrExit("�֐���` { ���K�v�ł�");
				}
				SetCode(LBlace);
			}
			PushItrCd();
			FuncLBlace_Flag = false;
			CurTk = NextToken();
			break;
		default:
			ErrExit("�s���ȋL�q�B", CurTk.Text);
	}
	ConvBlock();
	BackPatch(patchLine, GetLineNo());
	SetCodeEnd();
	GTbl[funcTblNum].FrameSize = Local_Addr;
	if (GTbl[funcTblNum].Name == "main") {
		MainTbl_Num = funcTblNum;
		if (GTbl[MainTbl_Num].Args != 0) {
			ErrExit("�{�C���^�v���^��main�֐��ɉ��������w�肷�邱�Ƃ͂ł��܂���B");
		}
	}
	InFunc_Flag = false;
}


// �ϐ��錾����
void VarDecl(TokenKind _tk)
{
	DtType type = NON_Type;
	TokenInfo vName; // �ϐ���
	int tblNum; // �ϐ��\�ԍ�
	bool gVarFlg= false; // ���[�J���ϐ��A�O���[�o���ϐ��̔���

	switch (_tk) {
		case Int:
			SetCode(VarDec_INT);
			type = INT_Type;
			break;
		case Double:
			SetCode(VarDec_DBL);
			type = DBL_Type;
			break;
		case Char:
			SetCode(VarDec_CHR);
			type = CHR_Type;
			break;
	}

	//SetCodeRest();
	while (1) {
		vName = CurTk = NextToken(); // ���̃g�[�N��
		VarNameCheck(CurTk); // �ϐ������d�����Ă��Ȃ���
		SetName(); // �ϐ����ݒ�
		SetAryLeng(); // �z�񒷐ݒ�
		TmpSyTbl.VarType = type; // �^�^�C�v
		TmpSyTbl.ScopeCntr = BlockNest_Num; // �u���b�N�[�x
		Enter(TmpSyTbl, VarID); 

		if (CurTk.TK == Assign) {
			TmpSyTbl.Clear();
			TmpSyTbl.Name = vName.Text;
			if ((tblNum = SearchName(TmpSyTbl.Name, 'F') != -1)) {
				if (TmpSyTbl.Name == "main") ErrExit("main�֐����Ăяo�����Ƃ͂ł��܂���B");
				SetCode(FuncCall, tblNum);
			}
			if ((tblNum = SearchName(TmpSyTbl.Name, 'V')) == -1) 
				ErrExit("�ϐ��錾���K�v�ł��B:", TmpSyTbl.Name);
			if (IsLocalName(TmpSyTbl.Name, VarID)) SetCode(LVar, tblNum);
			else {
				SetCode(GVar, tblNum);
				gVarFlg = true;
			}
			SetCode(CurTk.TK);
			CurTk = NextToken();
			ConvExpr();
		}

		if (CurTk.TK != Comma) break;
	}

	if (gVarFlg) {
		GVarIni.push_back(GetLineNo());
	}

	SetCodeEof();
}

void ConvExpr()
{
	int tblNum = -1;
	while (CurTk.TK != Comma && CurTk.TK != SemiC) {
		switch (CurTk.TK) {
		case Int: case Char: case Double: case Void:
		case If: case ElseIf: case Else: case For: case While: case Break:
		case FuncID: case Return: case VarID: case Printf:
			ErrExit("��������������܂���B", CurTk.Text);
			break;
		case Ident:
			SetName();
			if ((tblNum = SearchName(TmpSyTbl.Name, 'F')) != -1) {
				if (TmpSyTbl.Name == "main") ErrExit("main�֐����Ăяo�����Ƃ͂ł��܂���B");
				SetCode(FuncCall, tblNum);
				continue;
			}
			if ((tblNum = SearchName(TmpSyTbl.Name, 'V')) == -1) {
				ErrExit("�ϐ��錾���K�v�ł��B:", TmpSyTbl.Name);
				continue;
			}
			if (IsLocalName(TmpSyTbl.Name, VarID)) SetCode(LVar, tblNum);
			else SetCode(GVar, tblNum);
			continue;
		case IntNum:
		case DblNum:
			SetCode(CurTk.TK, SetLITERAL(CurTk.Num));
			break;
		case String:
			SetCode(CurTk.TK, SetLITERAL(CurTk.Text));
			break;
			// �������A�h���X�͖��� Addr
		case Addr:
			break;
		default:
			SetCode(CurTk.TK);
			break;
		}
		CurTk = NextToken();
	}
	if (CurTk.TK == Comma) SetCode(Comma);
}

// line�s��num_��ݒ�
void BackPatch(int _line, int _num)
{
	*(short int*)(InterCode[_line] + 1) = (short)_num;
}

// 2���炷
void BackPatch2(int _line, int _num)
{
	*(short int*)(InterCode[_line] + 2) = (short)_num;
}

// �R�[�h�i�[
void SetCode(int code_)
{
	*CodeBuffer_Ptr++ = code_;
}

// �R�[�h��short�l�i�[
int SetCode(int code_, int num_)
{
	*CodeBuffer_Ptr++ = (char)code_;
	*(short int*)CodeBuffer_Ptr = (short int)num_;
	CodeBuffer_Ptr += sizeof(short int);
	return GetLineNo();
}

// �������̍s, �c��̃e�L�X�g���i�[
void SetCodeRest()
{
	extern char *TokenP;
	strcpy(CodeBuffer_Ptr, TokenP);
	CodeBuffer_Ptr += strlen(TokenP) + 1;
}

// }, ;���s���ł��邱�Ƃ��`�F�b�N���i�[

void SetCodeEnd()
{
	if (CurTk.TK != RBlace) {
		ErrExit(CurTk.Text, "} ���K�v�ł��B");
	}
	SetCode(RBlace);
	CurTk = NextToken();
	SetCodeEof();
}

// �s�̖����ł��邱�Ƃ��m�F��, CodeBuffer��Code�Ɋi�[
void SetCodeEof()
{
	switch (CurTk.TK)
	{
		case SemiC:
			SetCode(SemiC);
			if ((CurTk = NextToken()).TK == EOF_Line) {
				PushItrCd();
				CurTk = NextLineToken(); // ���̍s
				SkipSpaceLine();
				break;
			}
			Conv();
			break;

		case RBlace:
			SetCode(RBlace);
			PushItrCd();
			CurTk = NextLineToken(); // ���̍s
			SkipSpaceLine();
			break;

		case EOF_Line:
			SetCode(EOF_Line);
			PushItrCd();
			CurTk = NextLineToken(); // ���̍s
			SkipSpaceLine();
			break;

		default:
			ErrExit("�s���ȋL�q�B", CurTk.Text);
	}
}