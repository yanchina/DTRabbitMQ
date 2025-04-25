// Copyright 2024 Dexter.Wan. All Rights Reserved. 
// EMail: 45141961@qq.com

#include "DTRabbitMQListen.h"
#include "DTRabbitMQClient.h"
#include "amqp_time.h"

// 构造函数
FDTRabbitMQListen::FDTRabbitMQListen(UDTRabbitMQClient* InRabbitMQClient, const FString & InKey, uint16 InChannel)
	: bRun(false), RabbitMQClient(InRabbitMQClient), Key(InKey), Channel(InChannel)
{
}

// 析构函数
FDTRabbitMQListen::~FDTRabbitMQListen()
{
	
}

bool FDTRabbitMQListen::Init()
{
	bRun = true;
	return true;
}

uint32 FDTRabbitMQListen::Run()
{
	while ( bRun )
	{
		// 线程等待
		FPlatformProcess::Sleep(0.001);
		
		// 释放底层缓存
		amqp_maybe_release_buffers(RabbitMQClient->AMQP_Conn);

		// 读取消息
		amqp_envelope_t Envelope;
		timeval Timeout = { 0, 1 };
		const amqp_rpc_reply_t Reply = amqp_consume_message(RabbitMQClient->AMQP_Conn, &Envelope, &Timeout, 0);
		if ( AMQP_RESPONSE_NORMAL != Reply.reply_type)
		{
			// 正常超时不管
			if ( Reply.reply_type == AMQP_RESPONSE_LIBRARY_EXCEPTION && Reply.library_error == AMQP_STATUS_TIMEOUT )
			{
				continue;
			}

			// 网络断开了
			if ( Reply.reply_type == AMQP_RESPONSE_LIBRARY_EXCEPTION && Reply.library_error == AMQP_STATUS_SOCKET_ERROR )
			{
				RabbitMQClient->ConnectionCloseBroadcast();
				return 1;
			}

			// 其他异常情况
			continue;
		}
		
		// 回调消息
		const std::string szMessage((char*)Envelope.message.body.bytes, Envelope.message.body.len);
		RabbitMQClient->MessageBroadcast(Envelope.delivery_tag, Key, FString(UTF8_TO_TCHAR(szMessage.c_str())));

		// 释放消息
		amqp_destroy_envelope(&Envelope);
	}
	
	return 0;
}

void FDTRabbitMQListen::Stop()
{
	bRun = false;
}
