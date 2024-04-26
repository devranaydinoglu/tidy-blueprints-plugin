// Copyright 2023 devran. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "TBManagerSubsystem.generated.h"

class UEdGraphNode;
class FBlueprintEditor;

class TBNode
{
public:
	TStrongObjectPtr<UEdGraphNode> Node;
	int32 NodeHeight;
	int32 NodeWidth;

public:
	TBNode()
		: NodeHeight(0), NodeWidth(0)
	{}
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

public:
	TBCollection()
		: Index(-1)
	{}
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

public:
	TBCollection* FindCollection(const UEdGraphNode* Node)
	{
		for (TBCollection& Collection : Collections)
		{
			if (Collection.ParentNode.Node.Get()->GetUniqueID() == Node->GetUniqueID()) return &Collection;
		}

		return nullptr;
	}
};

UCLASS()
class TIDYBLUEPRINTS_API UTBManagerSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()

private:
	FBlueprintEditor* BlueprintEditor = nullptr;

	FGraphPanelSelectionSet SelectedNodes;
public:

	/**
	 * Gets the current blueprint editor.
	 */
	void SetBlueprintEditor();

	/**
	 * Gets all the selected nodes in the current graph.
	 */
	void SetSelectedNodes();

	/**
	 * Entry point, called when Tidy Up button is clicked.
	 */
	void StartTidyUp();

private:
	/**
	 * Recursively traverse the execution sequence starting from the specified node.
	 *
	 * @param Node Starting point
	 * @param CollectionIndex Index to give each collection in the cluster an index based on the execution order
	 * @param SelectedNodes Selected nodes in the graph
	 * @param Cluster Cluster to traverse
	 */
	void TraverseSequence(UEdGraphNode* Node, int32& CollectionIndex, TBCluster& Cluster);

	/**
	 * Recursively get all child nodes of a node
	 *
	 * @param Node Node to get the child of
	 * @param InLinkedPin Pin which the node is linked to
	 * @param Collection Collection to add the node to
	 */
	void GetChildNodes(UEdGraphNode* Node, UEdGraphPin* InLinkedPin, TBCollection& Collection);

	TBNode PopulateNodeData(UEdGraphNode* Node);

	/**
	 * Checks whether the node is the first node in the selected nodes' execution sequence.
	 * @return Is first in sequence
	 */
	bool IsNodeFirstInSequence(const UEdGraphNode* Node);

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
