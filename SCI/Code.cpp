#pragma once

#include "SCI.h"
#include "Prototype.h"
CodeManager Code;                         // コード管理
int StartPC;                              // 実行開始行 
int PC = -1;                              // プログラムカウンタ 非実行中: -1
int BaseReg;                              // ベースレジスタ 
int SpReg;                                // スタックポインタ 
int MaxLine;                              // プログラム末尾行 
vector<char*> InterCode;                  // 変換済み内部コード格納 
char *CodePtr;                            // 内部コード解析用ポインタ  
double ReturnValue;                       // 関数戻値 
bool BreakFlg, ReturnFlg, ReturnNoneFlg, ContinueFlg; // 制御用フラグ 
Mymemory Dmem;                            // 主記憶  
vector<string> StrLiteral;                // 文字列リテラル格納  
vector<double> NumLiteral;                // 数値リテラル格納  
bool SyntaxChkFlg = false;                // 構文チェックのとき真  
extern vector<SymbolTbl> GTbl;            // 大域記号表  
int FactorVarAddr;						  // (Factor)変数アドレス
double FactorVarNum;					  // (Factor)変数値
unsigned NestCntr;                        // {}の深度
extern vector<int> GVarIni;          // グローバル変数に定義がある場合
//unsigned oneBlockPC = -1; // {}省略時の最終PC

const int IncDecSize = 100;
vector<double> PostInc(IncDecSize, 0);
vector<double> PostDec(IncDecSize, 0);

string TestOutPut;       // テスト結果の保存

class MyStack // stack<double>のラッパークラス
{
private:
	stack<int> st;
public:
	void Push(int num) { st.push(num); }          // 積み込み
	int Size() { return (int)st.size(); }            // スタックサイズ
	bool Empty() { return st.empty(); }              // スタックが空であるか
	int Pop()                                      // 一番上を取り出して削除
	{  
		if (st.empty()) ErrExit("スタックアンダーフロー"); 
		int topNum = st.top(); // 一番上の値をスタックから取り出し格納
		st.pop();                 // 一番上の値をスタックから消す
		return topNum;             // 一番上の値を返す
	}
	void Clear() {
		while (!st.empty()) st.pop();
	}
};

MyStack Stk; // オペランドスタック

void StartPrgCntr(int n)
{
	StartPC = n;
}

void Execute() // 実行 
{
	//Dmem.clear(); // 注意　メモリを初期化するとグローバル、ローカル変数がバグる
	//Stk.Clear();
	//PC = -1;
	//oneBlockPC = -1;

	NestCntr = 0;
	BaseReg = 0;                                        // ベースレジスタ初期値 
	
	SpReg = Dmem.size();                              // スタックポインタ初期値 
	Dmem.resize(SpReg + 1000);                              // 主記憶領域初期確保 

	BreakFlg = ReturnFlg = ReturnNoneFlg = ContinueFlg = false;
	MaxLine = InterCode.size() - 1;

	// グロバール変数に代入処理がある場合
	if (GVarIni.size() != 0) {
		for_each(GVarIni.begin(), GVarIni.end(), [](unsigned idx) { 
			PC = idx;
			Statement();
		});
	}

	PC = StartPC;

	while (PC <= MaxLine) {
		Statement();
	}
	PC = -1;                                                      // 非実行状態 
}

void LookIntrCode()
{
	BaseReg = 0; // ベースレジスタ初期値 
	SpReg = Dmem.size(); // スタックポインタ初期値 
	Dmem.resize(SpReg + 1000); // 主記憶領域初期確保 

	BreakFlg = ReturnFlg = ReturnNoneFlg = ContinueFlg = false;
	PC = StartPC;
	MaxLine = InterCode.size();

	PC = 1;
	while (PC < MaxLine) {
		std::cout << "PC: " << PC << " ";
		for (auto a = FirstCode(PC); a.Kind != EOF_Line; a = NextCode()) {
			std::cout << "(";
			std::cout << "種類: " << a.Kind;
			std::cout << " 値: " << a.IntVal;
			std::cout << " アドレス: " << a.JmpAddr;
			std::cout << ")";
		}
		++PC;
		std::cout << endl;
	}

}

void ForExpr3Skip(int topline) // for文三番目の式の最初の手前まで飛ばす
{
	PC = topline;
	Code = FirstCode(PC);
	while ((Code = NextCode()).Kind != SemiC);
	while ((Code = NextCode()).Kind != SemiC);
	Code = NextCode();
}

