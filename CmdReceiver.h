#pragma once
#include <atomic>
#include <thread>
#include <iostream>
#include <string>
using namespace std;
class CmdReceiver
{
	string _receivedCommand;
	thread _thread;
	atomic<bool> _isCommandProcessed;
	void _readFromKeyboardThread();
public:
	CmdReceiver();
	~CmdReceiver();
	CmdReceiver(const CmdReceiver& pOther, std::memory_order order = std::memory_order_seq_cst)
		: _isCommandProcessed(pOther._isCommandProcessed.load(order)) {}

	void setCommandProcessed();
	bool gotNewCommand();
	string getCommand();


	void joinThread();
};

