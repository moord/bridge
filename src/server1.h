/*
   сервер для режима "запрос" - "ответ"  по протоклолу "метра"
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

class session
        : public std::enable_shared_from_this<session>
{
public:
    session(tcp::socket socket)
        : socket_(std::move(socket))
    {
        std::cout << "session start\n";
    }

    ~session(){
        std::cout << "session stop\n";
    }
    void start()
    {
        do_read();
    }

private:
    void do_read()
    {
        auto self(shared_from_this());
        socket_.async_read_some(boost::asio::buffer(data_, max_length),
                                [this, self](boost::system::error_code ec, std::size_t length)
        {
            if (!ec)
            {
                bool send_answer = false;
                for(auto i = 0; i < length; i++){
                    if( rs.received_byte(data_[i])  ){
                        s_mpr_param prm;

                        prm.data = new unsigned char[512];

                        prm.rx_address = DEVICE_ADDR;
                        if( rs.get_packet(&prm,512) ){


                            switch(prm.command){
                            case 'S':
                                 if( prm.data[2] == ManageType::TYPE_MANAGE){
                                    dyn.set_manage(prm.data[0],prm.data[1]);
                                }
                                prm.data_size = 0;
                                break;
                            case 'G':
                                prm.data_size= dyn.copy(prm.data, 512);
                                 break;
                            default:
                                break;
                            }

                            std::swap(prm.rx_address, prm.tx_address);
                            rs.set_packet(&prm);

                            memcpy(data_,rs.packet, rs.size);
                            length = rs.size;
                        }

                        delete[] prm.data;

                        send_answer = true;
                        break;
                    }
                }
                if( send_answer){
                    do_write(length);

                }else{
                    do_read();
                }
            }
        });
    }

    void do_write(std::size_t length)
    {
        auto self(shared_from_this());
        boost::asio::async_write(socket_, boost::asio::buffer(data_, length),
                                 [this, self](boost::system::error_code ec, size_t)
        {
            if (!ec)
            {
                do_read();
            }
        });
    }

    tcp::socket socket_;
    enum { max_length = 1024 };
    char data_[max_length];

    MetraProtocol rs{512};
};

class server1
{
public:
    server1(boost::asio::io_service& io_context, short port)
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
                std::make_shared<session>(std::move(socket))->start();
            }

            do_accept();
        });
    }

    tcp::acceptor acceptor_;
};
