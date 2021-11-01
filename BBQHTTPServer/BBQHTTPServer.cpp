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
#include "iostream"
#include "chrono"
#include "thread"
#include "combaseapi.h"
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

using namespace std;
using namespace std::chrono;
using namespace std::this_thread;
using namespace Poco;
using namespace Poco::Net;


class BBQRequestHandler : public HTTPRequestHandler {

public:

	BBQRequestHandler(int pWaitTimeForMeal)
	{
		waitTimeForMeal = pWaitTimeForMeal;
	};

private:
	int waitTimeForMeal;

	void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
	{
		BBQ::Client client = initBBQClient(request);

		string command = getRequestCommand(request);

		logRequest(request, command);

		processRequest(request, client, command);

		sendResponse(response, client);
	}

	void processRequest(HTTPServerRequest& request, BBQ::Client& client, string command)
	{
		switch (BBQ::strToClientCmd(command)) 
		{
			case BBQ::ClientCommand::I_AM_HUNGRY_GIVE_ME_BBQ:
				if (client.State == BBQ::ClientState::New)
				{
					client.State = BBQ::ClientState::Waiting;
				}
				else if (client.State == BBQ::ClientState::Waiting)
				{
					waitForOrder(); //simulating waiting for order to get ready
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
				command = BBQ::srvRespToStr(BBQ::ServerResponse::OkWait);
				break;
			case BBQ::ClientState::Ready:
				command = BBQ::pushCmdFromOrder(client.Order);
				break;
			case BBQ::ClientState::Served:
				command = BBQ::srvRespToStr(BBQ::ServerResponse::Served);
				break;
			case BBQ::ClientState::Closed:
				command = BBQ::srvRespToStr(BBQ::ServerResponse::Closed);
		}

		ostr << command;

		logResponse(response, command);
	}

	void waitForOrder() //simulating waiting for order to get ready
	{
		srand(std::time(nullptr));
		sleep_for(seconds(rand() % waitTimeForMeal));
	}

	void logResponse(HTTPServerResponse& response, string command) {
		Application& app = Application::instance();

		app.logger().information("Response: " + command);
	}

	void logRequest(HTTPServerRequest& request, string command) {
		Application& app = Application::instance();

		app.logger().information("Request from " + request.clientAddress().toString());
		app.logger().information("Command: " + command);
	}

	string getRequestCommand(Poco::Net::HTTPServerRequest& request)
	{
		size_t size = (size_t)request.getContentLength();
		istream& stream = request.stream();
		string encoded_content;
		string content;
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
	
	BBQ::MenuItem getRandomMeal()
	{
		srand(std::time(nullptr));
		return (BBQ::MenuItem)(rand() % 3);
	}
};


class BBQRequestHandlerFactory : public HTTPRequestHandlerFactory
{
public:
	BBQRequestHandlerFactory(int waitTime)
	{
		waitTimeForMeal = waitTime;
	}

	HTTPRequestHandler* createRequestHandler(const HTTPServerRequest& request)
	{
		if (request.getURI() == "/")
		{
			return new BBQRequestHandler(waitTimeForMeal);
		}
		else
			return 0;
	}

private:
	int waitTimeForMeal;
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

		options.addOption(
			Option()
			.fullName("port")
			.description("default value 80")
			.required(false)
			.repeatable(false)
			.argument("value")
			.binding("Port"));

		options.addOption(
			Option()
			.fullName("wait-time")
			.description("Maximal time to wait for meal to get ready. Default value is 1 sec")
			.required(false)
			.repeatable(false)
			.argument("value")
			.binding("WaitTime"));
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
		helpFormatter.setHeader("A BBQ server over HTTP.");
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
			int port = 80;
			int waitTimeForMeal = 1;

			if (config().hasProperty("Port")) {
				port = stoi(config().getString("Port"));
			};

			if (config().hasProperty("WaitTime")) {
				waitTimeForMeal = stoi(config().getString("WaitTime"));
			};


			int maxQueued = 1;
			int maxThreads = 16;
			ThreadPool::defaultPool().addCapacity(maxThreads);

			HTTPServerParams* pParams = new HTTPServerParams;
			pParams->setMaxQueued(maxQueued);
			pParams->setMaxThreads(maxThreads);

			// set-up a server socket
			ServerSocket svs(port);
			// set-up a HTTPServer instance
			HTTPServer srv(new BBQRequestHandlerFactory(waitTimeForMeal), svs, pParams);
			// start the HTTPServer
			srv.start();

			logger().information("BBQ server started on port: " + to_string(srv.port()));

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
