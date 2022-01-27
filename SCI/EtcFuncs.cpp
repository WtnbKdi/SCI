#pragma once

#include "SCI.h"
#include "Prototype.h"


// �G���[���
string ErrMsg(const string& str1_, const string& str2_)
{
	if (str1_ == "") return str2_ + " ���K�v�ł��B";
	if (str2_ == "") return str1_ + " ���s���ł��B";
	return str2_ + " �� " + str1_ + " �̑O�ɕK�v�ł��B";
}

// �G���[�\��
void ErrExit(TypeObj fst1_, TypeObj fst2_, TypeObj fst3_, TypeObj fst4_)
{
	TypeObj toary[] = { fst1_, fst2_, fst3_, fst4_ };
	unsigned siz = 4;
	cerr << "�s�ԍ� :" << GetLineNo() << " �G���[ :";
	for(int i = 0; i < siz && toary[i].Str != "\1"; ++i) {
		if (toary[i].Type == 'd') cout << toary[i].Dbl;  // ���l���
		if (toary[i].Type == 's') cout << toary[i].Str;  // ��������
	}
	exit(1);
}

string DblToStr(double dnum_)
{
	ostringstream ostr;
	ostr << dnum_;
	return ostr.str();
}