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
	return GetCurrentBlueprintEditor()->GetFocusedGraph();
}

FGraphPanelSelectionSet UTBManagerSubsystem::GetSelectedNodes()
{
	CurrentBlueprintEditor = GetCurrentBlueprintEditor();
	return CurrentBlueprintEditor->GetSelectedNodes();
}

void UTBManagerSubsystem::StartTidyUp()
{
	FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();
	TBCluster Cluster;
	int32 CollectionIndex = 0;

	// TODO: fix collection index out of order (after getting first node in sequence, we should follow its ouput exec pin
	for (UObject* NodeObj : SelectedNodes)
	{
		UEdGraphNode* Node = Cast<UEdGraphNode>(NodeObj);

		TBNode NodeData = PopulateNodeData(Node);

		if (IsNodeExecutable(Node))
		{
			TBCollection Collection;
			Collection.Index = CollectionIndex;
			Collection.ParentNode = NodeData;

			// Get all non-executable nodes linked to the parent node
			for (UEdGraphPin* Pin : Node->Pins)
			{
				if (Pin->PinType.PinCategory != "exec")
				{
					for (UEdGraphPin* LinkedPin : Pin->LinkedTo)
					{
						if (Pin->Direction == EGPD_Input) Collection.InputNodes.Add(PopulateNodeData(LinkedPin->GetOwningNode()));
						else if (Pin->Direction == EGPD_Output) Collection.OutputNodes.Add(PopulateNodeData(LinkedPin->GetOwningNode()));
						RecursivelyGetChildNodes(LinkedPin->GetOwningNode(), LinkedPin, Collection);
					}
				}
			}

			Cluster.Collections.Add(Collection);
			CollectionIndex++;
		}

		if (IsNodeFirstInSequence(Node, SelectedNodes))
		{
			Cluster.StartingNode = PopulateNodeData(Node);
		}
	}

	UE_LOG(LogTemp, Error, TEXT("Cluster starting node: %s"), *Cluster.StartingNode.Node.Get()->GetNodeTitle(ENodeTitleType::FullTitle).ToString());

	for (int32 i = 0; i < Cluster.Collections.Num(); i++)
	{
		UE_LOG(LogTemp, Warning, TEXT("Collection parent node: %s"), *Cluster.Collections[i].ParentNode.Node.Get()->GetNodeTitle(ENodeTitleType::FullTitle).ToString());
		UE_LOG(LogTemp, Display, TEXT("Collection index: %s"), *FString::FromInt(Cluster.Collections[i].Index));

		for (int32 j = 0; j < Cluster.Collections[i].InputNodes.Num(); j++)
		{
			UE_LOG(LogTemp, Display, TEXT("Collection input node: %s"), *Cluster.Collections[i].InputNodes[j].Node.Get()->GetNodeTitle(ENodeTitleType::FullTitle).ToString());
		}
	}

}

void UTBManagerSubsystem::RecursivelyGetChildNodes(UEdGraphNode* Node, UEdGraphPin* InLinkedPin, TBCollection& Collection)
{
	if (Node->Pins.Num() < 2) return;

	for (UEdGraphPin* Pin : Node->Pins)
	{
		if (Pin->PinType.PinCategory != "exec" && Pin != InLinkedPin)
		{
			for (UEdGraphPin* LinkedPin : Pin->LinkedTo)
			{
				if (Pin->Direction == EGPD_Input) Collection.InputNodes.Add(PopulateNodeData(LinkedPin->GetOwningNode()));
				else if (Pin->Direction == EGPD_Output) Collection.OutputNodes.Add(PopulateNodeData(LinkedPin->GetOwningNode()));

				RecursivelyGetChildNodes(LinkedPin->GetOwningNode(), LinkedPin, Collection);
			}
		}
	}
}

TBNode UTBManagerSubsystem::PopulateNodeData(UEdGraphNode* Node)
{
	TBNode NodeData;
	NodeData.Node = TStrongObjectPtr(Node);
	FVector2D NodeSize = FVector2D(Node->NodeWidth, Node->NodeHeight);

	TSharedPtr<SGraphEditor> GraphEditor = GetCurrentBlueprintEditor()->OpenGraphAndBringToFront(GetCurrentGraph());
	FSlateRect Rect;
	if (GraphEditor->GetBoundsForNode(Node, Rect, 0.f))
	{
		NodeSize = FVector2D(Rect.Right - Rect.Left, Rect.Bottom - Rect.Top);
	}

	NodeData.NodeHeight = NodeSize.Y;
	NodeData.NodeWidth = NodeSize.X;

	return NodeData;
}

bool UTBManagerSubsystem::IsNodeFirstInSequence(const UEdGraphNode* Node, FGraphPanelSelectionSet SelectedNodes)
{
	int32 NumExecutePins = 0;
	for (const UEdGraphPin* ExecPin : Node->Pins)
	{
		if (ExecPin->PinType.PinCategory == "exec") NumExecutePins++;
	}
	if (Node->Pins.Num() < 3 && NumExecutePins > 0 && NumExecutePins < Node->Pins.Num()) return true;

	for (const UEdGraphPin* Pin : Node->Pins)
	{
		if (Pin->Direction == EGPD_Input && Pin->PinName == "execute")
		{
			for (const UEdGraphPin* LinkedPin : Pin->LinkedTo)
			{
				if (!SelectedNodes.Contains(LinkedPin->GetOwningNode()))
				{
					return true;
				}
			}
		}
	}

	return false;
}

bool UTBManagerSubsystem::IsNodeExecutable(const UEdGraphNode* Node)
{
	int32 NumExecPins = 0;
	for (const UEdGraphPin* Pin : Node->Pins)
	{
		if (Pin->PinType.PinCategory == "exec") NumExecPins++;
	}

	return NumExecPins > 0;
}

void UTBManagerSubsystem::SetNodePosition(UEdGraph* Graph, UEdGraphNode* Node, const FVector2D& NewPosition)
{
	Graph->GetSchema()->SetNodePosition(Node, NewPosition);
	Node->SnapToGrid(SNodePanel::GetSnapGridSize());
}