int EndlineOfIf_Last(int _pc)
{
	bool srchIfEndLineLoop = true;
	int srchIfEndLine = _pc;
	char *saveCodePtr = CodePtr;
	CodeManager saveCode = Code;
	while (srchIfEndLineLoop) {
		Code = FirstCode(_pc);
		++srchIfEndLine;
		switch (Code.Kind) {

		case ElseIf:
		case Else:
			break;

		case RBlace:
			Code = NextCode();
			if (Code.Kind != ElseIf && Code.Kind != Else) {
				Code = FirstCode(_pc + 1);
				if (Code.Kind != ElseIf && Code.Kind != Else) {
					srchIfEndLineLoop = false;
					break;
				}
			}
		}

		++_pc;
	}

	CodePtr = saveCodePtr;
	Code = saveCode;

	return srchIfEndLine - 1;
}

// 引数のデフォルト: void Statement(bool oneStmt = false);
void Statement() // 文 
{
	TokenKind tkIncDec = Others; // 前置インクリメント演算子の次を格納
	CodeManager save, cm;

	int top_line, // while, ifなどの先頭行番号
		end_line, // }の行番号
		varAdrs, // 変数アドレス
		end_line_last; // if elseなど, 最後の}の行番号を格納

	double wkVal, midDt, endDt, stepDt;
	bool eofChkFlag, lblaceFlg, forLoopFlg, forExpr2Flg, forExpr3Flg,
		forExpr3PostIncDecFlg, forExpr3ExprFlg;

	eofChkFlag = lblaceFlg = forLoopFlg = forExpr2Flg = forExpr3Flg = false;
	forExpr3PostIncDecFlg = forExpr3ExprFlg = false;

	if (PC > MaxLine) return;                       // プログラム終了
	Code = save = FirstCode(PC);
	top_line = PC; // 制御範囲の先頭と終端  
	end_line = Code.JmpAddr;
	
	switch (Code.Kind) {
	case If:
		NestCntr += 1;

		// if
		end_line = EndlineOfIf(PC);
		end_line_last = EndlineOfIf_Last(PC);

		if (GetExpression(If, 0)) {
			++PC;
			Block();
			NestCntr -= 1;
			PC = end_line_last + 1;
			break;
		}

		PC = end_line; // if文の}
		Code = FirstCode(PC);
		if ((Code = NextCode()).Kind == EOF_Line) {
			++PC;
			while (FirstCode(PC).Kind == EOF_Line) ++PC;
		}

		// elif 
		while (Code.Kind == ElseIf || FirstCode(PC).Kind == ElseIf) {
			eofChkFlag = true;
			if (Code.Kind != ElseIf) {
				Code = FirstCode(PC).Kind;
				end_line = EndlineOfIf(PC);
			}
			else end_line = EndlineOfIf_Etc(PC);
			

			Code = NextCode();

			if (GetExpression()) {
				NestCntr -= 1;
				++PC;
				Block();
				PC = end_line_last + 1;
				break;
			}

			PC = end_line;
			Code = FirstCode(PC);
			if ((Code = NextCode()).Kind == EOF_Line) {
				++PC;
				while (FirstCode(PC).Kind == EOF_Line) ++PC;
			}
		}

		// else
		if (Code.Kind == Else || FirstCode(PC).Kind == Else) {
			++PC;
			Block();
			PC = end_line_last + 1;

			NestCntr -= 1;
		}

		break;

	case Printf:
		sysFuncExe(Code.Kind);
		++PC;
		break;

	case Scanf:
		sysFuncExe(Code.Kind);
		++PC;
		break;

	case While:
		NestCntr += 1;

		for (;;) {
			if (!GetExpression(While, 0)) {
				NestCntr -= 1;
				break;
			}

			++PC;
			Block();
			
			if (BreakFlg || ReturnFlg || ReturnNoneFlg) {
				BreakFlg = false;
				break;
			}
			if (ContinueFlg) {
				PC = top_line;
				Code = FirstCode(PC);
				ContinueFlg = false;
				continue;
			}

			PC = top_line;
			Code = FirstCode(PC);
			
		}

		PC = end_line + 1;
		break;

	case Do:
		NestCntr += 1;
		for (;;) {
			++PC;
			Block();
			if (BreakFlg || ReturnFlg || ReturnNoneFlg) {
				BreakFlg = false;
				NestCntr -= 1;
				break;
			}
			if (ContinueFlg) {
				PC = top_line;
				Code = FirstCode(PC);
				ContinueFlg = false;
				continue;
			}
			Code = FirstCode(PC);
			Code = NextCode();

			if (!GetExpression(While, 0)) break;

			PC = top_line;
		}
		PC = end_line + 1;
		break;

	case For:
		NestCntr += 1;
		Code = NextCode(); // ( が入るはず
		Code = CheckNextCode(Code, LParen); // ( であるか
		
		if (Code.Kind == GVar || Code.Kind == LVar) { // for( [~] ; ~ ; ~)
			save = Code; // 変数
			varAdrs = GetMemAddr(Code); // 変数のアドレス
			Expression('=', 0); // 代入文(式)の解析
			Dmem.set(varAdrs, Stk.Pop()); // 代入文(式)の値を変数にセット
		}

		Code = CheckNextCode(Code, SemiC); // for( ~ [;] ~ ; ~)　セミコロンであるか確認し次のコードを返す、違ったらエラー
		
		if (Code.Kind != SemiC) { // // for( ~ ; [~] ; ~) 条件式がある場合
			forLoopFlg = GetExpression(0, 0); // for( ~ ; [~] ; ~) 条件式を解析
			if (!forLoopFlg) { // 条件式がfalseであればfor文を抜ける
				PC = end_line + 1; // 実行行番号を}の下に設定
				NestCntr -= 1;
				break;
			}
			forExpr2Flg = true;
		}

		forLoopFlg = true;

		Code = CheckNextCode(Code, SemiC); // for( ~ ; ~ [;] ~)

		if (Code.Kind != RParen) { // for( ~ ; ~ ; [ここに式がある場合] ) 
			forExpr3ExprFlg = true;
			while ((Code = NextCode()).Kind != RParen);
		}
		Code = CheckNextCode(Code, RParen);

		++PC;
		Block();

		if (forExpr3ExprFlg) {
			ForExpr3Skip(top_line);
			GetExpression();
			forExpr3ExprFlg = false;
		}

		while (forLoopFlg) { // 二回目のループ処理
			if (BreakFlg || ReturnFlg || ReturnNoneFlg) {
				BreakFlg = false;
				NestCntr -= 1;

				break;
			}
			if (ContinueFlg) {
				PC = top_line;
				Code = FirstCode(PC);
				ContinueFlg = false;
				continue;
			}

			PC = top_line;
			Code = FirstCode(PC);

			while ((Code = NextCode()).Kind != SemiC);

			if (forExpr2Flg) {
				forLoopFlg = GetExpression(SemiC, 0);
			}
			else {
				Code = NextCode();
			}

			if (!forLoopFlg) {
				NestCntr -= 1;
				break;
			}

			Code = CheckNextCode(Code, SemiC); // for( ~ ; ~ [;] ~)

			if (Code.Kind != RParen) { // for( ~ ; ~ ; [ここに式がある場合] ) 
				forExpr3ExprFlg = true;
				while ((Code = NextCode()).Kind != RParen);
			}
			Code = CheckNextCode(Code, RParen);


			++PC;
			Block();
			

			if (forExpr3ExprFlg) {
				ForExpr3Skip(top_line);
				GetExpression();
				forExpr3ExprFlg = false;
			}
		} 

		forExpr2Flg = false;
		PC = end_line + 1;
		break;

	case FuncCall:// 代入のない関数呼出 
		FncCall(Code.SymbolNum);
		(void)Stk.Pop();// 戻り値不要 
		++PC;
		break;

	case FuncDec_INT:// 関数定義はスキップ
	case FuncDec_DBL:
	case FuncDec_CHR:
		PC = end_line + 1;
		break;

	case GVar:
	case LVar:// 代入文 
		if (CheckNest(Code) - 1 > NestCntr && Code.Kind != GVar) {
			ErrExit("スコープ内で定義された変数は、外部からアクセスすることはできません");
			
		}
		switch (NextLookCode().Kind) { // 一つ先読みする
		case LSBracket:
		case Assign:
			varAdrs = GetMemAddr(Code);
			Expression('=', 0);                                // 代入時に型確定 
			Dmem.set(varAdrs, Stk.Pop());
			break;
		case PlusEq:
			varAdrs = GetMemAddr(Code);
			Expression(PlusEq, 0);                                // 代入時に型確定 
			Dmem.set(varAdrs, Dmem.get(varAdrs) + Stk.Pop());
			break;
		case MinusEq:
			varAdrs = GetMemAddr(Code);
			Expression(MinusEq, 0);                                 // 代入時に型確定 
			Dmem.set(varAdrs, Dmem.get(varAdrs) - Stk.Pop());
			break;
		case MultiEq:
			varAdrs = GetMemAddr(Code);
			Expression(MultiEq, 0);                                // 代入時に型確定 
			Dmem.set(varAdrs, Dmem.get(varAdrs) * Stk.Pop());
			break;
		case DiviEq:
			varAdrs = GetMemAddr(Code);
			Expression(DiviEq, 0);                               // 代入時に型確定 
			Dmem.set(varAdrs, Dmem.get(varAdrs) / Stk.Pop());
			break;
		case ModEq:
			varAdrs = GetMemAddr(Code);
			Expression(ModEq, 0);                                // 代入時に型確定 
			Dmem.set(varAdrs, (int)Dmem.get(varAdrs) % (int)Stk.Pop());
			break;

		case Inc:
		case Dec:
			GetExpression(0, 0);
			Code = NextCode();
		}

		++PC;
		break;

	case Inc:
	case Dec:
		tkIncDec = NextLookCode().Kind;
		if (tkIncDec == LVar || tkIncDec == GVar) {
			Expression();
		}
		++PC;
		break;

	case Return:
		if (NextLookCode().Kind == SemiC) { // return; の場合
			BreakFlg = true;
			ReturnNoneFlg = true;
			break;
		}

		wkVal = ReturnValue;
		Code = NextCode();

		// 「式」があれば戻り値を計算 
		if (Code.Kind != EOF_Line || Code.Kind != SemiC) {
			wkVal = GetExpression();
			ReturnFlg = true;
		}

		if (ReturnFlg) ReturnValue = wkVal;
		if (!ReturnFlg) ++PC;
		break;


	case Break:
		Code = NextCode();
		if (Code.Kind == SemiC) BreakFlg = true;
		break;

	case Continue:
		Code = NextCode();
		if (Code.Kind == SemiC) ContinueFlg = true;
		break;

	case RBlace:
		Code = NextCode();
		break;

	case VarDec_INT:
	case VarDec_DBL:
	case VarDec_CHR:
		while (Code.Kind != SemiC) {
			Code = NextCode(); // GVarまたはLVar
			if (Code.Kind != LVar && Code.Kind != GVar) break;
			if (CheckNest(Code) - 1 > NestCntr && Code.Kind != GVar)
				ErrExit("スコープ内で定義された変数は、外部からアクセスすることはできません");
			varAdrs = GetMemAddr(Code);
			Expression('=', 0);                               // 代入時に型確定 
			Dmem.set(varAdrs, Stk.Pop());
		}

		++PC;
		
		break;

	case EOF_Line:
	case LBlace:
		++PC;
		break;

	default:
		ErrExit("不正な記述です: ", KindToStr(Code.Kind));
	}
}


