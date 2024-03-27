// MySocket.cpp

#include "pch.h"
#include "MFCAIServer.h"
#include "MySocket.h"
#include "MFCAIServerDlg.h"
#include "string"
#include <vector>
#include <mysql.h>

// EOF_MARKER 정의
const char EOF_MARKER[] = "EOF";

MySocket::MySocket()
{
    m_pDlg = nullptr;
}

MySocket::~MySocket()
{
}

CString MySocket::lastFileName = _T("");
CCriticalSection MySocket::cs;

void MySocket::SetParentDlg(CMFCAIServerDlg* pDlg)
{
    m_pDlg = pDlg;
}

void MySocket::OnAccept(int nErrorCode)
{
    if (nErrorCode == 0)
    {
        MySocket* pClientSocket = new MySocket;
        if (Accept(*pClientSocket))
        {
            if (m_pDlg != nullptr)
            {
                m_pDlg->m_editCtrl.SetWindowText(_T("새 클라이언트가 연결되었습니다.\n"));
            }
        }
        else
        {
            delete pClientSocket;
            if (m_pDlg != nullptr)
            {
                m_pDlg->m_editCtrl.SetWindowText(_T("클라이언트 소켓 수락에 실패했습니다.\n"));
                delete pClientSocket;
            }
        }
    }
    else
    {
        if (m_pDlg != nullptr)
        {
            CString message;
            message.Format(_T("클라이언트 연결 수락 오류: %d\n"), nErrorCode);
            m_pDlg->m_editCtrl.SetWindowText(message);
        }
    }
}

void MySocket::OnReceive(int nErrorCode)
{
    if (nErrorCode == 0)
    {
        TRACE(_T("클라이언트로부터 데이터 수신 시작\n"));

        int fileSize = 0;
        int bytesRead = 0;
        while (bytesRead < sizeof(fileSize))
        {
            int result = Receive(&fileSize + bytesRead, sizeof(fileSize) - bytesRead);
            if (result == SOCKET_ERROR)
            {
                int nError = GetLastError();
                TRACE(_T("파일 크기 수신 에러: %d\n"), nError);
                Close();
                return;
            }
            bytesRead += result;
        }

        TRACE(_T("파일 크기: %d바이트\n"), fileSize);

        std::vector<char> buffer(fileSize);
        TRACE(_T("buffer 주소: %p, fileSize: %d\n"), buffer.data(), fileSize);

        int totalBytesRead = 0;
        bool eofReceived = false;

        while (totalBytesRead < fileSize && !eofReceived)
        {
            int bytesRead = Receive(buffer.data() + totalBytesRead, fileSize - totalBytesRead);
            if (bytesRead == SOCKET_ERROR)
            {
                int nError = GetLastError();
                TRACE(_T("데이터 수신 에러: %d\n"), nError);
                Close();
                return;
            }

            totalBytesRead += bytesRead;
            TRACE(_T("수신된 데이터: %d바이트\n"), bytesRead);
            TRACE(_T("totalBytesRead: %d\n"), totalBytesRead);

            // EOF 마커 확인
            std::string receivedData(buffer.data() + totalBytesRead - bytesRead, bytesRead);
            if (receivedData.find("EOF") != std::string::npos)
            {
                eofReceived = true;
                totalBytesRead -= 3; // "EOF" 마커 크기만큼 제외
            }
        }

        if (eofReceived)
        {
            TRACE(_T("파일 전송 완료 (EOF 마커 수신)\n"));
            TRACE(_T("EOF 마커 수신, totalBytesRead: %d\n"), totalBytesRead);
        }
        else
        {
            TRACE(_T("파일 전송 완료 (파일 크기만큼 수신)\n"));
        }

        TRACE(_T("총 수신된 데이터: %d바이트\n"), totalBytesRead);
        TRACE(_T("SendImageToPythonServer 호출, buffer 주소: %p, length: %d\n"), buffer.data(), totalBytesRead);

        auto* pData = new MyDataStructure{ buffer.data(), totalBytesRead };
        AfxBeginThread(DataProcessThread, pData);

        SendImageToPythonServer(buffer.data(), totalBytesRead);


    }
}

