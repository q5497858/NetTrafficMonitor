// NetTrafficMonitorDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "NetTrafficMonitor.h"
#include "NetTrafficMonitorDlg.h"
#include "NetTraffic.h"


// CNetTrafficMonitorDlg

IMPLEMENT_DYNAMIC(CNetTrafficMonitorDlg, CWnd)

CNetTrafficMonitorDlg::CNetTrafficMonitorDlg()
{
	m_iWidth = 80;
	m_iHeight = 30;
	m_iTransparency = 60;
	m_dwUploadTraffic = 0;
	m_dwDownloadTraffic = 0;
	m_bSelfStarting = FALSE;
}

CNetTrafficMonitorDlg::~CNetTrafficMonitorDlg()
{
}


BEGIN_MESSAGE_MAP(CNetTrafficMonitorDlg, CWnd)
	ON_WM_CREATE()
	ON_WM_CTLCOLOR()
	ON_WM_PAINT()
	ON_WM_MOVE()
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONUP()
	ON_WM_HSCROLL()
	ON_WM_KILLFOCUS()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_SELFSTARTING, OnSelfStarting)
	ON_BN_CLICKED(IDC_TRANSPARENCY, OnTransparency)
	ON_BN_CLICKED(IDC_EXIT, OnExit)
END_MESSAGE_MAP()



// CNetTrafficMonitorDlg 消息处理程序




BOOL CNetTrafficMonitorDlg::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO:  在此添加专用代码和/或调用基类
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	if (GetClassInfoEx(cs.hInstance, cs.lpszClass, &wcex))
	{
		return TRUE;
	}
	wcex.style = CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS;
	wcex.lpfnWndProc = AfxWndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = cs.hInstance;
	wcex.hIcon = wcex.hIconSm = LoadIcon(cs.hInstance, MAKEINTRESOURCE(IDR_MAINFRAME));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = cs.lpszClass;
	return RegisterClassEx(&wcex);

	//return CWnd::PreCreateWindow(cs);
}


int CNetTrafficMonitorDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  在此添加您专用的创建代码
	CString strText;
	GetWindowText(strText);
	HANDLE hMutex = CreateMutex(NULL, FALSE, strText);
	if (ERROR_ALREADY_EXISTS == GetLastError())									// 禁止程序多开
	{
		AfxMessageBox(_T("程序已经运行"));
		return -1;
	}

	CRect rcWorkArea;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWorkArea, 0);					// 获取系统工作区域(除去下方任务栏)
	MoveWindow(rcWorkArea.right - m_iWidth - 80, 60, m_iWidth, m_iHeight);

	ModifyStyle(WS_THICKFRAME, 0);												// 不可调大小
	ModifyStyleEx(WS_EX_APPWINDOW, WS_EX_TOOLWINDOW);							// 隐藏任务栏图标
	SetWindowPos(&wndTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);				// 置顶
	SetWindowLong(m_hWnd,
		GWL_EXSTYLE,
		GetWindowLong(m_hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);					// 为窗口加入WS_EX_LAYERED扩展属性
	SetLayeredWindowAttributes(0, (255 * m_iTransparency) / 100, LWA_ALPHA);	// m_iTransparency%透明度
	SetTimer(TIMER_TRAFFIC, 1000, NULL);										// 间隔1000ms刷新
	ShowWindow(SW_SHOW);

	TCHAR szFileFullPath[MAX_PATH] = { 0 };
	TCHAR szFileName[_MAX_FNAME] = { 0 };
	GetModuleFileName(NULL, szFileFullPath, MAX_PATH);							// 得到程序自身的全路径
	_wsplitpath(szFileFullPath, NULL, NULL, szFileName, NULL);					// 获得程序名
	HKEY hKey;
	LPCTSTR lpRun = _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run");	// 找到系统的启动项
	if (ERROR_SUCCESS == RegOpenKey(HKEY_CURRENT_USER, lpRun, &hKey))			// 打开启动项Key
	{
		DWORD dwType = REG_SZ, dwSize = MAX_PATH;
		TCHAR szValue[MAX_PATH] = { 0 };
		if (ERROR_SUCCESS == RegQueryValueEx(hKey, szFileName, NULL, &dwType, (LPBYTE)szValue, &dwSize))
		{
			m_bSelfStarting = TRUE;
		}
		else
		{
			m_bSelfStarting = FALSE;
		}
	}
	RegCloseKey(hKey);															// 关闭注册表

	m_pSlider = new CSliderCtrl();
	m_pSlider->Create(WS_VISIBLE | TBS_HORZ | TBS_AUTOTICKS, CRect(0, 0, m_iWidth, m_iHeight / 2), this, 1001);
	m_pSlider->SetRange(30, 100);
	m_pSlider->SetTicFreq(10);
	m_pSlider->SetPos(m_iTransparency);
	m_pSlider->ShowWindow(SW_HIDE);
	m_cToolTip.Create(m_pSlider);
	m_cToolTip.AddTool(m_pSlider);

	m_cCheckNo.LoadBitmap(IDB_CHECKNO);
	m_cCheckYes.LoadBitmap(IDB_CHECKYES);
	m_cExit.LoadBitmap(IDB_EXIT);

	return 0;
}


