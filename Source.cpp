#include <windows.h>
#include "resource.h"

#include <cmath>
#include <vector>
#include <array>
#include <random> // for std::mt19937
#include <ctime> // for std::time

HBITMAP CreateBitmapMask(HBITMAP hbmColour, COLORREF crTransparent);
class Player;
static std::vector< Player> team2{};
HDC hdcbuffer;


namespace Random
{
    std::mt19937 mt{ std::random_device{}() };

    int get(int min, int max)
    {
        std::uniform_int_distribution die{ min, max }; // we can create a distribution in any function that needs it
        return die(mt); // and then generate a random number from our global generator
    }
}



struct Position
{
    int x;
    int y;
};


void calcBezier(std::vector<POINT>& vec, const POINT& p0, const POINT& p1, const POINT& p2)
{

    float t = 10;

    for (int i = 0; i < t; ++i)
    {
        POINT point{ 0,0 };
        point.x = std::pow(1 - i / t, 2) * p0.x + 2 * (1 - i / t) * i / t * p1.x + std::pow(i / t, 2) * p2.x;
        point.y = std::pow(1 - i / t, 2) * p0.y + 2 * (1 - i / t) * i / t * p1.y + std::pow(i / t, 2) * p2.y;

        //point.x *= 10;
        //point.y *= 10;

        vec.push_back(point);
    }

    //return vec;
}






class Player
{
public:
    HWND m_hwnd;
    POINT m_pos;
    int m_idb;

    HDC m_hdcMem;
    HBITMAP m_hbitmap;
    HBITMAP m_hbmask;
    BITMAP m_bitmap;



    Player(const HWND& hwnd, const int& idb, const POINT& pos = { 0,0 }) :
        m_hwnd{ hwnd }, m_idb{ idb }, m_pos{ pos }
    {

        m_hbitmap = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(m_idb));
        if (m_hbitmap == NULL)
            MessageBox(m_hwnd, L"Could not load image! ", L"Error", MB_OK | MB_ICONEXCLAMATION);

        m_hdcMem = CreateCompatibleDC(0);

        m_hbmask = CreateBitmapMask(m_hbitmap, RGB(0, 0, 0));
        if (m_hbmask == NULL)
            MessageBox(hwnd, L"Could not create mask!", L"Error", MB_OK | MB_ICONEXCLAMATION);

        GetObject(m_hbitmap, sizeof(BITMAP), &m_bitmap);

       
    }

    ~Player() {
        DeleteDC(m_hdcMem);
        DeleteObject(m_hbitmap);
        DeleteObject(m_hbmask);
    }
           

};




bool playerNearBall(const Player& forward, const Player& ball)
{

    return (std::abs(forward.m_pos.x - ball.m_pos.x) <= 50 && std::abs(forward.m_pos.y - ball.m_pos.y) <= 50);
}

void team2Game(const Player& fwd, POINT& positions0, POINT& positions1, POINT& positions2, POINT& positions3,  const Player& ball, const int& cxClient, const int& cyClient, std::vector<POINT>& curve1, std::vector<POINT>& curve2)
{

    static bool goalScoredbyOpponent = false;
    static bool haflBackPass = false;
    //team2.at(0), forward, always tries to get the ball and score a goal
    
    if (!goalScoredbyOpponent)
    {
        if (positions0.x != ball.m_pos.x || positions0.y != ball.m_pos.y)
        {
            int xdistance = positions0.x - ball.m_pos.x;
            int ydistance = positions0.y - ball.m_pos.y;
            if (xdistance >= 0)
                positions0.x -= 2;
            if (ydistance >= 0)
                positions0.y -= 2;
            if (xdistance <= 0)
                positions0.x += 2;
            if (ydistance <= 0)
                positions0.y += 2;

            if (std::abs(xdistance) <= 5 && std::abs(ydistance) <= 5)
            {
                POINT p0 = { positions0.x, positions0.y };
                POINT p1 = { positions0.x / 2, cyClient / 2 };
                POINT p2 = { 0,cyClient / 2 };
                calcBezier(curve1, p0, p1, p2);
                goalScoredbyOpponent = true;
            }

            else
                goalScoredbyOpponent = false;


        }
    }


    //HalfBacks attack you only when you enter their territory and once gets the ball passes to the half back2

    if (!haflBackPass)
    {

        int xTerrDist = 600;
        int yTerrDist1 = 100;

        
        
        if (playerNearBall(fwd, ball) && (fwd.m_pos.x >= xTerrDist && fwd.m_pos.y >= yTerrDist1))
        {

            int xdistToBall = positions1.x - ball.m_pos.x;
            int ydistToBall = positions1.y - ball.m_pos.y;
            if (xdistToBall > 0)
                positions1.x -= 3;
            if (ydistToBall > 0)
                positions1.y -= 3;
            if (xdistToBall < 0)
                positions1.x += 3;
            if (ydistToBall < 0)
                positions1.y += 3;

            if (std::abs(xdistToBall) <= 8 && std::abs(ydistToBall) <= 8)
            {
                POINT passp0 = { positions1.x, positions1.y };
                POINT passp1 = { positions2.x + 35, positions2.y / 2 };
                POINT passp2 = { positions2.x, positions2.y+fwd.m_bitmap.bmHeight };
                calcBezier(curve2, passp0, passp1, passp2);
                haflBackPass = true;

            }
            else
                haflBackPass = false;

        }

        

        

    }



    //GoalKeeper moves up and down near the gate
    static bool glKeeperLimitUp{ true };
    if (positions3.y >= 370)
        glKeeperLimitUp = false;
    if (positions3.y <= 270)
        glKeeperLimitUp = true;
    
    if (glKeeperLimitUp)
        positions3.y += 4;
    if (!glKeeperLimitUp)
        positions3.y -= 4;
    


}

