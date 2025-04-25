// Copyright 2024 Dexter.Wan. All Rights Reserved. 
// EMail: 45141961@qq.com

#include "DTRabbitMQClient.h"
#include "DTRabbitMQListen.h"
#include "Async/TaskGraphInterfaces.h"
#include "Async/Async.h"
#include "amqp_time.h"
#include "rabbitmq-c/tcp_socket.h"
#include "string"

#define RABBITMQ_ERROR_MSG(b, m)				{ Success = b; ErrorMsg = m; }
#define RABBITMQ_CONDITION_ERROR_RETURN(c, m)	{ if ( c ) { RABBITMQ_ERROR_MSG(false, m); return; } }
#define RABBITMQ_EXECUTE_ERROR_RETURN(t, e)		{ if ( !MQ_Error(e, t, ErrorMsg) ) { Success = false; return; } }
#define RABBITMQ_EXECUTE_OK_RETURN				{ Success = true; ErrorMsg = TEXT("Success"); return; }
#define SAFE_POINTER_FUNC(p, func) 				{ if(p) {p->func;} }											
#define SAFE_POINTER_DELETE(p) 					{ if(p) { delete p; p = NULL;} }

DEFINE_LOG_CATEGORY(LOG_DTRabbitMQ);

// 对象销毁
void UDTRabbitMQClient::FinishDestroy()
{
	UObject::FinishDestroy();
	Disconnect(); 
}

// 断开广播
void UDTRabbitMQClient::ConnectionCloseBroadcast()
{
	AsyncTask( ENamedThreads::GameThread, [this]()
	{
		Disconnect();
		OnRabbitMQConnectionCloseMulticast.Broadcast();
	});
}

// 消息广播
void UDTRabbitMQClient::MessageBroadcast(int64 DeliveryTag, FString Key, FString Message)
{
	AsyncTask( ENamedThreads::GameThread, [this, DeliveryTag, Key, Message]()
	{
		if ( const FRabbitMQMessageMulticast * RabbitMQMessageMulticast = OnRabbitMQMessageMulticast.Find(Key) )
		{
			RabbitMQMessageMulticast->Broadcast(DeliveryTag, Message);
		}
	});
}

// 错误字符转换
FString UDTRabbitMQClient::AMQP_Error_String(int Code) const
{
	const std::string UTF8Error(amqp_error_string2(Code));
	return FString(UTF8_TO_TCHAR(UTF8Error.c_str()));
}

// 错误字符转换
FString UDTRabbitMQClient::AMQP_Error_String(void * pszData, int nSize) const
{
	const std::string UTF8Error((char*)pszData, nSize);
	return FString(UTF8_TO_TCHAR(UTF8Error.c_str()));
}

// 错误信息
bool UDTRabbitMQClient::MQ_Error(int Code, const TCHAR* Context, FString& ErrorMsg)
{
	if (Code < 0)
	{
		ErrorMsg = FString::Printf(TEXT("%s : %s"), Context, *AMQP_Error_String(Code) );
		UE_LOG(LOG_DTRabbitMQ, Error, TEXT("%s"), *ErrorMsg);
		if ( Code == AMQP_STATUS_SOCKET_ERROR )
		{
			ConnectionCloseBroadcast();
		}
		return false;
	}
	return true;
}

// 错误信息
bool UDTRabbitMQClient::MQ_Error(const amqp_rpc_reply_t& Reply, const TCHAR* Context, FString& ErrorMsg)
{
	switch (Reply.reply_type)
	{
	case AMQP_RESPONSE_NORMAL:
		{
			return true;
		}
	case AMQP_RESPONSE_NONE:
		{
			ErrorMsg = FString::Printf( TEXT("%s : missing RPC reply type!"), Context);
			break;
		}
	case AMQP_RESPONSE_LIBRARY_EXCEPTION:
		{
			ErrorMsg = FString::Printf( TEXT("%s: %s"), Context, *AMQP_Error_String(Reply.library_error));
			if ( Reply.library_error == AMQP_STATUS_SOCKET_ERROR )
			{
				ConnectionCloseBroadcast();
			}
			break;
		}
	case AMQP_RESPONSE_SERVER_EXCEPTION:
		{
			switch (Reply.reply.id)
			{
			case AMQP_CONNECTION_CLOSE_METHOD:
				{
					amqp_connection_close_t * m = (amqp_connection_close_t *)Reply.reply.decoded;
					ErrorMsg = FString::Printf( TEXT("%s: server connection error %uh, message: %s"), Context, m->reply_code, *AMQP_Error_String(m->reply_text.bytes, m->reply_text.len));
					ConnectionCloseBroadcast();
					break;
				}
			case AMQP_CHANNEL_CLOSE_METHOD:
				{
					amqp_channel_close_t * m = (amqp_channel_close_t *)Reply.reply.decoded;
					ErrorMsg = FString::Printf( TEXT("%s: server channel error %uh, message: %s"), Context, m->reply_code, *AMQP_Error_String(m->reply_text.bytes, m->reply_text.len));
					break;
				}
			default:
				{
					ErrorMsg = FString::Printf( TEXT("%s: unknown server error, method id 0x%08X"), Context, Reply.reply.id);
					break;
				}
			}
			break;
		}
	}
	
	UE_LOG(LOG_DTRabbitMQ, Error, TEXT("%s"), *ErrorMsg);
	return false;
}

