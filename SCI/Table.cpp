#pragma once

#include "SCI.h"
#include "Prototype.h"

vector<SymbolTbl> GTbl;  // 大域記号表
vector<SymbolTbl> LTbl;  // 局所記号表
int StartLTbl;           // 局所開始位置
int Enter(SymbolTbl& tb_, SymbolKind kind_) // 記号表登録 
{
	int num, memSiz;
	bool isLocal = IsLocalName(tb_.Name, kind_); 
	extern int Local_Addr;  // 局所変数アドレス管理 
	extern Mymemory Dmem;  // 主記憶 

	// 確認
	memSiz = tb_.AryLeng;
	if (memSiz == 0) memSiz = 1; // 単純変数の場合 
	tb_.SymbolKind = kind_;
	num = -1; // 重複確認 
	if (kind_ == FuncID)  num = SearchName(tb_.Name, 'G');
	if (kind_ == ParaID) num = SearchName(tb_.Name, 'L');
	if (num != -1) ErrExit("名前が重複しています: ", tb_.Name);

	// アドレス設定
	if (kind_ == FuncID) {
		tb_.Addr = GetLineNo(); // 関数開始行
	}
	else {
		if (isLocal) {  // ローカル 
			tb_.Addr = Local_Addr;
			Local_Addr += memSiz;
		}      
		else { // グローバル
			tb_.Addr = Dmem.size();                                          
			Dmem.resize(Dmem.size() + memSiz); // 大域領域確保 
		}
	}

	// 登録
	if (isLocal) { // 局所 
		num = LTbl.size(); 

		LTbl.push_back(tb_); 
	}           
	else { // 大域 
		num = GTbl.size(); 
		GTbl.push_back(tb_); 
	}           
	return num; // 登録位置 
}

void SetStartLTbl() // 局所記号表開始位置
{
	StartLTbl = LTbl.size();
}

int SearchName(const string& str_, int mode) // 名前検索 
{
	int num;
	switch (mode) {
	case 'G':  // 大域記号表検索
		for (num = 0; num < (int)GTbl.size(); ++num) {
			if (GTbl[num].Name == str_) return num;
		}
		break;
	case 'L':  // 局所記号表検索 
		for (num = StartLTbl; num<(int)LTbl.size(); ++num) {
			if (LTbl[num].Name == str_) return num;
		}
		break;
	case 'F':  // 関数名検索
		num = SearchName(str_, 'G');
		if (num != -1 && GTbl[num].SymbolKind == FuncID) return num;
		break;
	case 'V':  // 変数名検索 
		if (SearchName(str_, 'F') != -1) ErrExit("関数名と重複しています: ", str_);
		if ((num = SearchName(str_, 'G')) != -1) return num;    // グローバル変数実装
		if (IsLocalScope())  return SearchName(str_, 'L');      // 局所領域処理中 
		else                 return SearchName(str_, 'G');      // 大域領域処理中
	}
	return -1; // 見つからない
}

// ローカル変数であるか
bool IsLocalName(const string& name, SymbolKind kind) // 局所名なら真 
{
	if (SearchName(name, 'F') != -1) ErrExit("関数名と重複しています: ", name);
	if (SearchName(name, 'G') != -1) return false;
	else return ((kind == VarID) && IsLocalScope()) || (kind == ParaID);
}

vector<SymbolTbl>::iterator TableP(const CodeManager& cm_) // 反復子取得 
{
	if (cm_.Kind == LVar) 
		return LTbl.begin() + cm_.SymbolNum;            
	return 
		GTbl.begin() + cm_.SymbolNum;
	
	// Gvar Fcall 
}