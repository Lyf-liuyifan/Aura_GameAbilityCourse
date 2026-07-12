// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class Aura : ModuleRules
{
	public Aura(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"GameplayAbilities",
			"DeveloperSettings",
			// UMG 模块提供 UUserWidget / CreateWidget 等,AuraUserWidget.h 公开继承 UUserWidget
			// 且在头文件中 include 了 Blueprint/UserWidget.h,因此必须是 Public 依赖
			// 否则其他模块 include AuraUserWidget.h 时会找不到 UMG 类型,链接期报 LNK2019
			"UMG"});

		PrivateDependencyModuleNames.AddRange(new string[] { "GameplayTags",
			"GameplayTasks",
			"NavigationSystem",
			"Niagara",
			"AIModule",
			// Slate 是 UMG 的底层渲染/布局框架,UMG 构建于 Slate 之上
			// 显式声明可避免直接使用 Slate 类型(FSlateBrush、SWidget 等)时再出链接错误
			"Slate",
			"SlateCore",
			"MotionWarping"});

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
