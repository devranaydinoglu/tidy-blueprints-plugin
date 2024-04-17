// Copyright 2023 devran. All Rights Reserved.

#include "TBManagerSubsystem.h"

#include "BlueprintEditor.h"
#include "SNodePanel.h"
#include "Widgets/Docking/SDockTab.h"

FBlueprintEditor* UTBManagerSubsystem::GetCurrentBlueprintEditor()
{
	TArray<UObject*> EditedAssets = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->GetAllEditedAssets();

	for (UObject* Asset : EditedAssets)
	{
		IAssetEditorInstance* AssetEditorInstance = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->FindEditorForAsset(Asset, false);
		FAssetEditorToolkit* AssetEditorToolkit = static_cast<FAssetEditorToolkit*>(AssetEditorInstance);

		TSharedPtr<SDockTab> Tab = AssetEditorToolkit->GetTabManager()->GetOwnerTab();
		if (Tab->IsForeground())
		{
			FBlueprintEditor* BlueprintEditor = static_cast<FBlueprintEditor*>(AssetEditorToolkit);
			return BlueprintEditor;
		}
	}

	return nullptr;
}

UEdGraph* UTBManagerSubsystem::GetCurrentGraph()
{
	UE_LOG(LogTemp, Display, TEXT("Current Graph: %s"), *GetCurrentBlueprintEditor()->GetFocusedGraph()->GetName());
	return GetCurrentBlueprintEditor()->GetFocusedGraph();
}

void UTBManagerSubsystem::GetSelectedNodes(UEdGraph* Graph)
{

}

void UTBManagerSubsystem::StartTidyUp()
{
	FBlueprintEditor* BlueprintEditor = GetCurrentBlueprintEditor();
	FGraphPanelSelectionSet SelectedNodes = BlueprintEditor->GetSelectedNodes();

	UE_LOG(LogTemp, Display, TEXT("BlueprintEditor Name: %s"), *BlueprintEditor->GetBlueprintObj()->GetFriendlyName());
	GetCurrentGraph();
	for (auto Node : SelectedNodes)
	{
		UE_LOG(LogTemp, Display, TEXT("Node Title: %s"), *Cast<UEdGraphNode>(Node)->GetNodeTitle(ENodeTitleType::FullTitle).ToString());
		for (const UEdGraphPin* Pin : Cast<UEdGraphNode>(Node)->Pins)
		{
			UE_LOG(LogTemp, Display, TEXT("Pin Name: %s"), *Pin->PinName.ToString());
			UE_LOG(LogTemp, Display, TEXT("Pin Category: %s"), *Pin->PinType.PinCategory.ToString());
			const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EEdGraphPinDirection"), true);
			UE_LOG(LogTemp, Display, TEXT("Pin Direction: %s"), *EnumPtr->GetDisplayNameTextByValue(Pin->Direction).ToString());
		}
	}
}

void UTBManagerSubsystem::SetNodePosition(UEdGraph* Graph, UEdGraphNode* Node, const FVector2D& NewPosition)
{
	Graph->GetSchema()->SetNodePosition(Node, NewPosition);
	Node->SnapToGrid(SNodePanel::GetSnapGridSize());
}
