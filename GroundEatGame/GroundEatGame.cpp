// GroundEatGame.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "framework.h"
#include "GroundEatGame.h"
#include <iostream>
#include <cmath>
#include <vector>

#define MAX_LOADSTRING 100

using namespace std;

// >> : BITMAP
#pragma comment(lib, "msimg32.lib")

HBITMAP hBackImage;
BITMAP  bitBack;

HBITMAP hFrontImage;
BITMAP  bitFront;

void CreateBitmap();
void DeleteBitmap();
// << : BITMAP

// >> : Double Buffering
HBITMAP hDoubleBufferImage;
RECT rectView;

void DrawBitmapDoubleBuffering(HWND hWnd, HDC hdc);
// << : Double Buffering


// 키보드 입력
enum { Left, Right, Up, Down, Painting, Moving, None };
static int state = None;
static int paintState = None;
static int previousState = None;

//// 주인공
// 위치 및 반지름
static POINT center = { 300, 200 };
int radius = 10;
// 지나간 길
static vector<POINT> location;
static bool moveLR = TRUE;  // 지나간 선 위에 있을 때 좌우 통제
static bool moveUD = TRUE;  // 지나간 선 위에 있을 때 상하 통제

//// 배경
// 색칠
static vector<POINT> painted;    // 색칠할 지점, 계속 초기화하고 갱신할 변수
static vector<POINT> startRange; // startRange = 꺾인 점
static vector<POINT> endRange;   // endRange   = 그려진 선분과 만나는 점 
static int endRangeCount = 0; // endRange index 증가용

static vector<int> paintSize;  // 그릴 도형의 꼭짓점 개수 저장 벡터
static int paintSizeCount = 0; // 그릴 도형의 꼭짓점 개수
static bool endSaved = FALSE;

POINT* temp = new POINT;      // 색칠된 도형 저장
static vector<POINT*> shapes; // temp를 저장할 벡터


#ifdef UNICODE
#pragma comment(linker, "/entry:wWinMainCRTStartup /subsystem:console") 
#else
#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console") 
#endif


// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 여기에 코드를 입력합니다.

    // 전역 문자열을 초기화합니다.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_GROUNDEATGAME, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 애플리케이션 초기화를 수행합니다:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_GROUNDEATGAME));

    MSG msg;

    // 기본 메시지 루프입니다:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}


