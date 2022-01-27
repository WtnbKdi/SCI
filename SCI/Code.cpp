#pragma once

#include "SCI.h"
#include "Prototype.h"
CodeManager Code;                         // �R�[�h�Ǘ�
int StartPC;                              // ���s�J�n�s 
int PC = -1;                              // �v���O�����J�E���^ ����s��: -1
int BaseReg;                              // �x�[�X���W�X�^ 
int SpReg;                                // �X�^�b�N�|�C���^ 
int MaxLine;                              // �v���O���������s 
vector<char*> InterCode;                  // �ϊ��ςݓ����R�[�h�i�[ 
char *CodePtr;                            // �����R�[�h��͗p�|�C���^  
double ReturnValue;                       // �֐��ߒl 
bool BreakFlg, ReturnFlg, ReturnNoneFlg, ContinueFlg; // ����p�t���O 
Mymemory Dmem;                            // ��L��  
vector<string> StrLiteral;                // �����񃊃e�����i�[  
vector<double> NumLiteral;                // ���l���e�����i�[  
bool SyntaxChkFlg = false;                // �\���`�F�b�N�̂Ƃ��^  
extern vector<SymbolTbl> GTbl;            // ���L���\  
int FactorVarAddr;						  // (Factor)�ϐ��A�h���X
double FactorVarNum;					  // (Factor)�ϐ��l
unsigned NestCntr;                        // {}�̐[�x
extern vector<int> GVarIni;          // �O���[�o���ϐ��ɒ�`������ꍇ
//unsigned oneBlockPC = -1; // {}�ȗ����̍ŏIPC

const int IncDecSize = 100;
vector<double> PostInc(IncDecSize, 0);
vector<double> PostDec(IncDecSize, 0);

string TestOutPut;       // �e�X�g���ʂ̕ۑ�

class MyStack // stack<double>�̃��b�p�[�N���X
{
private:
	stack<int> st;
public:
	void Push(int num) { st.push(num); }          // �ςݍ���
	int Size() { return (int)st.size(); }            // �X�^�b�N�T�C�Y
	bool Empty() { return st.empty(); }              // �X�^�b�N����ł��邩
	int Pop()                                      // ��ԏ�����o���č폜
	{  
		if (st.empty()) ErrExit("�X�^�b�N�A���_�[�t���["); 
		int topNum = st.top(); // ��ԏ�̒l���X�^�b�N������o���i�[
		st.pop();                 // ��ԏ�̒l���X�^�b�N�������
		return topNum;             // ��ԏ�̒l��Ԃ�
	}
	void Clear() {
		while (!st.empty()) st.pop();
	}
};

MyStack Stk; // �I�y�����h�X�^�b�N

void StartPrgCntr(int n)
{
	StartPC = n;
}

void Execute() // ���s 
{
	//Dmem.clear(); // ���Ӂ@������������������ƃO���[�o���A���[�J���ϐ����o�O��
	//Stk.Clear();
	//PC = -1;
	//oneBlockPC = -1;

	NestCntr = 0;
	BaseReg = 0;                                        // �x�[�X���W�X�^�����l 
	
	SpReg = Dmem.size();                              // �X�^�b�N�|�C���^�����l 
	Dmem.resize(SpReg + 1000);                              // ��L���̈揉���m�� 

	BreakFlg = ReturnFlg = ReturnNoneFlg = ContinueFlg = false;
	MaxLine = InterCode.size() - 1;

	// �O���o�[���ϐ��ɑ������������ꍇ
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
	PC = -1;                                                      // ����s��� 
}

void LookIntrCode()
{
	BaseReg = 0; // �x�[�X���W�X�^�����l 
	SpReg = Dmem.size(); // �X�^�b�N�|�C���^�����l 
	Dmem.resize(SpReg + 1000); // ��L���̈揉���m�� 

	BreakFlg = ReturnFlg = ReturnNoneFlg = ContinueFlg = false;
	PC = StartPC;
	MaxLine = InterCode.size();

	PC = 1;
	while (PC < MaxLine) {
		std::cout << "PC: " << PC << " ";
		for (auto a = FirstCode(PC); a.Kind != EOF_Line; a = NextCode()) {
			std::cout << "(";
			std::cout << "���: " << a.Kind;
			std::cout << " �l: " << a.IntVal;
			std::cout << " �A�h���X: " << a.JmpAddr;
			std::cout << ")";
		}
		++PC;
		std::cout << endl;
	}

}

void ForExpr3Skip(int topline) // for���O�Ԗڂ̎��̍ŏ��̎�O�܂Ŕ�΂�
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

