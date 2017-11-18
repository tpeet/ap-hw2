#pragma warning(disable: 4290)
#pragma warning(disable: 4996)
#include "NetReceiver.h"

NetReceiver::NetReceiver(string filename)
{
	_file.open(filename, ios::out);
}

NetReceiver::~NetReceiver()
{
	_file.close();
}

void NetReceiver::connect(char* ip, int port) {
	try
	{


		if (!(_hStopCommandGot = CreateEvent(NULL, TRUE, FALSE, NULL)))
		{
			cout << "CreateEvent() failed, error " << GetLastError() << endl;
			return;
		}
		WSADATA WsaData;
		DWORD Error;
		if (Error = WSAStartup(MAKEWORD(2, 0), &WsaData)) // Initialize Windows socket support
		{
			cout << "WSAStartup() failed, error " << Error << endl;
			_SocketError = true;
		}

		sockaddr_in clientSocketInfo;
		clientSocketInfo.sin_family = AF_INET;
		clientSocketInfo.sin_addr.s_addr = inet_addr(ip);
		clientSocketInfo.sin_port = htons(port);  // port number is selected just for example
		if ((_hClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
		{
			cout << "socket() failed, error " << WSAGetLastError() << endl;
			_isStarted = false;
			_isConnected = false;
			_SocketError = true;
		}
		else if (WIN32::connect(_hClientSocket, (SOCKADDR*)&clientSocketInfo, sizeof(clientSocketInfo)) == SOCKET_ERROR)
		{
			cout << "Unable to connect to server, error " << WSAGetLastError() << endl;
			_isStarted = false;
			_isConnected = false;
			_SocketError = true;
		}
		else {
			_SocketError = false;
		}
		if (!_SocketError)
		{
			_thread = thread(&NetReceiver::_receiveThread, this);
			_isStarted = false;
			_isConnected = false;
		}
	}
	catch (exception& e)
	{
		cout << "Something went wrong with receiver thread. " << e.what() << endl;
	}
}

void NetReceiver::_receiveThread()
{
	_fatalError = false;
	cout << "########## Receiver thread started #############" << endl;
	//
	// Preparations
	//
	WSABUF DataBuf;  // Buffer for received data is a structure
	char ArrayInBuf[2048];
	DataBuf.buf = &ArrayInBuf[0];
	DataBuf.len = 2048;
	DWORD nReceivedBytes = 0, ReceiveFlags = 0;
	NetEvents[0] = _hStopCommandGot;
	WSAOVERLAPPED Overlapped;
	memset(&Overlapped, 0, sizeof Overlapped);
	Overlapped.hEvent = NetEvents[1] = WSACreateEvent(); // manual and nonsignaled
	DWORD Result, Error;
	//
	// Receiving loop
	//
	while (TRUE)
	{
		Result = WSARecv(_hClientSocket,
			&DataBuf,
			1,  // no comments here
			&nReceivedBytes,
			&ReceiveFlags, // no comments here
			&Overlapped,
			NULL);  // no comments here
		if (Result == SOCKET_ERROR)
		{  // Returned with socket error, let us examine why
			if ((Error = WSAGetLastError()) != WSA_IO_PENDING)
			{  // Unable to continue, for example because the server has closed the connection
				cout << "WSARecv() failed, error" << Error << endl;
				out();
				return;
			}
			DWORD WaitResult = WSAWaitForMultipleEvents(2, NetEvents, FALSE, WSA_INFINITE, FALSE); // wait for data
			switch (WaitResult) // analyse why the waiting ended
			{
			case WAIT_OBJECT_0:
				// Waiting stopped because hStopCommandGot has become signaled, i.e. the user has decided to exit
				out();
				return;
			case WAIT_OBJECT_0 + 1:
				// Waiting stopped because Overlapped.hEvent is now signaled, i.e. the receiving operation has ended. 
				// Now we have to see how many bytes we have got.
				WSAResetEvent(NetEvents[1]); // to be ready for the next data package
				if (WSAGetOverlappedResult(_hClientSocket, &Overlapped, &nReceivedBytes, FALSE, &ReceiveFlags))
				{

					
					if (nReceivedBytes > 0) {
						cout << nReceivedBytes << " bytes received" << endl;
						wcout << (wchar_t)(DataBuf.buf + sizeof(int)) << endl;
					}
						
					char ArrayOutBuf[2048];
					if (_bufWcsCompare(DataBuf.buf, L"Identify") == 0) {
						_sendPassword = true;
					}
					else if (_bufWcsCompare(DataBuf.buf, L"Accepted") == 0) {
						_isConnected = true;
					}
					else if (_isConnected && _isStarted) {
						if (!_file.is_open())
						{
							cout << "Error opening log file!" << endl;
							
						}
						int packageLength;
						int numChannels;
						int parsedBytes = 0;
						memcpy(&packageLength, DataBuf.buf + parsedBytes, sizeof(int));
						parsedBytes += sizeof(int);
						memcpy(&numChannels, DataBuf.buf + parsedBytes, sizeof(int));
						parsedBytes += sizeof(int);

						// timestamp
						auto t = time(nullptr);
						auto tm = *localtime(&t);
						_file << "Measurement results at " << put_time(&tm, "%Y-%m-%d %H:%M:%S") << endl;

						for (int i = 0; i < numChannels; i++) {
							int measurementsPoints;
							memcpy(&measurementsPoints, DataBuf.buf + parsedBytes, sizeof(int));
							parsedBytes += sizeof(int);
							char channelName[128];
							_parseStringFromBuf(channelName, DataBuf.buf, &parsedBytes);

							cout << channelName << ":" << endl;
							_file << channelName << ":" << endl;
							for (int j = 0; j < measurementsPoints; j++) {
								char measurementName[128];
								_parseStringFromBuf(measurementName, DataBuf.buf, &parsedBytes);
								cout << measurementName << ": ";
								_file << measurementName << ": ";
								if (strcmp(measurementName, "Level") == 0) {
									int measurement;
									memcpy(&measurement, DataBuf.buf + parsedBytes, 4);
									parsedBytes += 4;
									cout << measurement << " %" << endl;
									_file << measurement << " %" << endl;
								}
								else {
									double measurement;
									memcpy(&measurement, DataBuf.buf + parsedBytes, 8);
									parsedBytes += 8;

									if (strcmp(measurementName, "Temperature") == 0) {
										cout << std::fixed << setprecision(1) << measurement << " \370C" << endl;
										_file << std::fixed << setprecision(1) << measurement << " °C" << endl;
									}
									else if (strcmp(measurementName, "Pressure") == 0) {
										cout << std::fixed << std::setprecision(1) << measurement << " atm" << endl;
										_file << std::fixed << std::setprecision(1) << measurement << " atm" << endl;
									}
									else {
										cout << std::fixed << std::setprecision(3) << measurement << " m³/s" << endl;
										_file << std::fixed << std::setprecision(3) << measurement << " m³/s" << endl;
									}
								}
							}
						}
						_file << endl;
						_sendReady = true;
					}

					break;
				}
				else
				{	// Fatal problems
					if (GetLastError() == 10054) {
						cout << "WSAGetOverlappedResult() failed, probably due to the server failure, error " << GetLastError << endl;
					}
						
					else
						cout << "WSAGetOverlappedResult() failed, error " << GetLastError() << endl;
					out();
					return;

				}
			default: // Fatal problems
				cout << "WSAWaitForMultipleEvents() failed, error " << WSAGetLastError() << endl;
				out();
				return;
			}
		}
		else
		{  // Returned immediately without socket error
			if (nReceivedBytes <= 0)
			{  // When the receiving function has read nothing and returned immediately, the connection is off  
				cout << "Server has closed the connection" << endl;
				_isConnected = false;
				if (_hClientSocket != INVALID_SOCKET)
				{
					if (shutdown(_hClientSocket, SD_RECEIVE) == SOCKET_ERROR)
					{
						if ((Error = WSAGetLastError()) != WSAENOTCONN) // WSAENOTCONN means that the connection was not established,
																		// so the shut down was senseless
							cout << "shutdown() failed, error " << WSAGetLastError() << endl;
					}
					closesocket(_hClientSocket);
				}
				out();
				return;
			}
			else
			{
				cout <<  nReceivedBytes << " bytes received" << endl;
				char ArrayOutBuf[2048];
				if (_bufWcsCompare(DataBuf.buf, L"Identify") == 0) {
					_sendPassword = true;
				}
				else if (_bufWcsCompare(DataBuf.buf, L"Accepted") == 0) {
					_isConnected = true;
				}
				else if (_isConnected && _isStarted) {
					if (!_file.is_open())
					{
						cout << "Error opening log file!" << endl;

					}
					int packageLength;
					int numChannels;
					int parsedBytes = 0;
					memcpy(&packageLength, DataBuf.buf + parsedBytes, sizeof(int));
					parsedBytes += sizeof(int);
					memcpy(&numChannels, DataBuf.buf + parsedBytes, sizeof(int));
					parsedBytes += sizeof(int);

					// timestamp
					auto t = time(nullptr);
					auto tm = *localtime(&t);
					_file << "Measurement results at " << put_time(&tm, "%Y-%m-%d %H:%M:%S") << endl;

					for (int i = 0; i < numChannels; i++) {
						int measurementsPoints;
						memcpy(&measurementsPoints, DataBuf.buf + parsedBytes, sizeof(int));
						parsedBytes += sizeof(int);
						char channelName[128];
						_parseStringFromBuf(channelName, DataBuf.buf, &parsedBytes);

						cout << channelName << ":" << endl;
						_file << channelName << ":" << endl;
						for (int j = 0; j < measurementsPoints; j++) {
							char measurementName[128];
							_parseStringFromBuf(measurementName, DataBuf.buf, &parsedBytes);
							cout << measurementName << ": ";
							_file << measurementName << ": ";
							if (strcmp(measurementName, "Level") == 0) {
								int measurement;
								memcpy(&measurement, DataBuf.buf + parsedBytes, 4);
								parsedBytes += 4;
								cout << measurement << " %" << endl;
								_file << measurement << " %" << endl;
							}
							else {
								double measurement;
								memcpy(&measurement, DataBuf.buf + parsedBytes, 8);
								parsedBytes += 8;

								if (strcmp(measurementName, "Temperature") == 0) {
									cout << std::fixed << setprecision(1) << measurement << " \370C" << endl;
									_file << std::fixed << setprecision(1) << measurement << " °C" << endl;
								}
								else if (strcmp(measurementName, "Pressure") == 0) {
									cout << std::fixed << std::setprecision(1) << measurement << " atm" << endl;
									_file << std::fixed << std::setprecision(1) << measurement << " atm" << endl;
								}
								else {
									cout << std::fixed << std::setprecision(3) << measurement << " m³/s" << endl;
									_file << std::fixed << std::setprecision(3) << measurement << " m³/s" << endl;
								}
							}
						}
					}
					_file << endl;
					_sendReady = true;
				}

			}
		}
	}
}

void NetReceiver::start() {
	_isStarted = true;
}

void NetReceiver::pause() {
	_isStarted = false;
}

void NetReceiver::stop() {
	SetEvent(_hStopCommandGot);
}

SOCKET NetReceiver::getClientSocket() {
	return _hClientSocket;
}

void NetReceiver::_parseStringFromBuf(char *dst, char *buffer, int *parsedBytes)
{
	int strLength = 0;
	while (*(buffer + *parsedBytes + strLength) != '\0') {
		strLength++;
	}
	memcpy(dst, buffer + *parsedBytes, strLength + 1);
	*parsedBytes += strLength + 1;
}

int NetReceiver::_bufWcsCompare(char *buffer, wchar_t *str)
{
	char temp_buff[2048];
	memcpy(temp_buff, str, (wcslen(str) + 1) * sizeof(wchar_t));
	return strcmp(buffer + sizeof(int), temp_buff);
}

bool NetReceiver::gotSendPassword() {
	return _sendPassword;
}

bool NetReceiver::gotSendReady() {
	return _sendReady;
}

void NetReceiver::setPasswordSent() {
	_sendPassword = false;
}

void NetReceiver::setReadySent() {
	_sendReady = false;
}

void NetReceiver::out() {
	_fatalError = true;
	_isConnected = false;
	_isStarted = false;
	_sendReady = false;
	_sendPassword = false;
	
	//WSACloseEvent(NetEvents[1]);
	//if (_thread.joinable())
	//	_thread.join();


	//if (_hClientSocket != INVALID_SOCKET)
	//{
	//	if (shutdown(_hClientSocket, SD_RECEIVE) == SOCKET_ERROR)
	//	{
	//		DWORD Error;
	//		if ((Error = WSAGetLastError()) != WSAENOTCONN) // WSAENOTCONN means that the connection was not established,
	//														// so the shut down was senseless
	//		cout << "shutdown() failed" << WSAGetLastError() << endl;
	//	}
	//	closesocket(_hClientSocket);
	//}
	//_file.close();
	//WSACleanup(); // clean Windows sockets support
	//CloseHandle(_hStopCommandGot);	
}

void NetReceiver::joinThread() {
	_thread.join();
	cout << "########## Receiver thread joined #############" << endl;
	_fatalError = false;
}

bool NetReceiver::isStarted() {
	return _isStarted;
}

bool NetReceiver::isConnected() {
	return _isConnected;
}

bool NetReceiver::hasFatalError() {
	return _fatalError;
}

bool NetReceiver::hasSocketError() {
	return _SocketError;
}