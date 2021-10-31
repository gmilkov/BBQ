// BBQHTTPCleint.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <vector>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPMessage.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/URI.h>
#include <Poco/StreamCopier.h>
#include "../BBQProtocol/BBQ.h"
#include <Poco/Util/ServerApplication.h>

using namespace Poco::Util;
using namespace std;
using namespace Poco;
using namespace Poco::Net;

Poco::Net::NameValueCollection getCookies(vector<Poco::Net::HTTPCookie>& cookies) {
	Poco::Net::NameValueCollection nvc;
	std::vector<Poco::Net::HTTPCookie>::const_iterator it = cookies.begin();
	while (it != cookies.end()) {
		nvc.add((*it).getName(), (*it).getValue());
		++it;
	}
	return nvc;
}

BBQ::ServerResponse sendCommand(string command, HTTPClientSession& session, Poco::Net::NameValueCollection& cookies) {
	HTTPRequest request(HTTPRequest::HTTP_POST, "/", HTTPMessage::HTTP_1_1);

	request.setContentLength(command.length());

	request.setCookies(cookies);

	ostream& os = session.sendRequest(request);
	os << command;

	cout << "Sent:" << command << endl;

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

			cout << "Recieced: " << result << endl;

			BBQ::ServerResponse resp = BBQ::strToSrvResp(result);

			return resp;
		}
		else
		{
			cout << "Error: " << response.getStatus() << endl;
			return BBQ::ServerResponse::Closed;
		}
	}
	catch (Poco::Exception& exc) {
		cout << exc.displayText();
		return BBQ::ServerResponse::OkWait;
	}
}


int main(int argc, char** argv)
{
	if (argc < 2) return EXIT_FAILURE;
	bool useProxy = false;
	if (argc == 4) { useProxy = true; }

	URI uri(argv[1]);

	std::string path(uri.getPathAndQuery());
	if (path.empty()) path = "/";

	HTTPClientSession session(uri.getHost(), uri.getPort());

	if (useProxy)
	{
		Poco::Net::HTTPClientSession::ProxyConfig proxy;

		proxy.host = argv[2];
		proxy.port = stoi(argv[3]);
		proxy.authMethod = Poco::Net::HTTPClientSession::ProxyAuthentication::PROXY_AUTH_NONE;

		session.setProxyConfig(proxy);
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
		cout << "Error: " << exc.displayText();
	}

	cout << endl << "-- Press [Enter] to exit --" << endl;
	cin.get();
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
