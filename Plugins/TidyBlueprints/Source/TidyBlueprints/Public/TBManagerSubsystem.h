// Copyright 2023 devran. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "TBManagerSubsystem.generated.h"

class UEdGraphNode;
class FBlueprintEditor;

class UTBPin
{
	// Name of the pin as seen on the node
	FName PinName;

	// Data type of the pin
	FName PinType;

	// Direction of the pin on the node (input or output)
	TEnumAsByte<enum EEdGraphPinDirection> PinDirection;

	// Node which owns this pin
	TStrongObjectPtr<UEdGraphNode> OwningNode;
};

class TBNode
{
public:
	TStrongObjectPtr<UEdGraphNode> Node;
	int32 NodeHeight;
	int32 NodeWidth;
};

/**
 * A node connected by execution pins and all of its non-execution input and output nodes
 */
class TBCollection
{
public:
	// The index of this collection in the cluster
	int32 Index;

	// Node which is connected by execution pins
	TBNode ParentNode;

	// Input nodes ordered by X and Y positions on the graph
	TArray<TBNode> InputNodes;

	// Output nodes ordered by X and Y positions on the graph
	TArray<TBNode> OutputNodes;
};

/**
 * A sequence of nodes which are connected through pins.
 * It should not include nodes which are not part of the execution flow of the nodes sequence.
 */
class TBCluster
{
public:
	// First node in the sequence
	TBNode StartingNode;

	TArray<TBCollection> Collections;
};

UCLASS()
class TIDYBLUEPRINTS_API UTBManagerSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()

private:
	FBlueprintEditor* CurrentBlueprintEditor = nullptr;

public:

	/**
	 * Gets the current blueprint editor.
	 */
	FBlueprintEditor* GetCurrentBlueprintEditor();

	/**
	 * Gets the current graph.
	 */
	UEdGraph* GetCurrentGraph();

	/**
	 * Gets all the selected nodes in the current graph.
	 */
	FGraphPanelSelectionSet GetSelectedNodes();

	/**
	 * Entry point, called when Tidy Up button is clicked.
	 */
	void StartTidyUp();

private:

	void RecursivelyGetChildNodes(UEdGraphNode* Node, UEdGraphPin* InLinkedPin, TBCollection& Collection);

	TBNode PopulateNodeData(UEdGraphNode* Node);

	/**
	 * Checks whether the node is the first node in the selected nodes' execution sequence.
	 * @return Is first in sequence
	 */
	bool IsNodeFirstInSequence(const UEdGraphNode* Node, FGraphPanelSelectionSet SelectedNodes);

	/**
	 * Checks whether the node is executable by checking its pins.
	 * @return Executable
	 */
	bool IsNodeExecutable(const UEdGraphNode* Node);

	/**
	 * Updates the position of the provided node on the blueprint graph.
	 *
	 * @param Graph Current graph
	 * @param Node Node whose position should be updated
	 * @param NewPosition New coordinates of the node on the blueprint graph
	 */
	void SetNodePosition(UEdGraph* Graph, UEdGraphNode* Node, const FVector2D& NewPosition);

};
