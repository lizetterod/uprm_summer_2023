// Move axis.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "Move axis.h"
#include <stdio.h> 
#include <math.h>


#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
HANDLE hCom;

static struct HardwareInfo {
    char IPaddress[16];
    char rsrcName[64];
    char AntennaPort[6];
    char PositionerPort[6];
}g_Hardware = { "0", "0", "0", "0" };

// structure to control the COM Ports
struct ComStrc {
    char* ComName;
    int BaudRate;
    int ByteSize;
    int Parity;
    int StopBits;
}PosComPort;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    ComPortDialBox(HWND, UINT, WPARAM, LPARAM);
BOOL InitComPortList(HWND hwndDlg, int indCombo);
BOOL GetHardwareInfoFromFile(HWND hDlg, const char* szFileName);
INT_PTR CALLBACK MoveMotorsDialBox(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK STARTSETUPDialBox(HWND, UINT,WPARAM, LPARAM );
BOOL InitUNITSCOMBOList(HWND hwndDlg, int indCombo);
BOOL isNumber(const char* str);

// function defined in communication.c
void ExitOnError(const char* message);
BOOL OpenCom(struct ComStrc ComPort);
void CloseCom();
DWORD WriteCom(char* buf, int len);
void WriteComChar(char b);
int ReadCom(char* buf, int len);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_MOVEAXIS, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MOVEAXIS));

    MSG msg;

    // Main message loop:
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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MOVEAXIS));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_MOVEAXIS);
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

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

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
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_COMPORT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_COMPORTBOX), hWnd, ComPortDialBox);
                break;
            case IDM_MOVEMOTORS:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_MOVEMOTORSBOX), hWnd, MoveMotorsDialBox);
                break;
            case IDM_Options:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_STARTSETUPBOX), hWnd, STARTSETUPDialBox);
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
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
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

