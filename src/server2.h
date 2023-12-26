/*
   сервер для режима "подписка" (данные отправляются клиенту только при изменени состаяний динамического массива)
*/

#pragma once

#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <boost/asio.hpp>
#include "metra_protocol.h"
#include "dyn_array.h"

#include "include.h"

using boost::asio::ip::tcp;

extern DynamicArray dyn;

std::vector<std::string> split(const std::string &str, char d)
{
    std::vector<std::string> r;

    std::string::size_type start = 0;
    std::string::size_type stop = str.find_first_of(d);
    while(stop != std::string::npos)
    {
        r.push_back(str.substr(start, stop - start));

        start = stop + 1;
        stop = str.find_first_of(d, start);
    }

    r.push_back(str.substr(start));

    return r;
}

class session2
  : public std::enable_shared_from_this<session2>,
  public Observer<state_rec>,public Observer<sensor_rec>
{
public:
  session2(tcp::socket socket)
    : socket_(std::move(socket))
  {
   // auto statBulk = std::make_shared<cBulk>(BulkTypeSize::STATIC_SIZE);
      std::cout << "session start\n";
      dyn.subscribe_state(this);
      dyn.subscribe_sensor(this);
  }

  ~session2(){
      std::cout << "session stop\n";
      dyn.unsubscribe_state( this );
      dyn.unsubscribe_sensor( this );
  }
  void start()
  {

    str = "";
    for( auto &s: dyn.state){
        str += s.to_string() +"\r\n";
    }

    for( auto &s: dyn.sensor){
        str += s.to_string() +"\r\n";
    }

    do_write();

    do_read();
  }

  void update(state_rec &rec){
      str = rec.to_string() +"\r\n";
      do_write();
  };
  void update(sensor_rec &rec){
      str = rec.to_string() +"\r\n";
      do_write();
  };
private:
  void do_read()
  {
    auto self(shared_from_this());
    boost::asio::async_read_until(socket_, buff, '\n',
                                  [this, self](boost::system::error_code ec, std::size_t read_bytes)
         {
             if (!ec)
             {
                 std::istream in(&buff);
                 std::string msg;
                 std::getline(in, msg);

                 auto cmd = split(msg, ' ');
                 if(cmd.size() == 3 && cmd[0] == "MANAGE"){
                     try{
                        dyn.set_manage(std::stoi(cmd[1]), std::stoi((cmd[2])));
                     } catch(...){
                         //std::cout << "error\n";
                     }
                 }

                 do_read();
             }
         });
  }

  void do_write()
  {
    auto self(shared_from_this());
    boost::asio::async_write(socket_, boost::asio::buffer(str),
        [this, self](boost::system::error_code ec, size_t)
        {
          if (!ec)
          {
//            do_read();
          }
        });
  }

  tcp::socket socket_;
  boost::asio::streambuf buff;
  std::string str;
};

class server2
{
public:
  server2(boost::asio::io_service& io_context, short port)
    : acceptor_(io_context, tcp::endpoint(tcp::v4(), port))
  {
    do_accept();
  }

  void stop(){
       acceptor_.close();
  }
private:
  void do_accept()
  {
    acceptor_.async_accept(
        [this](boost::system::error_code ec, tcp::socket socket)
        {
          if (!ec)
          {
            std::make_shared<session2>(std::move(socket))->start();
          }

          do_accept();
        });
  }

  tcp::acceptor acceptor_;


};
