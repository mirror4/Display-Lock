// displayLock-win32.cpp : Defines the entry point for the application.
//

#include "header.h"
#include "displayLock-win32.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

HFONT hFont;

Args *ComboBoxArgs;

Menu menu;
HWND hWndListBox;
HWND hwndButtonStart;
HWND hwndButtonStop;

// messages
HANDLE hMessageStop;
HANDLE hMessageEmpty;

HANDLE CLIP;
int currentSel;
HANDLE mutex;

BOOL active;

// Forward declarations of functions included in this code module:
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

    // TODO: Place code here.
    // initalize menu
    initMenu(&menu);
    hMessageStop = CreateEvent(NULL, FALSE, TRUE, _T("STOP"));
    hMessageEmpty = CreateEvent(NULL, FALSE, FALSE, _T("EMPTY"));
    SetEvent(hMessageEmpty);

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_DISPLAYLOCKWIN32, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_DISPLAYLOCKWIN32));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, NULL, 0, 0))
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
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DISPLAYLOCKWIN32));
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_DISPLAYLOCKWIN32);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));


    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
      CW_USEDEFAULT, 0, 360, 200, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    SetEvent(hMessageEmpty);
    Args *ComboBoxArgs = malloc(sizeof(Args));

    currentSel = 0;
    ComboBoxArgs->menu = &menu;
    ComboBoxArgs->hWnd = &hWndListBox;
    ComboBoxArgs->mutex = &mutex;
    ComboBoxArgs->currentSel = &currentSel;

    winArgs *windowArgs = malloc(sizeof(winArgs));

    windowArgs->mutex = &mutex;
    windowArgs->active = &active;
    
    char selector[500];
    GetWindowTextA(hWndListBox, selector, 500);
    windowArgs->window = sortWindow(ComboBoxArgs, selector, menu.windows.count);

    switch (message)
    {
    case WM_CREATE:
    {
        hFont = CreateFont(18, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET,
            OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
            DEFAULT_PITCH | FF_DONTCARE, TEXT("Times New Roman"));

        SendMessage(hWnd, WM_SETFONT, (WPARAM)hFont, TRUE);


        // create dropdown menu
        hWndListBox = CreateWindowEx(
            WS_EX_CLIENTEDGE,
            TEXT("COMBOBOX"),   // Predefined class; Unicode assumed
            NULL,
            WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST | CBS_HASSTRINGS | CBS_AUTOHSCROLL | CBS_DISABLENOSCROLL | WS_VSCROLL,
            20,     // x position 
            20,     // y position 
            300,    // Button width
            200,    // Button height
            hWnd,   // Parent window
            (HMENU)IDC_DROPDOWNLIST,
            NULL,
            NULL);

        // start/top button
        hwndButtonStart = CreateWindowEx(
            WS_EX_WINDOWEDGE,
            TEXT("BUTTON"),  // Predefined class; Unicode assumed 
            TEXT("Start"),      // Button text 
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,  // Styles 
            110,         // x position 
            60,         // y position 
            100,        // Button width
            35,        // Button height
            hWnd,     // Parent window
            (HMENU)IDC_OPBUTTON_START,       // No menu.
            NULL,
            NULL); // Pointer not needed.
        
        // start/top button
        hwndButtonStop = CreateWindowEx(
            WS_EX_WINDOWEDGE,
            TEXT("BUTTON"),  // Predefined class; Unicode assumed 
            TEXT("Stop"),      // Button text 
            WS_DISABLED | WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,  // Styles 
            220,         // x position 
            60,         // y position 
            100,        // Button width
            35,        // Button height
            hWnd,     // Parent window
            (HMENU)IDC_OPBUTTON_STOP,       // No menu.
            NULL,
            NULL); // Pointer not needed.

        SendMessage(hwndButtonStart, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hwndButtonStop, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hWndListBox, WM_SETFONT, (WPARAM)hFont, TRUE);

        HANDLE handle = (HANDLE)_beginthread(updateComboBox, 0, (void*)ComboBoxArgs);

        SendMessage(hWndListBox, CB_SETCURSEL, 0, 0);
        
        break;
    }
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDC_OPBUTTON_START:
            {
                if(!menu.active)
                {
                    WaitForSingleObject(&ComboBoxArgs->mutex, INFINITE);    // wait for mutex

                    ComboBoxArgs->mutex = CreateMutex(NULL, FALSE, NULL);   // lock the mutex

                    *(windowArgs->active) = TRUE;

                    //ComboBoxArgs->menu->selectedWindow = sortWindow(ComboBoxArgs, selector, menu.windows.count);
                    //menu.selectedWindow = sortWindow(ComboBoxArgs, selector, menu.windows.count);
                    ComboBoxArgs->menu->active = TRUE;
                    menu.active = TRUE;
                    EnableWindow(hWndListBox, FALSE);
                    EnableWindow(hwndButtonStop, TRUE);
                    EnableWindow(hwndButtonStart, FALSE);
                    windowArgs->window = sortWindow(ComboBoxArgs, selector, menu.windows.count);
                    active = TRUE;

                    if(windowArgs->window != NULL)
                    {
                        ReleaseMutex(&ComboBoxArgs->mutex);
                        CLIP = (HANDLE)_beginthreadex(NULL, 0, cursorLockEx, (void*)windowArgs, 0, NULL);
                        //CLIP = (HANDLE)_beginthread(cursorLock, 0, (void*)windowArgs);
                    }

                }
                
                break;
            }
            case IDC_OPBUTTON_STOP:
            {
                if (menu.active)
                {
                    menu.active = FALSE;
                    *(windowArgs->active) = FALSE;
                    active = FALSE;
                    ComboBoxArgs->mutex = CreateMutex(NULL, FALSE, NULL);   // lock the mutex
                    //SetEvent(hMessageStop);

                    EnableWindow(hwndButtonStop, FALSE);
                    EnableWindow(hwndButtonStart, TRUE);

                    ReleaseMutex(&ComboBoxArgs->mutex);
                    //WaitForSingleObject(CLIP, INFINITE);
                    ClipCursor(NULL);
                    EnableWindow(hWndListBox, TRUE);
                    CloseHandle(CLIP);
                }
                break;
            }
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                menu.live = FALSE;
                free(ComboBoxArgs);
                free(windowArgs);
                CloseHandle(CLIP);
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        DeleteObject(hFont);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
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
