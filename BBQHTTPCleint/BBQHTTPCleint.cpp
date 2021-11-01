#include "Poco/Util/Application.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/HelpFormatter.h"
#include "Poco/Util/AbstractConfiguration.h"
#include "Poco/AutoPtr.h"
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <Poco/URI.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPMessage.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/URI.h>
#include <Poco/StreamCopier.h>
#include "../BBQProtocol/BBQ.h"
#include <Poco/Util/ServerApplication.h>


using namespace std;
using namespace Poco;
using namespace Poco::Net;
using namespace Poco::Util;

using Poco::Util::Application;
using Poco::Util::Option;
using Poco::Util::OptionSet;
using Poco::Util::HelpFormatter;
using Poco::Util::AbstractConfiguration;
using Poco::Util::OptionCallback;
using Poco::AutoPtr;


class BBQHTTPClientApp : public Application
	/// This sample demonstrates some of the features of the Util::Application class,
	/// such as configuration file handling and command line arguments processing.
	///
	/// Try SampleApp --help (on Unix platforms) or SampleApp /help (elsewhere) for
	/// more information.
{
public:
	BBQHTTPClientApp() : _helpRequested(false)
	{
	}

protected:
	void initialize(Application& self)
	{
		loadConfiguration(); // load default configuration files, if present
		Application::initialize(self);
		// add your own initialization code here
	}

	void uninitialize()
	{
		// add your own uninitialization code here
		Application::uninitialize();
	}

	void reinitialize(Application& self)
	{
		Application::reinitialize(self);
		// add your own reinitialization code here
	}

	void defineOptions(OptionSet& options)
	{
		Application::defineOptions(options);

		options.addOption(
			Option("help", "h", "display help information on command line arguments")
			.required(false)
			.repeatable(false)
			.callback(OptionCallback<BBQHTTPClientApp>(this, &BBQHTTPClientApp::handleHelp)));

		options.addOption(
			Option()
			.fullName("server")
			.description("BBQ Server to connect to")
			.required(true)
			.repeatable(false)
			.argument("value")
			.binding("Server"));

		options.addOption(
			Option()
			.fullName("port")
			.description("BBQ Protocol port")
			.required(false)
			.repeatable(false)
			.argument("value")
			.binding("ServerPort"));

		options.addOption(
			Option()
			.fullName("proxy")
			.description("optional if you use proxy")
			.required(false)
			.repeatable(false)
			.argument("value")
			.binding("Proxy"));

		options.addOption(
			Option()
			.fullName("proxy-port")
			.description("proxy port")
			.required(false)
			.repeatable(false)
			.argument("value")
			.binding("ProxyPort"));
	}

	void handleHelp(const std::string& name, const std::string& value)
	{
		_helpRequested = true;
		displayHelp();
		stopOptionsProcessing();
	}

	void handleDefine(const std::string& name, const std::string& value)
	{
		defineProperty(value);
	}

	void handleConfig(const std::string& name, const std::string& value)
	{
		loadConfiguration(value);
	}

	void displayHelp()
	{
		HelpFormatter helpFormatter(options());
		helpFormatter.setCommand(commandName());
		helpFormatter.setUsage("OPTIONS");
		helpFormatter.setHeader("BBQ Client over HTTP.");
		helpFormatter.format(std::cout);
	}

	void defineProperty(const std::string& def)
	{
		std::string name;
		std::string value;
		std::string::size_type pos = def.find('=');
		if (pos != std::string::npos)
		{
			name.assign(def, 0, pos);
			value.assign(def, pos + 1, def.length() - pos);
		}
		else name = def;
		config().setString(name, value);
	}

	int main(const std::vector<std::string>& args)
	{
		if (!_helpRequested)
		{
			int httpPort = 80;

			if (config().hasProperty("ServerPort")) {
				httpPort = stoi(config().getString("ServerPort"));
			};

			HTTPClientSession session(config().getString("Server"), httpPort);

			logger().information("Connecting to " + session.getHost() + " on port " + to_string(session.getPort()));

			if (config().hasProperty("Proxy"))
			{
				Poco::Net::HTTPClientSession::ProxyConfig proxy;

				proxy.host = config().getString("Proxy");
				proxy.port = stoi(config().getString("ProxyPort"));
				proxy.authMethod = Poco::Net::HTTPClientSession::ProxyAuthentication::PROXY_AUTH_NONE;

				session.setProxyConfig(proxy);

				logger().information("Using proxy " + session.getProxyHost() + " on port " + to_string(session.getProxyPort()));
			};

			string command = BBQ::clientCmdToStr(BBQ::ClientCommand::I_AM_HUNGRY_GIVE_ME_BBQ);

			try {

				Poco::Net::NameValueCollection cookies;

				BBQ::ServerResponse response = sendCommand(command, session, cookies);

				while (response == BBQ::ServerResponse::OkWait) {
					response = sendCommand(command, session, cookies);
				}

				if (response == BBQ::ServerResponse::BeefReady || response == BBQ::ServerResponse::ChickenReady)
				{
					command = BBQ::clientCmdToStr(BBQ::ClientCommand::I_TAKE_THAT);
				}
				else if (response == BBQ::ServerResponse::MammothReady)
				{
					command = BBQ::clientCmdToStr(BBQ::ClientCommand::NO_THANKS);
				}

				response = sendCommand(command, session, cookies);
			}
			catch (Poco::Exception& exc) {
				logger().error("Error: " + exc.displayText());
			}
		}

		return Application::EXIT_OK;
	}

	void printProperties(const std::string& base)
	{
		AbstractConfiguration::Keys keys;
		config().keys(base, keys);
		if (keys.empty())
		{
			if (config().hasProperty(base))
			{
				std::string msg;
				msg.append(base);
				msg.append(" = ");
				msg.append(config().getString(base));
				logger().information(msg);
			}
		}
		else
		{
			for (AbstractConfiguration::Keys::const_iterator it = keys.begin(); it != keys.end(); ++it)
			{
				std::string fullKey = base;
				if (!fullKey.empty()) fullKey += '.';
				fullKey.append(*it);
				printProperties(fullKey);
			}
		}
	}

	BBQ::ServerResponse sendCommand(string command, HTTPClientSession& session, Poco::Net::NameValueCollection& cookies) {
		HTTPRequest request(HTTPRequest::HTTP_POST, "/", HTTPMessage::HTTP_1_1);
	
		request.setContentLength(command.length());
	
		request.setCookies(cookies);
	
		ostream& os = session.sendRequest(request);
		os << command;
	
		logger().information("Sent: " + command);
	
		try {
			HTTPResponse response;
			std::istream& rs = session.receiveResponse(response);
	
			if (response.getStatus() == HTTPResponse::HTTP_OK)
			{
				vector<Poco::Net::HTTPCookie> newCookies;
				response.getCookies(newCookies);
				if (!newCookies.empty()) {
					cookies = getCookies(newCookies);
				}
	
				string encoded_content(std::istreambuf_iterator<char>(rs), {});
	
				string result;
				Poco::URI::decode(encoded_content, result);
	
				logger().information("Recieced: " + result);
	
				BBQ::ServerResponse resp = BBQ::strToSrvResp(result);
	
				return resp;
			}
			else
			{
				logger().error("Error: " + response.getStatus());
				return BBQ::ServerResponse::Closed;
			}
		}
		catch (Poco::Exception& exc) {
			logger().error("Error: " + exc.displayText());
			return BBQ::ServerResponse::OkWait;
		}
	}

	Poco::Net::NameValueCollection getCookies(vector<Poco::Net::HTTPCookie>& cookies) {
		Poco::Net::NameValueCollection nvc;
		std::vector<Poco::Net::HTTPCookie>::const_iterator it = cookies.begin();
		while (it != cookies.end()) {
			nvc.add((*it).getName(), (*it).getValue());
			++it;
		}
		return nvc;
	}

private:
	bool _helpRequested;
};


POCO_APP_MAIN(BBQHTTPClientApp)