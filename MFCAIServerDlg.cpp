
// MFCAIServerDlg.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "MFCAIServer.h"
#include "MFCAIServerDlg.h"
#include "afxdialogex.h"
#include "MySocket.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
protected:
	afx_msg LRESULT OnUpdateUI(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CMFCAIServerDlg 대화 상자



CMFCAIServerDlg::CMFCAIServerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MFCAISERVER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMFCAIServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PREPICTURE, m_prepicture);
	DDX_Control(pDX, IDC_AFTPICTURE, m_aftpicture);
	DDX_Control(pDX, IDC_OPEN_BUTTON, m_open_server);
	DDX_Control(pDX, IDC_EXIT_BUTTON, m_exit_button);
	DDX_Control(pDX, IDC_EDIT3, m_response_edit);
}

BEGIN_MESSAGE_MAP(CMFCAIServerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_OPEN_BUTTON, &CMFCAIServerDlg::OnBnClickedOpenButton)
	ON_MESSAGE(WM_UPDATE_UI, &CMFCAIServerDlg::OnUpdateUI) // 메시지 매핑 추가
	ON_BN_CLICKED(IDC_EXIT_BUTTON, &CMFCAIServerDlg::OnBnClickedExitButton)
	ON_MESSAGE(WM_USER_UPDATE_UI, &CMFCAIServerDlg::OnUpdateUI2)
END_MESSAGE_MAP()


// CMFCAIServerDlg 메시지 처리기

BOOL CMFCAIServerDlg::OnInitDialog()
{
	TRACE(_T("프로그램 시작"));
	CDialogEx::OnInitDialog();
	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.
	m_editCtrl.Attach(GetDlgItem(IDC_EDIT1)->m_hWnd);

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CMFCAIServerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 애플리케이션의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CMFCAIServerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CMFCAIServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CMFCAIServerDlg::OnBnClickedOpenButton()
{
	if (!m_serverSocket.Create(17000))
	{
		AfxMessageBox(_T("소켓 생성 실패!"));
		return;
	}

	if (!m_serverSocket.Listen())
	{
		AfxMessageBox(_T("소켓 리스닝 실패!"));
		m_serverSocket.Close();
		return;
	}
	//서버 중복 열기 비활성화
	m_open_server.EnableWindow(FALSE);

	AfxMessageBox(_T("서버거 열렸습니다 클라이언트를 대기합니다."));
}

LRESULT CMFCAIServerDlg::OnUpdateUI(WPARAM wParam, LPARAM lParam)
{
	// 이미지 경로를 포인터로부터 추출
	CString* pStrFilePath = reinterpret_cast<CString*>(lParam);
	TRACE(_T("Received file path: %s\n"), *pStrFilePath);
	if (pStrFilePath != nullptr)
	{
		// CImage 객체를 사용하여 이미지 로드
		CImage image;
		HRESULT hResult = image.Load(*pStrFilePath);
		if (SUCCEEDED(hResult))
		{
			// IDC_AFTPICTURE 컨트롤의 크기에 맞게 이미지 크기 조정
			CRect rect;
			m_aftpicture.GetClientRect(&rect);
			int width = rect.Width();
			int height = rect.Height();

			// 메모리 DC 생성
			CDC memDC;
			memDC.CreateCompatibleDC(NULL);
			CBitmap bitmap;
			bitmap.CreateCompatibleBitmap(GetDC(), width, height);
			CBitmap* pOldBitmap = memDC.SelectObject(&bitmap);

			// 메모리 DC에 이미지 그리기
			image.StretchBlt(memDC, 0, 0, width, height, SRCCOPY);

			// 메모리 DC의 내용을 IDC_AFTPICTURE 컨트롤에 전송
			CClientDC dc(&m_aftpicture);
			dc.BitBlt(0, 0, width, height, &memDC, 0, 0, SRCCOPY);

			// 리소스 정리
			memDC.SelectObject(pOldBitmap);
			bitmap.DeleteObject();
			memDC.DeleteDC();
		}	
		else
		{
			AfxMessageBox(_T("이미지 로드 실패"));
		}

		// 사용이 완료되었으므로 메모리를 해제합니다.
		delete pStrFilePath;
	}
	else
	{
		// 이 경우, lParam은 nullptr 이므로 경로가 전달되지 않았다고 간주할 수 있음
		AfxMessageBox(_T("유효하지 않은 이미지 경로"));
	}
	return 0; // 처리된 메시지임을 나타냅니다.
}


void CMFCAIServerDlg::OnBnClickedExitButton()
{
	EndDialog(IDCANCEL);
}


void CMFCAIServerDlg::DisplayResult(const std::string& resultData)
{
	CString result(resultData.c_str());
	m_response_edit.SetWindowText(result);
}

LRESULT CMFCAIServerDlg::OnUpdateUI2(WPARAM wParam, LPARAM lParam)
{
	TRACE(_T("OnUpdateUI2 실행\n"));  // 로그 추가

	CString* pResultStr = reinterpret_cast<CString*>(lParam);
	if (pResultStr != nullptr)
	{
		TRACE(_T("결과 문자열: %s\n"), *pResultStr); // 로그 추가
		MySocket::cs.Lock(); // 멀티스레드 보호 시작
		MySocket::InsertIntoDatabase(MySocket::lastFileName, *pResultStr);
		MySocket::cs.Unlock(); // 멀티스레드 보호 끝
		m_response_edit.SetWindowText(*pResultStr);

		// CSV 파일로 내보내기를 여기서 호출
		CString strCSVPath = GetDownloadsFolderPath() + _T("\\screwdatabase_export.csv");
		m_serverSocket.ExportDataToCSV(strCSVPath);

		delete pResultStr;
	}
	return 0;
}
CString CMFCAIServerDlg::GetDownloadsFolderPath()
{
    PWSTR pszPath = NULL;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Downloads, 0, NULL, &pszPath)))
    {
        // PWSTR에서 CString으로 경로 변환하고 SHGetKnownFolderPath에 의해 할당된 메모리 해제
        CString strPath(pszPath);
        CoTaskMemFree(pszPath);
        return strPath;
    }
    else
    {
        AfxMessageBox(_T("다운로드 폴더 경로를 찾을 수 없습니다."));
        return _T("");
    }
}