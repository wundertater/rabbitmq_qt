#ifndef RABBITMQCLIENT_VALIDATION_H
#define RABBITMQCLIENT_VALIDATION_H

#include <amqp.h>

#include <string>

/**
 * /brief Валидирует rpc reply
 *
 * /param reply RPC ответ.
 * /param context Строка, которая описывает контекст вызова, будет предварять сообщение в случае ошибки.
 *
 * Выводит сообщение об ошибках и предупреждения в лог.
 *
 * /return Строка, содержащая сообщение об ошибке. Пустая строка, если ошибок нет.
 */
std::string validation(const amqp_rpc_reply_t& reply, const std::string &context);

/**
 * /brief Валидирует rpc reply конкретно после вызова amqp_consume_message
 *
 * /connection Объект соединения.
 * /param reply RPC ответ.
 * /param context Строка, которая описывает контекст вызова, будет предварять сообщение в случае ошибки.
 *
 * Выводит сообщение об ошибках и предупреждения в лог.
 *
 * /return Строка, содержащая сообщение об ошибке. Пустая строка, если ошибок нет.
 */
std::string validationAfterConsumeMessage(amqp_connection_state_t connection, const amqp_rpc_reply_t& reply, const std::string &context);

#endif
