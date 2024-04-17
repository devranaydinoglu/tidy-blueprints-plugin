// Copyright 2023 devran. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "TBManagerSubsystem.generated.h"

class UEdGraphNode;
class FBlueprintEditor;

UCLASS()
class TIDYBLUEPRINTS_API UTBManagerSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()

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
	void GetSelectedNodes(UEdGraph* Graph);

	/**
	 * Entry point, called when Tidy Up button is clicked.
	 */
	void StartTidyUp();

private:

	/**
	 * Updates the position of the provided node on the blueprint graph.
	 *
	 * @param Graph Current graph
	 * @param Node Node whose position should be updated
	 * @param NewPosition New coordinates of the node on the blueprint graph
	 */
	void SetNodePosition(UEdGraph* Graph, UEdGraphNode* Node, const FVector2D& NewPosition);

};
