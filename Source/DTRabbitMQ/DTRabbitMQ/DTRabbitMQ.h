// Copyright 2024 Dexter.Wan. All Rights Reserved. 
// EMail: 45141961@qq.com

#pragma once
#include "Modules/ModuleManager.h"

class FDTRabbitMQModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
