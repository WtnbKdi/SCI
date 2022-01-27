#include <iostream>
#include <array>	// array
#include <cstdio>	// _popen
#include <memory>	// shared_ptr
#include <string>	// string
#include "Prototype.h"
extern vector<SymbolTbl> LTbl;
extern vector<SymbolTbl> GTbl;

extern string TestOutPut;       // �e�X�g���ʂ̕ۑ�
extern int TestOutPutCnt;

// ���͊֐��̊m�F�v���O���� �ꏊ
const char* InputCheck[] = {
	"check\\input1.c",
	"end",
};

// �ϐ���`�̊m�F�v���O���� �ꏊ
const char* VarDefineCheck[] = {
	"check\\vardefine1.c",
	"check\\vardefine2.c",
	"check\\vardefine3.c",
	"check\\vardefine4.c",
	"end",
};

// �O���[�o���ϐ��̊m�F�v���O���� �ꏊ
const char* GlobalVarCheck[] = {
	"check\\globalvar1.c",
	"check\\globalvar2.c",
	"check\\globalvar3.c",
	"end",
};
// ���̊m�F�v���O���� �ꏊ
const char* ExprCheck[] = {
	"check\\expr1.c",
	"check\\expr2.c",
	"check\\expr3.c",
	"check\\expr4.c",
	"end",
};

// (���Z�q)������Z�q�̊m�F�v���O���� �ꏊ
const char* OpeAssignCheck[] = {
	"check\\opeassign1.c",
	"end",
};

// �_�����Z�q�̊m�F�v���O���� �ꏊ
const char* LogicOpeCheck[] = {
	"check\\logicope1.c",
	"check\\logicope2.c",
	"end",
};

// ��r���Z�q�̊m�F�v���O���� �ꏊ
const char* CondOpeCheck[] = {
	"check\\condope1.c",
	"end",
};

// ��r���Z�q�Ƙ_�����Z�q�̊m�F�v���O���� �ꏊ
const char* LogiCondopeCheck[] = {
	"check\\logicondope1.c",
	"end",
};

// �z��̊m�F�v���O���� �ꏊ
const char* ArrayCheck[] = {
	"check\\array1.c",
	"end",
};

// �֐��̊m�F�v���O���� �ꏊ
const char* FuncCheck[] = {
	"check\\func1.c",
	"check\\func2.c",
	"check\\func3.c",
	"end",
};

// if���̊m�F�v���O���� �ꏊ
const char* IfCheck[] = {
	"check\\if1.c",
	"check\\if2.c",
	"check\\if3.c",
	"check\\if4.c",
	"check\\if5.c",
	"check\\if6.c",
	"end",
};

// for���̊m�F�v���O���� �ꏊ
const char* ForCheck[] = {
	"check\\for1.c",
	"check\\for2.c",
	"check\\for3.c",
	"end",
};

// while���̊m�F�v���O���� �ꏊ
const char* WhileCheck[] = {
	"check\\while1.c",
	"check\\while2.c",
	"check\\while3.c",
	"end",
};

// do-while���̊m�F�v���O���� �ꏊ
const char* DoWhileCheck[] = {
	"check\\dowhile1.c",
	"check\\dowhile2.c",
	"check\\dowhile3.c",
	"end",
};

// �ċA�֐��̊m�F�v���O���� �ꏊ
const char* SaikiCheck[] = {
	"check\\saiki1.c",
	"end",
};

// ��u�C���N�������g�̊m�F�v���O���� �ꏊ
const char* PostIncCheck[] = {
	"check\\postinc1.c",
	"check\\postinc2.c",
	"check\\postinc3.c",
	"check\\postinc4.c",
	"end",
};

// ��u�f�N�������g�̊m�F�v���O���� �ꏊ
const char* PostDecCheck[] = {
	"check\\postdec1.c",
	"check\\postdec2.c",
	"check\\postdec3.c",
	"check\\postdec4.c",
	"end",
};

// �O�u�C���N�������g�̊m�F�v���O���� �ꏊ
const char* UnaryIncCheck[] = {
	"check\\unaryinc1.c",
	"check\\unaryinc2.c",
	"check\\unaryinc3.c",
	"check\\unaryinc4.c",
	"end",
};

