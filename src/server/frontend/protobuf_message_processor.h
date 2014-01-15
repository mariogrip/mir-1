/*
 * Copyright © 2012. 2014 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Alan Griffiths <alan@octopull.co.uk>
 */


#ifndef MIR_FRONTEND_PROTOBUF_MESSAGE_PROCESSOR_H_
#define MIR_FRONTEND_PROTOBUF_MESSAGE_PROCESSOR_H_

#include "message_processor.h"
#include "protobuf_response_processor.h"

#include "mir_protobuf.pb.h"

#include <memory>

namespace mir
{
namespace protobuf { class DisplayServer; }

namespace frontend
{
class MessageProcessorReport;

namespace detail
{
class ProtobufMessageProcessor : public MessageProcessor
{
public:
    ProtobufMessageProcessor(
        std::shared_ptr<ProtobufMessageSender> const& sender,
        std::shared_ptr<protobuf::DisplayServer> const& display_server,
        std::shared_ptr<MessageProcessorReport> const& report);

    ~ProtobufMessageProcessor() noexcept {}

private:
    template<class ResultMessage>
    void send_response(::google::protobuf::uint32 id, ResultMessage* response);

    bool process_message(std::istream& msg);

    bool dispatch(mir::protobuf::wire::Invocation const& invocation);

    template<class ParameterMessage, class ResultMessage>
    void invoke(
        void (protobuf::DisplayServer::*function)(
            ::google::protobuf::RpcController* controller,
            const ParameterMessage* request,
            ResultMessage* response,
            ::google::protobuf::Closure* done),
        mir::protobuf::wire::Invocation const& invocation);

    std::shared_ptr<protobuf::DisplayServer> const display_server;
    std::shared_ptr<MessageProcessorReport> const report;
    std::shared_ptr<ProtobufMessageSender> responder;
};

// TODO specializing on the the message type to determine how we send FDs seems a bit of a frig.
template<>
void ProtobufMessageProcessor::send_response(::google::protobuf::uint32 id, protobuf::Buffer* response);
template<>
void ProtobufMessageProcessor::send_response(::google::protobuf::uint32 id, protobuf::Connection* response);
template<>
void ProtobufMessageProcessor::send_response(::google::protobuf::uint32 id, protobuf::Surface* response);
}
}
}

#endif /* PROTOBUF_MESSAGE_PROCESSOR_H_ */
