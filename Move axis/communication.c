#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

HANDLE hCom;

struct ComStrc{
    char * ComName;
    int BaudRate;
    int ByteSize;
    int Parity;
    int StopBits;
};

void ExitOnError(const char* message)
{
    printf("%s error", message);
    getchar();
    exit(1);
}

BOOL OpenCom(struct ComStrc ComPort)
{
    BOOL fRes, Modify=FALSE;
    DWORD dw;
    DCB dcb;
    COMMTIMEOUTS ct;
    
    hCom = CreateFile(ComPort.ComName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if(hCom==INVALID_HANDLE_VALUE)
    { 
        fRes = FALSE;                           // ExitOnError(COM_name);  // can't open COM port
    }else
    {
        if(!SetupComm(hCom, 4096, 4096))     // asigne the tx and rx buffer size
            ExitOnError("Comm setup");
        
        //JVR - The following properties can be changed for working with communications for other hardware
        if(!GetCommState(hCom, &dcb))               //ExitOnError("GetCommState");
            ExitOnError("Getting comm state");

        if (dcb.BaudRate != ComPort.BaudRate){
           dcb.BaudRate = ComPort.BaudRate;
           Modify = TRUE;
        }
        
        if (dcb.ByteSize != ComPort.ByteSize){
           dcb.ByteSize = ComPort.ByteSize;
           Modify = TRUE;
        }

        if ( dcb.Parity != ComPort.Parity ){
           dcb.Parity = ComPort.Parity;
           Modify = TRUE;
        }

        //printf("dcb.StopBits = %d\n",dcb.StopBits);
        if ( dcb.StopBits != ComPort.StopBits){
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
        if(!SetCommTimeouts(hCom, &ct))           //ExitOnError("SetCommTimeouts");
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
    if(!WriteFile(hCom, buf, len, &nSend, NULL)) exit(1);
    
    return nSend;
}

void WriteComChar(char b)
{
    WriteCom(&b, 1);
}

int ReadCom(char *buf, int len)
{
    DWORD nRec;
    if(!ReadFile(hCom, buf, len, &nRec, NULL)) exit(1);
    return nRec;
}