void Block() // ブロック終端までの文を実行 
{
	TokenKind k;
	while (!BreakFlg && !ReturnFlg && !ContinueFlg && !ReturnNoneFlg) {  // break,returnで終了
		k = LookCode(PC);                                       // 次の先頭コード 
		if (k == Else || k == ElseIf || k == RBlace) break;              // ブロック正常終了 
		Statement();
	}
}

// 関数呼び出し構文チェック
void FuncCallSyntax(int fnum_)
{
	int argCnt = 0;
	Code = NextCode();
	Code = CheckNextCode(Code, '(');
	if (Code.Kind != ')') {                                       // 引数がある 
		for (;; Code = NextCode()) {
			(void)GetExpression(); ++argCnt;                // 引数式処理と引数個数 
			if (Code.Kind != ',') break;                        // , なら引数が続く 
		}
	}
	Code = CheckNextCode(Code, ')');                                 // ) のはず 
	if (argCnt != GTbl[fnum_].Args)                       // 引数個数チェック 
		ErrExit(GTbl[fnum_].Name, " 関数の引数個数が誤っています。");
	Stk.Push(1.0);                                                // 無難な戻値 
}

int GetExpression(int kind1_, int kind2_) // 結果を返すexpression 
{
	Expression(kind1_, kind2_);
	return Stk.Pop();
}