// Message handler for com port box.
INT_PTR CALLBACK ComPortDialBox(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    char szBuffer[256], PosComPort[5];
    FILE* fp;
    int item;

    switch (message)
    {
    case WM_INITDIALOG:
        // Initialize the positioner drop-down list box. 
        InitComPortList(hDlg, IDC_POSPORTCOMBO);
        // Select the first COM port.
        SendDlgItemMessage(hDlg, IDC_POSPORTCOMBO, CB_SETCURSEL, 0, 0);

        // open harware file and Initialize the com port and instrument address
        GetHardwareInfoFromFile(hDlg, "hardware.txt");
        item = (int)g_Hardware.PositionerPort[3] - 48;

        SendDlgItemMessage(hDlg, IDC_POSPORTCOMBO, CB_SETCURSEL, item, 0);
        //SetDlgItemText(hDlg, IDC_INSTRUMENADD, g_Hardware.rsrcName);
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_SAVE:
            SendDlgItemMessage(hDlg, IDC_POSPORTCOMBO, WM_GETTEXT, sizeof(szBuffer), (LPARAM)szBuffer);
            if (szBuffer[0])
                strcpy(PosComPort, szBuffer);
            else {
                MessageBox(hDlg, TEXT("Select a valid COM port for the positioner!"), NULL, MB_ICONERROR | MB_OK);
                return FALSE;
            }
           
            fp = fopen("hardware.txt", "w");
            fprintf(fp, "PosComPort= %s:\n", PosComPort);
            fclose(fp);
            return FALSE;
        case IDOK:
            EndDialog(hDlg, TRUE);
            return FALSE;

        case IDCANCEL:
            EndDialog(hDlg, FALSE);
            return FALSE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

BOOL InitComPortList(HWND hwndDlg, int indCombo)
{
    HWND hwndCombo;
    DWORD dwIndex;
    int index;
    const char* ComboBoxItems[] = { "", "COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7", "COM8" };

    hwndCombo = GetDlgItem(hwndDlg, indCombo);

    for (index = 0; index <= 8; index++)
        dwIndex = SendMessage(hwndCombo, CB_ADDSTRING, 0, (LPARAM)(LPCSTR)ComboBoxItems[index]);
    return TRUE;
}

// GetHardwareInfo funtion read the hardware file a get the antenna and positioner COM ports
BOOL GetHardwareInfoFromFile(HWND hDlg, const char* szFileName)
{
    FILE* fp;
    char   szBuffer[BUFSIZ], szVariableName[BUFSIZ], szVariableValue[BUFSIZ];

    fp = fopen(szFileName, "r");
    if (fp != NULL) {
        //if (fgets(szBuffer, sizeof szBuffer, fp) != NULL) {
        //    sscanf(szBuffer, "%s %s", &szVariableName, &g_Hardware.AntennaPort);
        //}
        //else
        //    return 0;
        if (fgets(szBuffer, sizeof szBuffer, fp) != NULL) {
            sscanf(szBuffer, "%s %s", &szVariableName, &g_Hardware.PositionerPort);
        }
        else
            return 0;

        /*if (fgets(szBuffer, sizeof szBuffer, fp) != NULL)
            sscanf(szBuffer, "%s %s", &szVariableName, &g_Hardware.rsrcName);
        else
            return 0;

        if (fgets(szBuffer, sizeof szBuffer, fp) != NULL)
            sscanf(szBuffer, "%s %s", &szVariableName, &g_Hardware.IPaddress);
        else
            return 0;*/

        fclose(fp);
        return 1;
    }
    return 0;
}

// Message handler for com port box.
INT_PTR CALLBACK MoveMotorsDialBox(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    HWND           hText;
    static int     Distance, Velocity, Motor;
    char           strValue[10];
    char            txBuffer[128] = { 0 }, rxBuffer[128] = { 0 };

    switch (message)
    {
    case WM_INITDIALOG:
        CheckRadioButton(hDlg, IDC_MOTOR1, IDC_MOTOR4, IDC_MOTOR1);
        GetHardwareInfoFromFile(hDlg, "hardware.txt");
        PosComPort.ComName = g_Hardware.PositionerPort;
        PosComPort.BaudRate = 9600;
        PosComPort.ByteSize = 8;
        PosComPort.Parity = 0;  //NOPARITY
        PosComPort.StopBits = 0;

        return (INT_PTR)TRUE;

    case WM_COMMAND:
        // get and verify TRM address
        hText = GetDlgItem(hDlg, IDC_DISTANCE);
        GetWindowText(hText, strValue, 10);
        Distance = atoi(strValue);

        hText = GetDlgItem(hDlg, IDC_Velocity);
        GetWindowText(hText, strValue, 10);
        Velocity = atoi(strValue);
        
        if (IsDlgButtonChecked(hDlg, IDC_MOTOR1) != BST_CHECKED)
            if (IsDlgButtonChecked(hDlg, IDC_MOTOR2) != BST_CHECKED)
                if (IsDlgButtonChecked(hDlg, IDC_MOTOR3) != BST_CHECKED)
                    if (IsDlgButtonChecked(hDlg, IDC_MOTOR4) != BST_CHECKED) {
                        MessageBox(hDlg, TEXT("Set a motor!"),
                            TEXT("blablabla"), MB_ICONEXCLAMATION | MB_OK);
                        return 0;
                    }
                    else {
                        Motor = 4;
                    }
                else {
                    Motor = 3;
                }
            else {
                Motor = 2;
            }
        else {
            Motor = 1;
        }


        {
        case IDC_Negative:
           

            //ElementPosition = InitialPosition + OffsetDistance;
            //MotorSteps = round(ElementPosition / 0.005);
            //ProbePosition = MotorSteps * 0.005;
            if (OpenCom(PosComPort)) {                   // port is valid?
                sprintf(txBuffer, "E,C,S%dM%d,I%d-M%d,R", Motor, Velocity, Motor, Distance);  //I1M-0
                printf("%s\n", txBuffer);
                WriteCom(txBuffer, strlen(txBuffer));
                Sleep(100);
                CloseCom();
            }

            return FALSE;
        case IDC_Positive:
            if (OpenCom(PosComPort)) {                   // port is valid?
                sprintf(txBuffer, "E,C,S%dM%d,I%dM%d,R", Motor, Velocity, Motor, Distance);  //I1M-0
                printf("%s\n", txBuffer);
                WriteCom(txBuffer, strlen(txBuffer));
                Sleep(100);
                CloseCom();
            }
       
        case IDC_Origin:


            //ElementPosition = InitialPosition + OffsetDistance;
            //MotorSteps = round(ElementPosition / 0.005);
            //ProbePosition = MotorSteps * 0.005;
            if (OpenCom(PosComPort)) {                   // port is valid?
                sprintf(txBuffer, "E,C,S%dM%d,I%d-M0,R", Motor, Velocity, Motor);  //I1M-0
                printf("%s\n", txBuffer);
                WriteCom(txBuffer, strlen(txBuffer));
                Sleep(100);
                CloseCom();
            }

            return FALSE;

        case IDCANCEL:
            EndDialog(hDlg, FALSE);
            return FALSE; 
        }
        break;
    }
    return (INT_PTR)FALSE;
}


void ExitOnError(const char* message)
{
    printf("%s error", message);
    getchar();
    exit(1);
}

BOOL OpenCom(struct ComStrc ComPort)
{
    BOOL fRes, Modify = FALSE;
    DWORD dw;
    DCB dcb;
    COMMTIMEOUTS ct;

    hCom = CreateFile(ComPort.ComName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hCom == INVALID_HANDLE_VALUE)
    {
        fRes = FALSE;                           // ExitOnError(COM_name);  // can't open COM port
    }
    else
    {
        if (!SetupComm(hCom, 4096, 4096))     // asigne the tx and rx buffer size
            ExitOnError("Comm setup");

        //JVR - The following properties can be changed for working with communications for other hardware
        if (!GetCommState(hCom, &dcb))               //ExitOnError("GetCommState");
            ExitOnError("Getting comm state");

        if (dcb.BaudRate != ComPort.BaudRate) {
            dcb.BaudRate = ComPort.BaudRate;
            Modify = TRUE;
        }

        if (dcb.ByteSize != ComPort.ByteSize) {
            dcb.ByteSize = ComPort.ByteSize;
            Modify = TRUE;
        }

        if (dcb.Parity != ComPort.Parity) {
            dcb.Parity = ComPort.Parity;
            Modify = TRUE;
        }

        //printf("dcb.StopBits = %d\n",dcb.StopBits);
        if (dcb.StopBits != ComPort.StopBits) {
            dcb.StopBits = ComPort.StopBits;
            Modify = TRUE;
        }

        ((DWORD*)(&dcb))[2] = 0x1001;              // set port properties for TXDI + no flow-control

        if (Modify)
            if (!SetCommState(hCom, &dcb))             // ExitOnError("SetCommState");        
                ExitOnError("Comm state set");

        // set the timeouts to 0
        ct.ReadIntervalTimeout = MAXDWORD;
        ct.ReadTotalTimeoutMultiplier = 0;
        ct.ReadTotalTimeoutConstant = 0;
        ct.WriteTotalTimeoutMultiplier = 0;
        ct.WriteTotalTimeoutConstant = 0;
        if (!SetCommTimeouts(hCom, &ct))           //ExitOnError("SetCommTimeouts");
            ExitOnError("Comm timeouts set");
        fRes = TRUE;

    }
    return fRes;
}

void CloseCom()
{
    CloseHandle(hCom);
}


DWORD WriteCom(char* buf, int len)
{
    DWORD nSend;
    if (!WriteFile(hCom, buf, len, &nSend, NULL)) exit(1);

    return nSend;
}

void WriteComChar(char b)
{
    WriteCom(&b, 1);
}

int ReadCom(char* buf, int len)
{
    DWORD nRec;
    if (!ReadFile(hCom, buf, len, &nRec, NULL)) exit(1);
    return nRec;
}
    
// Message handler for com port box.
INT_PTR CALLBACK STARTSETUPDialBox(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    char szBuffer[BUFSIZ];
    static float Dx, Dy, thetax, thetay, zp, a, b, centerx, centery, Mx, My, Value, px, py, f_GHz, dx, dy, Lambda, Probex, Probey, ProbetoAUTdis;
    static double pi = 3.1410926536210386;
    static double c = 299792458;
    int             TextLength;
    HWND           hText;
    static BOOL        FIXDELTA=true;
    switch (message)
    {
    case WM_INITDIALOG:
        // Initialize the positioner drop-down list box. 
        InitUNITSCOMBOList(hDlg, IDC_UNITSCOMBO);
        // Select the first COM port.
        SendDlgItemMessage(hDlg, IDC_UNITSCOMBO, CB_SETCURSEL, 0, 0);

        // open harware file and Initialize the com port and instrument address
        Dx = 200;
        Dy = 200;
        thetax = 60;
        thetay = 60;
        zp = 90;
        px = 22.86;
        py = 10.16;
        f_GHz = 10;
        
        sprintf(szBuffer, "%.2lf", Dx);
        SetDlgItemText(hDlg, IDC_MOTDISTx, szBuffer);
        sprintf(szBuffer, "%.2lf", Dy);
        SetDlgItemText(hDlg, IDC_MOTDISTy, szBuffer);
        sprintf(szBuffer, "%.2lf", thetax);
        SetDlgItemText(hDlg, IDC_THETAX, szBuffer);
        sprintf(szBuffer, "%.2lf", thetay);
        SetDlgItemText(hDlg, IDC_THETAY, szBuffer);
        sprintf(szBuffer, "%.2lf", zp);
        SetDlgItemText(hDlg, IDC_AUTDIS, szBuffer);
        sprintf(szBuffer, "%.2lf", f_GHz);
        SetDlgItemText(hDlg, IDC_Freq, szBuffer);
        sprintf(szBuffer, "%.2lf", px);
        SetDlgItemText(hDlg, IDC_Probex, szBuffer);
        sprintf(szBuffer, "%.2lf", py);
        SetDlgItemText(hDlg, IDC_Probey, szBuffer);
        
        a = Dx +px+ 2 * zp * tan(thetax*pi/180);
        sprintf(szBuffer, "%.2lf", a);
        SetDlgItemText(hDlg, IDC_SPANA, szBuffer);

        b = Dy +py+ 2 * zp * tan(thetay * pi / 180);
        sprintf(szBuffer, "%.2lf", b);
        SetDlgItemText(hDlg, IDC_SPANB, szBuffer);

         Lambda = c/(f_GHz*(1e9));

         dx = (Lambda / 2) * 1000;
         sprintf(szBuffer, "%.2lf", dx);
         SetDlgItemText(hDlg, IDC_DELTAA, szBuffer);
        
         dy = (Lambda / 2) * 1000;
         sprintf(szBuffer, "%.2lf", dy);
         SetDlgItemText(hDlg, IDC_DELTAB, szBuffer);

         // 1) find number of points and display the values
         Mx = 1 + int(0.5 + a / dx);
         sprintf(szBuffer, "%.2lf", Mx);
         SetDlgItemText(hDlg, IDC_POINTSA, szBuffer);
         // 
         My = 1 + int(0.5 + b / dy);
         sprintf(szBuffer, "%.2lf", My);
         SetDlgItemText(hDlg, IDC_POINTSB, szBuffer);

         EnableWindow(GetDlgItem(hDlg, IDC_DELTAA), false);
         EnableWindow(GetDlgItem(hDlg, IDC_DELTAB), false);
         EnableWindow(GetDlgItem(hDlg, IDC_FIXDELTA), false);
         FIXDELTA = true;
        //SendDlgItemMessage(hDlg, IDC_POSPORTCOMBO, CB_SETCURSEL, item, 0);
        ////SetDlgItemText(hDlg, IDC_INSTRUMENADD, g_Hardware.rsrcName);


        return (INT_PTR)TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_FINDSAMPLINGPAR:
            hText = GetDlgItem(hDlg, IDC_MOTDISTx);
            TextLength = GetWindowTextLengthA(hText)+1;
            GetWindowText(hText, szBuffer, TextLength);
            Value = atof(szBuffer);
            if (!isNumber(szBuffer) || Value <0)
            {
                MessageBox(hDlg, TEXT("AUT dimension along x-axis is not a valid value!"), NULL, MB_ICONERROR | MB_OK);
                return 0;
            }
            else {
                Dx = Value;
            }
            hText = GetDlgItem(hDlg, IDC_MOTDISTy);
            TextLength = GetWindowTextLengthA(hText) + 1;
            GetWindowText(hText, szBuffer, TextLength);
            Value = atof(szBuffer);
            if (!isNumber(szBuffer) || Value < 0)
            {
                MessageBox(hDlg, TEXT("AUT dimension along y-axis is not a valid value!"), NULL, MB_ICONERROR | MB_OK);
                return 0;
            }
            else {
                Dy = Value;
            }
            hText = GetDlgItem(hDlg, IDC_THETAX);
            TextLength = GetWindowTextLengthA(hText) + 1;
            GetWindowText(hText, szBuffer, TextLength);
            Value = atof(szBuffer);
            if (!isNumber(szBuffer) || Value < 0)
            {
                MessageBox(hDlg, TEXT("theta dimension along x-axis is not a valid value!"), NULL, MB_ICONERROR | MB_OK);
                return 0;
            }
            else {
                thetax = Value;
            }
            hText = GetDlgItem(hDlg, IDC_THETAY);
            TextLength = GetWindowTextLengthA(hText) + 1;
            GetWindowText(hText, szBuffer, TextLength);
            Value = atof(szBuffer);
            if (!isNumber(szBuffer) || Value < 0)
            {
                MessageBox(hDlg, TEXT("theta dimension along y-axis is not a valid value!"), NULL, MB_ICONERROR | MB_OK);
                return 0;
            }
            else {
                thetay = Value;
            }
            hText = GetDlgItem(hDlg, IDC_Probex);
            TextLength = GetWindowTextLengthA(hText) + 1;
            GetWindowText(hText, szBuffer, TextLength);
            Value = atof(szBuffer);
            if (!isNumber(szBuffer) || Value < 0)
            {
                MessageBox(hDlg, TEXT("Probe dimension along x-axis is not a valid value!"), NULL, MB_ICONERROR | MB_OK);
                return 0;
            }
            else {
                Probex = Value;
            }
            hText = GetDlgItem(hDlg, IDC_Probey);
            TextLength = GetWindowTextLengthA(hText) + 1;
            GetWindowText(hText, szBuffer, TextLength);
            Value = atof(szBuffer);
            if (!isNumber(szBuffer) || Value < 0)
            {
                MessageBox(hDlg, TEXT("Probe dimension along y-axis is not a valid value!"), NULL, MB_ICONERROR | MB_OK);
                return 0;
            }
            else {
                Probey = Value;
            }

            hText = GetDlgItem(hDlg, IDC_AUTDIS);
            TextLength = GetWindowTextLengthA(hText) + 1;
            GetWindowText(hText, szBuffer, TextLength);
            Value = atof(szBuffer);
            if (!isNumber(szBuffer) || Value < 0)
            {
                MessageBox(hDlg, TEXT("Probe distance to AUT dimension is not a valid value!"), NULL, MB_ICONERROR | MB_OK);
                return 0;
            }
            else {
                ProbetoAUTdis = Value;
            }

            a = Dx + px + 2 * zp * tan(thetax * pi / 180);
            sprintf(szBuffer, "%.2lf", a);
            SetDlgItemText(hDlg, IDC_SPANA, szBuffer);

            b = Dy + py + 2 * zp * tan(thetay * pi / 180);
            sprintf(szBuffer, "%.2lf", b);
            SetDlgItemText(hDlg, IDC_SPANB, szBuffer);

            if (FIXDELTA)
            {
                // 2) read number of points Mx and My

                hText = GetDlgItem(hDlg, IDC_POINTSA);
                TextLength = GetWindowTextLengthA(hText) + 1;
                GetWindowText(hText, szBuffer, TextLength);
                Value = atof(szBuffer);
                if (!isNumber(szBuffer) || Value < 0)
                {
                    MessageBox(hDlg, TEXT("Number of points is not a valid value!"), NULL, MB_ICONERROR | MB_OK);
                    return 0;
                }
                else {
                    Mx = Value;
                }


                hText = GetDlgItem(hDlg, IDC_POINTSB);
                TextLength = GetWindowTextLengthA(hText) + 1;
                GetWindowText(hText, szBuffer, TextLength);
                Value = atof(szBuffer);
                if (!isNumber(szBuffer) || Value < 0)
                {
                    MessageBox(hDlg, TEXT("Number of points is not a valid value!"), NULL, MB_ICONERROR | MB_OK);
                    return 0;
                }
                else {
                    My = Value;
                }


                    dx = int(a / max(1, Mx - 1));
                    sprintf(szBuffer, "%.2lf", dx);
                    SetDlgItemText(hDlg, IDC_DELTAA, szBuffer);
                // 
                    dy = int(a / max(1, My - 1));
                    sprintf(szBuffer, "%.2lf", dy);
                    SetDlgItemText(hDlg, IDC_DELTAB, szBuffer);

                    //EnableWindow(GetDlgItem(hDlg, IDC_DELTAA), false);
                    //EnableWindow(GetDlgItem(hDlg, IDC_DELTAB), false);
                    //EnableWindow(GetDlgItem(hDlg, IDC_FIXDELTA), false);
                    //FIXDELTA = true;

            }
            else
            {   // read dx and dy 

                hText = GetDlgItem(hDlg, IDC_DELTAA);
                TextLength = GetWindowTextLengthA(hText) + 1;
                GetWindowText(hText, szBuffer, TextLength);
                Value = atof(szBuffer);
                if (!isNumber(szBuffer) || Value < 0)
                {
                    MessageBox(hDlg, TEXT("Delta 'a' is not a valid value!"), NULL, MB_ICONERROR | MB_OK);
                    return 0;
                }
                else {
                    dx = Value;
                }
                hText = GetDlgItem(hDlg, IDC_DELTAB);
                TextLength = GetWindowTextLengthA(hText) + 1;
                GetWindowText(hText, szBuffer, TextLength);
                Value = atof(szBuffer);
                if (!isNumber(szBuffer) || Value < 0)
                {
                    MessageBox(hDlg, TEXT("Delta 'b' is not a valid value!"), NULL, MB_ICONERROR | MB_OK);
                    return 0;
                }
                else {
                    dy = Value;
                }


                // 5) find number of points and display thye values
                Mx = 1 + int(0.5 + a / dx);
                sprintf(szBuffer, "%.2lf", Mx);
                SetDlgItemText(hDlg, IDC_POINTSA, szBuffer);
                // 
                My = 1 + int(0.5 + b / dy);
                sprintf(szBuffer, "%.2lf", My);
                SetDlgItemText(hDlg, IDC_POINTSB, szBuffer);

                //EnableWindow(GetDlgItem(hDlg, IDC_DELTAA), false);
                //EnableWindow(GetDlgItem(hDlg, IDC_DELTAB), false);
                //EnableWindow(GetDlgItem(hDlg, IDC_FIXDELTA), false);
                //FIXDELTA = true;

            }
         return false;
        case IDC_FIXNUMBERPOINTS:

            EnableWindow(GetDlgItem(hDlg, IDC_DELTAA), true);
            EnableWindow(GetDlgItem(hDlg, IDC_DELTAB), true);
            EnableWindow(GetDlgItem(hDlg, IDC_FIXDELTA), true);
            FIXDELTA = false;

            EnableWindow(GetDlgItem(hDlg, IDC_POINTSA), false);
            EnableWindow(GetDlgItem(hDlg, IDC_POINTSB), false);
            EnableWindow(GetDlgItem(hDlg, IDC_FIXNUMBERPOINTS), false);
            return false;
        case IDC_FIXDELTA:

            EnableWindow(GetDlgItem(hDlg, IDC_DELTAA), false);
            EnableWindow(GetDlgItem(hDlg, IDC_DELTAB), false);
            EnableWindow(GetDlgItem(hDlg, IDC_FIXDELTA), false);
            FIXDELTA = true;

            EnableWindow(GetDlgItem(hDlg, IDC_POINTSA), true);
            EnableWindow(GetDlgItem(hDlg, IDC_POINTSB), true);
            EnableWindow(GetDlgItem(hDlg, IDC_FIXNUMBERPOINTS), true);

            return false;
        /*case IDC_MOTDISTx:
            hText = GetDlgItem(hDlg, IDC_MOTDISTx);
            GetWindowText(hText, szBuffer, 10);
            Value = atof(szBuffer);
            if (Value < 0)
            {
                sprintf(szBuffer, "%.2lf", Dx);
                SetDlgItemText(hDlg, IDC_MOTDISTx, szBuffer);
                MessageBox(hDlg, TEXT("Dx is not a valid value!"), NULL, MB_ICONERROR | MB_OK);
            }
            else {
                Dx = Value;
                a = Dx +px+ 2 * zp * tan(thetax * pi / 180);
                sprintf(szBuffer, "%.2lf", a);
                SetDlgItemText(hDlg, IDC_SPANA, szBuffer);

                b = Dy + py + 2 * zp * tan(thetay * pi / 180);
                sprintf(szBuffer, "%.2lf", b);
                SetDlgItemText(hDlg, IDC_SPANB, szBuffer);
               
            }
            return false;
        case IDC_MOTDISTy:
            hText = GetDlgItem(hDlg, IDC_MOTDISTy);
            GetWindowText(hText, szBuffer, 10);
            Value = atof(szBuffer);
            if (Value < 0)
            {
                sprintf(szBuffer, "%.2lf", Dy);
                SetDlgItemText(hDlg, IDC_MOTDISTy, szBuffer);
                MessageBox(hDlg, TEXT("Dy is not a valid value!"), NULL, MB_ICONERROR | MB_OK);
            }
            else {
                Dy = Value;
                a = Dx + px + 2 * zp * tan(thetax * pi / 180);
                sprintf(szBuffer, "%.2lf", a);
                SetDlgItemText(hDlg, IDC_SPANA, szBuffer);

                b = Dy + py + 2 * zp * tan(thetay * pi / 180);
                sprintf(szBuffer, "%.2lf", b);
                SetDlgItemText(hDlg, IDC_SPANB, szBuffer);
            }
            return false;
        case IDC_THETAX:
            hText = GetDlgItem(hDlg, IDC_THETAX);
            GetWindowText(hText, szBuffer, 10);
            Value = atof(szBuffer);
            if (Value < 0)
            {
                sprintf(szBuffer, "%.2lf", thetax);
                SetDlgItemText(hDlg, IDC_THETAX, szBuffer);
                MessageBox(hDlg, TEXT("thetax is not a valid value!"), NULL, MB_ICONERROR | MB_OK);
            }
            else {
                thetax = Value;
                a = Dx + px + 2 * zp * tan(thetax * pi / 180);
                sprintf(szBuffer, "%.2lf", a);
                SetDlgItemText(hDlg, IDC_SPANA, szBuffer);

                b = Dy + py + 2 * zp * tan(thetay * pi / 180);
                sprintf(szBuffer, "%.2lf", b);
                SetDlgItemText(hDlg, IDC_SPANB, szBuffer);
            }
            return false;
        case IDC_THETAY:
            hText = GetDlgItem(hDlg, IDC_THETAY);
            GetWindowText(hText, szBuffer, 10);
            Value = atof(szBuffer);
            if (Value < 0)
            {
                sprintf(szBuffer, "%.2lf", thetay);
                SetDlgItemText(hDlg, IDC_THETAY, szBuffer);
                MessageBox(hDlg, TEXT("thetay is not a valid value!"), NULL, MB_ICONERROR | MB_OK);
            }
            else {
                thetay = Value;
                a = Dx + px + 2 * zp * tan(thetax * pi / 180);
                sprintf(szBuffer, "%.2lf", a);
                SetDlgItemText(hDlg, IDC_SPANA, szBuffer);

                b = Dy + py + 2 * zp * tan(thetay * pi / 180);
                sprintf(szBuffer, "%.2lf", b);
                SetDlgItemText(hDlg, IDC_SPANB, szBuffer);
            }
            return FALSE;
        case IDC_AUTDIS:
            hText = GetDlgItem(hDlg, IDC_AUTDIS);
            GetWindowText(hText, szBuffer, 10);
            Value = atof(szBuffer);
            if (Value < 0)
            {
                sprintf(szBuffer, "%.2lf", zp);
                SetDlgItemText(hDlg, IDC_AUTDIS, szBuffer);
                MessageBox(hDlg, TEXT("thetay is not a valid value!"), NULL, MB_ICONERROR | MB_OK);
            }
            else {
                zp = Value;
                a = Dx + px + 2 * zp * tan(thetax * pi / 180);
                sprintf(szBuffer, "%.2lf", a);
                SetDlgItemText(hDlg, IDC_SPANA, szBuffer);

                b = Dy+py + 2 * zp * tan(thetay * pi / 180);
                sprintf(szBuffer, "%.2lf", b);
                SetDlgItemText(hDlg, IDC_SPANB, szBuffer);
            }
            return FALSE;*/
          
            



           /* SendDlgItemMessage(hDlg, IDC_POSPORTCOMBO, WM_GETTEXT, sizeof(szBuffer), (LPARAM)szBuffer);
            if (szBuffer[0])
                strcpy(PosComPort, szBuffer);
            else {
                MessageBox(hDlg, TEXT("Select a valid COM port for the positioner!"), NULL, MB_ICONERROR | MB_OK);
                return FALSE;
            }

            fp = fopen("hardware.txt", "w");
            fprintf(fp, "PosComPort= %s:\n", PosComPort);
            fclose(fp);*/
            //return FALSE;
        case IDOK:
            EndDialog(hDlg, TRUE);
            return FALSE;

        case IDCANCEL:
            EndDialog(hDlg, FALSE);
            return FALSE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

BOOL InitUNITSCOMBOList(HWND hwndDlg, int indCombo)
{
    HWND hwndCombo;
    DWORD dwIndex;
    int index;
    const char* ComboBoxItems[] = { "mm","cm","in",};

    hwndCombo = GetDlgItem(hwndDlg, indCombo);

    for (index = 0; index <= 3; index++)
        dwIndex = SendMessage(hwndCombo, CB_ADDSTRING, 0, (LPARAM)(LPCSTR)ComboBoxItems[index]);
    return TRUE;
}

// Function to check the string character by character
BOOL isNumber(const char* str)
{
    int i, length, ndots = 0;

    length = strlen(str);
    for (i = 0; i < length; i++)
        if (!isdigit(str[i]))
        {
            if (i == 0 & str[i] != '-')
                return false;

            if (i > 0 & str[i] == '.')
                if (ndots == 0)
                    ndots = 1;
                else
                    return false;
            else
                if (str[i] != '-')
                    return false;
        }
    return true;
}


//case WM_TIMER:
//    switch (wParam)
//    {
//    case TIMER_MILSEC1:
//        [once - per - second processing]
//        break;
//    case TIMER_MILSEC2:
//        [once - per - minute processing]
//        break;
//    }
//    return 0;