HBRUSH CNetTrafficMonitorDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CWnd::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  在此更改 DC 的任何特性
	CFont font;
	font.CreateFont(17, 0, 0, 0, 550,
		false, false, false,
		GB2312_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY, FF_SCRIPT,
		_T("微软雅黑"));
	pDC->SelectObject(&font);

	// TODO:  如果默认的不是所需画笔，则返回另一个画笔
	return hbr;
}


void CNetTrafficMonitorDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO:  在此处添加消息处理程序代码
	// 不为绘图消息调用 CWnd::OnPaint()
	CFont font;
	font.CreateFont(15, 0, 0, 0, 800,
		false, false, false,
		GB2312_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY, FF_SCRIPT,
		_T("微软雅黑"));
	dc.SelectObject(&font);
	dc.SetBkMode(TRANSPARENT);

	dc.DrawText(_T("↑"), CRect(0, 0, 10, m_iHeight / 2), DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS);
	dc.DrawText(_T("↓"), CRect(0, m_iHeight / 2, 10, m_iHeight), DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS);
	CString strText;
	if (m_dwUploadTraffic / (1024 * 1024) >= 1)
	{
		strText.Format(_T("%.2f MB/s "), m_dwUploadTraffic / (1024 * 1024.00));
	}
	else
	{
		strText.Format(_T("%.2f KB/s "), m_dwUploadTraffic / 1024.00);
	}
	dc.DrawText(strText, CRect(10, 0, m_iWidth, m_iHeight / 2), DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS);
	if (m_dwDownloadTraffic / (1024 * 1024) >= 1)
	{
		strText.Format(_T("%.2f MB/s "), m_dwDownloadTraffic / (1024 * 1024.00));
	}
	else
	{
		strText.Format(_T("%.2f KB/s "), m_dwDownloadTraffic / 1024.00);
	}
	dc.DrawText(strText, CRect(10, m_iHeight / 2, m_iWidth, m_iHeight), DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS);
}


void CNetTrafficMonitorDlg::OnMove(int x, int y)
{
	CWnd::OnMove(x, y);

	// TODO:  在此处添加消息处理程序代码
	CRect rcWorkArea;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWorkArea, 0);		// 获取系统工作区域(除去下方任务栏)
	CRect rc;
	GetWindowRect(&rc);
	if (rc.left < 20)
	{
		rc.left = 0;
		rc.right = m_iWidth;
	}
	if (rcWorkArea.right - rc.right < 20)
	{
		rc.left = rcWorkArea.right - m_iWidth;
		rc.right = rcWorkArea.right;
	}
	if (rc.top < 20)
	{
		rc.top = 0;
		rc.bottom = m_iHeight;
	}
	if (rcWorkArea.bottom - rc.bottom < 20)
	{
		rc.top = rcWorkArea.bottom - m_iHeight;
		rc.bottom = rcWorkArea.bottom;
	}
	MoveWindow(&rc);
}


void CNetTrafficMonitorDlg::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	// TODO:  在此处添加消息处理程序代码
}


void CNetTrafficMonitorDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值
	lpMMI->ptMinTrackSize.x = lpMMI->ptMaxTrackSize.x = lpMMI->ptMaxSize.x = m_iWidth;
	lpMMI->ptMinTrackSize.y = lpMMI->ptMaxTrackSize.y = lpMMI->ptMaxSize.y = m_iHeight;

	CWnd::OnGetMinMaxInfo(lpMMI);
}


void CNetTrafficMonitorDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值
	if (m_pSlider->IsWindowVisible())
	{
		m_pSlider->ShowWindow(SW_HIDE);
	}

	SendMessage(WM_NCLBUTTONDOWN, HTCAPTION, 0);

	CWnd::OnLButtonDown(nFlags, point);
}


void CNetTrafficMonitorDlg::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值
	SendMessage(WM_NCLBUTTONDBLCLK, HTCAPTION, 0);

	CWnd::OnLButtonDblClk(nFlags, point);
}


