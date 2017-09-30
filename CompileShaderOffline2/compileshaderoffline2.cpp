
// compile shader *.fx file offline using fxc
// Using the Effect-Compiler tool in a Subprocess
// https://msdn.microsoft.com/en-us/library/windows/desktop/bb509710(v=vs.85).aspx#using_the_effect-compiler_tool_in_a_subprocess
//
// message display to pipe buffer
// Creating a Child Process with Redirected Input and Output
// https://msdn.microsoft.com/en-us/library/windows/desktop/ms682499%28v=vs.85%29.aspx
//

#include <windows.h>
#include <iostream>
#include <string>

using namespace std;


void main()
{
	WCHAR curDir[MAX_PATH];
	GetCurrentDirectoryW(sizeof(curDir), curDir);

	HANDLE hReadPipe, hWritePipe;
	HANDLE hErrReadPipe, hErrWritePipe = NULL;

	SECURITY_ATTRIBUTES sa;
	ZeroMemory(&sa, sizeof(sa));
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = NULL;

	if (!CreatePipe(&hReadPipe, &hWritePipe, &sa, 0))
	{
		cout << "Error CreatePipe" << endl;
		return;
	}

	if (!SetHandleInformation(hReadPipe, HANDLE_FLAG_INHERIT, 0))
		return;

	if (!CreatePipe(&hErrReadPipe, &hErrWritePipe, &sa, 0))
	{
		cout << "Error CreatePipe" << endl;
		return;
	}

	if (!SetHandleInformation(hErrReadPipe, HANDLE_FLAG_INHERIT, 0))
		return;

	PROCESS_INFORMATION ProcessInfo;
	ZeroMemory(&ProcessInfo, sizeof(ProcessInfo));

	STARTUPINFO StartupInfo;
	ZeroMemory(&StartupInfo, sizeof(StartupInfo));
	StartupInfo.cb = sizeof(StartupInfo);
	StartupInfo.hStdError = hErrWritePipe;
	StartupInfo.hStdOutput = hWritePipe;
	StartupInfo.dwFlags |= STARTF_USESTDHANDLES;

	BOOL result = CreateProcess(L"fxc.exe", L"/Fc /Od /Zi /T fx_5_0 /Fo pos-color.fxo pos-color.fx"
		, NULL, NULL, TRUE, 0, NULL, curDir, &StartupInfo, &ProcessInfo);
	if (!result)
	{
		cout << "Error CreateProcess" << endl;
		return;
	}

	const DWORD BUFSIZE = 4096;
	BYTE buff[BUFSIZE];
	HANDLE WaitHandles[] = {ProcessInfo.hProcess, hReadPipe, hErrReadPipe };
	while (1)
	{
		const DWORD dwWaitResult = WaitForMultipleObjects(3, WaitHandles, FALSE, 60000L);

		// Read from the pipes...
		DWORD dwBytesRead, dwBytesAvailable;
		while (PeekNamedPipe(hReadPipe, NULL, 0, NULL, &dwBytesAvailable, NULL) && dwBytesAvailable)
		{
			ReadFile(hReadPipe, buff, BUFSIZE - 1, &dwBytesRead, 0);
			cout << std::string((char*)buff, (size_t)dwBytesRead);
		}
		while (PeekNamedPipe(hErrReadPipe, NULL, 0, NULL, &dwBytesAvailable, NULL) && dwBytesAvailable)
		{
			ReadFile(hErrReadPipe, buff, BUFSIZE - 1, &dwBytesRead, 0);
			cerr << std::string((char*)buff, (size_t)dwBytesRead);
		}

		// Process is done, or we timed out:
		if (dwWaitResult == WAIT_OBJECT_0 || dwWaitResult == WAIT_TIMEOUT)
			break;
	}

	CloseHandle(hReadPipe);
	CloseHandle(hWritePipe);
	CloseHandle(hErrReadPipe);
	CloseHandle(hErrWritePipe);
}
