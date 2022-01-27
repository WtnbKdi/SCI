#include <iostream>
#include <array>	// array
#include <cstdio>	// _popen
#include <memory>	// shared_ptr
#include <string>	// string
#include "Prototype.h"
extern vector<SymbolTbl> LTbl;
extern vector<SymbolTbl> GTbl;

extern string TestOutPut;       // テスト結果の保存
extern int TestOutPutCnt;

// 入力関数の確認プログラム 場所
const char* InputCheck[] = {
	"check\\input1.c",
	"end",
};

// 変数定義の確認プログラム 場所
const char* VarDefineCheck[] = {
	"check\\vardefine1.c",
	"check\\vardefine2.c",
	"check\\vardefine3.c",
	"check\\vardefine4.c",
	"end",
};

// グローバル変数の確認プログラム 場所
const char* GlobalVarCheck[] = {
	"check\\globalvar1.c",
	"check\\globalvar2.c",
	"check\\globalvar3.c",
	"end",
};
// 式の確認プログラム 場所
const char* ExprCheck[] = {
	"check\\expr1.c",
	"check\\expr2.c",
	"check\\expr3.c",
	"check\\expr4.c",
	"end",
};

// (演算子)代入演算子の確認プログラム 場所
const char* OpeAssignCheck[] = {
	"check\\opeassign1.c",
	"end",
};

// 論理演算子の確認プログラム 場所
const char* LogicOpeCheck[] = {
	"check\\logicope1.c",
	"check\\logicope2.c",
	"end",
};

// 比較演算子の確認プログラム 場所
const char* CondOpeCheck[] = {
	"check\\condope1.c",
	"end",
};

// 比較演算子と論理演算子の確認プログラム 場所
const char* LogiCondopeCheck[] = {
	"check\\logicondope1.c",
	"end",
};

// 配列の確認プログラム 場所
const char* ArrayCheck[] = {
	"check\\array1.c",
	"end",
};

// 関数の確認プログラム 場所
const char* FuncCheck[] = {
	"check\\func1.c",
	"check\\func2.c",
	"check\\func3.c",
	"end",
};

// if文の確認プログラム 場所
const char* IfCheck[] = {
	"check\\if1.c",
	"check\\if2.c",
	"check\\if3.c",
	"check\\if4.c",
	"check\\if5.c",
	"check\\if6.c",
	"end",
};

// for文の確認プログラム 場所
const char* ForCheck[] = {
	"check\\for1.c",
	"check\\for2.c",
	"check\\for3.c",
	"end",
};

// while文の確認プログラム 場所
const char* WhileCheck[] = {
	"check\\while1.c",
	"check\\while2.c",
	"check\\while3.c",
	"end",
};

// do-while文の確認プログラム 場所
const char* DoWhileCheck[] = {
	"check\\dowhile1.c",
	"check\\dowhile2.c",
	"check\\dowhile3.c",
	"end",
};

// 再帰関数の確認プログラム 場所
const char* SaikiCheck[] = {
	"check\\saiki1.c",
	"end",
};

// 後置インクリメントの確認プログラム 場所
const char* PostIncCheck[] = {
	"check\\postinc1.c",
	"check\\postinc2.c",
	"check\\postinc3.c",
	"check\\postinc4.c",
	"end",
};

// 後置デクリメントの確認プログラム 場所
const char* PostDecCheck[] = {
	"check\\postdec1.c",
	"check\\postdec2.c",
	"check\\postdec3.c",
	"check\\postdec4.c",
	"end",
};

// 前置インクリメントの確認プログラム 場所
const char* UnaryIncCheck[] = {
	"check\\unaryinc1.c",
	"check\\unaryinc2.c",
	"check\\unaryinc3.c",
	"check\\unaryinc4.c",
	"end",
};

// 前置インクリメントの確認プログラム 場所
const char* UnaryDecCheck[] = {
	"check\\unarydec1.c",
	"check\\unarydec2.c",
	"check\\unarydec3.c",
	"check\\unarydec4.c",
	"end",
};

