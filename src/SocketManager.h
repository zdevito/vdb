#ifndef _SOCKET_MANAGER_H
#define _SOCKET_MANAGER_H
typedef bool (*SocketManagerCallback_t)(const char * line, void * data);
void SocketManager_init(SocketManagerCallback_t cb, void *d);
void SocketManager_reenableCallbacks();
#endif