#pragma once

#include "SCI.h"
#include "Prototype.h"

vector<SymbolTbl> GTbl;  // ���L���\
vector<SymbolTbl> LTbl;  // �Ǐ��L���\
int StartLTbl;           // �Ǐ��J�n�ʒu
int Enter(SymbolTbl& tb_, SymbolKind kind_) // �L���\�o�^ 
{
	int num, memSiz;
	bool isLocal = IsLocalName(tb_.Name, kind_); 
	extern int Local_Addr;  // �Ǐ��ϐ��A�h���X�Ǘ� 
	extern Mymemory Dmem;  // ��L�� 

	// �m�F
	memSiz = tb_.AryLeng;
	if (memSiz == 0) memSiz = 1; // �P���ϐ��̏ꍇ 
	tb_.SymbolKind = kind_;
	num = -1; // �d���m�F 
	if (kind_ == FuncID)  num = SearchName(tb_.Name, 'G');
	if (kind_ == ParaID) num = SearchName(tb_.Name, 'L');
	if (num != -1) ErrExit("���O���d�����Ă��܂�: ", tb_.Name);

	// �A�h���X�ݒ�
	if (kind_ == FuncID) {
		tb_.Addr = GetLineNo(); // �֐��J�n�s
	}
	else {
		if (isLocal) {  // ���[�J�� 
			tb_.Addr = Local_Addr;
			Local_Addr += memSiz;
		}      
		else { // �O���[�o��
			tb_.Addr = Dmem.size();                                          
			Dmem.resize(Dmem.size() + memSiz); // ���̈�m�� 
		}
	}

	// �o�^
	if (isLocal) { // �Ǐ� 
		num = LTbl.size(); 

		LTbl.push_back(tb_); 
	}           
	else { // ��� 
		num = GTbl.size(); 
		GTbl.push_back(tb_); 
	}           
	return num; // �o�^�ʒu 
}

void SetStartLTbl() // �Ǐ��L���\�J�n�ʒu
{
	StartLTbl = LTbl.size();
}

int SearchName(const string& str_, int mode) // ���O���� 
{
	int num;
	switch (mode) {
	case 'G':  // ���L���\����
		for (num = 0; num < (int)GTbl.size(); ++num) {
			if (GTbl[num].Name == str_) return num;
		}
		break;
	case 'L':  // �Ǐ��L���\���� 
		for (num = StartLTbl; num<(int)LTbl.size(); ++num) {
			if (LTbl[num].Name == str_) return num;
		}
		break;
	case 'F':  // �֐�������
		num = SearchName(str_, 'G');
		if (num != -1 && GTbl[num].SymbolKind == FuncID) return num;
		break;
	case 'V':  // �ϐ������� 
		if (SearchName(str_, 'F') != -1) ErrExit("�֐����Əd�����Ă��܂�: ", str_);
		if ((num = SearchName(str_, 'G')) != -1) return num;    // �O���[�o���ϐ�����
		if (IsLocalScope())  return SearchName(str_, 'L');      // �Ǐ��̈揈���� 
		else                 return SearchName(str_, 'G');      // ���̈揈����
	}
	return -1; // ������Ȃ�
}

// ���[�J���ϐ��ł��邩
bool IsLocalName(const string& name, SymbolKind kind) // �Ǐ����Ȃ�^ 
{
	if (SearchName(name, 'F') != -1) ErrExit("�֐����Əd�����Ă��܂�: ", name);
	if (SearchName(name, 'G') != -1) return false;
	else return ((kind == VarID) && IsLocalScope()) || (kind == ParaID);
}

vector<SymbolTbl>::iterator TableP(const CodeManager& cm_) // �����q�擾 
{
	if (cm_.Kind == LVar) 
		return LTbl.begin() + cm_.SymbolNum;            
	return 
		GTbl.begin() + cm_.SymbolNum;
	
	// Gvar Fcall 
}