void Expression(int kind1_, int kind2_) // 確認付きexpression
{
	if (kind1_ != 0) Code = CheckNextCode(Code, kind1_);
	Expression();
	if (kind2_ != 0) Code = CheckNextCode(Code, kind2_);
}

void Expression() // 式 
{
	Term(1);
}

void Term(int _num) 
{
	TokenKind ope;
	if (_num == 8) {
		Factor(); 
		return; 
	}
	Term(_num + 1);
	while (_num == OpOrder(Code.Kind)) { // 強度ごとの一致する演算子の処理               
		ope = Code.Kind;
		Code = NextCode();
		Term(_num + 1);
		if (_num == 1) {
			switch (ope) {
				case Assign:
					Dmem.set(FactorVarAddr, Stk.Pop()); 
					break;
				case PlusEq:
					Dmem.set(FactorVarAddr, Dmem.get(FactorVarAddr) + Stk.Pop());
					break;
				case MinusEq:
					Dmem.set(FactorVarAddr, Dmem.get(FactorVarAddr) - Stk.Pop());
					break;
				case MultiEq:
					Dmem.set(FactorVarAddr, Dmem.get(FactorVarAddr) * Stk.Pop());
					break;
				case DiviEq:
					Dmem.set(FactorVarAddr, Dmem.get(FactorVarAddr) / Stk.Pop());
					break;
				case ModEq:
					Dmem.set(FactorVarAddr, (int)Dmem.get(FactorVarAddr) % (int)Stk.Pop());
					break;
			}
			Stk.Push(Dmem.get(FactorVarAddr));
		}
		else {
			BinaryExpr(ope);
		}
	}
}