// 创建客户端对象
void UDTRabbitMQClient::CreateRabbitMQClient(UDTRabbitMQClient*& RabbitMQClient)
{
	// 创建对象
	RabbitMQClient = NewObject<UDTRabbitMQClient>();
}

// 连接服务器
void UDTRabbitMQClient::Connect(const FString& Host, int Port, const FString& UserName, const FString& Password, const FString & VirtualHost, int Heartbeat, int Channel, bool& Success, FString& ErrorMsg)
{
	// 新建基础连接
	if ( AMQP_Conn != nullptr ) { Disconnect(); }
	AMQP_Conn = amqp_new_connection();

	// 新建一个Socket
	amqp_socket_t * AMQP_Socket = amqp_tcp_socket_new(AMQP_Conn);
	RABBITMQ_CONDITION_ERROR_RETURN(AMQP_Socket == nullptr, TEXT("Connect - New Socket Error"));

	// 连接服务器
	const std::string szHost = TCHAR_TO_UTF8(*Host);
	RABBITMQ_EXECUTE_ERROR_RETURN(TEXT("Connect - Opening Socket"), amqp_socket_open(AMQP_Socket, szHost.c_str(), Port));
	
	// 登录服务器
	const std::string szUserName = TCHAR_TO_UTF8(*UserName);
	const std::string szPassword = TCHAR_TO_UTF8(*Password);
	const std::string szVirtualHost = TCHAR_TO_UTF8(*VirtualHost);
	RABBITMQ_EXECUTE_ERROR_RETURN(TEXT("Connect - Logging in"), amqp_login(AMQP_Conn, szVirtualHost.c_str(), 0, AMQP_DEFAULT_FRAME_SIZE, Heartbeat, AMQP_SASL_METHOD_PLAIN, szUserName.c_str(), szPassword.c_str()));

	// 打开通道
	const uint16 RealChannel = Channel;
	amqp_channel_open(AMQP_Conn, RealChannel);
	RABBITMQ_EXECUTE_ERROR_RETURN(TEXT("Connect - Opening channel"), amqp_get_rpc_reply(AMQP_Conn));
	Channels.AddUnique(RealChannel);

	// 执行成功
	RABBITMQ_EXECUTE_OK_RETURN;
}

// 发送消息
void UDTRabbitMQClient::Publish(int Channel, const FString& Exchange, const FString& RoutingKey, const FString& Body, bool& Success, FString& ErrorMsg)
{
	// 判断基础连接
	RABBITMQ_CONDITION_ERROR_RETURN(AMQP_Conn == nullptr, TEXT("Publish - RabbitMQClient was not created normally, please use 'Create RabbitMQ Client' And 'Connect'."));

	// 判断有效通道
	const uint16 RealChannel = Channel;
	if ( Channels.Find(RealChannel) == INDEX_NONE )
	{
		amqp_channel_open(AMQP_Conn, RealChannel);
		RABBITMQ_EXECUTE_ERROR_RETURN(TEXT("Publish - Opening channel"), amqp_get_rpc_reply(AMQP_Conn));
		Channels.AddUnique(RealChannel);
	}
	
	// 定义消息属性
	amqp_basic_properties_t Properties;
	Properties._flags = AMQP_BASIC_CONTENT_TYPE_FLAG|AMQP_BASIC_DELIVERY_MODE_FLAG;
	Properties.content_type = amqp_cstring_bytes("text/plain");
	Properties.delivery_mode = 2;
	
	// 转换消息数据
	const std::string szExchange = TCHAR_TO_UTF8(*Exchange);
	const std::string szRoutingKey = TCHAR_TO_UTF8(*RoutingKey);
	const std::string szBody = TCHAR_TO_UTF8(*Body);

	// 发送数据
	RABBITMQ_EXECUTE_ERROR_RETURN(TEXT("Publish - Publishing"), amqp_basic_publish(AMQP_Conn, RealChannel, amqp_cstring_bytes(szExchange.c_str()),
									amqp_cstring_bytes(szRoutingKey.c_str()), 0, 0,
									&Properties, amqp_cstring_bytes(szBody.c_str())));

	// 执行成功
	RABBITMQ_EXECUTE_OK_RETURN;
}

