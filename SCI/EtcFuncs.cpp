#pragma once

#include "SCI.h"
#include "Prototype.h"


// エラー情報
string ErrMsg(const string& str1_, const string& str2_)
{
	if (str1_ == "") return str2_ + " が必要です。";
	if (str2_ == "") return str1_ + " が不正です。";
	return str2_ + " が " + str1_ + " の前に必要です。";
}

// エラー表示
void ErrExit(TypeObj fst1_, TypeObj fst2_, TypeObj fst3_, TypeObj fst4_)
{
	TypeObj toary[] = { fst1_, fst2_, fst3_, fst4_ };
	unsigned siz = 4;
	cerr << "行番号 :" << GetLineNo() << " エラー :";
	for(int i = 0; i < siz && toary[i].Str != "\1"; ++i) {
		if (toary[i].Type == 'd') cout << toary[i].Dbl;  // 数値情報
		if (toary[i].Type == 's') cout << toary[i].Str;  // 文字列情報
	}
	exit(1);
}

string DblToStr(double dnum_)
{
	ostringstream ostr;
	ostr << dnum_;
	return ostr.str();
}