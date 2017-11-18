#pragma warning(disable: 4290)
#pragma warning(disable: 4996)
#include <TCHAR.H>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
//#define _CRT_SECURE_NO_WARNINGS
#include "Winsock2.h" // necessary for sockets, Windows.h is not needed.
#include "mswsock.h"
#include "process.h" 
#include <fstream>
#include <iomanip>      // std::setprecision
#include <atomic>
#include <thread>
#include <iostream>
#include <string>
#include "CmdReceiver.h"
#include "NetReceiver.h"
#include "NetSender.h"
#include "Source.h"

using namespace std;

int main(int argc, char *argv[])
{
	string filename;

	if (argc == 2) {
		filename = (string)argv[1];
		cout << "File path was read from command line: " << filename << endl;
	}
	else
		filename = "D:\\Kool\\Advanced programming\\log.txt";

	CmdReceiver cmdReceiver = CmdReceiver();
	NetReceiver netReceiver = NetReceiver(filename);
	NetSender netSender = NetSender();
	string receivedCommand;
	while (true) {
		
		//
		// If new command from keyboard has been received
		//
		if (cmdReceiver.gotNewCommand()) {
			receivedCommand = cmdReceiver.getCommand();
			if (receivedCommand == "connect") {
				if (netReceiver.isConnected()) {
					cout << "The client is already connected" << endl;
				}
				else {
					netReceiver.connect("127.0.0.1", 1234);
					if (!netReceiver.hasSocketError())
						netSender.startThread(netReceiver.getClientSocket());
				}
				cmdReceiver.setCommandProcessed();
			}
			else if (receivedCommand == "start")
			{
				if (netReceiver.isConnected() && !netReceiver.isStarted()) {
					netReceiver.start();
					netSender.start();
				}
				else if (netReceiver.isStarted())
					cout << "The data sending is already started. Can't start again" << endl;
				else
					cout << "Can't start as you're not connected to the emulator" << endl;
				cmdReceiver.setCommandProcessed();
			}
			else if (receivedCommand == "break") {
				if (netReceiver.isConnected() && netReceiver.isStarted()) {
					netReceiver.pause();
					netSender.pause();
				}
				else if (!netReceiver.isStarted())
					cout << "Can't use 'break' as the sending is not started" << endl;
				else
					cout << "Can't use 'break' as you're not connected to the emulator" << endl;
				cmdReceiver.setCommandProcessed();
			}
			else if (receivedCommand == "stop") {
				if (netReceiver.isConnected()) {
					netSender.stop();
					netSender.joinThread();
					netReceiver.stop();
					netReceiver.joinThread();
				}
				else
					cout << "Can't stop as you're not connected to the emulator" << endl;
				cmdReceiver.setCommandProcessed();
			}
			else if (receivedCommand == "exit") {
				if (netReceiver.isConnected()) {
					netSender.stop();
					netSender.joinThread();
					netReceiver.stop();
					netReceiver.joinThread();
				}
				cmdReceiver.joinThread();
				return 0;
			}
			else {
				cout << "Command not recognized: " << cmdReceiver.getCommand() << endl;
				cmdReceiver.setCommandProcessed();
			}

		}

		// When emulator crashes
		if (netReceiver.hasFatalError()) {
			netReceiver.joinThread();
			netSender.exit();
			netSender.joinThread();
		}

		// communicating the "coursework" password between threads
		if (netReceiver.gotSendPassword()) {
			netSender.setSendPassword();
			netReceiver.setPasswordSent();
		}

		// communication the "ready" command through threads
		if (netReceiver.gotSendReady()) {
			netSender.setSendReady();
			netReceiver.setReadySent();
		}


	}

	return 0;
}



