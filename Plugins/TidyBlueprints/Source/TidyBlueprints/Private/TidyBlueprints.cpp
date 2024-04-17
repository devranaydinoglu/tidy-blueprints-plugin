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

    UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FTidyBlueprintsModule::RegisterNodeMenuItem));
}

void FTidyBlueprintsModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

void FTidyBlueprintsModule::RegisterNodeMenuItem()
{
    const TArray<FName> NodeMenuNames = {
        "GraphEditor.GraphNodeContextMenu.K2Node_CallFunction",
        "GraphEditor.GraphNodeContextMenu.K2Node_VariableGet",
        "GraphEditor.GraphNodeContextMenu.K2Node_VariableSet",
        "GraphEditor.GraphNodeContextMenu.K2Node_IfThenElse",
        "GraphEditor.GraphNodeContextMenu.K2Node_CustomEvent",
        "GraphEditor.GraphNodeContextMenu.K2Node_Timeline"
    };

    for (const FName& MenuName : NodeMenuNames)
    {
        UToolMenu* Menu = UToolMenus::Get()->ExtendMenu(MenuName);
        FToolMenuSection& Section = Menu->FindOrAddSection("EdGraphSchemaOrganization");

        FToolUIActionChoice TidyUpAction(FExecuteAction::CreateLambda([]()
            {
                UE_LOG(LogTemp, Display, TEXT("Tidy Up process initiated"));

                if (GEditor) GEditor->GetEditorSubsystem<UTBManagerSubsystem>()->StartTidyUp();
            }
        ));

        Section.AddEntry(FToolMenuEntry::InitMenuEntry(FName("TidyUp"), FText::FromString("Tidy Up"), FText::FromString("Start the Tidy Up process"), FSlateIcon(), TidyUpAction));
    }
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FTidyBlueprintsModule, TidyBlueprints)