HBITMAP CreateBitmapMask(HBITMAP hbmColour, COLORREF crTransparent)
{
    HDC hdcMem, hdcMem2;
    HBITMAP hbmMask;
    BITMAP bm;

    GetObject(hbmColour, sizeof(BITMAP), &bm);
    hbmMask = CreateBitmap(bm.bmWidth, bm.bmHeight, 1, 1, NULL);

    hdcMem = CreateCompatibleDC(0);
    hdcMem2 = CreateCompatibleDC(0);

    SelectObject(hdcMem, hbmColour);
    SelectObject(hdcMem2, hbmMask);

    SetBkColor(hdcMem, crTransparent);

    BitBlt(hdcMem2, 0, 0, bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, SRCCOPY);

    BitBlt(hdcMem, 0, 0, bm.bmWidth, bm.bmHeight, hdcMem2, 0, 0, SRCINVERT);

    DeleteDC(hdcMem);
    DeleteDC(hdcMem2);

    return hbmMask;
}

class Opponent :public Player
{
public:

    Opponent(const HWND& hwnd, const int& idb, const POINT& pos = { Random::get(400,1000),Random::get(0,600) }) : Player(hwnd, idb, pos) {}
};

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


void draw(const HDC &hdc, const RECT &rect, const Player& field, const Player& fwd, const Player& ball,   std::vector<Player>& team2, const int& cxClient, const int& cyClient )
{
    //HDC hdcbuffer = CreateCompatibleDC(hdc);
    hdcbuffer = CreateCompatibleDC(hdc);
    HBITMAP hbitmapBuffer = CreateCompatibleBitmap(hdc, rect.right, rect.bottom);

    SelectObject(hdcbuffer, hbitmapBuffer);

    //FIELD
    SelectObject(field.m_hdcMem, field.m_hbitmap);
    StretchBlt(hdcbuffer, 0, 0, cxClient, cyClient, field.m_hdcMem, 0, 0, field.m_bitmap.bmWidth, field.m_bitmap.bmHeight, SRCCOPY);


    //PLAYER
    SelectObject(fwd.m_hdcMem, fwd.m_hbmask);
    BitBlt(hdcbuffer, fwd.m_pos.x, fwd.m_pos.y, fwd.m_bitmap.bmWidth, fwd.m_bitmap.bmHeight, fwd.m_hdcMem, 0, 0, SRCCOPY);
    SelectObject(fwd.m_hdcMem, fwd.m_hbitmap);
    BitBlt(hdcbuffer, fwd.m_pos.x, fwd.m_pos.y, fwd.m_bitmap.bmWidth, fwd.m_bitmap.bmHeight, fwd.m_hdcMem, 0, 0, SRCPAINT);

    //Opponent Team Drawing
    for (int i = 0; i < team2.size(); ++i)
    {

        SelectObject(team2.at(i).m_hdcMem, team2.at(i).m_hbitmap);
        BitBlt(hdcbuffer, team2.at(i).m_pos.x, team2.at(i).m_pos.y, team2.at(i).m_bitmap.bmWidth, team2.at(i).m_bitmap.bmHeight, team2.at(i).m_hdcMem, 0, 0, SRCCOPY);
    }

    //BALL
    SelectObject(ball.m_hdcMem, ball.m_hbmask);
    BitBlt(hdcbuffer, ball.m_pos.x, ball.m_pos.y, ball.m_bitmap.bmWidth, ball.m_bitmap.bmHeight, ball.m_hdcMem, 0, 0, SRCCOPY);
    SelectObject(ball.m_hdcMem, ball.m_hbitmap);
    BitBlt(hdcbuffer, ball.m_pos.x, ball.m_pos.y, ball.m_bitmap.bmWidth, ball.m_bitmap.bmHeight, ball.m_hdcMem, 0, 0, SRCPAINT);
         

    

    BitBlt(hdc, 0, 0, rect.right, rect.bottom, hdcbuffer, 0, 0, SRCCOPY);
    SelectObject(hdcbuffer, hbitmapBuffer);
    DeleteDC(hdcbuffer);
    DeleteObject(hbitmapBuffer);

   
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    // Register the window class.
    const wchar_t CLASS_NAME[] = L"Sample Window Class";

    WNDCLASS wc = { };

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.style = CS_HREDRAW | CS_VREDRAW;//When a rect is invalid it resizes the window
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);//When a rect is invalid it repaints it back to WHITE


    RegisterClass(&wc);

    // Create the window.

    HWND hwnd = CreateWindowEx(
        0,                              // Optional window styles.
        CLASS_NAME,                     // Window class
        L"Learn to Program Windows",    // Window text
        WS_OVERLAPPEDWINDOW | WS_VSCROLL,            // Window style

        // Size and position
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

        NULL,       // Parent window    
        NULL,       // Menu
        hInstance,  // Instance handle
        NULL        // Additional application data
    );

    if (hwnd == NULL)
    {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);

    // Run the message loop.

    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;

    PAINTSTRUCT ps;
    RECT rect;

    static int cyClient, cxClient, cyChar, cxChar, cxCaps, xTeam2Move, yTeam2Move;



    TEXTMETRIC tm;

    static Player fwd{ hwnd,IDB_PLAYER, POINT{100,100} };
    static Player ball{ hwnd,IDB_BALL, POINT{0,300} };
    static Player field{ hwnd,IDB_FIELD };
    static int timer{};
    static int timer2{};
    static int timer3{};

    static bool ballTaken{ false };
    const int size{ 4 };
    

    
    static POINT positions0 = {500,300};
    static POINT positions1 = {750,100};
    static POINT positions2 = {750,500};
    static POINT positions3 = { 1000,300 };
    

    

    /*
    POINT p0 = { 0,0 };
    POINT p1 = { 300 ,300 };
    POINT p2 = { 600,300 };

    std::vector<POINT> curve{};
    calcBezier(curve,ball.m_pos, p1, p2);
    */

    //static std::vector< Player> team2{};

    Player opp1{ hwnd,IDB_OPPONENT, positions0 };
    Player opp2{ hwnd,IDB_OPPONENT, positions1 };
    Player opp3{ hwnd,IDB_OPPONENT, positions2 };
    Player opp4{ hwnd,IDB_OPPONENT, positions3 };

    team2.push_back(opp1);
    team2.push_back(opp2);
    team2.push_back(opp3);
    team2.push_back(opp4);
    

    std::vector<POINT> curve{};
    static std::vector<POINT> curve1;
    static std::vector<POINT> curve2;

    switch (uMsg)
    {

    case WM_CREATE:
       
        SetTimer(hwnd, 2, 100, NULL);

        return 0;

    case WM_SIZE:
        cxClient = LOWORD(lParam); //lParam containts width and height of the client area
        cyClient = HIWORD(lParam);

        return 0;
    case WM_PAINT:
    {
        
        hdc = BeginPaint(hwnd, &ps); // !!! Use only with WM_PAINT. Its hdc points to the invalidated (to-be-redrawn) area of the window
        GetClientRect(hwnd, &rect);
        
        draw(hdc, rect, field, fwd, ball, team2, cxClient, cyClient );

        
        EndPaint(hwnd, &ps);
        return 0;
    }


    case WM_KEYDOWN:
    {

        switch (wParam)
        {
        case VK_RIGHT:

            fwd.m_pos.x += 4;
            
            if (ballTaken)
                ball.m_pos = POINT{ fwd.m_pos.x + ball.m_bitmap.bmWidth,fwd.m_pos.y + ball.m_bitmap.bmHeight };
            InvalidateRect(hwnd, NULL, FALSE);

            break;

        case VK_LEFT:

            fwd.m_pos.x -= 4;
            if (ballTaken)
                ball.m_pos = POINT{ fwd.m_pos.x + ball.m_bitmap.bmWidth,fwd.m_pos.y + ball.m_bitmap.bmHeight };
            InvalidateRect(hwnd, NULL, FALSE);

            break;

        case VK_UP:
            fwd.m_pos.y -= 4;;
            if (ballTaken)
                ball.m_pos = POINT{ fwd.m_pos.x + ball.m_bitmap.bmWidth,fwd.m_pos.y + ball.m_bitmap.bmHeight };
            
            InvalidateRect(hwnd, NULL, FALSE);
            break;

        case VK_DOWN:
            fwd.m_pos.y += 4;
            if (ballTaken)
                ball.m_pos = POINT{ fwd.m_pos.x + ball.m_bitmap.bmWidth,fwd.m_pos.y + ball.m_bitmap.bmHeight };
            InvalidateRect(hwnd, NULL, FALSE);
            break;

        }


        return 0;
    }

    case WM_CHAR:
    {
        switch (wParam)
        {
        case 'a':


            if (playerNearBall(fwd, ball))
                ballTaken = true;
                        
                   

            if (!playerNearBall(fwd, ball))
                ballTaken = false;
                          
                        
            break;

        case 'd':
            if (ballTaken)
            {
                SetTimer(hwnd, 1, 100, NULL);
                timer = 0;
                ballTaken = false;
            }

            
                

            break;
        }

        return 0;
    }

    case WM_LBUTTONDOWN:
    {

        return 0;
    }

    case WM_RBUTTONDOWN:
    {


        return 0;
    }


    case WM_TIMER:
        switch (wParam)
        {
            //std::vector<POINT> curve{};
        case 1:


            //std::vector<POINT> curve{};
            calcBezier(curve, ball.m_pos, POINT{ (cxClient - ball.m_pos.x), (cyClient - ball.m_pos.y) / 2 }, POINT{ cxClient + 150,cyClient / 2 + 25 });



            if (timer < curve.size())
            {
                ball.m_pos = curve.at(timer);
                ++timer;
                InvalidateRect(hwnd, NULL, FALSE);

            }
            else
                KillTimer(hwnd, 1);


            break;


        case 2:
            //std::vector<POINT> curve2;
            team2Game(fwd, positions0, positions1, positions2, positions3, ball, cxClient, cyClient, curve1, curve2);
            if (curve1.size() > 0)
            {
                SetTimer(hwnd, 3, 100, NULL);
                ballTaken = false;

            }
            else
            {
                KillTimer(hwnd, 3);
            }

            if (curve2.size() > 0)
            {
                SetTimer(hwnd, 4, 100, NULL);
                ballTaken = false;
            }
            else
            {
                KillTimer(hwnd, 4);
            }
            InvalidateRect(hwnd, NULL, FALSE);
            break;


        case 3:
            if (timer2 < curve1.size())
            {
                ball.m_pos = curve1.at(timer2);
                ++timer2;
                InvalidateRect(hwnd, NULL, FALSE);
            }
            //InvalidateRect(hwnd, NULL, FALSE);
            break;

        case 4:
            if (timer3 < curve2.size())
            {
                ball.m_pos = curve2.at(timer3);
                ++timer3;
                InvalidateRect(hwnd, NULL, FALSE);
            }

            //InvalidateRect(hwnd, NULL, FALSE);
            break;

        }
        return 0;

    case WM_ERASEBKGND:
    {
        return true;
    }


    case WM_DESTROY:

        //DeleteObject(hbitmap);
        //DeleteObject(hbmMaskPlayer);
        //KillTimer(hwnd, 1);
        KillTimer(hwnd, 2);
        //KillTimer(hwnd, 3);
        //KillTimer(hwnd, 4);
        PostQuitMessage(0);
        return 0;

    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