void Factor() // 因子 
{
	TokenKind tk = Code.Kind;
	if (SyntaxChkFlg) {                                          // 構文chk時 
		switch (tk) {
		case Not:
		case Minus:
		case Plus:
			Code = NextCode();
			Factor();
			Stk.Pop();
			Stk.Push(1.0);
			break;
		case LParen:
			Expression('(', ')');
			break;
		case IntNum:
		case DblNum:
			Stk.Push(1.0);
			Code = NextCode();
			break;
		case GVar:
		case LVar:
			(void)GetMemAddr(Code);
			Stk.Push(1.0);
			break;
		case FuncCall:
			FuncCallSyntax(Code.SymbolNum);
			break;
		case EOF_Line:
			ErrExit("式が不正です。");
		default:
			ErrExit("式誤り:", KindToStr(Code));            // a + = などで発生 
		}
		return;
	}

	switch (tk) {                                                     // 実行時 
		case Inc:
			Code = NextCode();
			Factor();
			Dmem.set(FactorVarAddr, FactorVarNum + 1);
			Stk.Pop();
			Stk.Push(Dmem.get(FactorVarAddr));
			break;

		case Dec:
			Code = NextCode();
			Factor();
			Dmem.set(FactorVarAddr, FactorVarNum - 1);
			Stk.Push(Stk.Pop() - 1);
			break;
	
		case Not:
		case Minus:
		case Plus:
			Code = NextCode(); // 次の値を取得し 
			Factor();
			if (tk == Not) Stk.Push(!Stk.Pop());                      // !処理する 
			if (tk == Minus) Stk.Push(-Stk.Pop());                    // -処理する 
			break;                                            // 単項+は何もしない 
		case LParen:
			Expression('(', ')');
			break;
		case IntNum:
		case DblNum:
			Stk.Push(Code.IntVal);
			Code = NextCode();
			break;
		case GVar:
		case LVar:
			CheckDtType(Code); // 値設定済みの変数か 
			FactorVarAddr = GetMemAddr(Code); // 変数のアドレスを取得
			FactorVarNum = Dmem.get(FactorVarAddr); // 変数の値を取得
			switch (Code.Kind) {
				case Inc:
					Dmem.set(FactorVarAddr, FactorVarNum + 1);
					Stk.Push(FactorVarNum);
					Code = NextCode();
					break;
				case Dec:
					Dmem.set(FactorVarAddr, FactorVarNum - 1);
					Stk.Push(FactorVarNum);
					Code = NextCode();
					break;
				default:
					Stk.Push(FactorVarNum);
			}
			break;
		case FuncCall:
			FncCall(Code.SymbolNum);
			break;
	}
}

int OpOrder(TokenKind _tk) // 二項演算子の優先順位 
{
	switch (_tk) {
		case Multi:
		case Divi:
		case Mod:
			return 7; // *  /  %  
		case Plus:
		case Minus:
			return 6; // +  -       
		case Less:
		case LessEq:
		case Great:
		case GreatEq:
			return 5; // <  <= > >= 
		case Equal:
		case NotEq:
			return 4; // == !=      
		case DblAnd:
			return 3; // &&         
		case DblOr:
			return 2; // ||   
		case Assign:
		case PlusEq:
		case MinusEq:
		case MultiEq:
		case DiviEq:
		case ModEq:
			return 1; // =
		default:
			return 0; // 該当なし   
	}
}

