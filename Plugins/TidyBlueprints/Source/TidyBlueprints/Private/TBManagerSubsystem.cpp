// Copyright 2023 devran. All Rights Reserved.

#include "TBManagerSubsystem.h"

#include "BlueprintEditor.h"
#include "SGraphNode.h"
#include "SGraphPanel.h"
#include "SGraphPin.h"
#include "SNodePanel.h"
#include "Widgets/Docking/SDockTab.h"


void UTBManagerSubsystem::StartTidyUp()
{
	SetBlueprintEditor();
	SetSelectedNodes();

	TBCluster Cluster;

	for (UObject* NodeObj : SelectedNodes)
	{
		UEdGraphNode* Node = Cast<UEdGraphNode>(NodeObj);

		TBNode NodeData = PopulateNodeData(Node);

		if (IsNodeExecutable(Node))
		{
			TBCollection Collection;
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
						GetChildNodes(LinkedPin->GetOwningNode(), LinkedPin, Collection);
					}
				}
			}

			Cluster.Collections.Add(Collection);
		}

		if (IsNodeFirstInSequence(Node))
		{
			Cluster.StartingNode = PopulateNodeData(Node);
			//break;
		}
	}

	int32 CollectionIndex = 0;
	TraverseSequence(Cluster.StartingNode.Node.Get(), CollectionIndex, Cluster);
	Cluster.Collections.Sort([](const TBCollection& Col1, const TBCollection& Col2)
		{
			return Col1.Index < Col2.Index;
		});

	SetCollectionNodePositions(Cluster);
}

void UTBManagerSubsystem::TraverseSequence(UEdGraphNode* Node, int32& CollectionIndex, TBCluster& Cluster)
{
	if (!SelectedNodes.Contains(Node)) return;

	TBCollection* Collection = Cluster.FindCollection(Node);
	if (Collection->Index == -1) Collection->Index = CollectionIndex++;

	for (const UEdGraphPin* Pin : Node->Pins)
	{
		if (Pin->Direction == EGPD_Output && Pin->PinType.PinCategory == "exec")
		{
			for (UEdGraphPin* LinkedPin : Pin->LinkedTo)
			{
				TraverseSequence(LinkedPin->GetOwningNode(), CollectionIndex, Cluster);
			}
		}
	}
}

void UTBManagerSubsystem::GetChildNodes(UEdGraphNode* Node, UEdGraphPin* InLinkedPin, TBCollection& Collection)
{
	if (Node->Pins.Num() < 2) return;

	for (UEdGraphPin* Pin : Node->Pins)
	{
		if (Pin->PinType.PinCategory != "exec")
		{
			if (Pin != InLinkedPin)
			{
				for (UEdGraphPin* LinkedPin : Pin->LinkedTo)
				{
					if (Pin->Direction == EGPD_Input)
					{
						Collection.InputNodes.Add(PopulateNodeData(LinkedPin->GetOwningNode()));
					}
					else if (Pin->Direction == EGPD_Output)
					{
						Collection.OutputNodes.Add(PopulateNodeData(LinkedPin->GetOwningNode()));
					}

					GetChildNodes(LinkedPin->GetOwningNode(), LinkedPin, Collection);
				}
			}
		}
	}
}

TBNode UTBManagerSubsystem::PopulateNodeData(UEdGraphNode* Node)
{
	TBNode NodeData;
	NodeData.Node = TStrongObjectPtr(Node);
	NodeData.Size = GetNodeWidget(Node)->GetDesiredSize();

	return NodeData;
}

bool UTBManagerSubsystem::IsNodeFirstInSequence(const UEdGraphNode* Node)
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

void UTBManagerSubsystem::SetNodePosition(UEdGraphNode* Node, const FVector2D& NewPosition)
{
	BlueprintEditor->GetFocusedGraph()->GetSchema()->SetNodePosition(Node, NewPosition);
	Node->SnapToGrid(SNodePanel::GetSnapGridSize());
}