//
//  함수: MyRegisterClass()
//
//  용도: 창 클래스를 등록합니다.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GROUNDEATGAME));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_GROUNDEATGAME);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   함수: InitInstance(HINSTANCE, int)
//
//   용도: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
//   주석:
//
//        이 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
//        주 프로그램 창을 만든 다음 표시합니다.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      600, 100, 800, 800, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  용도: 주 창의 메시지를 처리합니다.
//
//  WM_COMMAND  - 애플리케이션 메뉴를 처리합니다.
//  WM_PAINT    - 주 창을 그립니다.
//  WM_DESTROY  - 종료 메시지를 게시하고 반환합니다.
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;

    switch (message)
    {
    case WM_CREATE: // 초기화 값 세팅
    {
        //SetTimer(hWnd, 1, 100, NULL);
        location.push_back(center);

        GetClientRect(hWnd, &rectView);
        CreateBitmap();
    }
    break;

    case WM_TIMER:
    {   
        //InvalidateRect(hWnd, NULL, TRUE);
    }
    break;

    case WM_KEYDOWN: // 눌리면 발생
    {
        // 상태 구분
        if (GetAsyncKeyState(VK_SPACE) & 0x8000)
        {
            paintState = Painting;
        }
        else if (wParam == VK_LEFT || wParam == VK_RIGHT || wParam == VK_UP || wParam == VK_DOWN)
        {
            paintState = Moving;
        }
        else
            paintState = None;

        // 그리기 상태
        if (paintState == Painting)
        {
            if (GetAsyncKeyState(VK_LEFT) & 0x8000)
            {
                if (previousState != Left && previousState != Right)
                {
                    painted.push_back(center);
                    startRange.push_back(center);
                    paintSizeCount++;
                }
                else
                {
                    if (state != Right && moveLR == TRUE)
                    {
                        center.x -= 5;
                        state = Left;
                    }
                }
                previousState = Left;
            }
            else if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
            {
                if (previousState != Right && previousState != Left)
                {
                    painted.push_back(center);
                    startRange.push_back(center);
                    paintSizeCount++;
                }
                else
                {
                    if (state != Left && moveLR == TRUE)
                    {
                        center.x += 5;
                        state = Right;
                    }
                }
                previousState = Right;
            }
            else if (GetAsyncKeyState(VK_UP) & 0x8000)
            {
                if (previousState != Up && previousState != Down)
                {
                    painted.push_back(center);
                    startRange.push_back(center);
                    paintSizeCount++;
                }
                else
                {
                    if (state != Down && moveUD == TRUE)
                    {
                        center.y -= 5;
                        state = Up;
                    }
                }
                previousState = Up;
            }
            else if (GetAsyncKeyState(VK_DOWN) & 0x8000)
            {
                if (previousState != Down && previousState != Up)
                {
                    painted.push_back(center);
                    startRange.push_back(center);
                    paintSizeCount++;
                }
                else
                {
                    if (state != Up && moveUD == TRUE)
                    {
                        center.y += 5;
                        state = Down;
                    }
                }
                previousState = Down;
            }
            else
                break;
            
            location.push_back(center);
        }
        // 테두리에서 이동할수 있는 상태
        else if(paintState == Moving)
        {
            endSaved = FALSE;

            if (GetAsyncKeyState(VK_LEFT) & 0x8000)
            {
                if (moveLR == TRUE)
                {
                    for (int i = 1; i < startRange.size(); i++)
                    {
                        if (startRange[i - 1].x <= center.x && center.x <= startRange[i].x)
                            center.x -= 5;
                    }
                    state = Left;
                }
            }
            else if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
            {
                if (moveLR == TRUE)
                {
                    for (int i = 1; i < startRange.size(); i++)
                    {
                        if (startRange[i - 1].x <= center.x && center.x <= startRange[i].x)
                            center.x += 5;
                    }
                    state = Right;
                }
            }
            else if (GetAsyncKeyState(VK_UP) & 0x8000)
            {
                if (moveUD == TRUE)
                {
                    for (int i = 1; i < startRange.size(); i++)
                    {
                        if (startRange[i - 1].y <= center.y && center.y <= startRange[i].y)
                            center.y -= 5;
                    }
                    state = Up;
                }
            }
            else if (GetAsyncKeyState(VK_DOWN) & 0x8000)
            {
                if (moveUD == TRUE)
                {
                    for (int i = 1; i < startRange.size(); i++)
                    {
                        if (startRange[i - 1].y <= center.y && center.y <= startRange[i].y)
                            center.y += 5;
                    }
                    state = Down;
                }
                
            }
        }

        // 선분 위에 있음을 체크
        if (startRange.size() > 1)
        {
            for (int i = 1; i < startRange.size(); i++)
            {
                if (startRange[i - 1].x == startRange[i].x && center.x == startRange[i].x) // 일직선에 있을 때(가로)
                {
                    if (startRange[i - 1].y < center.y && center.y < startRange[i].y)
                        moveLR = FALSE;
                    else
                        moveLR = TRUE;
                }
                if (startRange[i - 1].y == startRange[i].y && center.y == startRange[i].y) // 일직선에 있을 때(세로)
                {
                    if (startRange[i - 1].x < center.x && center.x < startRange[i].x)
                        moveUD = FALSE;
                    else
                        moveUD = TRUE;
                }
            }
        }

        // endRange 설정
        if (startRange.size() > 1 && paintState == Painting)
        {
            for (int i = 1; i < startRange.size(); i++)
            {
                if (startRange[i - 1].x == startRange[i].x && center.x == startRange[i].x) // 일직선에 있을 때(가로)
                {
                    if (startRange[i - 1].y < center.y && center.y < startRange[i].y)
                    {
                        endRange.push_back(center);
                        endSaved = TRUE;
                        break;
                    }
                }
                if (startRange[i - 1].y == startRange[i].y && center.y == startRange[i].y) // 일직선에 있을 때(세로)
                {
                    if (startRange[i - 1].x < center.x && center.x < startRange[i].x)
                    {
                        endRange.push_back(center);
                        endSaved = TRUE;
                        break;
                    }
                }
            }
        }

        InvalidateRect(hWnd, NULL, FALSE);
    }
    break;
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // 메뉴 선택을 구문 분석합니다:
        switch (wmId)
        {
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;

    case WM_PAINT:
    {
        hdc = BeginPaint(hWnd, &ps);

        // 뒷배경 & 더블버퍼링
        DrawBitmapDoubleBuffering(hWnd, hdc);

        // 그리기 상태
        if (paintState == Painting)
        {
            // 선그리기 시작점
            MoveToEx(hdc, location.front().x, location.front().y, NULL);

            // 조작에 따른 선 그리기
            for (int i = 1; i < location.size(); i++)
            {
                LineTo(hdc, location[i].x, location[i].y);
            }

            if (endSaved == TRUE)
            {
                // 영역 표시하기
                if (endRange.size() == endRangeCount + 1) // endRange.size()가 1이여야 첫 색칠이 시작됨
                {
                    cout << "색칠 되는 도형의 수 : " << endRangeCount << endl;
                    //cout << "paint size count : " << paintSizeCount << endl;

                    // 영역 색칠하기
                    for (int m = 0; m < painted.size(); m++)
                    {
                        if (endRangeCount == 0 && m == 0)
                            temp[0] = endRange[endRangeCount];
                        else
                            temp[m] = painted[m];

                    }
                    temp[painted.size()] = endRange[endRangeCount++];

                    shapes.push_back(temp);

                    cout << "Start" << endl;
                    for (int i = 0; i < paintSizeCount; i++)
                    {
                        //cout << "temp : " << temp[i].x << ", " << temp[i].y << endl;
                        //cout << "painted : " << painted[i].x << ", " << painted[i].y << endl;
                    }

                    // 색칠된 부분이면 선이 표시되지 않게끔 설정
                    location.clear();
                    painted.clear();

                    // 현재 위치를 시작점으로 설정
                    location.push_back({ center.x, center.y });
                    painted.push_back({ center.x, center.y });

                    // 도형의 꼭짓점 개수 파악
                    paintSize.push_back(paintSizeCount);
                    paintSizeCount = 0;

                    endSaved = FALSE;
                }
            }
        }
        // 테두리에서 이동할수 있는 상태
        else if (paintState == Moving)
        {
            for (int i = 0; i < location.size(); i++)
                LineTo(hdc, location[i].x, location[i].y);
        }
        // 플레이어
        Ellipse(hdc, center.x - radius, center.y - radius, center.x + radius, center.y + radius);
        
        EndPaint(hWnd, &ps);
    }
    break;

    case WM_DESTROY:
        //KillTimer(hWnd, 1);
        delete[] temp;
        DeleteBitmap();
        PostQuitMessage(0);
    break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// 정보 대화 상자의 메시지 처리기입니다.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

void CreateBitmap()
{
    // >> : 수지
    hBackImage = (HBITMAP)LoadImage(NULL, TEXT("images/수지.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);

    if (hBackImage == NULL) // 이미지가 출력되지 않을 때
    {
        DWORD dwError = GetLastError();
        MessageBox(NULL, _T("이미지 로드 에러1"), _T("에러"), MB_OK);
        return;
    }

    GetObject(hBackImage, sizeof(BITMAP), &bitBack);

    // >> : background
    hFrontImage = (HBITMAP)LoadImage(NULL, TEXT("images/Background.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);

    if (hFrontImage == NULL) // 이미지가 출력되지 않을 때
    {
        DWORD dwError = GetLastError();
        MessageBox(NULL, _T("이미지 로드 에러4"), _T("에러"), MB_OK);
        return;
    }

    GetObject(hFrontImage, sizeof(BITMAP), &bitFront);
}

void DrawBitmapDoubleBuffering(HWND hWnd, HDC hdc)
{
    HDC hMemDC, hMemDC2;
    HBITMAP hOldBitmap, hOldBitmap2;
    int bx, by;

    hMemDC = CreateCompatibleDC(hdc);
    if (hDoubleBufferImage == NULL)
    {
        hDoubleBufferImage = CreateCompatibleBitmap(hdc, rectView.right, rectView.bottom);
    }
    hOldBitmap = (HBITMAP)SelectObject(hMemDC, hDoubleBufferImage); //  기본은 검정색

    // >> : 수지
    {
        hMemDC2 = CreateCompatibleDC(hMemDC); // 같은 포맷
        hOldBitmap2 = (HBITMAP)SelectObject(hMemDC2, hBackImage);
        bx = bitBack.bmWidth; // 전체 너비
        by = bitBack.bmHeight; // 전체 높이

        BitBlt(hMemDC, 0, 0, bx, by, hMemDC2, 0, 0, SRCCOPY); // 그림 그리기

        SelectObject(hMemDC2, hOldBitmap2);
        DeleteDC(hMemDC2);
    }

    // >> : background
    {
        hMemDC2 = CreateCompatibleDC(hMemDC); // 같은 포맷
        hOldBitmap2 = (HBITMAP)SelectObject(hMemDC2, hFrontImage);
        bx = bitFront.bmWidth; // 전체 너비
        by = bitFront.bmHeight; // 전체 높이

        HBRUSH hBrush = CreateSolidBrush(RGB(255, 0, 255));
        HBRUSH oldBrush = (HBRUSH)SelectObject(hMemDC2, hBrush);

        // 색칠될 부분
        if (paintSize.size() > 0)
        {
            for (int i = 0; i < shapes.size(); i++)
                Polygon(hMemDC2, shapes[i], paintSize[i]);
        }

        SelectObject(hMemDC2, oldBrush);
        DeleteObject(hBrush);

        TransparentBlt(hMemDC, 0, 0, bx, by, hMemDC2, 0, 0, bx, by, RGB(255, 0, 255));

        SelectObject(hMemDC2, hOldBitmap2);
        DeleteDC(hMemDC2);
    }

    // >> : hdc에 그려주기
    TransparentBlt(hdc, 0, 0, rectView.right, rectView.bottom, hMemDC, 0, 0, rectView.right, rectView.bottom, RGB(255, 0, 255));

    SelectObject(hMemDC, hOldBitmap);
    DeleteDC(hMemDC);
}

void DeleteBitmap()
{
    DeleteObject(hBackImage);
}
