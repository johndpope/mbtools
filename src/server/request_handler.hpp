//
// request_handler.hpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HTTP_SERVER_REQUEST_HANDLER_HPP
#define HTTP_SERVER_REQUEST_HANDLER_HPP

#include <string>
#include <boost/noncopyable.hpp>
#include <memory>

namespace http {
namespace server {

struct reply;
struct request;

/// The common handler for all incoming requests.
class request_handler: private boost::noncopyable
{
public:

    explicit request_handler() = default;

    /// Handle a request and produce a reply.
    virtual void handle_request(const request& req, reply& rep) = 0;
};

} // namespace server
} // namespace http

#endif // HTTP_SERVER2_REQUEST_HANDLER_HPP
