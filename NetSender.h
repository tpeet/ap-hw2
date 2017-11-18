#pragma once
#include "Winsock2.h" // necessary for sockets, Windows.h is not needed.
#include "mswsock.h"
#include "process.h" 
#include <iostream>
#include <string>
#include <thread>
using namespace std;
class NetSender
{
	HANDLE hSendStart;
	HANDLE hSendBreak;
	HANDLE hSendStop;
	HANDLE hSendReady;
	HANDLE hSendPassword;
	HANDLE hExit;
	SOCKET hClientSocket;
	thread _thread;

	void SendNet();
	void sendMsg(wchar_t * str);
	void out();
public:
	NetSender();
	~NetSender();
	NetSender(const NetSender& pOther, std::memory_order order = std::memory_order_seq_cst) {} // needed for threading

	void setSendStart();
	void setSendReady();
	void setSendPassword();

	void startThread(SOCKET clientSocket);
	void start();
	void pause();
	void stop();
	void joinThread();
	void exit();
};

