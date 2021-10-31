#pragma once
#include <string>
using namespace std;

namespace BBQ
{
	const string CLIENT_CMD_HUNGRY = "I AM HUNGRY, GIVE ME BBQ";
	const string CLIENT_CMD_NO_THANKS = "NO THANKS";
	const string CLIENT_CMD_TAKE = "I TAKE THAT!!!";

	const string SERVER_RESP_OK_WAIT = "OK, WAIT";
	const string SERVER_RESP_CLOSED_BYE = "CLOSED BYE";
	const string SERVER_RESP_SERVED_BYE = "SERVED BYE";
	const string SERVER_RESP_CHICKEN_READY = "CHICKEN READY";
	const string SERVER_RESP_BEEF_READY = "BEEF READY";
	const string SERVER_RESP_MAMMOTH_READY = "LAST MONTH MAMMOTH READY";

	enum class MenuItem { Chicken, Beef, Mammoth };

	enum class ClientCommand {
		I_AM_HUNGRY_GIVE_ME_BBQ,
		NO_THANKS,
		I_TAKE_THAT
	};

	enum class ClientState { New, Waiting, Ready, Served, Closed };
	enum class ServerResponse { OkWait, Closed, Served, ChickenReady, BeefReady, MammothReady };

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



	ClientCommand strToClientCmd(string command) {
		if (command == CLIENT_CMD_HUNGRY)
		{
			return BBQ::ClientCommand::I_AM_HUNGRY_GIVE_ME_BBQ;
		}
		else if (command == CLIENT_CMD_NO_THANKS)
		{
			return BBQ::ClientCommand::NO_THANKS;
		}
		else if (command == CLIENT_CMD_TAKE)
		{
			return BBQ::ClientCommand::I_TAKE_THAT;
		}
		else
		{
			throw "ClinetCommand does not exist.";
		}
	};

	string clientCmdToStr(ClientCommand command) {
		switch (command)
		{
			case ClientCommand::I_AM_HUNGRY_GIVE_ME_BBQ: 
				return CLIENT_CMD_HUNGRY;
				break;
			case ClientCommand::I_TAKE_THAT:			 
				return CLIENT_CMD_TAKE;
				break;
			case ClientCommand::NO_THANKS:				 
				return CLIENT_CMD_NO_THANKS;
				break;
		}
	}

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

	ServerResponse strToSrvResp(string str)
	{
		if (str == SERVER_RESP_CLOSED_BYE)
		{
			return ServerResponse::Closed;
		}
		else if (str == SERVER_RESP_OK_WAIT)
		{
			return ServerResponse::OkWait;
		}
		else if (str == SERVER_RESP_SERVED_BYE)
		{
			return ServerResponse::Served;
		}
		else if (str == SERVER_RESP_BEEF_READY)
		{
			return ServerResponse::BeefReady;
		}
		else if (str == SERVER_RESP_CHICKEN_READY)
		{
			return ServerResponse::ChickenReady;
		}
		else if (str == SERVER_RESP_MAMMOTH_READY)
		{
			return ServerResponse::MammothReady;
		}
		else {
			throw "ServerResponse does not exist.";
		}
	}

	string srvRespToStr(ServerResponse response)
	{
		switch (response)
		{
			case ServerResponse::Closed: 
				return SERVER_RESP_CLOSED_BYE;
				break;
			case ServerResponse::OkWait:
				return SERVER_RESP_OK_WAIT;
				break;
			case ServerResponse::Served:
				return SERVER_RESP_SERVED_BYE;
				break;
			case ServerResponse::BeefReady:
				return SERVER_RESP_BEEF_READY;
				break;
			case ServerResponse::ChickenReady:
				return SERVER_RESP_CHICKEN_READY;
				break;
			case ServerResponse::MammothReady:
				return SERVER_RESP_MAMMOTH_READY;
				break;
		}
	}

	string menuItemToStr(MenuItem item) {
		switch (item)
		{
			case MenuItem::Beef:	return "Beef";	  break;
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

	string pushCmdFromOrder(MenuItem order)
	{
		switch (order)
		{
		case BBQ::MenuItem::Chicken: return srvRespToStr(ServerResponse::ChickenReady); break;
			case BBQ::MenuItem::Beef: return srvRespToStr(ServerResponse::BeefReady);	break;
			case BBQ::MenuItem::Mammoth: return srvRespToStr(ServerResponse::MammothReady); break;
		}
	}

}