void BinaryExpr(TokenKind _op) // 二項演算 
{
	int d = 0, d2 = Stk.Pop(), d1 = Stk.Pop();

	if ((_op == Divi || _op == Mod || _op == Divi) && d2 == 0)
		ErrExit("ゼロ除算です。");

	switch (_op) {
		case Plus:    d = d1 + d2;  break;
		case Minus:   d = d1 - d2;  break;
		case Multi:   d = d1 * d2;  break;
		case Divi:    d = d1 / d2; break;
		case Mod:     d = (int)d1 % (int)d2; break;
		case Less:    d = d1 <  d2; break;
		case LessEq:  d = d1 <= d2; break;
		case Great:   d = d1 >  d2; break;
		case GreatEq: d = d1 >= d2; break;
		case Equal:   d = d1 == d2; break;
		case NotEq:   d = d1 != d2; break;
		case DblAnd:  d = d1 && d2; break;
		case DblOr:   d = d1 || d2; break;
	}

	Stk.Push(d);
}

void FncCall(int fncNum_) // 関数呼出 
{
	int  n, argCt = 0;
	vector<int> vc;

	// 実引数積み込み
	NextCode();  // 関数名 ( スキップ *
	Code = NextCode();
	if (Code.Kind != ')') {                                       // 引数がある 
		for (;; Code = NextCode()) {
			Expression();
			++argCt;                          // 引数式処理と引数個数 
			if (Code.Kind != ',') break;                        // , なら引数が続く 
		}
	}

	Code = NextCode();                                            // ) スキップ 

																  // 引数積み込み順序変更
	for (n = 0; n<argCt; n++) vc.push_back(Stk.Pop());  // 後ろから引数積込に修正 
	for (n = 0; n<argCt; n++) { Stk.Push(vc[n]); }

	FncExec(fncNum_);                                                // 関数実行 
}


void FncExec(int fncNum_) // 関数実行 
{
	// 関数入口処理1
	int save_Pc = PC;                                     // 現在の実行行を保存 
	int save_baseReg = BaseReg;                          // 現在のbaseRegを保存 
	int save_spReg = SpReg;                                // 現在のspRegを保存 
	char *save_code_ptr = CodePtr;         // 現在の実行行解析用ポインタを保存 
	CodeManager save_code = Code;  // 現在のcodeを保存 
	DtType saveType;

																// 関数入口処理2		###################################											// 関数入口処理2
	PC = GTbl[fncNum_].Addr;                                   // 新しいPc設定 
	BaseReg = SpReg;                                // 新しいベースレジスタ設定 
	SpReg += GTbl[fncNum_].FrameSize;          
	// フレーム確保 
	Dmem.Resize(SpReg);                            // 主記憶の有効領域確保 
	ReturnValue = 1.0;                                          // 戻り値既定値 
																//cout << "PC: " << PC << endl;
	Code = FirstCode(PC);                                     // 先頭コード取得 
	NextCode(); // Func ( スキップ 

	Code = NextCode();
	if (Code.Kind != ')') {                                         // 引数あり 
		for (;; Code = NextCode()) {
			switch (Code.Kind) {
				case VarDec_INT:
					saveType = INT_Type;
					break;
				case VarDec_DBL:
					saveType = DBL_Type;
					break;
				case VarDec_CHR:
					saveType = CHR_Type;
					break;
			}

			Code = NextCode(); // 変数名のはず

			SetDtType(Code, saveType);                               // 代入時に型確定 
			Dmem.set(GetMemAddr(Code), Stk.Pop());                 // 実引数値格納 
			if (Code.Kind != ',') break;                                // 引数終了 
		}
	}
	Code = NextCode();                                            // ) スキップ 
	++PC;
	Block(); // 関数本体処理 
	ReturnFlg = false;
	ReturnNoneFlg = false;
	// 関数出口処理
	Stk.Push(ReturnValue);                                        // 戻り値設定 
	PC = save_Pc;                                   // 呼出前の環境を復活 
	BaseReg = save_baseReg;
	SpReg = save_spReg;
	CodePtr = save_code_ptr;
	Code = save_code;
}


int GetMemAddr(const CodeManager& cm_)
{
	int addr, index, len;
	addr = GetTopAddr(cm_);
	len = TableP(cm_)->AryLeng;
	Code = NextCode();
	if (len == 0) return addr;                                     // 非配列変数 
	index = GetExpression('[', ']');
	if (index < 0 || len <= index)
		ErrExit(index, " は添字範囲外です（添字範囲:0-", len - 1, "）");
	return addr + index;                                             // 添字加算 
}


// 変数の先頭(配列のときもその先頭)アドレスを返す
int GetTopAddr(const CodeManager& cm_)
{
	switch (cm_.Kind) {
		case GVar: return TableP(cm_)->Addr;
		case LVar: return TableP(cm_)->Addr + BaseReg;
		default: ErrExit("変数名が必要です: ", KindToStr(cm_));
	}
	return 0; 
}

