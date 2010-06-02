/*
 * proxy.hpp
 *
 *  Created on: May 27, 2010
 *      Author: nbryskin
 */

#ifndef PROXY_HPP_
#define PROXY_HPP_

#include <boost/ptr_container/ptr_set.hpp>
#include <boost/asio.hpp>

#include "common.hpp"
#include "resolver.hpp"
#include "session.hpp"

class proxy : public boost::noncopyable
{
public:
    proxy(asio::io_service& io, const ip::tcp::endpoint& inbound, const ip::tcp::endpoint& outbound_http, const ip::udp::endpoint& outbound_ns, const ip::udp::endpoint& name_server);

    // called by main (parent)
    void start();

    // called by session (child)
    resolver& get_resolver();

    // called by session (child)
    void finished_session(session* session, const boost::system::error_code& ec);

    void dump_channels_state() const;

protected:
    void start_accept();

    void handle_accept(const boost::system::error_code& ec, session* new_session);
    void start_session(session* new_session);

    void update_statistics();

private:
    typedef boost::ptr_set<session> session_cont;
    ip::tcp::acceptor acceptor;
    resolver resolver_;
    session_cont sessions;
    ip::tcp::endpoint outbound_http;
    static logger log;
};

#endif /* PROXY_HPP_ */
