#ifndef __VERIFY_HPP
#define __VERIFY_HPP

#include "StateEvent.hpp"
#include <iostream>
#include <istream>
#include <ostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

using boost::asio::ip::tcp;

typedef boost::signal<void(const StateEvent)> Signal;

class Verifyer
{
public:
  Verifyer(boost::asio::io_service& io_service, int file_number,
	   Signal& signal)
    : _io(io_service),
      resolver_(io_service),
      socket_(io_service),
      _signal(signal),
      file_number(file_number)
  {
    start();
  }

private:
  void start() {
      socket_.close();
      std::ostream request_stream(&request_);
      request_stream << "GET " 
	<< "/items/2CFBF522062AA7EE!" << file_number << " HTTP/1.0\r\n";
      request_stream << "Host: storage.live.com \r\n";
      request_stream << "Accept: */*\r\n";
      request_stream << "Connection: close\r\n\r\n";

      tcp::resolver::query query("storage.live.com", "http");
      resolver_.async_resolve(query,
			      boost::bind(&Verifyer::handle_resolve, this,
					  boost::asio::placeholders::error,
					  boost::asio::placeholders::iterator));
  }
  void handle_resolve(const boost::system::error_code& err,
		      tcp::resolver::iterator endpoint_iterator) {
      if (!err) {
	  tcp::endpoint endpoint = *endpoint_iterator;
	  socket_.async_connect(endpoint,
				boost::bind(&Verifyer::handle_connect, this,
					    boost::asio::placeholders::error, ++endpoint_iterator));
      } else {
	  _signal(StateEvent(StateEvent::VerifyFail, -1));
	  start();
      }
  }

  void handle_connect(const boost::system::error_code& err,
		      tcp::resolver::iterator endpoint_iterator) {
      if (!err) {
	  boost::asio::async_write(socket_, request_,
				   boost::bind(&Verifyer::handle_write_request, this,
					       boost::asio::placeholders::error));
      } else if (endpoint_iterator != tcp::resolver::iterator()) {
	  socket_.close();
	  tcp::endpoint endpoint = *endpoint_iterator;
	  socket_.async_connect(endpoint,
				boost::bind(&Verifyer::handle_connect, this,
					    boost::asio::placeholders::error, ++endpoint_iterator));
      } else {
	  _signal(StateEvent(StateEvent::VerifyFail, -1));
	  start();
      }
  }

  void handle_write_request(const boost::system::error_code& err)
    {
      if (!err) {
	  boost::asio::async_read_until(socket_, response_, "\r\n",
					boost::bind(&Verifyer::handle_read_status_line, this,
						    boost::asio::placeholders::error));
      } else {
	  _signal(StateEvent(StateEvent::VerifyFail, -1));
	  start();
      }
    }

  void handle_read_status_line(const boost::system::error_code& err)
    {
      if (!err) {
	  // Check that response is OK.
	  std::istream response_stream(&response_);
	  std::string http_version;
	  response_stream >> http_version;
	  unsigned int status_code;
	  response_stream >> status_code;
	  std::string status_message;
	  std::getline(response_stream, status_message);
	  if (!response_stream || http_version.substr(0, 5) != "HTTP/") {
	      _signal(StateEvent(StateEvent::VerifyFail, -1));
	      start();
	  }

	  if (status_code != 200 || status_code != 302) {
	      _signal(StateEvent(StateEvent::VerifyFail, -1));
	      start();
	  } else {
	      boost::asio::deadline_timer t(_io,
					    boost::posix_time::seconds(5));
	      t.wait();
	      _signal(StateEvent(StateEvent::VerifyOK, -1));
	      start();
	  }
      } else {
	  _signal(StateEvent(StateEvent::VerifyFail, -1));
	  start();
      }
    }

private:
  boost::asio::io_service& _io;
  tcp::resolver resolver_;
  tcp::socket socket_;
  boost::asio::streambuf request_;
  boost::asio::streambuf response_;
  Signal& _signal;
  int file_number;
};

#endif
