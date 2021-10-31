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

			std::cout << response.getStatus() << " " << response.getReason() << std::endl;

			string encoded_content(std::istreambuf_iterator<char>(rs), {});

			string result;
			Poco::URI::decode(encoded_content, result);

			BBQ::ServerResponse resp = BBQ::strToSrvResp(result);

			return resp;
		}
		else
		{
			return BBQ::ServerResponse::OkWait;
		}
	}
	catch (Poco::Exception& exc) {
		cout << exc.displayText();
		return BBQ::ServerResponse::OkWait;
	}
}


int main(std::vector<std::string>& args)
{
	URI uri("http://localhost:80");

	std::string path(uri.getPathAndQuery());
	if (path.empty()) path = "/";

	HTTPClientSession session(uri.getHost(), uri.getPort());

	string command = BBQ::clientCmdToStr(BBQ::ClientCommand::I_AM_HUNGRY_GIVE_ME_BBQ);

	try {

		Poco::Net::NameValueCollection cookies;
		
		BBQ::ServerResponse response = sendCommand(command, session, cookies);
		
		cout << BBQ::srvRespToStr(response);

		while (response == BBQ::ServerResponse::OkWait) {
			response = sendCommand(command, session, cookies);
			cout << BBQ::srvRespToStr(response);
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
		cout << BBQ::srvRespToStr(response);
	}
	catch (Poco::Exception& exc) {
		cout << exc.displayText();
	}
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
