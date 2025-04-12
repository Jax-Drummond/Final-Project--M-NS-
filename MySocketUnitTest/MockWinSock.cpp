#define WIN32_LEAN_AND_MEAN
#include "pch.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>

int mock_connect_result = 0;
int mock_accept_result = 2;

extern "C" {
    SOCKET WSAAPI socket(int af, int type, int protocol) { return 1; }
    int WSAAPI connect(SOCKET s, const struct sockaddr* name, int namelen) { return 0; }
    int WSAAPI bind(SOCKET s, const struct sockaddr* name, int namelen) { return 0; }
    int WSAAPI listen(SOCKET s, int backlog) { return 0; }
    SOCKET WSAAPI accept(SOCKET s, struct sockaddr* addr, int* addrlen) { return 2; }
    int WSAAPI send(SOCKET s, const char* buf, int len, int flags) { return 5; }
    int WSAAPI sendto(SOCKET s, const char* buf, int len, int flags, const struct sockaddr* to, int tolen) { return 5; }
    int WSAAPI recv(SOCKET s, char* buf, int len, int flags) { strncpy(buf, "hello", len); return 5; }
    int WSAAPI recvfrom(SOCKET s, char* buf, int len, int flags, struct sockaddr* from, int* fromlen) { strncpy(buf, "world", len); return 5; }
    int WSAAPI closesocket(SOCKET s) { return 0; }
    int WSAAPI WSAStartup(WORD wVersionRequested, LPWSADATA lpWSAData) { return 0; }
    u_short WSAAPI htons(u_short hostshort) {
        return hostshort; // mock: return value as-is
    }

    int WSAAPI inet_pton(int af, const char* src, void* dst) {
        // Mock conversion: just fill the buffer with dummy IP address (e.g., 127.0.0.1)
        if (af == AF_INET && dst) {
            ((sockaddr_in*)dst)->sin_addr.s_addr = 0x0100007F; // 127.0.0.1 in hex
            return 1; // success
        }
        return 0;
    }
}