// continue文の確認プログラム 場所
const char* ContinueCheck[] = {
	"check\\continue1.c",
	"check\\continue2.c",
	"end",
};

// break文の確認プログラム 場所
const char* BreakCheck[] = {
	"check\\break1.c",
	"check\\break2.c",
	"end",
};

// コメントの確認プログラム 場所
const char* ComentCheck[] = {
	"check\\coment1.c",
	"end",
};

// アルゴリズムの確認プログラム 場所
const char* AlgoCheck[] = {
	"check\\algo1.c",
	"check\\algo2.c",
	"check\\algo3.c",
	"end",
};

// cmdの結果を取得
bool GetCmdMs(string cmd, string& stdOut, int& exitCode) {
	// コマンド取得
	shared_ptr<FILE> pipe(_popen(cmd.c_str(), "r"), [&](FILE* p) { exitCode = _pclose(p); });
	if (!pipe) { // nullptrであれば失敗
		return false;
	}
	array<char, 256> buf; // バッファにcmdの結果を入れていく
	while (!feof(pipe.get())) {
		if (fgets(buf.data(), buf.size(), pipe.get()) != nullptr) {
			stdOut += buf.data();
		}
	}
	return true;
}

// テスト
void Test(const char **dir)
{
	string cmd, cmd1, cmd1_2, cmd2, stdOut;
	int exitCode;
	static int errCnt = 0;
	for (int i = 0; dir[i] != "end"; ++i) {
		cmd1 = "clang ";
		cmd1_2 = " -w -o ans.exe";
		cmd2 = "ans.exe";
		stdOut.clear();
		ConvToItrCode(dir[i]); // 内部コードへ変換
		Execute(); // インタプリタ実行
		cmd1 += dir[i];
		cmd1 += cmd1_2;
		GetCmdMs(cmd1, stdOut, exitCode); // ソースコードをコンパイラでコンパイル
		GetCmdMs(cmd2, stdOut, exitCode); // コンパイラの実行結果を取得
		if (TestOutPut == stdOut) { // インタプリタの結果とコンパイラの結果を比較
			cout << endl;
			cout << dir[i] << "は正しく動作しました" << endl;
			cout << "コンパイラの出力: " << stdOut << endl;
			cout << "インタプリタの出力: " << TestOutPut << endl;
		}
		else {
			++errCnt; // エラー数
			cout << endl;
			cout << dir[i]<< "不一致" << endl;
			cout << "正しい値: " << stdOut << endl;
			cout << "出力値: " << TestOutPut << endl;
		}
		TestOutPut.clear();
	}

	cout << errCnt << "個のエラー" << endl;
}

void SingleTest(const char* dir)
{
	string stdOut;
	int exitCode;

	TestOutPut.clear();

	ConvToItrCode(dir); // 内部コードへ変換
	Execute(); // インタプリタ実行

	GetCmdMs("clang check\\test.c -w -o ans.exe", stdOut, exitCode); // ソースコードをコンパイラでコンパイル
	GetCmdMs("ans.exe", stdOut, exitCode); // コンパイラの実行結果を取得
	if (TestOutPut == stdOut) { // インタプリタの結果とコンパイラの結果を比較
		cout << endl;
		cout << dir << "は正しく動作しました" << endl;
		cout << "コンパイラの出力: " << stdOut << endl;
		cout << "インタプリタの出力: " << TestOutPut << endl;
	}
	else {
		cout << endl;
		cout << dir << "不一致" << endl;
		cout << "正しい値: " << stdOut << endl;
		cout << "出力値: " << TestOutPut << endl;
	}
}

int main(int argc, char*argv[])
{
	if (argc == 1) { exit(1); }
	ConvToItrCode(argv[1]); // 内部コードへ変換
	Execute(); // 実行
	return 0;
}