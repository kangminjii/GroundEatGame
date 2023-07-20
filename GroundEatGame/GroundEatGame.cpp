﻿// GroundEatGame.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "framework.h"
#include "GroundEatGame.h"
#include <iostream>
#include <cmath>
#include <list>

#define MAX_LOADSTRING 100
using namespace std;


#ifdef UNICODE
#pragma comment(linker, "/entry:wWinMainCRTStartup /subsystem:console") 
#else
#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console") 
#endif


//bool isCollided(POINT circle, POINT dot);


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
    HBRUSH hBrush, oldBrush;

    // 화면 경계선
    static RECT rectView;

    // 키보드 입력
    enum{Left, Right, Up, Down, Painting, Moving, None};
    static int state = None;
    static int paintState = None;
    static int previousState = None;
    
    //// 주인공
    // 위치
    static POINT center = { 100, 100 };
    int radius = 10;
    // 지나간 길
    static list<POINT> location;
    // 색칠할 지점
    static list<POINT> painted;
    // painted 좌표 2개로 범위 지정
    static list<POINT> startRange;
    static list<POINT> endRange;

    // 색칠된 도형의 좌표 저장

    switch (message)
    {
    case WM_CREATE: // 초기화 값 세팅
    {
        SetTimer(hWnd, 1, 500, NULL);

        location.push_back(center);
        painted.push_back(center);
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
                if (previousState != Left)
                    painted.push_back(center);
                else
                {
                    if (state != Right)
                    {
                        center.x -= 5;
                        state = Left;
                    }
                }
                previousState = Left;
            }
            else if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
            {
                if (previousState != Right)
                    painted.push_back(center);
                else
                {
                    if (state != Left)
                    {
                        center.x += 5;
                        state = Right;
                    }
                }
                previousState = Right;
            }
            else if (GetAsyncKeyState(VK_UP) & 0x8000)
            {
                if (previousState != Up)
                    painted.push_back(center);
                else
                {
                    if (state != Down)
                    {
                        center.y -= 5;
                        state = Up;
                    }
                }
                previousState = Up;
            }
            else if (GetAsyncKeyState(VK_DOWN) & 0x8000)
            {
                if (previousState != Down)
                    painted.push_back(center);
                else
                {
                    if (state != Up)
                    {
                        center.y += 5;
                        state = Down;
                    }
                }
                previousState = Down;
            }
            else
                painted.push_back(center);
          
            list<POINT>::iterator it3;
            for (it3 = painted.begin(); it3 != painted.end(); it3++)
            {
                cout << "it3 : " << it3 << endl;
                cout << "painted: " << (*it3).x << ", " << (*it3).y << endl;

            }
            if (startRange.size() == 0)
                startRange.push_back(painted.back());
            else if(startRange.size() > 1)
            {
                cout << "error" << endl;

                startRange.push_back(painted.back());
                endRange.push_back(painted.back());
            }

            location.push_back(center);
        }
        // 테두리에서 이동할수 있는 상태
        else if(paintState == Moving)
        {
            if (GetAsyncKeyState(VK_LEFT) & 0x8000)
            {
                center.x -= 5;
                state = Left;
            }
            else if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
            {
                center.x += 5;
                state = Right;
            }
            else if (GetAsyncKeyState(VK_UP) & 0x8000)
            {
                center.y -= 5;
                state = Up;
            }
            else if (GetAsyncKeyState(VK_DOWN) & 0x8000)
            {
                center.y += 5;
                state = Down;
            }
        }
        
        InvalidateRect(hWnd, NULL, TRUE);
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

        // 선그리기 시작점
        MoveToEx(hdc, location.front().x, location.front().y, NULL);

        // 그리기 상태
        list<POINT>::iterator it = location.begin();

        if (paintState == Painting)
        {
            // 조작에 따른 선 그리기

            list<POINT>::iterator it = location.begin();

            for (it = location.begin(); it != location.end(); it++)
                LineTo(hdc, (*it).x, (*it).y);

            // 영역 표시하기

            list<POINT>::iterator it1;
            list<POINT>::iterator it2;

            if (startRange.size() > 1)
            {
                for (it1 = startRange.begin(), it2 = endRange.begin(); it1 != startRange.end() && it2 != endRange.end(); it1++, it2++)
                {
                    // 지나간 부분의 선과 만났을 때
                    if (((*it1).x <= center.x && center.x <= (*it2).x) && ((*it1).y <= center.y && center.y <= (*it2).y))
                    {
                        POINT temp[200];

                        for (int j = 0; j < painted.size(); j++)
                        {
                            temp[j] = painted.front();
                            painted.pop_front();
                        }

                        Polygon(hdc, temp, painted.size());
                    }
                }
            }
           
            // 영역 색칠하기
            POINT temp[20];
            for (int j = 0; j < painted.size(); j++)
            {
                temp[j] = painted.front();
                painted.pop_front();
            }
            hBrush = CreateSolidBrush(RGB(0, 0, 255));
            oldBrush = (HBRUSH)SelectObject(hdc, hBrush);

            Polygon(hdc, temp, painted.size());

            SelectObject(hdc, oldBrush);
            DeleteObject(hBrush);
           
            // 플레이어
            Ellipse(hdc, location.back().x - radius, location.back().y - radius, location.back().x + radius, location.back().y + radius);
        }
        // 테두리에서 이동할수 있는 상태
        else if (paintState == Moving)
        {
            list<POINT>::iterator it = location.begin();

            for (it = location.begin(); it != location.end(); it++)
                LineTo(hdc, (*it).x, (*it).y);
            
            Ellipse(hdc, center.x - radius, center.y - radius, center.x + radius, center.y + radius);
        }
       
        EndPaint(hWnd, &ps);
    }
    break;

    case WM_DESTROY:
        KillTimer(hWnd, 1);
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


//bool isCollided(POINT circle, POINT dot)
//{
//    double distance = sqrt((circle.x - dot.x) * (circle.x - dot.x) + (circle.y - dot.y) * (circle.y - dot.y));
//    
//    if (distance < 1)   return TRUE;
//    else return FALSE;
//}