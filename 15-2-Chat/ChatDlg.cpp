// ChatDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Chat.h"
#include "ChatDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChatDlg dialog

CChatDlg::CChatDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CChatDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CChatDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CChatDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChatDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CChatDlg, CDialog)
	//{{AFX_MSG_MAP(CChatDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_SEND, OnBtnSend)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_RECVDATA,OnRecvData)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChatDlg message handlers

BOOL CChatDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	// 初始化
	InitSocket();
	// 创建进程用于接受
	RECVPARAM *pRecvParm=new RECVPARAM;
	pRecvParm->sock=m_socket;
	pRecvParm->hwnd=m_hWnd;

	HANDLE hThread=CreateThread(NULL,0,this->RecvProc,(LPVOID)pRecvParm,0,NULL);
	
	SetDlgItemText(IDC_IPADDRESS1,"127.0.0.1");
		
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CChatDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CChatDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CChatDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

BOOL CChatDlg::InitSocket()
{
	m_socket = socket(AF_INET,SOCK_DGRAM,0);
	if (INVALID_SOCKET==m_socket)
	{
		MessageBox("套接字创建失败！");
		return FALSE;
	}
	SOCKADDR_IN addrSock;
	addrSock.sin_family=AF_INET;
	addrSock.sin_port=htons(6000);
	// 接收任何ip
	addrSock.sin_addr.S_un.S_addr=htonl(INADDR_ANY);//inet_addr("127.0.0.1");
	
	int nRet;
	nRet =bind(m_socket,(SOCKADDR*)&addrSock,sizeof(SOCKADDR));
	if (SOCKET_ERROR == nRet)
	{
		closesocket(m_socket);
		MessageBox("绑定失败!");
		return FALSE;
	}
	return TRUE;
}

DWORD WINAPI CChatDlg::RecvProc(LPVOID lpParameter)
{
	SOCKET sock=((RECVPARAM*)lpParameter)->sock;
	HWND hwnd=((RECVPARAM*)lpParameter)->hwnd;

	SOCKADDR_IN addFrom;
	int len=sizeof(SOCKADDR);

	char recvBuf[200];
	char tempBuf[200];
	int retval;
	while (1)
	{
		retval=recvfrom(sock,recvBuf,200,0,(SOCKADDR*)&addFrom,&len);
		if (SOCKET_ERROR == retval)
			break;
		sprintf(tempBuf,"%s说:%s",inet_ntoa(addFrom.sin_addr),recvBuf);
		// 调用输出函数
		::PostMessage(hwnd,WM_RECVDATA,0,(LPARAM)tempBuf);
	}
	return 0;
}

DWORD WINAPI CChatDlg::SendProc(LPVOID lpParameter)
{

	return 0;
}

void CChatDlg::OnRecvData(WPARAM wParam,LPARAM lParam)
{
	CString str=(char*)lParam;
	CString strTemp;
	GetDlgItemText(IDC_EDIT_RECV,strTemp);

/*  // 上往下的输入效果
	COleDateTime time=COleDateTime::GetCurrentTime();
	str = "\t"+ time.Format(VAR_TIMEVALUEONLY)+"\r\n"+str;
	str+="\r\n";
	str+=strTemp;
	SetDlgItemText(IDC_EDIT_RECV,str);
*/
	// 下往上的输入效果
	COleDateTime time=COleDateTime::GetCurrentTime();
	strTemp =  strTemp +"\r\n\t"+time.Format(VAR_TIMEVALUEONLY)+"\r\n"+str;
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_RECV);

	SetDlgItemText(IDC_EDIT_RECV,strTemp);

	pEdit->LineScroll(pEdit->GetLineCount()-1);
}

void CChatDlg::OnBtnSend() 
{
	// TODO: Add your control notification handler code here
	DWORD dwIP;
	((CIPAddressCtrl*)GetDlgItem(IDC_IPADDRESS1))->GetAddress(dwIP);
	
	SOCKADDR_IN addrTo;
	addrTo.sin_family=AF_INET;
	addrTo.sin_port=htons(6000);
	addrTo.sin_addr.S_un.S_addr=htonl(dwIP);

	CString strSend;
	GetDlgItemText(IDC_EDIT_SEND,strSend);
	if (strSend.IsEmpty())
	{
		AfxMessageBox("不允许发送空消息");
		return;
	}
	sendto(m_socket,strSend,strSend.GetLength()+1,0,
		(SOCKADDR*)&addrTo,sizeof(SOCKADDR));
	SetDlgItemText(IDC_EDIT_SEND,"");
}

void CChatDlg::OnOK() 
{
	// TODO: Add extra validation here
	
	//CDialog::OnOK();
}
