#include "validation.h"

#include <QString>
#include <QDebug>

#include <sstream>

std::string validation(const amqp_rpc_reply_t& reply, const std::string &context)
{
  std::string errorMsg = context + ": ";

  if (reply.reply_type == AMQP_RESPONSE_NORMAL)
  {
    return "";
  }
  else if (reply.reply_type == AMQP_RESPONSE_NONE)
  {
    errorMsg += "missing RPC reply type";
  }
  else if (reply.reply_type == AMQP_RESPONSE_LIBRARY_EXCEPTION)
  {
    errorMsg += amqp_error_string2(reply.library_error);
  }
  else if (reply.reply_type == AMQP_RESPONSE_SERVER_EXCEPTION)
  {
    if (reply.reply.id == AMQP_CONNECTION_CLOSE_METHOD)
    {
      amqp_connection_close_t* method = reinterpret_cast<amqp_connection_close_t*>(reply.reply.decoded);

      errorMsg += "server connection error " + std::to_string(method->reply_code) +
              ", message: " + std::string(reinterpret_cast<char*>(method->reply_text.bytes), method->reply_text.len);
    }
    else if (reply.reply.id == AMQP_CHANNEL_CLOSE_METHOD)
    {
      amqp_channel_close_t* method = reinterpret_cast<amqp_channel_close_t*>(reply.reply.decoded);

      errorMsg += "server channel error " + std::to_string(method->reply_code) +
              ", message: " + std::string(reinterpret_cast<char*>(method->reply_text.bytes), method->reply_text.len);
    }
    else
    {
      std::ostringstream oss;
      oss << "unknown server error, method id " << std::hex << "0x" << reply.reply.id;
      errorMsg += oss.str();
    }
  }
  else
  {
    errorMsg += "unknown RPC reply type";
  }

  qCritical() << QString::fromStdString(errorMsg);
  return errorMsg;
}

std::string validationAfterConsumeMessage(amqp_connection_state_t connection, const amqp_rpc_reply_t& reply, const std::string &context)
{
  amqp_frame_t frame;
  if (AMQP_RESPONSE_NORMAL != reply.reply_type)
  {
    if (AMQP_RESPONSE_LIBRARY_EXCEPTION == reply.reply_type &&
        AMQP_STATUS_UNEXPECTED_STATE == reply.library_error)
    {
      int status = amqp_simple_wait_frame(connection, &frame);

      if (AMQP_STATUS_OK != status)
      {
        std::string errorMsg = context + ": failed to wait for the next frame: " + amqp_error_string2(status);
        qCritical() << QString::fromStdString(errorMsg);
        return errorMsg;
      }

      else if (AMQP_FRAME_METHOD == frame.frame_type)
      {
        if (frame.payload.method.id == AMQP_BASIC_RETURN_METHOD)
        {
          qWarning() << QString::fromStdString(context) + ": the message was returned back";
          amqp_message_t message;
          auto ret = amqp_read_message(connection, frame.channel, &message, 0);
          amqp_destroy_message(&message);
          if (AMQP_RESPONSE_NORMAL != ret.reply_type)
          {
            std::string errorMsg = context + ": failed to read returned message";
            qCritical() << QString::fromStdString(errorMsg);
            return errorMsg;
          }
        }

        else if (frame.payload.method.id == AMQP_CHANNEL_CLOSE_METHOD)
        {
          std::string errorMsg = context + ": channel closed";
          qCritical() << QString::fromStdString(errorMsg);
          return errorMsg;
        }

        else if (frame.payload.method.id == AMQP_CONNECTION_CLOSE_METHOD)
        {
          std::string errorMsg = context + ": connection closed";
          qCritical() << QString::fromStdString(errorMsg);
          return errorMsg;
        }

        else if (frame.payload.method.id != AMQP_BASIC_ACK_METHOD)
        {
          std::ostringstream oss;
          oss << context << ": unexpected method was received, id:" << std::hex << "0x" << frame.payload.method.id;
          qCritical() << QString::fromStdString(oss.str());
          return oss.str();
        }
      }
    }
  }
  return "";
}
