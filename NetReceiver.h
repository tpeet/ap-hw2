#pragma once
#include "Winsock2.h" // necessary for sockets, Windows.h is not needed.
#include "mswsock.h"
#include "process.h" 
#include <fstream>
#include <iostream>
#include <iomanip>		//time 
#include <thread>
#include <atomic>
#include <sstream>
using namespace std;
class NetReceiver
{
	
	SOCKET _hClientSocket;
	HANDLE _hStopCommandGot;

	atomic<bool> _sendReady;
	atomic<bool> _sendPassword;

	ofstream _file;
	atomic<bool> _isStarted;
	atomic<bool> _isConnected;
	atomic<bool> _SocketError;
	atomic<bool> _fatalError;
	HANDLE NetEvents[2];

	thread _thread;

	void _receiveThread();
	void _parseStringFromBuf(char *dst, char *buffer, int *parsedBytes);
	int _bufWcsCompare(char *buffer, wchar_t *str);
	void out();
public:
	NetReceiver(string filename);
	~NetReceiver();
	NetReceiver(const NetReceiver& pOther, std::memory_order order = std::memory_order_seq_cst) {} // needed for threading

	void connect(char* ip, int port); // connect to TCP server and start Receiver thread
	void start();
	void pause();
	void stop(); // signal Stop handle to stop connection to the server

	bool isStarted();
	bool isConnected();
	bool hasFatalError();
	bool hasSocketError();

	SOCKET getClientSocket();

	// send commands from Receiver thread to Sender thread
	bool gotSendReady();
	bool gotSendPassword();
	void setReadySent();
	void setPasswordSent(); 
	void joinThread();
};