void MySocket::SendImageToPythonServer(char* buffer, int length)
{
    TRACE(_T("SendImageToPythonServer 내부, buffer 주소: %p, length: %d\n"), buffer, length);

    // 유효성 검사
    if (buffer == nullptr || length <= 0) {
        TRACE(_T("전송할 데이터가 없거나 버퍼가 nullptr입니다.\n"));
        return;
    }

    CString pythonServerIP = _T("10.10.20.114");
    int pythonServerPort = 18000;
    CSocket pythonSocket;

    if (!pythonSocket.Create()) {
        TRACE(_T("Python 서버 소켓 생성 실패\n"));
        return;
    }

    if (!pythonSocket.Connect(pythonServerIP, pythonServerPort)) {
        TRACE(_T("Python 서버 연결 실패\n"));
        pythonSocket.Close();
        return;
    }

    // 이미지 크기 정보 전송
    TRACE(_T("이미지 크기 정보 전송, 버퍼 주소: %p, 크기: %d\n"), &length, sizeof(length));
    if (pythonSocket.Send(reinterpret_cast<char*>(&length), sizeof(length)) == SOCKET_ERROR) {
        TRACE(_T("이미지 크기 정보 전송 오류\n"));
        pythonSocket.Close();
        return;
    }

    bool resend = true;
    while (resend) 
    {
        resend = false;

        // 이미지 데이터 전송
        int totalBytesSent = 0;
        while (totalBytesSent < length) 
        {
            int bytesToSend = min(4096, length - totalBytesSent);
            int bytesSent = pythonSocket.Send(buffer + totalBytesSent, bytesToSend);
            if (bytesSent == SOCKET_ERROR) 
            {
                TRACE(_T("이미지 데이터 전송 오류: %d\n"), pythonSocket.GetLastError());
                pythonSocket.Close();
                return;
            }
            totalBytesSent += bytesSent;
        }

        // EOF_MARKER 전송
        if (pythonSocket.Send(EOF_MARKER, sizeof(EOF_MARKER) - 1) == SOCKET_ERROR) {
            TRACE(_T("EOF_MARKER 전송 오류: %d\n"), pythonSocket.GetLastError());
            pythonSocket.Close();
            return;
        }

        // 서버로부터 응답 수신
        char response[10];
        int bytesRead = pythonSocket.Receive(response, sizeof(response) - 1);
        if (bytesRead > 0) 
        {
            response[bytesRead] = '\0';
            if (strcmp(response, "RESEND") == 0) 
            {
                TRACE(_T("서버에서 재전송 요청 수신\n"));
                resend = true;
            }
        }
    }
    // 기존 응답 수신 로직 대체
    std::string resultData;
    char tempBuffer[256]; // 적당한 크기의 버퍼 설정
    int nReceived;

    // 서버로부터 응답 수신 대기
    while ((nReceived = pythonSocket.Receive(tempBuffer, sizeof(tempBuffer) - 1, 0)) > 0)
    {
        tempBuffer[nReceived] = '\0'; // NULL 종료 문자를 추가하여 문자열 완성
        resultData.append(tempBuffer); // 문자열에 추가

        if (resultData.find("EOF") != std::string::npos) // 종료 문자열 "EOF"를 찾으면
        {
            resultData.erase(resultData.find("EOF")); // "EOF" 삭제
            break; // 반복문 탈출
        }
    }

    // 수신된 응답을 대화 상자의 UI에 표시
    if (!resultData.empty())
    {
        TRACE(_T("서버로부터 응답 받음: %s\n"), CString(resultData.c_str()));
        // 메인 윈도우의 메시지 핸들러에 결과 데이터를 전달합니다.
        // 여기서 WM_USER_UPDATE_UI 메시지는 UI를 업데이트하는 사용자 정의 메시지입니다.
        CString* pResultStr = new CString(resultData.c_str());
        AfxGetMainWnd()->PostMessage(WM_USER_UPDATE_UI, 0, reinterpret_cast<LPARAM>(pResultStr));
    }

    pythonSocket.Close();
}

void MySocket::OnClose(int nErrorCode)
{
    if (nErrorCode == 0)
    {
        TRACE(_T("클라이언트 연결 종료\n"));
    }
    else
    {
        TRACE(_T("클라이언트 연결 종료 오류: %d\n"), nErrorCode);
    }
    CAsyncSocket::OnClose(nErrorCode);
}

