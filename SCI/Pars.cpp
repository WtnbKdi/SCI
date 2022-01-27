#pragma once

#include "SCI.h"
#include "Prototype.h"

#define NO_DEFINE_ADDR 0        // 番地未定

TokenInfo CurTk;                // 処理中のトークン
SymbolTbl TmpSyTbl;             // 一時的格納記号表
int BlockNest_Num;              // ブロック深度
int Local_Addr;                 // ローカル変数アドレス
int MainTbl_Num;                // メイン関数記号表位置
int LoopNest_Num;		        // ループネスト
bool InFunc_Flag;               // 関数内で処理中であるか
char CodeBuffer[LINE_SIZE + 1]; // 内部コード生成バッファ
char *CodeBuffer_Ptr;			// 内部コード生成バッファのポインタ
extern vector<char*> InterCode;             // 変換後内部コード
vector<int> GVarIni;
char *Token_Ptr = nullptr;
bool FuncLBlace_Flag;           // 関数名 ( ) ココ に { があるか
extern vector<SymbolTbl> GTbl;

// 初期化
void Ini()
{
	GTbl.clear();
	Token_Ptr = nullptr;
	memset(CodeBuffer, 0, LINE_SIZE + 1);
	InterCode.clear();
	GVarIni.clear();
	GVarIni.resize(1);
	InitTkTable(); // 文字種類表を初期化
	MainTbl_Num = -1;
	BlockNest_Num = LoopNest_Num = 0;
	InFunc_Flag = false;
	FuncLBlace_Flag = false;
	CodeBuffer_Ptr = CodeBuffer;
}

// ローカルスコープ内である->ture
bool IsLocalScope()
{
	return InFunc_Flag;
}

// 空白の行をスキップ
void SkipSpaceLine()
{
	while (CurTk.TK == EOF_Line) {
		PushItrCd();
		CurTk = NextLineToken();
	}
}

// コードを変換
void ConvToItrCode(const char* _filename)
{
	string fName; // 関数名を格納
	Ini(); // 初期化
	FileOpen(_filename); // ソースファイルを開く
	// 先に関数名を記号表に登録しておく
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
				NoCheckSetName(fName.c_str()); // TmpSymTblに名前を仮保存
				Enter(TmpSyTbl, FuncID); // 記号表に登録
				break;
		}
	}
	
	// 内部コードへ変換
	PushItrCd();
	FileOpen(_filename);
	// 次の一行の先頭のトークンを取得
	CurTk = NextLineToken();
	while (CurTk.TK != EOF_Prg) Conv();
	// main関数があるときの設定処理
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

// 先頭のコードを処理, 残りの部分はConvRestで処理

