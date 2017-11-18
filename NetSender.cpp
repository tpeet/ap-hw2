#pragma warning(disable: 4290)
#pragma warning(disable: 4996)
#include "NetSender.h"
using namespace std;


NetSender::NetSender()
{
	if (!(hSendBreak= CreateEvent(NULL, TRUE, FALSE, NULL)) || 
		!(hSendReady = CreateEvent(NULL, TRUE, FALSE, NULL)) ||
		!(hSendStart = CreateEvent(NULL, TRUE, FALSE, NULL)) ||
		!(hSendStop = CreateEvent(NULL, TRUE, FALSE, NULL)) ||
		!(hSendPassword = CreateEvent(NULL, TRUE, FALSE, NULL)) ||
		!(hExit = CreateEvent(NULL, TRUE, FALSE, NULL))
		) 
	{
		cout << "CreateEvent() failed, error " << GetLastError() << endl;
		return;
	}
}


NetSender::~NetSender()
{
}


void NetSender::SendNet()
{
	cout << "########## Send thread started #############" << endl;
	DWORD WaitResult;
	HANDLE SendEvents[6];
	SendEvents[0] = hSendStart;
	SendEvents[1] = hSendBreak;
	SendEvents[2] = hSendStop;
	SendEvents[3] = hSendReady;
	SendEvents[4] = hSendPassword;
	SendEvents[5] = hExit;
	while (TRUE) {
		WaitResult = WaitForMultipleObjects(6, SendEvents,
			FALSE, // wait until one of the events becomes signaled
			INFINITE);

		if (WaitResult == WAIT_OBJECT_0) { // send start
			cout << "---------------send start-----------" << endl;
			sendMsg(L"Start");
			ResetEvent(hSendStart);
		}
		else if (WaitResult == WAIT_OBJECT_0 + 1) { //send break
			cout << "---------------send break-----------" << endl;
			sendMsg(L"Break");
			ResetEvent(hSendBreak);
		}
		else if (WaitResult == WAIT_OBJECT_0 + 2) { // send stop
			cout << "---------------send stop-----------" << endl;
			sendMsg(L"Stop");
			ResetEvent(hSendStop);
			return;
		}
		else if (WaitResult == WAIT_OBJECT_0 + 3) { // send ready
			cout << "---------------send ready-----------" << endl;
			sendMsg(L"Ready");
			ResetEvent(hSendReady);
		}
		else if (WaitResult == WAIT_OBJECT_0 + 4) { // send ready
			cout << "---------------send 'coursework'-----------" << endl;
			sendMsg(L"coursework");
			ResetEvent(hSendPassword);
		}
		else if (WaitResult == WAIT_OBJECT_0 + 5) { // send ready
			ResetEvent(hExit);
			return;
		}
	}

}

void NetSender::sendMsg(wchar_t * str)
{
	char temp_buff[2048];
	int messageLength = sizeof(int) + (wcslen(str) + 1) * sizeof(wchar_t);
	memcpy(temp_buff, &messageLength, sizeof(int));
	memcpy(temp_buff + sizeof(int), str, (wcslen(str) + 1) * sizeof(wchar_t));
	send(hClientSocket, temp_buff, messageLength, 0);
}

void NetSender::startThread(SOCKET clientSocket) {
	try
	{
		_thread = thread(&NetSender::SendNet, this);
		hClientSocket = clientSocket;
	}
	catch (exception& e)
	{
		cout << "Something went wrong with sender thread. " << e.what() << endl;
	}
}

void NetSender::stop() {
	SetEvent(hSendStop);
}

void NetSender::setSendStart() {
	SetEvent(hSendStart);
}
void NetSender::setSendReady() {
	SetEvent(hSendReady);
}

void NetSender::setSendPassword() {
	SetEvent(hSendPassword);
}

void NetSender::pause() {
	SetEvent(hSendBreak);
}

void NetSender::start() {
	SetEvent(hSendStart);
}

void NetSender::out() {
	/*if (_thread.joinable())
		_thread.join();*/
}

void NetSender::exit() {
	SetEvent(hExit);
	cout << "exit event set" << endl;
}

void NetSender::joinThread() {
	_thread.join();
	cout << "########## Send thread joined #############" << endl;
}