void UDTRabbitMQClient::Acknowledges(int Channel, int64 DeliveryTag, bool Multiple, bool & Success, FString & ErrorMsg)
{
	// 判断基础连接
	RABBITMQ_CONDITION_ERROR_RETURN(AMQP_Conn == nullptr, TEXT("Acknowledges - RabbitMQClient was not created normally, please use 'Create RabbitMQ Client' And 'Connect'."));

	// 判断有效通道
	const uint16 RealChannel = Channel;
	if ( Channels.Find(RealChannel) == INDEX_NONE )
	{
		amqp_channel_open(AMQP_Conn, RealChannel);
		RABBITMQ_EXECUTE_ERROR_RETURN(TEXT("Acknowledges - Opening channel"), amqp_get_rpc_reply(AMQP_Conn));
		Channels.AddUnique(RealChannel);
	}

	// 发送消息
	RABBITMQ_EXECUTE_ERROR_RETURN(TEXT("Acknowledges - Acknowledging"), amqp_basic_ack(AMQP_Conn, RealChannel, DeliveryTag, Multiple));
	
	// 执行成功
	RABBITMQ_EXECUTE_OK_RETURN;
}

// 绑定消息回调
void UDTRabbitMQClient::BindMessageDelegate(int Channel, const FString & QueueName, bool AutoAcknowledges, const FRabbitMQMessage OnRabbitMQMessage, bool & Success, FString & ErrorMsg)
{
	// 判断基础连接
	RABBITMQ_CONDITION_ERROR_RETURN(AMQP_Conn == nullptr, TEXT("BindMessageDelegate - RabbitMQClient was not created normally, please use 'Create RabbitMQ Client' And 'Connect'."));

	// 判断有效通道
	const uint16 RealChannel = Channel;
	if ( Channels.Find(RealChannel) == INDEX_NONE )
	{
		amqp_channel_open(AMQP_Conn, RealChannel);
		RABBITMQ_EXECUTE_ERROR_RETURN(TEXT("BindMessageDelegate - Opening channel"), amqp_get_rpc_reply(AMQP_Conn));
		Channels.AddUnique(RealChannel);
	}
	
	// 生成关键词
	const FString Key = FString::Printf(TEXT("%s_%d"), *QueueName, RealChannel);
	FRabbitMQMessageMulticast * RabbitMQMessageMulticast = OnRabbitMQMessageMulticast.Find(Key);
	if ( RabbitMQMessageMulticast == nullptr )
	{
		// 启动监听
		const std::string szQueueName = TCHAR_TO_UTF8(*QueueName);
		amqp_basic_consume(AMQP_Conn, 1, amqp_cstring_bytes(szQueueName.c_str()), amqp_empty_bytes, 0, AutoAcknowledges, 0, amqp_empty_table);
		RABBITMQ_EXECUTE_ERROR_RETURN(TEXT("BindMessageDelegate - Consuming"), amqp_get_rpc_reply(AMQP_Conn));
		
		// 启动监听线程
		FListenThread ListenThread;
		ListenThread.Runnable = new FDTRabbitMQListen(this, Key, RealChannel);
		ListenThread.RunnableThread = FRunnableThread::Create(ListenThread.Runnable, *Key);
		ListenThreads.Add(ListenThread);
		RabbitMQMessageMulticast = &OnRabbitMQMessageMulticast.Add(Key);
	}
	
	// 添加到广播
	RabbitMQMessageMulticast->Add(OnRabbitMQMessage);

	// 执行成功
	RABBITMQ_EXECUTE_OK_RETURN;
}

// 绑定连接关闭
void UDTRabbitMQClient::BindConnectionCloseDelegate(const FRabbitMQConnectionClose OnConnectionClose)
{
	OnRabbitMQConnectionCloseMulticast.Add(OnConnectionClose);
}

// 断开服务器
void UDTRabbitMQClient::Disconnect()
{
	if ( AMQP_Conn != nullptr )
	{
		for ( const uint16 Channel : Channels )
		{
			amqp_channel_close(AMQP_Conn, Channel, AMQP_REPLY_SUCCESS);
		}
		for ( auto& ListenThread : ListenThreads )
		{
			SAFE_POINTER_FUNC(ListenThread.RunnableThread, Kill(true));
			SAFE_POINTER_DELETE(ListenThread.Runnable);
			SAFE_POINTER_DELETE(ListenThread.RunnableThread);
		}
		amqp_connection_close(AMQP_Conn, AMQP_REPLY_SUCCESS);
		amqp_destroy_connection(AMQP_Conn);
	}
	Channels.Empty();
	ListenThreads.Empty();
	OnRabbitMQMessageMulticast.Empty();
	AMQP_Conn = nullptr;
}