void CNetTrafficMonitorDlg::OnRButtonUp(UINT nFlags, CPoint point)
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值
	if (m_pSlider->IsWindowVisible())
	{
		m_pSlider->ShowWindow(SW_HIDE);
	}

	LPPOINT lpPoint = new tagPOINT();
	GetCursorPos(lpPoint);

	CMenu menu;
	menu.CreatePopupMenu();	// 声明一个弹出式菜单
	menu.AppendMenu(MF_STRING, IDC_SELFSTARTING, _T("开机启动"));
	CString strText;
	strText.Format(_T("透明度: %d%%"), m_iTransparency);
	menu.AppendMenu(MF_STRING, IDC_TRANSPARENCY, strText);
	menu.AppendMenu(MF_STRING, IDC_EXIT, _T("退出"));	// 增加菜单项“退出”，点击则发送消息给主窗口将程序结束
	if (m_bSelfStarting)
	{
		menu.SetMenuItemBitmaps(IDC_SELFSTARTING, MF_BYCOMMAND | MF_STRING | MF_ENABLED, &m_cCheckYes, &m_cCheckYes);
	}
	else
	{
		menu.SetMenuItemBitmaps(IDC_SELFSTARTING, MF_BYCOMMAND | MF_STRING | MF_ENABLED, &m_cCheckNo, &m_cCheckNo);
	}
	menu.SetMenuItemBitmaps(IDC_EXIT, MF_BYCOMMAND | MF_STRING | MF_ENABLED, &m_cExit, &m_cExit);
	//SetForegroundWindow();
	menu.TrackPopupMenu(TPM_LEFTALIGN, lpPoint->x, lpPoint->y, this);	// 确定弹出式菜单的位置
	menu.Detach();	// 资源回收
	menu.DestroyMenu();
	delete lpPoint;

	CWnd::OnRButtonUp(nFlags, point);
}


void CNetTrafficMonitorDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值
	CString strText;
	strText.Format(_T("%d%%"), m_pSlider->GetPos());
	m_cToolTip.UpdateTipText(strText, m_pSlider);

	m_iTransparency = m_pSlider->GetPos();
	SetLayeredWindowAttributes(0, (255 * m_iTransparency) / 100, LWA_ALPHA);	// m_iTransparency%透明度

	CWnd::OnHScroll(nSBCode, nPos, pScrollBar);
}


void CNetTrafficMonitorDlg::OnKillFocus(CWnd* pNewWnd)
{
	CWnd::OnKillFocus(pNewWnd);

	// TODO:  在此处添加消息处理程序代码
	if (m_pSlider->IsWindowVisible() && GetFocus() != m_pSlider)
	{
		m_pSlider->ShowWindow(SW_HIDE);
	}
}


void CNetTrafficMonitorDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值
	if (TIMER_TRAFFIC == nIDEvent)
	{
		CNetTraffic* pNetTraffic = CNetTraffic::create_instance();
		pNetTraffic->RefreshInterfacesTraffic();

		m_dwUploadTraffic = 0;
		m_dwDownloadTraffic = 0;
		int iNetworkInterfacesCount = pNetTraffic->GetNetworkInterfacesCount();
		for (int i = 0; i < iNetworkInterfacesCount; ++i)
		{
			m_dwUploadTraffic += pNetTraffic->GetIncrementalOutgoingTraffic(i);
		}
		for (int i = 0; i < iNetworkInterfacesCount; ++i)
		{
			m_dwDownloadTraffic += pNetTraffic->GetIncrementalIncomingTraffic(i);
		}

		Invalidate();
	}

	CWnd::OnTimer(nIDEvent);
}


void CNetTrafficMonitorDlg::OnSelfStarting()
{
	TCHAR szFileFullPath[MAX_PATH] = { 0 };
	TCHAR szFileName[_MAX_FNAME] = { 0 };
	GetModuleFileName(NULL, szFileFullPath, MAX_PATH);							// 得到程序自身的全路径
	_wsplitpath(szFileFullPath, NULL, NULL, szFileName, NULL);					// 获得程序名
	HKEY hKey;
	LPCTSTR lpRun = _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run");	// 找到系统的启动项
	if (ERROR_SUCCESS == RegOpenKey(HKEY_CURRENT_USER, lpRun, &hKey))			// 打开启动项Key
	{
		if (m_bSelfStarting)
		{
			if (ERROR_SUCCESS == RegDeleteValue(hKey, szFileName))				// 删除一个子Key
			{
				m_bSelfStarting = FALSE;
			}
		}
		else
		{
			if (ERROR_SUCCESS == RegSetValueEx(hKey,
				szFileName,
				0,
				REG_SZ,
				(LPBYTE)szFileFullPath,
				(lstrlen(szFileFullPath) + 1) * sizeof(TCHAR)))					// 添加一个子Key,并设置值
			{
				m_bSelfStarting = TRUE;
			}
		}
	}
	RegCloseKey(hKey);															// 关闭注册表
}


void CNetTrafficMonitorDlg::OnTransparency()
{
	if (!m_pSlider->IsWindowVisible())
	{
		m_pSlider->ShowWindow(SW_SHOW);
	}
}


void CNetTrafficMonitorDlg::OnExit()
{
	m_pSlider->DestroyWindow();
	m_cToolTip.DestroyWindow();
	DestroyWindow();
}
