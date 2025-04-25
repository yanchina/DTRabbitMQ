// Copyright 2024 Dexter.Wan. All Rights Reserved. 
// EMail: 45141961@qq.com

#pragma once

#include "CoreMinimal.h"
#include "rabbitmq-c/amqp.h"
#include "UObject/Object.h"
#include "HAL/Runnable.h"
#include "HAL/RunnableThread.h"
#include "DTRabbitMQClient.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LOG_DTRabbitMQ, Log, All);

struct FListenThread
{
	FRunnableThread*		RunnableThread;
	FRunnable*				Runnable;

	FListenThread() : RunnableThread(nullptr), Runnable(nullptr) {}
	FListenThread(const FListenThread& R) { *this = R; }
	FListenThread& operator= (const FListenThread& R) { RunnableThread = R.RunnableThread; Runnable = R.Runnable; return *this; }
};

UCLASS(BlueprintType, meta=(DisplayName="DT RabbitMQ Client"))
class DTRABBITMQ_API UDTRabbitMQClient : public UObject
{
	GENERATED_BODY()

public:
	
	DECLARE_DYNAMIC_DELEGATE(FRabbitMQConnectionClose);
	DECLARE_DYNAMIC_DELEGATE_TwoParams(FRabbitMQMessage, int64, DeliveryTag, const FString&, Message);
	
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRabbitMQConnectionCloseMulticast);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FRabbitMQMessageMulticast, int64, DeliveryTag, const FString&, Message);
	
public:
	TArray<uint16>									Channels;
	TArray<FListenThread>							ListenThreads;
	TMap<FString, FRabbitMQMessageMulticast>		OnRabbitMQMessageMulticast;
	FRabbitMQConnectionCloseMulticast				OnRabbitMQConnectionCloseMulticast;
	amqp_connection_state_t							AMQP_Conn = nullptr;
	
	// object interface
public:
	/**
	 * Object destruction
	 */
	virtual void FinishDestroy() override;

public:
	void ConnectionCloseBroadcast();
	void MessageBroadcast(int64 DeliveryTag, FString Key, FString Message);

private:
	FString AMQP_Error_String(int Code) const;
	FString AMQP_Error_String(void * pszData, int nSize) const;
	bool MQ_Error(int Code, const TCHAR* Context, FString& ErrorMsg);
	bool MQ_Error(const amqp_rpc_reply_t& Reply, const TCHAR* Context, FString& ErrorMsg);

public:
	/**
	 * Create an RabbitMQ client object
	 */
	UFUNCTION(BlueprintCallable, meta=(DisplayName="Create RabbitMQ Client"), Category="DT RabbitMQ")
	static void CreateRabbitMQClient(UPARAM(DisplayName="RabbitMQ Client") UDTRabbitMQClient *& RabbitMQClient);

	/**
	 * Open a socket connection login to the broker
	 * Param Host : connect to this host.
	 * Param Port : connect on this remote port.
	 * Param UserName : the user name for connecting to the broker.
	 * Param Password : the password for connecting to the broker.
	 * Param VirtualHost : the virtual host to connect to on the broker. The default on most brokers is "/"
	 * Param Heartbeat : the number of seconds between heartbeat frames to request of the broker. A value of 0 disables heartbeats.
	 * Param Channel : the channel identifier
	 */
	UFUNCTION(BlueprintCallable, meta=(Port=5672, VirtualHost="/", UserName="guest", Password="guest", Heartbeat=60, Channel=1), Category="DT RabbitMQ")
	void Connect( const FString & Host, int Port, const FString & UserName, const FString & Password, const FString & VirtualHost, int Heartbeat, int Channel, bool & Success, FString & ErrorMsg );

	/**
	 * Publish a message to the broker
	 *
	 * Publish a message on an exchange with a routing key.
	 * 
	 * Param Channel : the channel identifier
	 * Param Exchange : the exchange on the broker to publish to
	 * Param RoutingKey : the routing key to use when publishing the message
	 * Param Body : the message body
	 */
	UFUNCTION(BlueprintCallable, meta=(Channel=1), Category="DT RabbitMQ")
	void Publish(int Channel, const FString & Exchange, const FString & RoutingKey, const FString & Body, bool & Success, FString & ErrorMsg );


	/**
	 * Acknowledges a message
	 * 
	 * Param Channel : the channel identifier
	 * Param DeliveryTag : the delivery tag of the message to be ack'd
	 * Param Multiple : if true ack all messages up to this delivery tag, if false ack only this delivery tag
	 */
	UFUNCTION(BlueprintCallable, meta=(Channel=1, Multiple=false), Category="DT RabbitMQ")
	void Acknowledges(int Channel, int64 DeliveryTag, bool Multiple, bool & Success, FString & ErrorMsg);
	
	/**
	 * This method is called when a message arrives from the server.
	 *
	 * Param Channel : the channel identifier
	 * Param QueueName : the queue name
	 * Param AutoAcknowledges : if true automatically determine that message has been processed
	 */
	UFUNCTION(BlueprintCallable, meta=(Channel=1, AutoAcknowledges=true), Category="DT RabbitMQ")
	void BindMessageDelegate(int Channel, const FString & QueueName, bool AutoAcknowledges, UPARAM(DisplayName="On RabbitMQ Message") const FRabbitMQMessage OnRabbitMQMessage, bool & Success, FString & ErrorMsg);

	/**
	 * This method is called when the connection to the server is close.
	 */
	UFUNCTION(BlueprintCallable, Category="DT RabbitMQ")
	void BindConnectionCloseDelegate(const FRabbitMQConnectionClose OnConnectionClose);
	
	/**
	 * Disconnects from the server.
	 */
	UFUNCTION(BlueprintCallable, Category="DT RabbitMQ")
	void Disconnect();
};
