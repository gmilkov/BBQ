#pragma once
#include <string>
using namespace std;

namespace BBQ
{
	enum class MenuItem { Chicken, Beef, Mammoth };

	enum class ClientCommand {
		I_AM_HUNGRY_GIVE_ME_BBQ,
		NO_THANKS,
		I_TAKE_THAT,
		NONE
	};

	enum class ClientState { New, Waiting, Ready, Served, Closed };
	enum class ServerResponse { OkWait, Closed, Served };

	class Client {
	public:
		string SessionId;
		MenuItem Order;
		ClientState State;

		Client() {};

		Client(string pSessionId, MenuItem pOrder, ClientState pState)
		{
			SessionId = pSessionId;
			Order = pOrder;
			State = pState;
		};
	};

	ClientCommand parse_cmd(string command) {
		if (command == "I AM HUNGRY, GIVE ME BBQ")
		{
			return BBQ::ClientCommand::I_AM_HUNGRY_GIVE_ME_BBQ;
		}
		else if (command == "NO THANKS")
		{
			return BBQ::ClientCommand::NO_THANKS;
		}
		else if (command == "I TAKE THAT!!!")
		{
			return BBQ::ClientCommand::I_TAKE_THAT;
		}
		else
		{
			return BBQ::ClientCommand::NONE;
		}
	};

	string clientStateToStr(ClientState state) {
		switch (state)
		{
		case ClientState::Closed:	return "Closed"; break;
		case ClientState::New:		return "New"; break;
		case ClientState::Ready:	return "Ready"; break;
		case ClientState::Served:	return "Served"; break;
		case ClientState::Waiting:	return "Waiting"; break;
		}
	}

	ClientState strToClientState(string str) {
		if (str == "Closed")
		{
			return ClientState::Closed;
		}
		else if (str == "New")
		{
			return ClientState::New;
		}
		else if (str == "Ready")
		{
			return ClientState::Ready;
		}
		else if (str == "Served")
		{
			return ClientState::Served;
		}
		else if (str == "Waiting")
		{
			return ClientState::Waiting;
		}
		else {
			throw "ClientState does not exist.";
		}
	};

	string menuItemToStr(MenuItem item) {
		switch (item)
		{
		case MenuItem::Beef: return "Beef"; break;
		case MenuItem::Chicken: return "Chicken"; break;
		case MenuItem::Mammoth: return "Mammoth"; break;
		}
	};

	MenuItem strToMenuItem(string str) {
		if (str == "Beef")
		{
			return MenuItem::Beef;
		}
		else if (str == "Chicken")
		{
			return MenuItem::Chicken;
		}
		else if (str == "Mammoth")
		{
			return MenuItem::Mammoth;
		}
		else {
			throw "MenuItem does not exist.";
		}
	};
}