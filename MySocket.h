#pragma once

// MySocket 명령 대상
#include <ShlObj.h>
#include "afxsock.h"
#include <string>

// CMFCAIServerDlg에 대한 전방 선언
class CMFCAIServerDlg;

//데이터 처리를 위한 구조체
struct MyDataStructure
{
	char* buffer; //데이터 버퍼
	int length; //버퍼 길이
};

class MySocket : public CAsyncSocket
{
public:
	MySocket();
	virtual ~MySocket();

	static CString lastFileName; // 파일 이름을 저장하기 위한 정적 멤버 변수
	static CCriticalSection cs; // 멀티스레드 환경에서 사용할 크리티컬 섹션

	//대화 상자 클래스 접근하기 위한 포인터 설정
	void SetParentDlg(CMFCAIServerDlg* pDlg);
	void SendImageToPythonServer(char* buffer, int length);
	void static InsertIntoDatabase(const CString& fileName, const CString& screwType);

	void ExportDataToCSV(const CString& csvFilePath);

protected:
	//클라이언트 연결 수락 메서드
	virtual void OnAccept(int nErrorCode);
	//클라이언트 데이터 수신 호출 메서드
	virtual void OnReceive(int nErrorCode);
	//연결 종료 호출 메서드
	virtual void OnClose(int nErrorCode);
	//파이썬 결과 처리 메서드
	void ProcessServerResult(const std::string& resultData);

private:
	CMFCAIServerDlg* m_pDlg; //대화 상자 클래스 포인터
};

UINT DataProcessThread(LPVOID pParam);