// 確認付コード取得

CodeManager CheckNextCode(const CodeManager& cm_, int kind2_) 
{
	if (cm_.Kind != kind2_) {
		if (kind2_ == EOF_Line) ErrExit("不正な記述です: ", KindToStr(cm_));
		if (cm_.Kind == EOF_Line) ErrExit(KindToStr(kind2_), " が必要です。");
		ErrExit(KindToStr(kind2_) + " が " + KindToStr(cm_) + " の前に必要です。");
	}
	return NextCode();
}

void CheckDtType(const CodeManager& cm_) // 型あり確認 
{
	if (TableP(cm_)->VarType == NON_Type)
		ErrExit("初期化されていない変数が使用されました: ", KindToStr(cm_));
}

int CheckNest(const CodeManager& cm_)
{
	return TableP(cm_)->ScopeCntr;
}

int EndlineOfIf(int line_) // if文の対応}位置 
{
	char *save = CodePtr;
	CodeManager cd = FirstCode(line_); // 先頭を取得
	for (;;) {
		line_ = cd.JmpAddr;
		cd = FirstCode(line_);
		if (cd.Kind == RBlace) break;
	}

	CodePtr = save;

	return line_;
}

int EndlineOfIf_Etc(int line_) // if文の対応}位置 
{
	char *save = CodePtr;
	CodeManager cd = FirstCode(line_); // 先頭を取得

	cd = NextCode(); // 先頭の次を取得
	for (;;) {
		line_ = cd.JmpAddr;
		cd = FirstCode(line_);
		if (cd.Kind == RBlace) break;
	}

	CodePtr = save;

	return line_;
}

void PrintfFunc(string s, int *o, int size)
{
	switch(size + 1) {
		case 1:
			printf(s.c_str(), o[0]);
			break;
		case 2:
			printf(s.c_str(), o[0], o[1]);
			break;
		case 3:
			printf(s.c_str(), o[0], o[1], o[2]);
			break;
		case 4:
			printf(s.c_str(), o[0], o[1], o[2], o[3]);
			break;
		case 5:
			printf(s.c_str(), o[0], o[1], o[2], o[3], o[4]);
			break;
		default:
			ErrExit("インタプリンタのprintfの値引数は5個までです。");
			break;
	}
}

void sysFuncExe(TokenKind tk_) 
{
	int o[10];
	int oCnt = 0;
	int res;
	string str;
	stringstream ss;
	string printfstr, scanfstr;
	int Adrs;
	switch (tk_) {
		case Scanf:
			Code = NextCode();
			Code = CheckNextCode(Code, LParen); // (であるか
			if (Code.Kind != String) {
				ErrExit("scanf関数の書式文字列がありません。");
				break;
			}
			scanfstr = Code.Text; // printfの書式設定　"%d"とか
			Code = NextCode();

			Code = CheckNextCode(Code, Comma);
			scanf(scanfstr.c_str(), &Code.IntVal);
			res = Code.IntVal;   
			Dmem.set(GetMemAddr(Code), res);
			break;
			Code = NextCode();
			Code = CheckNextCode(Code, RParen);
			break;
		case Printf:
			Code = NextCode();
			Code = CheckNextCode(Code, LParen); // (であるか
			if (Code.Kind != String) {
				ErrExit("printf関数の書式文字列がありません。");
				break;
			}
			printfstr = Code.Text; // printfの書式設定　"%d"とか
			Code = NextCode();
			// 文字表示だけの場合
			if (Code.Kind == RParen) {
				printf(printfstr.c_str());
				break;
			}
			Code = CheckNextCode(Code, Comma);
			o[oCnt] = GetExpression();

			ss << o[oCnt]; // テストプログラムに使う結果
			TestOutPut += ss.str();
			ss.clear();

			while (Code.Kind == Comma) {
				Code = NextCode();
				o[++oCnt] = GetExpression();
				ss << o[oCnt]; // テストプログラムに使う結果
				TestOutPut += ss.str();
			}
			Code = CheckNextCode(Code, RParen);
			PrintfFunc(printfstr, o, oCnt);
			break;
	}
}

TokenKind LookCode(int line) // line行の先頭コード 
{
	return (TokenKind)(unsigned char)InterCode[line][0];
}

void SetDtType(const CodeManager& cm_, char typ_) // 型設定 
{
	int memAdrs = GetTopAddr(cm_);
	vector<SymbolTbl>::iterator p = TableP(cm_);

	//if (p->VarType != NON_Type) return; // すでに型が決定している 
	p->VarType = DBL_Type;
	if (p->AryLeng != 0) {                           // 配列なら内容をゼロ初期化 
		for (int n = 0; n < p->AryLeng; n++) { Dmem.set(memAdrs + n, 0); }
	}
}

