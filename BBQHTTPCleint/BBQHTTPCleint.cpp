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
using namespace std;
using namespace Poco;
using namespace Poco::Net;

void sendCommand(string command, HTTPClientSession& session, string& result) {
    HTTPRequest request(HTTPRequest::HTTP_POST, "/", HTTPMessage::HTTP_1_1);

    request.setContentLength(command.length());

    ostream& os = session.sendRequest(request);
    os << command;

    HTTPResponse response;
    std::istream& rs = session.receiveResponse(response);

    if (response.getStatus() == HTTPResponse::HTTP_OK)
    {
        std::cout << response.getStatus() << " " << response.getReason() << std::endl;

        string encoded_content(std::istreambuf_iterator<char>(rs), {});

        Poco::URI::decode(encoded_content, result);
    } 
    else
    {
        cout << response.getStatus();
    }
}


int main(std::vector<std::string> &args)
{
    URI uri("http://localhost:80");

    std::string path(uri.getPathAndQuery());
    if (path.empty()) path = "/";

    HTTPClientSession session(uri.getHost(), uri.getPort());

    string result;

    sendCommand("I AM HUNGRY, GIVE ME BBQ", session, result);

    cout << result;
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
