
// compile shader *.fx file offline using fxc
// Using the Effect-Compiler tool in a Subprocess
// https://msdn.microsoft.com/en-us/library/windows/desktop/bb509710(v=vs.85).aspx#using_the_effect-compiler_tool_in_a_subprocess
//
// message display to console standard output
//

#include <windows.h>
#include <iostream>
#include <string>

using namespace std;

void main()
{
	WCHAR curDir[MAX_PATH];
	GetCurrentDirectoryW(sizeof(curDir), curDir);

	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);

	PROCESS_INFORMATION ProcessInfo;
	ZeroMemory(&ProcessInfo, sizeof(ProcessInfo));

	STARTUPINFO StartupInfo;
	ZeroMemory(&StartupInfo, sizeof(StartupInfo));
	StartupInfo.cb = sizeof(StartupInfo);
	StartupInfo.hStdError = hStdout;
	StartupInfo.hStdOutput = hStdout;
	StartupInfo.dwFlags |= STARTF_USESTDHANDLES;

	BOOL result = CreateProcess(L"fxc.exe", L"/Fc /Od /Zi /T fx_5_0 /Fo pos-color.fxo pos-color.fx"
		, NULL, NULL, TRUE, 0, NULL, curDir, &StartupInfo, &ProcessInfo);
}
