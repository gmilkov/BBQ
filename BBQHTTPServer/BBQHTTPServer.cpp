// BBQ.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/Exception.h"
#include "Poco/ThreadPool.h"
#include "Poco/Util/ServerApplication.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/HelpFormatter.h"
#include "Poco/URI.h"
#include "Poco/UUID.h"
#include "Poco/UUIDGenerator.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <combaseapi.h>
#include "../BBQProtocol/BBQ.h"

using Poco::Net::ServerSocket;
using Poco::Net::HTTPRequestHandler;
using Poco::Net::HTTPRequestHandlerFactory;
using Poco::Net::HTTPServer;
using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPServerResponse;
using Poco::Net::HTTPServerParams;
using Poco::Util::ServerApplication;
using Poco::Util::Application;
using Poco::Util::Option;
using Poco::Util::OptionSet;
using Poco::Util::HelpFormatter;
using Poco::ThreadPool;
using Poco::URI;
using Poco::UUID;

using namespace Poco;
using namespace Poco::Net;
using namespace std;
using namespace std::chrono;
using namespace std::this_thread;


class BBQRequestHandler : public HTTPRequestHandler {

public:
	void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
	{
		logRequest(request);

		BBQ::Client client = initBBQClient(request);

		processRequest(request, client);

		sendResponse(response, client);
	}

	void processRequest(HTTPServerRequest& request, BBQ::Client& client)
	{
		string command = getRequestCommand(request);

		switch (BBQ::parse_cmd(command)) 
		{
			case BBQ::ClientCommand::I_AM_HUNGRY_GIVE_ME_BBQ:
				if (client.State == BBQ::ClientState::New)
				{
					client.State = BBQ::ClientState::Waiting;
				}
				else if (client.State == BBQ::ClientState::Waiting)
				{
					waitForOrder();
					client.State = BBQ::ClientState::Ready;
				}
				break;
			case BBQ::ClientCommand::I_TAKE_THAT:
				client.State = BBQ::ClientState::Served;
				break;
			case BBQ::ClientCommand::NO_THANKS:
				client.State = BBQ::ClientState::Closed;
				break;
		}
	}

	void sendResponse(HTTPServerResponse& response, BBQ::Client client)
	{
		response.addCookie(HTTPCookie("session", client.SessionId));
		response.addCookie(HTTPCookie("state", clientStateToStr(client.State)));
		response.addCookie(HTTPCookie("order", menuItemToStr(client.Order)));

		response.setChunkedTransferEncoding(false);
		response.setContentType("text/html");

		std::ostream& ostr = response.send();
		string command;

		switch (client.State)
		{
			case BBQ::ClientState::Waiting:
				command = "OK, WAIT";
				break;
			case BBQ::ClientState::Ready:
				command = push_order_ready(client.Order);
				break;
			case BBQ::ClientState::Served:
				command = "SERVED BYE";
				break;
			case BBQ::ClientState::Closed:
				command = "CLOSED BUY";
		}

		ostr << command;
	}

	void waitForOrder()
	{
		sleep_for(seconds(15));
	}

	string push_order_ready(BBQ::MenuItem order)
	{
		switch (order)
		{
			case BBQ::MenuItem::Chicken:	return "CHICKEN READY"; break;
			case BBQ::MenuItem::Beef:		return "BEEF READY";	break;
			case BBQ::MenuItem::Mammoth:	return "LAST MONTH MAMMOTH READY"; break;
		}
	}


	BBQ::MenuItem getRandomMeal()
	{
		return (BBQ::MenuItem)(rand() % 3);
	}

	void logRequest(HTTPServerRequest& request) {
		Application& app = Application::instance();

		string command = getRequestCommand(request);

		app.logger().information("Request from " + request.clientAddress().toString());
		app.logger().information("Command: " + command);
	}

	string getRequestCommand(Poco::Net::HTTPServerRequest& request)
	{
		size_t size = (size_t)request.getContentLength();
		std::istream& stream = request.stream();
		std::string encoded_content;
		std::string content;
		encoded_content.resize(size);
		stream.read(&encoded_content[0], size);
		Poco::URI::decode(encoded_content, content);
		return content;
	}

	BBQ::Client initBBQClient(HTTPServerRequest& request)
	{
		NameValueCollection cookies;
		request.getCookies(cookies);

		string sessionId;
		BBQ::ClientState clientState;
		BBQ::MenuItem clientOrder;

		if (cookies.has("session")) {
			sessionId = cookies.get("session");
			clientState = BBQ::strToClientState(cookies.get("state"));
			clientOrder = BBQ::strToMenuItem(cookies.get("order"));
		}
		else {
			UUIDGenerator uuiGenerator;
			UUID guid = uuiGenerator.create();
			sessionId = guid.toString();
			clientState = BBQ::ClientState::New;
			clientOrder = getRandomMeal();
		}

		return BBQ::Client(sessionId, clientOrder, clientState);
	}
};


class BBQRequestHandlerFactory : public HTTPRequestHandlerFactory
{
public:
	HTTPRequestHandler* createRequestHandler(const HTTPServerRequest& request)
	{
		if (request.getURI() == "/")
		{
			return new BBQRequestHandler();
		}
		else
			return 0;
	}
};


class HTTPBBQServer : public Poco::Util::ServerApplication
{
public:
	HTTPBBQServer() : _helpRequested(false)
	{
	}

	~HTTPBBQServer()
	{
	}

protected:
	void initialize(Application& self)
	{
		loadConfiguration(); // load default configuration files, if present
		ServerApplication::initialize(self);
	}

	void uninitialize()
	{
		ServerApplication::uninitialize();
	}

	void defineOptions(OptionSet& options)
	{
		ServerApplication::defineOptions(options);

		options.addOption(
			Option("help", "h", "display help information on command line arguments")
			.required(false)
			.repeatable(false));
	}

	void handleOption(const std::string& name, const std::string& value)
	{
		ServerApplication::handleOption(name, value);

		if (name == "help")
			_helpRequested = true;
	}

	void displayHelp()
	{
		HelpFormatter helpFormatter(options());
		helpFormatter.setCommand(commandName());
		helpFormatter.setUsage("OPTIONS");
		helpFormatter.setHeader("A web server that serves BBQ meals.");
		helpFormatter.format(std::cout);
	}

	int main(const std::vector<std::string>& args)
	{
		if (_helpRequested)
		{
			displayHelp();
		}
		else
		{
			unsigned short port = 80;
			int maxQueued = 1;
			int maxThreads = 16;
			ThreadPool::defaultPool().addCapacity(maxThreads);

			HTTPServerParams* pParams = new HTTPServerParams;
			pParams->setMaxQueued(maxQueued);
			pParams->setMaxThreads(maxThreads);

			// set-up a server socket
			ServerSocket svs(port);
			// set-up a HTTPServer instance
			HTTPServer srv(new BBQRequestHandlerFactory(), svs, pParams);
			// start the HTTPServer
			srv.start();

			cout << "BBQ server started on port: " << srv.port() << endl;

			// wait for CTRL-C or kill
			waitForTerminationRequest();
			// Stop the HTTPServer
			srv.stop();
		}
		return Application::EXIT_OK;
	}

private:
	bool _helpRequested;
};

int main(int argc, char** argv)
{
	HTTPBBQServer app;
	app.run(argc, argv);
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