// �O�u�C���N�������g�̊m�F�v���O���� �ꏊ
const char* UnaryDecCheck[] = {
	"check\\unarydec1.c",
	"check\\unarydec2.c",
	"check\\unarydec3.c",
	"check\\unarydec4.c",
	"end",
};

// continue���̊m�F�v���O���� �ꏊ
const char* ContinueCheck[] = {
	"check\\continue1.c",
	"check\\continue2.c",
	"end",
};

// break���̊m�F�v���O���� �ꏊ
const char* BreakCheck[] = {
	"check\\break1.c",
	"check\\break2.c",
	"end",
};

// �R�����g�̊m�F�v���O���� �ꏊ
const char* ComentCheck[] = {
	"check\\coment1.c",
	"end",
};

// �A���S���Y���̊m�F�v���O���� �ꏊ
const char* AlgoCheck[] = {
	"check\\algo1.c",
	"check\\algo2.c",
	"check\\algo3.c",
	"end",
};

// cmd�̌��ʂ��擾
bool GetCmdMs(string cmd, string& stdOut, int& exitCode) {
	// �R�}���h�擾
	shared_ptr<FILE> pipe(_popen(cmd.c_str(), "r"), [&](FILE* p) { exitCode = _pclose(p); });
	if (!pipe) { // nullptr�ł���Ύ��s
		return false;
	}
	array<char, 256> buf; // �o�b�t�@��cmd�̌��ʂ����Ă���
	while (!feof(pipe.get())) {
		if (fgets(buf.data(), buf.size(), pipe.get()) != nullptr) {
			stdOut += buf.data();
		}
	}
	return true;
}

// �e�X�g
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
		ConvToItrCode(dir[i]); // �����R�[�h�֕ϊ�
		Execute(); // �C���^�v���^���s
		cmd1 += dir[i];
		cmd1 += cmd1_2;
		GetCmdMs(cmd1, stdOut, exitCode); // �\�[�X�R�[�h���R���p�C���ŃR���p�C��
		GetCmdMs(cmd2, stdOut, exitCode); // �R���p�C���̎��s���ʂ��擾
		if (TestOutPut == stdOut) { // �C���^�v���^�̌��ʂƃR���p�C���̌��ʂ��r
			cout << endl;
			cout << dir[i] << "�͐��������삵�܂���" << endl;
			cout << "�R���p�C���̏o��: " << stdOut << endl;
			cout << "�C���^�v���^�̏o��: " << TestOutPut << endl;
		}
		else {
			++errCnt; // �G���[��
			cout << endl;
			cout << dir[i]<< "�s��v" << endl;
			cout << "�������l: " << stdOut << endl;
			cout << "�o�͒l: " << TestOutPut << endl;
		}
		TestOutPut.clear();
	}

	cout << errCnt << "�̃G���[" << endl;
}

void SingleTest(const char* dir)
{
	string stdOut;
	int exitCode;

	TestOutPut.clear();

	ConvToItrCode(dir); // �����R�[�h�֕ϊ�
	Execute(); // �C���^�v���^���s

	GetCmdMs("clang check\\test.c -w -o ans.exe", stdOut, exitCode); // �\�[�X�R�[�h���R���p�C���ŃR���p�C��
	GetCmdMs("ans.exe", stdOut, exitCode); // �R���p�C���̎��s���ʂ��擾
	if (TestOutPut == stdOut) { // �C���^�v���^�̌��ʂƃR���p�C���̌��ʂ��r
		cout << endl;
		cout << dir << "�͐��������삵�܂���" << endl;
		cout << "�R���p�C���̏o��: " << stdOut << endl;
		cout << "�C���^�v���^�̏o��: " << TestOutPut << endl;
	}
	else {
		cout << endl;
		cout << dir << "�s��v" << endl;
		cout << "�������l: " << stdOut << endl;
		cout << "�o�͒l: " << TestOutPut << endl;
	}
}

int main(int argc, char*argv[])
{
	if (argc == 1) { exit(1); }
	ConvToItrCode(argv[1]); // �����R�[�h�֕ϊ�
	Execute(); // ���s
	return 0;
}