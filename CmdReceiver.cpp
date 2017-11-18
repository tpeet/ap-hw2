#include "CmdReceiver.h"

using namespace std;

CmdReceiver::CmdReceiver() {
	try
	{
		_thread = thread(&CmdReceiver::_readFromKeyboardThread, this);
		_isCommandProcessed = true;
	}
	catch (exception& e)
	{
		cout << "Something went wrong with receiver thread. " << e.what() << endl;
	}
}




CmdReceiver::~CmdReceiver()
{
}

void CmdReceiver::_readFromKeyboardThread() {
	cout << "########## Keyboard thread started #############" << endl;
	while (true) {
		if (_isCommandProcessed) {
			cout << "Insert command" << endl;
			if (getline(cin, _receivedCommand)) {

				_isCommandProcessed = false;
				if (_receivedCommand == "exit")
					return;
			}
			else {
				cout << "error" << endl;
			}
		}
	}
}

string CmdReceiver::getCommand() {
	return _receivedCommand;
}
bool CmdReceiver::gotNewCommand() {
	return !_isCommandProcessed;
}
void CmdReceiver::setCommandProcessed() {
	_isCommandProcessed = true;
}

void CmdReceiver::joinThread() {
	if (_thread.joinable())
		_thread.join();
	cout << "########## Keyboard thread joined #############" << endl;
}