void CheckEofLine() // コード確認
{
	if (Code.Kind != EOF_Line) ErrExit("不正な記述です: ", KindToStr(Code));
}

// 先頭コード取得
CodeManager FirstCode(int line_)
{
	CodePtr = InterCode[line_]; // ポインタを行先頭に設定
	return NextCode();
}

// コードを取得
CodeManager NextCode() 
{
	TokenKind tk;
	short int jmpAddr, tblNum;
	
	if (*CodePtr == '\0') return CodeManager(EOF_Line);

	tk = (TokenKind)*(unsigned char*)CodePtr++;

	switch (tk)
	{
		case FuncDec_INT:
		case FuncDec_DBL:
		case FuncDec_CHR:
		case While: 
		case For: 
		case Do:
		case If: 
		case ElseIf: 
		case Else:
			jmpAddr = *(short int*)CodePtr;
			CodePtr += sizeof(short int);
			return CodeManager(tk, -1, jmpAddr);
		case String:
			tblNum = *(short int*)CodePtr;
			CodePtr += sizeof(short int);
			return CodeManager(tk, StrLiteral[tblNum].c_str());
		case IntNum:
		case DblNum:
			tblNum = *(short int*)CodePtr;
			CodePtr += sizeof(short int);
			return CodeManager(tk, NumLiteral[tblNum]);
		case FuncCall:
		case GVar:
		case LVar:
			tblNum = *(short int*)CodePtr;
			CodePtr += sizeof(short int);
			return CodeManager(tk, tblNum, -1);
		default:
			return CodeManager(tk);
	}
}

// コードを一つ先読み
CodeManager NextLookCode()
{
	if (*CodePtr == '\0') return CodeManager(EOF_Line);
	return CodeManager((TokenKind)*(unsigned char*)CodePtr);
}

int SetLITERAL(double dbl_)
{
	// NumLiteralの値と引数の値が一致した時にその添え字を返す
	for (int num = 0; num < NumLiteral.size(); ++num) 
		if (NumLiteral[num] == dbl_) 
			return num; 
	
	// 一致しなかった場合, NumLiteralの最後部に値を追加
	NumLiteral.push_back(dbl_); 

	return NumLiteral.size() - 1; // 追加した値の添え字(位置)を返す
}

int SetLITERAL(const string& str_)
{
	for (int num = 0; num < StrLiteral.size(); ++num) {
		if (StrLiteral[num] == str_) {
			return num;
		}
	}

	StrLiteral.push_back(str_);

	return StrLiteral.size() - 1;
}

// 構文チェック
void SyntaxCheck()
{
	SyntaxChkFlg = true;
	for (PC = 1; PC < (int)InterCode.size(); ++PC) {
		Code = FirstCode(PC);
		switch (Code.Kind)
		{
			// 確認済み
		case FuncDec_INT:
		case FuncDec_DBL:
		case FuncDec_CHR:
		case VarDec_INT:
		case VarDec_DBL:
		case VarDec_CHR:
			break;
			//case Else:
		case SemiC:
		case RBlace:
			Code = NextCode();
			CheckEofLine();
		case If:
		case While:
			IfWhileSyntax();
			CheckEofLine();
			break;
		case For:
			ForSyntax();
			CheckEofLine();
			break;
		case FuncCall:
			FuncCallSyntax(Code.SymbolNum);
			CheckEofLine();
			(void)Stk.Pop();
			break;
		case GVar:
		case LVar:
			(void)GetMemAddr(Code);
			(void)GetExpression('=', EOF_Line);
			break;
		case Return:
			Code = NextCode();
			if (Code.Kind != EOF_Line) (void)GetExpression();
			CheckEofLine();
			break;
		case Break:
		case Continue:

			Code = NextCode();
			CheckEofLine();
			break;
		case EOF_Line:
			break;
		default:
			ErrExit("不正な記述です: ", KindToStr(Code.Kind));
		}
	}
}

void IfWhileSyntax()
{
	Code = NextCode();
	Code = CheckNextCode(Code, '(');
	GetExpression();
	Code = CheckNextCode(Code, ')');
}

void ForSyntax()
{
	Code = NextCode();
	Code = CheckNextCode(Code, '(');

	Code = CheckNextCode(Code, ';');
	GetExpression();
	Code = CheckNextCode(Code, ';');
	GetExpression();
	Code = CheckNextCode(Code, ')');
}