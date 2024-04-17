// Copyright Epic Games, Inc. All Rights Reserved.

#include "TidyBlueprints.h"

#include "Editor.h"
#include "TBManagerSubsystem.h"
#include "ToolMenus.h"
#include "ToolMenu.h"
#include "ToolMenuSection.h"

#define LOCTEXT_NAMESPACE "FTidyBlueprintsModule"

void FTidyBlueprintsModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

    UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FTidyBlueprintsModule::RegisterFunctionNodeMenuItem));
    UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FTidyBlueprintsModule::RegisterVariableNodeMenuItem));
}

void FTidyBlueprintsModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

void FTidyBlueprintsModule::RegisterFunctionNodeMenuItem()
{
    UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("GraphEditor.GraphNodeContextMenu.K2Node_CallFunction");
    FToolMenuSection& Section = Menu->FindOrAddSection("EdGraphSchemaOrganization");

    FToolUIActionChoice TidyUpAction(FExecuteAction::CreateLambda([]()
        {
            UE_LOG(LogTemp, Display, TEXT("Tidy Up process initiated"));

            if (GEditor)
            {
                GEditor->GetEditorSubsystem<UTBManagerSubsystem>()->StartTidyUp();
            }
        }
    ));

    Section.AddEntry(FToolMenuEntry::InitMenuEntry(FName("TidyUp"), FText::FromString("Tidy Up"), FText::FromString("Start the Tidy Up process"), FSlateIcon(), TidyUpAction));
}

void FTidyBlueprintsModule::RegisterVariableNodeMenuItem()
{
    UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("GraphEditor.GraphNodeContextMenu.K2Node_VariableGet");
    FToolMenuSection& Section = Menu->FindOrAddSection("EdGraphSchemaOrganization");

    FToolUIActionChoice TidyUpAction(FExecuteAction::CreateLambda([]()
        {
            UE_LOG(LogTemp, Display, TEXT("Tidy Up process initiated"));

            if (GEditor)
            {
                GEditor->GetEditorSubsystem<UTBManagerSubsystem>()->StartTidyUp();
            }
        }
    ));

    Section.AddEntry(FToolMenuEntry::InitMenuEntry(FName("TidyUp"), FText::FromString("Tidy Up"), FText::FromString("Start the Tidy Up process"), FSlateIcon(), TidyUpAction));
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FTidyBlueprintsModule, TidyBlueprints)