void UTBManagerSubsystem::SetCollectionNodePositions(const TBCluster& Cluster)
{
	int32 Index = 0;

	for (const TBCollection& Collection : Cluster.Collections)
	{
		for (int32 i = 0; i < Collection.InputNodes.Num(); i++)
		{
			if (CollectionLayoutType == CollectionLayoutType::STACKED)
			{
				FVector2D TargetPosition(Collection.ParentNode.Node.Get()->NodePosX,
					Collection.ParentNode.Node.Get()->NodePosY + Collection.ParentNode.Size.Y + CollectionNodesPaddingY);

				if (i - 1 > -1)
				{
					TargetPosition = FVector2D(Collection.InputNodes[i - 1].Node.Get()->NodePosX,
						Collection.InputNodes[i - 1].Node.Get()->NodePosY + Collection.InputNodes[i - 1].Size.Y + CollectionNodesPaddingY);
				}

				SetNodePosition(Collection.InputNodes[i].Node.Get(), TargetPosition);
			}
			else if (CollectionLayoutType == CollectionLayoutType::LIST)
			{
				FVector2D TargetPosition(Collection.ParentNode.Node.Get()->NodePosX - Collection.InputNodes[i].Size.X - CollectionNodesPaddingX,
					Collection.ParentNode.Node.Get()->NodePosY + Collection.ParentNode.Size.Y / 2 + CollectionNodesPaddingY);

				if (i - 1 > -1)
				{
					TargetPosition = FVector2D(Collection.InputNodes[i - 1].Node.Get()->NodePosX - Collection.InputNodes[i].Size.X - CollectionNodesPaddingX,
						Collection.InputNodes[i - 1].Node.Get()->NodePosY + CollectionNodesPaddingY);
				}

				SetNodePosition(Collection.InputNodes[i].Node.Get(), TargetPosition);
			}
		}

		Index++;
	}
}

void UTBManagerSubsystem::SetBlueprintEditor()
{
	TArray<UObject*> EditedAssets = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->GetAllEditedAssets();

	for (UObject* Asset : EditedAssets)
	{
		IAssetEditorInstance* AssetEditorInstance = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->FindEditorForAsset(Asset, false);
		FAssetEditorToolkit* AssetEditorToolkit = static_cast<FAssetEditorToolkit*>(AssetEditorInstance);

		TSharedPtr<SDockTab> Tab = AssetEditorToolkit->GetTabManager()->GetOwnerTab();
		if (Tab->IsForeground())
		{
			BlueprintEditor = static_cast<FBlueprintEditor*>(AssetEditorToolkit);
		}
	}
}

TSharedPtr<SGraphEditor> UTBManagerSubsystem::GetCurrentGraphEditor()
{
	return SGraphEditor::FindGraphEditorForGraph(BlueprintEditor->GetFocusedGraph());
}

SGraphPanel* UTBManagerSubsystem::GetCurrentGraphPanel()
{
	return GetCurrentGraphEditor()->GetGraphPanel();
}

void UTBManagerSubsystem::SetSelectedNodes()
{
	SelectedNodes = BlueprintEditor->GetSelectedNodes();
}

SGraphNode* UTBManagerSubsystem::GetNodeWidget(const UEdGraphNode* Node)
{
	return GetCurrentGraphPanel()->GetNodeWidgetFromGuid(Node->NodeGuid).Get();
}

FVector2D UTBManagerSubsystem::GetPinOffset(const UEdGraphPin* Pin)
{
	SGraphNode* NodeWidget = GetNodeWidget(Pin->GetOwningNode());
	TSharedPtr<SGraphPin> PinWidget = NodeWidget->FindWidgetForPin(const_cast<UEdGraphPin*>(Pin));
	if (PinWidget.IsValid()) return PinWidget->GetNodeOffset();

	return FVector2D::ZeroVector;
}

FVector2D UTBManagerSubsystem::GetExecutableNodeTargetPosition(const TBNode& Node)
{

}
