#pragma once
#include "MapData.h"

#pragma region ===== 初期化 =====

void InitEditor();

#pragma endregion


#pragma region ===== 更新 =====

void UpdateEditor();

#pragma endregion


#pragma region ===== 描画 =====

void DrawEditor();

#pragma endregion


#pragma region ===== CSV保存 / 読込 =====

void SaveMap();
void SaveMapAsCurrentName();
void LoadMap(int mapIndex);
void LoadMapByName(const char* mapName);
bool IsEditorMapDirty();
void MarkEditorMapDirty();
void ClearEditorMapDirty();
//void RequestEditorLoadMap(const char* mapName);
#pragma endregion


#pragma region ===== Undo =====

void PushUndo();
void UndoMap();

#pragma endregion


#pragma region ===== 壁処理 =====

void AutoConnectWalls();
void UpdateWallTile(int y, int z, int x);

#pragma endregion


#pragma region ===== UI =====

void DrawEditorUI();
void UpdateEditorUI(int mx, int my, int lClick);
bool UpdateEditorUIWheel(int mx, int my, int wheel);
bool IsEditorWorkspacePoint(int mx, int my);
void SaveEditorFromUI();
void RefreshEditorKitList();
int GetEditorKitCount();
const char* GetEditorKitName(int index);
int GetSelectedEditorKitIndex();
int GetEditorKitThumbnailHandle(int index);
bool IsEditorKitFromCsv(int index);
bool RebuildEditorBaseKitCsv();
bool RefreshSelectedEditorKitThumbnail();
void SelectEditorKit(int index);
void StartEditorKitRegistrationFromSelection();

#pragma endregion


#pragma region ===== 共通処理 =====

void ClearCurrentLayer();
void ResetAllMap();
extern int CollisionCornerScaleMap[BLOCK_NUM_Y][BLOCK_NUM_Z][BLOCK_NUM_X];
extern int CollisionCornerThicknessMap[BLOCK_NUM_Y][BLOCK_NUM_Z][BLOCK_NUM_X];
extern char gameCurrentMapName[64];

#pragma endregion
