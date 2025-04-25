// Copyright 2024 Dexter.Wan. All Rights Reserved. 
// EMail: 45141961@qq.com

using UnrealBuildTool;

public class DTRabbitMQ : ModuleRules
{
	public DTRabbitMQ(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(new string[] {} );
		PrivateIncludePaths.AddRange( new string[] { ModuleDirectory + "/DTRabbitMQ_C/" } );
		PublicDependencyModuleNames.AddRange( new string[] { "Core", } );
		PrivateDependencyModuleNames.AddRange( new string[] { "CoreUObject", "Engine", "Slate", "SlateCore", } );
		PrivateDefinitions.Add("AMQP_STATIC"); 
		PrivateDefinitions.Add("HAVE_CONFIG_H");
		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			PrivateDefinitions.Add("_WIN32");
			PrivateDefinitions.Add("WIN32");
			PrivateDefinitions.Add("_WINDOWS");
		}
		else if (Target.Platform == UnrealTargetPlatform.Linux)
		{
		}
	}
}
