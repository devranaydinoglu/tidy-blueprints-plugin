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
	FVector2D Size;

public:
	TBNode()
		: Size(FVector2D::ZeroVector)
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

	// Padding applied to the edges of the collection
	int32 Padding;

public:
	TBCollection()
		: Index(-1), Padding(0)
	{}

	int32 CalculatePadding();
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

enum class CollectionLayoutType
{
	STACKED,
	LIST
};

UCLASS()
class TIDYBLUEPRINTS_API UTBManagerSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()

private:
	FBlueprintEditor* BlueprintEditor = nullptr;

	FGraphPanelSelectionSet SelectedNodes;

	// Config properties
	CollectionLayoutType CollectionLayoutType = CollectionLayoutType::STACKED;

	int32 CollectionNodesPaddingX = 3;
	int32 CollectionNodesPaddingY = 6;

public:

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
	void SetNodePosition(UEdGraphNode* Node, const FVector2D& NewPosition);

	/**
	 * Sets the positions of the nodes of a collection on the blueprint graph.
	 */
	void SetCollectionNodePositions(const TBCluster& Cluster);

	/**
	 * Gets the current blueprint editor.
	 */
	void SetBlueprintEditor();

	/**
	 * Gets the graph editor based on the active top level window
	 *
	 * @return 
	 */
	TSharedPtr<SGraphEditor> GetCurrentGraphEditor();

	/**
	 * Gets the current graph panel from the current blueprint editor.
	 */
	SGraphPanel* GetCurrentGraphPanel();

	/**
	 * Gets all the selected nodes in the current graph.
	 */
	void SetSelectedNodes();

	/**
	 * Gets the slate widget of a graph node.
	 *
	 * @return Slate widget
	 */
	SGraphNode* GetNodeWidget(const UEdGraphNode* Node);

	/**
	 * Gets the offset of a pin in relation to its owning node.
	 *
	 * @param Node Owning node
	 * @return Pin offset
	 */
	FVector2D GetPinOffset(const UEdGraphPin* Pin);

	FVector2D GetExecutableNodeTargetPosition(const TBNode& Node);
};