void Conv()
{
	TokenKind tmpTK;
	bool lblaceFlg = false, semicFlg = false;
	// 変数定義, 関数定義であるか調べる
	switch (CurTk.TK) {
		case Int:
		case Double:
		case Char:
			tmpTK = CurTk.TK; // 型情報
			if ((CurTk = NextToken()).TK != Ident) break;
			if ( ((CurTk = NextToken()).TK == SemiC) 
				|| (CurTk.TK == LSBracket) 
				|| (CurTk.TK == Comma)
				|| (CurTk.TK == Assign)) {
				TwoBackNextToken(); // NextTokenを二つ前に巻き戻す
				// int a [;] -> TwoBackNextToken() -> NextToken() -> int [a] ;
				// int a [,] -> int [a] ,
				// int a [[] -> int [a] [
				VarDecl(tmpTK);
				break;
			}
			else if (CurTk.TK == LParen) { // 関数定義 int a [(] -> int [a] (
				ThreeBackNextToken(); // NextTokenを二つ前に巻き戻す
				FuncDecl(tmpTK);
				break;
			}
			break;

		case Do:
			LoopNest_Num += 1;
			ConvBlockCall();
			if (CurTk.TK != RBlace) ErrExit(CurTk.Text, "} が必要です。");
			SetCode(RBlace);
			LoopNest_Num -= 1;
			CurTk = NextToken(); // whileが入るはず
			SkipSpaceLine();
			if (CurTk.TK != While) {
				ErrExit(CurTk.Text, "while が必要です。");
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
			if (CurTk.TK != RBlace) ErrExit(CurTk.Text, "} が必要です。");
			SetCode(RBlace);
			CurTk = NextToken(); // } の次
			SkipSpaceLine();
			while (CurTk.TK == ElseIf) {
				ConvBlockCall();
				if (CurTk.TK != RBlace) ErrExit(CurTk.Text, "} が必要です。");
				SetCode(RBlace);
				CurTk = NextToken();
				SkipSpaceLine();
			}
			if (CurTk.TK == Else) {
				ConvBlockCall();
				if (CurTk.TK != RBlace) ErrExit(CurTk.Text, "} が必要です。");
				SetCode(RBlace);
				CurTk = NextToken();
				SkipSpaceLine();
			}
			break;
		case Break:
		case Continue:
			if (LoopNest_Num <= 0) ErrExit("breakの位置が正しくありません。");
			SetCode(CurTk.TK);
			CurTk = NextToken();
			ConvRest();
			break;
		case Return:
			if (!InFunc_Flag) ErrExit("returnの位置が正しくありません。");
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
			ErrExit("不正な } です。");
			break;
		// #include ~ は無視
	    // // ~ は無視
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

// 変数名確認
void VarNameCheck(TokenInfo tk_)
{
	if (tk_.TK != Ident) ErrExit(ErrMsg(tk_.Text, "識別子"));
	if (SearchName(tk_.Text, 'V') != -1)
		ErrExit("識別子が重複しています: ", tk_.Text);
}

// 変数名確認なしで設定
void NoCheckSetName(const char* str_)
{
	TmpSyTbl.Clear();
	TmpSyTbl.Name = str_;
	CurTk = NextToken();
}

// 名前を設定
void SetName(const char* str_)
{
	if (CurTk.TK != Ident) ErrExit("識別子がありません。: ", CurTk.Text);
	TmpSyTbl.Clear();
	TmpSyTbl.Name = str_;
	CurTk = NextToken();
}

void SetName()
{
	if (CurTk.TK != Ident) ErrExit("識別子がありません。->", CurTk.Text);
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
		ErrExit("配列長は正の整数で指定してください。 :", CurTk.Text);
	}
	TmpSyTbl.AryLeng = (int)CurTk.Num;
	CurTk = CheckNextToken(NextToken(), ']');
	if (CurTk.TK == '[') ErrExit("この言語は多次元配列に対応していません。");
}

// 変換済み内部コードを格納
void PushItrCd()
{
	int siz = 0;
	char *pushcode = nullptr;
	*CodeBuffer_Ptr++ = '\0';
	if ((siz = CodeBuffer_Ptr - CodeBuffer) >= LINE_SIZE)
		ErrExit("一行の文字数が制限値を超えました。コードを短くしてください。");
	try {
		pushcode = new char[siz];
		memcpy(pushcode, CodeBuffer, siz);
		InterCode.push_back(pushcode);
	}
	catch (bad_alloc) { ErrExit("メモリの確保に失敗しました。"); }
	CodeBuffer_Ptr = CodeBuffer;
}

// ブロック処理
void ConvBlockCall()
{
	bool lblaceFlg = false, semicFlg = false;
	TokenKind tmpTk = CurTk.TK; // 予約語(If, Else...)が入るはず
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
	TokenKind tmpTk = CurTk.TK; // 予約語(If, Else...)が入るはず
	int patchline = SetCode(CurTk.TK, NO_DEFINE_ADDR);
	CurTk = NextToken();
	ConvRest(lblaceFlg, semicFlg); 
	_lblaceFlg = lblaceFlg;
	_semicFlg = semicFlg;
	if (!lblaceFlg && semicFlg) { // while (条件式) 文;
		BackPatch(patchline, lineNum);
	}
	else if (!lblaceFlg && !semicFlg) { // while (条件式) 
		Conv();                                         //     文;
		BackPatch(patchline, lineNum + 1);
	}
	else {
		ConvBlock();
		if ((TokenKind)(unsigned char)*InterCode[patchline] != tmpTk)
			BackPatch2(patchline, GetLineNo());
		else BackPatch(patchline, GetLineNo());
	}
}

// ブロック処理
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

// 残りの文を処理
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
			ErrExit("文が正しくありません。", CurTk.Text);
			break;
		case If: case ElseIf: case Else: case For: case While: case Break:
		case FuncID: case Return: case VarID: case Printf:
			ErrExit("文が正しくありません。", CurTk.Text);
			break;
		case Ident:
			SetName();
			if ((tblNum = SearchName(TmpSyTbl.Name, 'F')) != -1) {
				if (TmpSyTbl.Name == "main") ErrExit("main関数を呼び出すことはできません。");
				SetCode(FuncCall, tblNum);
				continue;
			}
			if ((tblNum = SearchName(TmpSyTbl.Name, 'V')) == -1) {
				ErrExit("変数宣言が必要です。:", TmpSyTbl.Name);
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
					if (TmpSyTbl.Name == "main") ErrExit("main関数を呼び出すことはできません。");
					SetCode(FuncCall, tblNum);
					continue;
				}
				if ((tblNum = SearchName(TmpSyTbl.Name, 'V')) == -1) {
					ErrExit("変数宣言が必要です。:", TmpSyTbl.Name);
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
// 関数定義処理
void FuncDecl(TokenKind tk_)
{	
	TokenInfo oneBackTK;
	int tblNum, patchLine, funcTblNum;
	if (BlockNest_Num > 0) ErrExit("関数定義の位置が正しくありません。");
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
	oneBackTK = CurTk; // oneBackTKには型がはいるはず
	CurTk = NextToken(); // CurTkには識別子がはいるはず
	if (CurTk.TK == LSBracket) {
		ErrExit("関数の戻り値型に配列は対応していません。");
	}

	funcTblNum = SearchName(CurTk.Text, 'F');

	// 戻り値の型を設定
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

	// 仮引数の解析
	CurTk = CheckNextToken(NextToken(), '('); // ( のはず
	SetCode('(');
	if (CurTk.TK != ')') { // 関数定義の引数処理 // int a, int b
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
				ErrExit("引数の型が正しくありません");
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
					ErrExit("引数の型が正しくありません");
					break;
			}
		}
	}

	CurTk = CheckNextToken(CurTk, ')');
	SetCode(')');

	switch (CurTk.TK) {
		case LBlace: // 関数名の次が { だった時
			FuncLBlace_Flag = true;
			CurTk = NextToken();
			SetCode(LBlace);
			if(CurTk.TK == EOF_Line)
				PushItrCd();
			break;
		case EOF_Line: // 関数名の次が{なしで改行されていた時
			PushItrCd(); // 内部コードへ変換
			CurTk = NextLineToken(); // 次の行
			SkipSpaceLine();
			if (!FuncLBlace_Flag) {
				if (CurTk.TK != LBlace) {
					ErrExit("関数定義 { が必要です");
				}
				SetCode(LBlace);
			}
			PushItrCd();
			FuncLBlace_Flag = false;
			CurTk = NextToken();
			break;
		default:
			ErrExit("不正な記述。", CurTk.Text);
	}
	ConvBlock();
	BackPatch(patchLine, GetLineNo());
	SetCodeEnd();
	GTbl[funcTblNum].FrameSize = Local_Addr;
	if (GTbl[funcTblNum].Name == "main") {
		MainTbl_Num = funcTblNum;
		if (GTbl[MainTbl_Num].Args != 0) {
			ErrExit("本インタプリタはmain関数に仮引数を指定することはできません。");
		}
	}
	InFunc_Flag = false;
}


// 変数宣言処理
void VarDecl(TokenKind _tk)
{
	DtType type = NON_Type;
	TokenInfo vName; // 変数名
	int tblNum; // 変数表番号
	bool gVarFlg= false; // ローカル変数、グローバル変数の判別

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
		vName = CurTk = NextToken(); // 次のトークン
		VarNameCheck(CurTk); // 変数名が重複していないか
		SetName(); // 変数名設定
		SetAryLeng(); // 配列長設定
		TmpSyTbl.VarType = type; // 型タイプ
		TmpSyTbl.ScopeCntr = BlockNest_Num; // ブロック深度
		Enter(TmpSyTbl, VarID); 

		if (CurTk.TK == Assign) {
			TmpSyTbl.Clear();
			TmpSyTbl.Name = vName.Text;
			if ((tblNum = SearchName(TmpSyTbl.Name, 'F') != -1)) {
				if (TmpSyTbl.Name == "main") ErrExit("main関数を呼び出すことはできません。");
				SetCode(FuncCall, tblNum);
			}
			if ((tblNum = SearchName(TmpSyTbl.Name, 'V')) == -1) 
				ErrExit("変数宣言が必要です。:", TmpSyTbl.Name);
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
			ErrExit("文が正しくありません。", CurTk.Text);
			break;
		case Ident:
			SetName();
			if ((tblNum = SearchName(TmpSyTbl.Name, 'F')) != -1) {
				if (TmpSyTbl.Name == "main") ErrExit("main関数を呼び出すことはできません。");
				SetCode(FuncCall, tblNum);
				continue;
			}
			if ((tblNum = SearchName(TmpSyTbl.Name, 'V')) == -1) {
				ErrExit("変数宣言が必要です。:", TmpSyTbl.Name);
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
			// 今だけアドレスは無視 Addr
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

// line行にnum_を設定
void BackPatch(int _line, int _num)
{
	*(short int*)(InterCode[_line] + 1) = (short)_num;
}

// 2つずらす
void BackPatch2(int _line, int _num)
{
	*(short int*)(InterCode[_line] + 2) = (short)_num;
}

// コード格納
void SetCode(int code_)
{
	*CodeBuffer_Ptr++ = code_;
}

// コードとshort値格納
int SetCode(int code_, int num_)
{
	*CodeBuffer_Ptr++ = (char)code_;
	*(short int*)CodeBuffer_Ptr = (short int)num_;
	CodeBuffer_Ptr += sizeof(short int);
	return GetLineNo();
}

// 処理中の行, 残りのテキストを格納
void SetCodeRest()
{
	extern char *TokenP;
	strcpy(CodeBuffer_Ptr, TokenP);
	CodeBuffer_Ptr += strlen(TokenP) + 1;
}

// }, ;が行末であることをチェックし格納

void SetCodeEnd()
{
	if (CurTk.TK != RBlace) {
		ErrExit(CurTk.Text, "} が必要です。");
	}
	SetCode(RBlace);
	CurTk = NextToken();
	SetCodeEof();
}

// 行の末尾であることを確認し, CodeBufferをCodeに格納
void SetCodeEof()
{
	switch (CurTk.TK)
	{
		case SemiC:
			SetCode(SemiC);
			if ((CurTk = NextToken()).TK == EOF_Line) {
				PushItrCd();
				CurTk = NextLineToken(); // 次の行
				SkipSpaceLine();
				break;
			}
			Conv();
			break;

		case RBlace:
			SetCode(RBlace);
			PushItrCd();
			CurTk = NextLineToken(); // 次の行
			SkipSpaceLine();
			break;

		case EOF_Line:
			SetCode(EOF_Line);
			PushItrCd();
			CurTk = NextLineToken(); // 次の行
			SkipSpaceLine();
			break;

		default:
			ErrExit("不正な記述。", CurTk.Text);
	}
}