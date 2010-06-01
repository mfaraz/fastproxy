/*
 * proxy.cpp
 *
 *  Created on: May 31, 2010
 *      Author: nbryskin
 */

#include <iostream>
#include <boost/bind.hpp>

#include "proxy.hpp"
#include "statistics.hpp"
#include "session.hpp"

logger proxy::log = logger(keywords::channel = "proxy");

proxy::proxy(asio::io_service& io, const ip::tcp::endpoint& inbound, const ip::tcp::endpoint& outbound_http, const ip::udp::endpoint& outbound_ns, const ip::udp::endpoint& name_server)
    : acceptor(io, inbound)
    , resolver_(io, outbound_ns, name_server)
    , outbound_http(outbound_http)
{
    TRACE() << "start listening on " << inbound;
    acceptor.listen();
}

// called by main (parent)
void proxy::start()
{
    start_accept();
    resolver_.start();
}

// called by session (child)
resolver& proxy::get_resolver()
{
    return resolver_;
}

// called by session (child)
void proxy::finished_session(session* session, const boost::system::error_code& ec)
{
    TRACE_ERROR(ec);
    sessions.erase(session);
    delete session;
}

void proxy::start_accept()
{
    std::unique_ptr<session> new_sess(new session(acceptor.io_service(), *this));
    acceptor.async_accept(new_sess->socket(), boost::bind(&proxy::handle_accept, this, placeholders::error, new_sess.get()));
    new_sess.release();
}

void proxy::handle_accept(const boost::system::error_code& ec, session* new_session)
{
    std::unique_ptr<session> session_ptr(new_session);
    TRACE_ERROR(ec);
    if (ec)
        return;

    start_session(session_ptr.get());
    session_ptr.release();
}

void proxy::start_session(session* new_session)
{
    sessions.insert(new_session);
    new_session->start();
    start_accept();
}