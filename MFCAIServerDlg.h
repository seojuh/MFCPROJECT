
// MFCAIServerDlg.h: 헤더 파일
//

#pragma once
#include "MySocket.h"
#include <vector>

#define WM_UPDATE_UI (WM_USER + 1)
#define WM_USER_UPDATE_UI (WM_APP + 1)
// CMFCAIServerDlg 대화 상자
class CMFCAIServerDlg : public CDialogEx
{
// 생성입니다.
public:
	CMFCAIServerDlg(CWnd* pParent = nullptr);	// 표준 생성자입니다.
	CEdit m_editCtrl;

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MFCAISERVER_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CStatic m_prepicture;
	CStatic m_aftpicture;
	CButton m_open_server;
	CButton m_exit_button;
	MySocket m_serverSocket;
	afx_msg void OnBnClickedOpenButton();
	afx_msg LRESULT OnUpdateUI(WPARAM wParam, LPARAM lParam); 
	afx_msg void OnBnClickedExitButton();
	void DisplayResult(const std::string& resultData);
	LRESULT OnUpdateUI2(WPARAM wParam, LPARAM lParam);
	CString GetDownloadsFolderPath();
	CEdit m_response_edit;
};