// �����̃f�t�H���g: void Statement(bool oneStmt = false);
void Statement() // �� 
{
	TokenKind tkIncDec = Others; // �O�u�C���N�������g���Z�q�̎����i�[
	CodeManager save, cm;

	int top_line, // while, if�Ȃǂ̐擪�s�ԍ�
		end_line, // }�̍s�ԍ�
		varAdrs, // �ϐ��A�h���X
		end_line_last; // if else�Ȃ�, �Ō��}�̍s�ԍ����i�[

	double wkVal, midDt, endDt, stepDt;
	bool eofChkFlag, lblaceFlg, forLoopFlg, forExpr2Flg, forExpr3Flg,
		forExpr3PostIncDecFlg, forExpr3ExprFlg;

	eofChkFlag = lblaceFlg = forLoopFlg = forExpr2Flg = forExpr3Flg = false;
	forExpr3PostIncDecFlg = forExpr3ExprFlg = false;

	if (PC > MaxLine) return;                       // �v���O�����I��
	Code = save = FirstCode(PC);
	top_line = PC; // ����͈͂̐擪�ƏI�[  
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

		PC = end_line; // if����}
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
		Code = NextCode(); // ( ������͂�
		Code = CheckNextCode(Code, LParen); // ( �ł��邩
		
		if (Code.Kind == GVar || Code.Kind == LVar) { // for( [~] ; ~ ; ~)
			save = Code; // �ϐ�
			varAdrs = GetMemAddr(Code); // �ϐ��̃A�h���X
			Expression('=', 0); // �����(��)�̉��
			Dmem.set(varAdrs, Stk.Pop()); // �����(��)�̒l��ϐ��ɃZ�b�g
		}

		Code = CheckNextCode(Code, SemiC); // for( ~ [;] ~ ; ~)�@�Z�~�R�����ł��邩�m�F�����̃R�[�h��Ԃ��A�������G���[
		
		if (Code.Kind != SemiC) { // // for( ~ ; [~] ; ~) ������������ꍇ
			forLoopFlg = GetExpression(0, 0); // for( ~ ; [~] ; ~) �����������
			if (!forLoopFlg) { // ��������false�ł����for���𔲂���
				PC = end_line + 1; // ���s�s�ԍ���}�̉��ɐݒ�
				NestCntr -= 1;
				break;
			}
			forExpr2Flg = true;
		}

		forLoopFlg = true;

		Code = CheckNextCode(Code, SemiC); // for( ~ ; ~ [;] ~)

		if (Code.Kind != RParen) { // for( ~ ; ~ ; [�����Ɏ�������ꍇ] ) 
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

		while (forLoopFlg) { // ���ڂ̃��[�v����
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

			if (Code.Kind != RParen) { // for( ~ ; ~ ; [�����Ɏ�������ꍇ] ) 
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

	case FuncCall:// ����̂Ȃ��֐��ďo 
		FncCall(Code.SymbolNum);
		(void)Stk.Pop();// �߂�l�s�v 
		++PC;
		break;

	case FuncDec_INT:// �֐���`�̓X�L�b�v
	case FuncDec_DBL:
	case FuncDec_CHR:
		PC = end_line + 1;
		break;

	case GVar:
	case LVar:// ����� 
		if (CheckNest(Code) - 1 > NestCntr && Code.Kind != GVar) {
			ErrExit("�X�R�[�v���Œ�`���ꂽ�ϐ��́A�O������A�N�Z�X���邱�Ƃ͂ł��܂���");
			
		}
		switch (NextLookCode().Kind) { // ���ǂ݂���
		case LSBracket:
		case Assign:
			varAdrs = GetMemAddr(Code);
			Expression('=', 0);                                // ������Ɍ^�m�� 
			Dmem.set(varAdrs, Stk.Pop());
			break;
		case PlusEq:
			varAdrs = GetMemAddr(Code);
			Expression(PlusEq, 0);                                // ������Ɍ^�m�� 
			Dmem.set(varAdrs, Dmem.get(varAdrs) + Stk.Pop());
			break;
		case MinusEq:
			varAdrs = GetMemAddr(Code);
			Expression(MinusEq, 0);                                 // ������Ɍ^�m�� 
			Dmem.set(varAdrs, Dmem.get(varAdrs) - Stk.Pop());
			break;
		case MultiEq:
			varAdrs = GetMemAddr(Code);
			Expression(MultiEq, 0);                                // ������Ɍ^�m�� 
			Dmem.set(varAdrs, Dmem.get(varAdrs) * Stk.Pop());
			break;
		case DiviEq:
			varAdrs = GetMemAddr(Code);
			Expression(DiviEq, 0);                               // ������Ɍ^�m�� 
			Dmem.set(varAdrs, Dmem.get(varAdrs) / Stk.Pop());
			break;
		case ModEq:
			varAdrs = GetMemAddr(Code);
			Expression(ModEq, 0);                                // ������Ɍ^�m�� 
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
		if (NextLookCode().Kind == SemiC) { // return; �̏ꍇ
			BreakFlg = true;
			ReturnNoneFlg = true;
			break;
		}

		wkVal = ReturnValue;
		Code = NextCode();

		// �u���v������Ζ߂�l���v�Z 
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
			Code = NextCode(); // GVar�܂���LVar
			if (Code.Kind != LVar && Code.Kind != GVar) break;
			if (CheckNest(Code) - 1 > NestCntr && Code.Kind != GVar)
				ErrExit("�X�R�[�v���Œ�`���ꂽ�ϐ��́A�O������A�N�Z�X���邱�Ƃ͂ł��܂���");
			varAdrs = GetMemAddr(Code);
			Expression('=', 0);                               // ������Ɍ^�m�� 
			Dmem.set(varAdrs, Stk.Pop());
		}

		++PC;
		
		break;

	case EOF_Line:
	case LBlace:
		++PC;
		break;

	default:
		ErrExit("�s���ȋL�q�ł�: ", KindToStr(Code.Kind));
	}
}


void Block() // �u���b�N�I�[�܂ł̕������s 
{
	TokenKind k;
	while (!BreakFlg && !ReturnFlg && !ContinueFlg && !ReturnNoneFlg) {  // break,return�ŏI��
		k = LookCode(PC);                                       // ���̐擪�R�[�h 
		if (k == Else || k == ElseIf || k == RBlace) break;              // �u���b�N����I�� 
		Statement();
	}
}

// �֐��Ăяo���\���`�F�b�N
void FuncCallSyntax(int fnum_)
{
	int argCnt = 0;
	Code = NextCode();
	Code = CheckNextCode(Code, '(');
	if (Code.Kind != ')') {                                       // ���������� 
		for (;; Code = NextCode()) {
			(void)GetExpression(); ++argCnt;                // �����������ƈ����� 
			if (Code.Kind != ',') break;                        // , �Ȃ���������� 
		}
	}
	Code = CheckNextCode(Code, ')');                                 // ) �̂͂� 
	if (argCnt != GTbl[fnum_].Args)                       // �������`�F�b�N 
		ErrExit(GTbl[fnum_].Name, " �֐��̈�����������Ă��܂��B");
	Stk.Push(1.0);                                                // ����Ȗߒl 
}

int GetExpression(int kind1_, int kind2_) // ���ʂ�Ԃ�expression 
{
	Expression(kind1_, kind2_);
	return Stk.Pop();
}


void Expression(int kind1_, int kind2_) // �m�F�t��expression
{
	if (kind1_ != 0) Code = CheckNextCode(Code, kind1_);
	Expression();
	if (kind2_ != 0) Code = CheckNextCode(Code, kind2_);
}

void Expression() // �� 
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
	while (_num == OpOrder(Code.Kind)) { // ���x���Ƃ̈�v���鉉�Z�q�̏���               
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

void Factor() // ���q 
{
	TokenKind tk = Code.Kind;
	if (SyntaxChkFlg) {                                          // �\��chk�� 
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
			ErrExit("�����s���ł��B");
		default:
			ErrExit("�����:", KindToStr(Code));            // a + = �ȂǂŔ��� 
		}
		return;
	}

	switch (tk) {                                                     // ���s�� 
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
			Code = NextCode(); // ���̒l���擾�� 
			Factor();
			if (tk == Not) Stk.Push(!Stk.Pop());                      // !�������� 
			if (tk == Minus) Stk.Push(-Stk.Pop());                    // -�������� 
			break;                                            // �P��+�͉������Ȃ� 
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
			CheckDtType(Code); // �l�ݒ�ς݂̕ϐ��� 
			FactorVarAddr = GetMemAddr(Code); // �ϐ��̃A�h���X���擾
			FactorVarNum = Dmem.get(FactorVarAddr); // �ϐ��̒l���擾
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

int OpOrder(TokenKind _tk) // �񍀉��Z�q�̗D�揇�� 
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
			return 0; // �Y���Ȃ�   
	}
}

void BinaryExpr(TokenKind _op) // �񍀉��Z 
{
	int d = 0, d2 = Stk.Pop(), d1 = Stk.Pop();

	if ((_op == Divi || _op == Mod || _op == Divi) && d2 == 0)
		ErrExit("�[�����Z�ł��B");

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

void FncCall(int fncNum_) // �֐��ďo 
{
	int  n, argCt = 0;
	vector<int> vc;

	// �������ςݍ���
	NextCode();  // �֐��� ( �X�L�b�v *
	Code = NextCode();
	if (Code.Kind != ')') {                                       // ���������� 
		for (;; Code = NextCode()) {
			Expression();
			++argCt;                          // �����������ƈ����� 
			if (Code.Kind != ',') break;                        // , �Ȃ���������� 
		}
	}

	Code = NextCode();                                            // ) �X�L�b�v 

																  // �����ςݍ��ݏ����ύX
	for (n = 0; n<argCt; n++) vc.push_back(Stk.Pop());  // ��납������ύ��ɏC�� 
	for (n = 0; n<argCt; n++) { Stk.Push(vc[n]); }

	FncExec(fncNum_);                                                // �֐����s 
}


void FncExec(int fncNum_) // �֐����s 
{
	// �֐���������1
	int save_Pc = PC;                                     // ���݂̎��s�s��ۑ� 
	int save_baseReg = BaseReg;                          // ���݂�baseReg��ۑ� 
	int save_spReg = SpReg;                                // ���݂�spReg��ۑ� 
	char *save_code_ptr = CodePtr;         // ���݂̎��s�s��͗p�|�C���^��ۑ� 
	CodeManager save_code = Code;  // ���݂�code��ۑ� 
	DtType saveType;

																// �֐���������2		###################################											// �֐���������2
	PC = GTbl[fncNum_].Addr;                                   // �V����Pc�ݒ� 
	BaseReg = SpReg;                                // �V�����x�[�X���W�X�^�ݒ� 
	SpReg += GTbl[fncNum_].FrameSize;          
	// �t���[���m�� 
	Dmem.Resize(SpReg);                            // ��L���̗L���̈�m�� 
	ReturnValue = 1.0;                                          // �߂�l����l 
																//cout << "PC: " << PC << endl;
	Code = FirstCode(PC);                                     // �擪�R�[�h�擾 
	NextCode(); // Func ( �X�L�b�v 

	Code = NextCode();
	if (Code.Kind != ')') {                                         // �������� 
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

			Code = NextCode(); // �ϐ����̂͂�

			SetDtType(Code, saveType);                               // ������Ɍ^�m�� 
			Dmem.set(GetMemAddr(Code), Stk.Pop());                 // �������l�i�[ 
			if (Code.Kind != ',') break;                                // �����I�� 
		}
	}
	Code = NextCode();                                            // ) �X�L�b�v 
	++PC;
	Block(); // �֐��{�̏��� 
	ReturnFlg = false;
	ReturnNoneFlg = false;
	// �֐��o������
	Stk.Push(ReturnValue);                                        // �߂�l�ݒ� 
	PC = save_Pc;                                   // �ďo�O�̊��𕜊� 
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
	if (len == 0) return addr;                                     // ��z��ϐ� 
	index = GetExpression('[', ']');
	if (index < 0 || len <= index)
		ErrExit(index, " �͓Y���͈͊O�ł��i�Y���͈�:0-", len - 1, "�j");
	return addr + index;                                             // �Y�����Z 
}


// �ϐ��̐擪(�z��̂Ƃ������̐擪)�A�h���X��Ԃ�
int GetTopAddr(const CodeManager& cm_)
{
	switch (cm_.Kind) {
		case GVar: return TableP(cm_)->Addr;
		case LVar: return TableP(cm_)->Addr + BaseReg;
		default: ErrExit("�ϐ������K�v�ł�: ", KindToStr(cm_));
	}
	return 0; 
}

// �m�F�t�R�[�h�擾

CodeManager CheckNextCode(const CodeManager& cm_, int kind2_) 
{
	if (cm_.Kind != kind2_) {
		if (kind2_ == EOF_Line) ErrExit("�s���ȋL�q�ł�: ", KindToStr(cm_));
		if (cm_.Kind == EOF_Line) ErrExit(KindToStr(kind2_), " ���K�v�ł��B");
		ErrExit(KindToStr(kind2_) + " �� " + KindToStr(cm_) + " �̑O�ɕK�v�ł��B");
	}
	return NextCode();
}

void CheckDtType(const CodeManager& cm_) // �^����m�F 
{
	if (TableP(cm_)->VarType == NON_Type)
		ErrExit("����������Ă��Ȃ��ϐ����g�p����܂���: ", KindToStr(cm_));
}

int CheckNest(const CodeManager& cm_)
{
	return TableP(cm_)->ScopeCntr;
}

int EndlineOfIf(int line_) // if���̑Ή�}�ʒu 
{
	char *save = CodePtr;
	CodeManager cd = FirstCode(line_); // �擪���擾
	for (;;) {
		line_ = cd.JmpAddr;
		cd = FirstCode(line_);
		if (cd.Kind == RBlace) break;
	}

	CodePtr = save;

	return line_;
}

int EndlineOfIf_Etc(int line_) // if���̑Ή�}�ʒu 
{
	char *save = CodePtr;
	CodeManager cd = FirstCode(line_); // �擪���擾

	cd = NextCode(); // �擪�̎����擾
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
			ErrExit("�C���^�v�����^��printf�̒l������5�܂łł��B");
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
			Code = CheckNextCode(Code, LParen); // (�ł��邩
			if (Code.Kind != String) {
				ErrExit("scanf�֐��̏��������񂪂���܂���B");
				break;
			}
			scanfstr = Code.Text; // printf�̏����ݒ�@"%d"�Ƃ�
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
			Code = CheckNextCode(Code, LParen); // (�ł��邩
			if (Code.Kind != String) {
				ErrExit("printf�֐��̏��������񂪂���܂���B");
				break;
			}
			printfstr = Code.Text; // printf�̏����ݒ�@"%d"�Ƃ�
			Code = NextCode();
			// �����\�������̏ꍇ
			if (Code.Kind == RParen) {
				printf(printfstr.c_str());
				break;
			}
			Code = CheckNextCode(Code, Comma);
			o[oCnt] = GetExpression();

			ss << o[oCnt]; // �e�X�g�v���O�����Ɏg������
			TestOutPut += ss.str();
			ss.clear();

			while (Code.Kind == Comma) {
				Code = NextCode();
				o[++oCnt] = GetExpression();
				ss << o[oCnt]; // �e�X�g�v���O�����Ɏg������
				TestOutPut += ss.str();
			}
			Code = CheckNextCode(Code, RParen);
			PrintfFunc(printfstr, o, oCnt);
			break;
	}
}

TokenKind LookCode(int line) // line�s�̐擪�R�[�h 
{
	return (TokenKind)(unsigned char)InterCode[line][0];
}

void SetDtType(const CodeManager& cm_, char typ_) // �^�ݒ� 
{
	int memAdrs = GetTopAddr(cm_);
	vector<SymbolTbl>::iterator p = TableP(cm_);

	//if (p->VarType != NON_Type) return; // ���łɌ^�����肵�Ă��� 
	p->VarType = DBL_Type;
	if (p->AryLeng != 0) {                           // �z��Ȃ���e���[�������� 
		for (int n = 0; n < p->AryLeng; n++) { Dmem.set(memAdrs + n, 0); }
	}
}

void CheckEofLine() // �R�[�h�m�F
{
	if (Code.Kind != EOF_Line) ErrExit("�s���ȋL�q�ł�: ", KindToStr(Code));
}

// �擪�R�[�h�擾
CodeManager FirstCode(int line_)
{
	CodePtr = InterCode[line_]; // �|�C���^���s�擪�ɐݒ�
	return NextCode();
}

// �R�[�h���擾
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

// �R�[�h�����ǂ�
CodeManager NextLookCode()
{
	if (*CodePtr == '\0') return CodeManager(EOF_Line);
	return CodeManager((TokenKind)*(unsigned char*)CodePtr);
}

int SetLITERAL(double dbl_)
{
	// NumLiteral�̒l�ƈ����̒l����v�������ɂ��̓Y������Ԃ�
	for (int num = 0; num < NumLiteral.size(); ++num) 
		if (NumLiteral[num] == dbl_) 
			return num; 
	
	// ��v���Ȃ������ꍇ, NumLiteral�̍Ō㕔�ɒl��ǉ�
	NumLiteral.push_back(dbl_); 

	return NumLiteral.size() - 1; // �ǉ������l�̓Y����(�ʒu)��Ԃ�
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

// �\���`�F�b�N
void SyntaxCheck()
{
	SyntaxChkFlg = true;
	for (PC = 1; PC < (int)InterCode.size(); ++PC) {
		Code = FirstCode(PC);
		switch (Code.Kind)
		{
			// �m�F�ς�
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
			ErrExit("�s���ȋL�q�ł�: ", KindToStr(Code.Kind));
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