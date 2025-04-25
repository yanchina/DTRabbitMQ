// Copyright 2024 Dexter.Wan. All Rights Reserved. 
// EMail: 45141961@qq.com

#pragma once
#include "HAL/Runnable.h"

class UDTRabbitMQClient;

class FDTRabbitMQListen : public FRunnable
{
	
private:
	volatile bool							bRun;
	UDTRabbitMQClient *						RabbitMQClient;
	FString									Key;
	uint16									Channel;
	
public:
	// 构造函数
	FDTRabbitMQListen(UDTRabbitMQClient * InRabbitMQClient, const FString & InKey, uint16 InChannel);
	// 析构函数
	virtual ~FDTRabbitMQListen() override;

public:
	// 初始化
	virtual bool Init() override;
	// 运行
	virtual uint32 Run() override;
	// 停止
	virtual void Stop() override;
};
