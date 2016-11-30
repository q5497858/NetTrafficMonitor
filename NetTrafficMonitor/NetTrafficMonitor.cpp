
// NetTrafficMonitor.cpp : 定义应用程序的类行为。
//

#include "stdafx.h"
#include "NetTrafficMonitor.h"
#include "NetTrafficMonitorDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CNetTrafficMonitorApp

BEGIN_MESSAGE_MAP(CNetTrafficMonitorApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CNetTrafficMonitorApp 构造

CNetTrafficMonitorApp::CNetTrafficMonitorApp()
{
	// TODO:  在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}


// 唯一的一个 CNetTrafficMonitorApp 对象

CNetTrafficMonitorApp theApp;


// CNetTrafficMonitorApp 初始化

BOOL CNetTrafficMonitorApp::InitInstance()
{
	CWinApp::InitInstance();

	HWND hShellTrayWnd = ::FindWindow(_T("Shell_TrayWnd"), NULL);
	HWND hTrayNotifyWnd = ::FindWindowEx(hShellTrayWnd, NULL, _T("TrayNotifyWnd"), NULL);

	RECT rcTrayNotify;
	::GetWindowRect(hTrayNotifyWnd, &rcTrayNotify);

	int nWidth = 80;
	m_pMainWnd = new CNetTrafficMonitorDlg();
	m_pMainWnd->CreateEx(0, AfxRegisterWndClass(CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS),
		_T("NetTrafficMonitor"), WS_CHILDWINDOW | WS_VISIBLE,
		rcTrayNotify.right, 0, nWidth, rcTrayNotify.bottom - rcTrayNotify.top, hTrayNotifyWnd, NULL);

	// 由于对话框已关闭，所以将返回 FALSE 以便退出应用程序，
	//  而不是启动应用程序的消息泵。
	//return FALSE;
	return TRUE;
}