UINT DataProcessThread(LPVOID pParam)
{
    auto pData = reinterpret_cast<MyDataStructure*>(pParam);

    TRACE(_T("DataProcessThread 시작, buffer 주소: %p, length: %d\n"), pData->buffer, pData->length);

    TCHAR szFolderPath[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_MYPICTURES, NULL, SHGFP_TYPE_CURRENT, szFolderPath)))
    {
        TRACE(_T("사진 폴더 경로 가져오기 성공: %s\n"), szFolderPath);

        CTime currentTime = CTime::GetCurrentTime();
        CString strFileName;
        strFileName.Format(_T("%04d-%02d-%02d_%02d-%02d-%02d.jpg"),
            currentTime.GetYear(), currentTime.GetMonth(), currentTime.GetDay(),
            currentTime.GetHour(), currentTime.GetMinute(), currentTime.GetSecond());

        CString strFilePath;
        strFilePath.Format(_T("%s\\%s"), szFolderPath, strFileName);

        CFile file;
        if (file.Open(strFilePath, CFile::modeCreate | CFile::modeWrite))
        {
            TRACE(_T("파일 열기 성공: %s\n"), strFilePath);
            file.Write(pData->buffer, pData->length);
            file.Close();
            TRACE(_T("파일 저장 완료\n"));

            CMFCAIServerDlg* pDlg = static_cast<CMFCAIServerDlg*>(theApp.m_pMainWnd);
            if (pDlg != nullptr)
            {
                pDlg->m_editCtrl.SetWindowText(_T("파일을 저장했습니다\r\n"));
                CString message;
                message.Format(_T("파일 저장 완료: %s\r\n"), strFilePath);
                pDlg->m_editCtrl.ReplaceSel(message);

                CString* pStrFilePath = new CString(strFilePath);
                CString messageFilePath;
                messageFilePath.Format(_T("Posting message with file path: %s\r\n"), *pStrFilePath);
                pDlg->m_editCtrl.ReplaceSel(messageFilePath);

                MySocket::cs.Lock(); // 멀티스레드 보호 시작
                MySocket::lastFileName = strFileName; // 파일 이름 저장
                MySocket::cs.Unlock(); // 멀티스레드 보호 끝

                if (!pDlg->PostMessage(WM_UPDATE_UI, 0, reinterpret_cast<LPARAM>(pStrFilePath)))
                {
                    TRACE(_T("PostMessage 실패\n"));
                    delete pStrFilePath;
                }
            }
        }
        else
        {
            TRACE(_T("파일 열기 실패: %s\n"), strFilePath);
            CMFCAIServerDlg* pDlg = static_cast<CMFCAIServerDlg*>(theApp.m_pMainWnd);
            if (pDlg != nullptr)
            {
                CString message;
                message.Format(_T("파일 저장 실패: %s\r\n"), strFilePath);
                pDlg->m_editCtrl.ReplaceSel(message);
            }
        }

        delete pData;
        TRACE(_T("DataProcessThread 종료\n"));
        return 0;
    }
    else
    {
        TRACE(_T("사진 폴더 경로 가져오기 실패\n"));
        CMFCAIServerDlg* pDlg = static_cast<CMFCAIServerDlg*>(theApp.m_pMainWnd);
        if (pDlg != nullptr)
        {
            pDlg->m_editCtrl.SetWindowText(_T("사진 폴더 경로 가져오기 실패\r\n"));
        }
    }

    delete pData;
    TRACE(_T("DataProcessThread 종료\n"));
    return 0;
}

void MySocket::ProcessServerResult(const std::string& resultData)
{
    // 결과 데이터를 처리하는 로직
    // 예: 결과 데이터를 대화 상자의 컨트롤에 표시
    if (m_pDlg != nullptr)
    {
        m_pDlg->DisplayResult(resultData);
    }
}

void MySocket::InsertIntoDatabase(const CString& fileName, const CString& screwType)
{
    MYSQL* conn;
    MYSQL_RES* res;
    MYSQL_ROW row;

    const char* server = "localhost";
    const char* user = "root";
    const char* password = "1234"; /* set me first */
    const char* database = "screwdatabase";

    conn = mysql_init(NULL);

    // 데이터베이스에 연결
    if (!mysql_real_connect(conn, server, user, password, database, 0, NULL, 0)) {
        TRACE("MySQL connection failed: %s\n", mysql_error(conn));
        return;
    }

    // 쿼리를 작성
    CString query;
    query.Format(_T("INSERT INTO screwtable (name, screw_type) VALUES ('%s', '%s')"),
        fileName, screwType);

    cs.Lock(); // 멀티스레드 보호 시작
    // 쿼리를 실행
    if (mysql_query(conn, CT2A(query))) {
        TRACE("Insert query failed: %s\n", mysql_error(conn));
    }
    else 
    {
        TRACE("Insert query succeeded.\n");
    }
    cs.Unlock(); // 멀티스레드 보호 끝

    // 연결 종료
    mysql_close(conn);
}

void MySocket::ExportDataToCSV(const CString& csvFilePath)
{
    MYSQL* conn;
    MYSQL_RES* res;
    MYSQL_ROW row;

    const char* server = "localhost";
    const char* user = "root";
    const char* password = "1234"; // 실제 비밀번호로 변경해야 함.
    const char* database = "screwdatabase";

    conn = mysql_init(NULL);

    // 데이터베이스에 연결
    if (!mysql_real_connect(conn, server, user, password, database, 0, NULL, 0)) {
        TRACE("MySQL connection failed: %s\n", mysql_error(conn));
        return;
    }

    // 문자 인코딩을 UTF-8로 설정
    mysql_set_character_set(conn, "utf8");

    // 쿼리 실행
    if (mysql_query(conn, "SELECT num, name, screw_type FROM screwtable")) {
        TRACE("Query execution failed: %s\n", mysql_error(conn));
        mysql_close(conn);
        return;
    }

    res = mysql_use_result(conn);

    // CSV 파일 열기
    CStdioFile file;
    if (!file.Open(csvFilePath, CFile::modeCreate | CFile::modeWrite | CFile::typeText)) {
        TRACE("Failed to open CSV file\n");
        mysql_free_result(res);
        mysql_close(conn);
        return;
    }

    // 헤더 작성
    file.WriteString(_T("num,name,screw_type\n"));

    // 데이터베이스 결과 처리 및 CSV 파일에 쓰기
    while ((row = mysql_fetch_row(res)) != NULL)
    {
        // NULL 종료 문자를 각 필드 끝에 추가
        CString csvLine;
        csvLine.Format(_T("%s,%s,%s\n"), CString(row[0]), CString(row[1]), CString(row[2]));
        file.WriteString(csvLine);
    }

    // 자원 정리
    file.Close();
    mysql_free_result(res);
    mysql_close(conn);
}
