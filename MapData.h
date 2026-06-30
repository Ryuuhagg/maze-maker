#pragma once
#include "DxLib.h"

/// <summary>
/// このスクリプトはEditorとゲームに表示するための2つで必要となるためずっとオン
/// </summary>

#pragma region ===== 画面設定 =====

const int SCREEN_W = 1280;
const int SCREEN_H = 720;
const int UI_WIDTH = 260;

#pragma endregion


#pragma region ===== マップサイズ =====

const int BLOCK_NUM_X = 20;
const int BLOCK_NUM_Y = 8;
const int BLOCK_NUM_Z = 20;

const float BLOCK_SIZE = 400.0f;

const float cornerRadius = BLOCK_SIZE * 0.18f;

#pragma endregion


#pragma region ===== タブ種類 =====

enum TabType
{
    FLOOR = 0,
    WALL,
    CORNER,
    DECO,
    ENEMY,
    EVENT,
    TAB_MAX
};

#pragma endregion


#pragma region ===== モデル数上限 =====

const int MODEL_MAX = 32;

#pragma endregion


#pragma region ===== マップデータ =====

extern int FloorMap[BLOCK_NUM_Y][BLOCK_NUM_Z][BLOCK_NUM_X];
extern int FloorRot[BLOCK_NUM_Y][BLOCK_NUM_Z][BLOCK_NUM_X];

extern int WallMapA[BLOCK_NUM_Y][BLOCK_NUM_Z][BLOCK_NUM_X];
extern int WallMapB[BLOCK_NUM_Y][BLOCK_NUM_Z][BLOCK_NUM_X];

extern int WallRotA[BLOCK_NUM_Y][BLOCK_NUM_Z][BLOCK_NUM_X];
extern int WallRotB[BLOCK_NUM_Y][BLOCK_NUM_Z][BLOCK_NUM_X];

extern int CornerMap[BLOCK_NUM_Y][BLOCK_NUM_Z][BLOCK_NUM_X];
extern int CornerRot[BLOCK_NUM_Y][BLOCK_NUM_Z][BLOCK_NUM_X];

extern int DecoMap[BLOCK_NUM_Y][BLOCK_NUM_Z][BLOCK_NUM_X];
extern int DecoRot[BLOCK_NUM_Y][BLOCK_NUM_Z][BLOCK_NUM_X];

// 2026-06-11: EVENTとENEMYを別CSV/別配列で保存するため追加。
extern int EnemyMap[BLOCK_NUM_Y][BLOCK_NUM_Z][BLOCK_NUM_X];
extern int EventMap[BLOCK_NUM_Y][BLOCK_NUM_Z][BLOCK_NUM_X];
// 2026-05-11: エディターで手動設定した当たり判定を保存し、Loader側でも使うため追加。
extern int CollisionMap[BLOCK_NUM_Y][BLOCK_NUM_Z][BLOCK_NUM_X];
// 2026-05-20: 手動当たり判定をBoxCollider風に直接編集するため、中心オフセットとサイズを共有する。
extern int CollisionBoxOffsetXMap[BLOCK_NUM_Y][BLOCK_NUM_Z][BLOCK_NUM_X];
extern int CollisionBoxOffsetZMap[BLOCK_NUM_Y][BLOCK_NUM_Z][BLOCK_NUM_X];
extern int CollisionBoxSizeXMap[BLOCK_NUM_Y][BLOCK_NUM_Z][BLOCK_NUM_X];
extern int CollisionBoxSizeZMap[BLOCK_NUM_Y][BLOCK_NUM_Z][BLOCK_NUM_X];
// 2026-05-11: Loaderの壁ライン判定と同じ「辺単位」の当たり判定を編集・保存するため追加。
extern int CollisionEdgeMap[BLOCK_NUM_Y][BLOCK_NUM_Z][BLOCK_NUM_X];
// 2026-05-11: 辺当たり判定を伸ばしたり縮めたりする長さ倍率を保存するため追加。
extern int CollisionEdgeScaleMap[BLOCK_NUM_Y][BLOCK_NUM_Z][BLOCK_NUM_X][4];
// 2026-05-11: 辺当たり判定の厚さ倍率を編集・保存するため追加。
extern int CollisionEdgeThicknessMap[BLOCK_NUM_Y][BLOCK_NUM_Z][BLOCK_NUM_X][4];
// 2026-05-13: コーナー当たり判定の長さ・厚み・奥行を保存/読込/Undoで共有するため追加。
extern int CollisionCornerScaleMap[BLOCK_NUM_Y][BLOCK_NUM_Z][BLOCK_NUM_X];
extern int CollisionCornerThicknessMap[BLOCK_NUM_Y][BLOCK_NUM_Z][BLOCK_NUM_X];
extern int CollisionCornerOffsetMap[BLOCK_NUM_Y][BLOCK_NUM_Z][BLOCK_NUM_X];

#pragma endregion


#pragma region ===== エディタ状態 =====

extern int currentLayer;
extern int currentRot;
extern int currentMapIndex;
// 2026-05-26: 名前を付けて保存した後、同じCSVへ上書き保存するため現在のマップ名を共有する。
extern char gameCurrentMapName[64];
// 2026-05-26: マップ選択時に、番号ではなくCSV名で選べるよう現在選択中の名前を共有する。
extern char selectedMapName[64];

extern int currentTab;
extern int selectedModel;
// 2026-06-25: コピー配置中の案内をUI下部に出すため、コピー状態をUI描画側から参照する。
extern bool pasteMode;
extern int copyRotation;
extern int copySizeX;
extern int copySizeZ;
// 2026-06-02: 通常編集画面とコライダー専用画面をUI/入力側で共有するため追加。
extern int editorScreenMode;
// 2026-06-02: 右側素材一覧のスクロール量をMapEditor/MapUIで共有するため追加。
extern int editorPaletteScrollY;

#pragma endregion


#pragma region ===== マウスカーソル位置 =====

extern int hoverX;
extern int hoverZ;

#pragma endregion


#pragma region ===== カメラ =====

extern float camRotY;
extern float camRotX;
extern float camDist;

#pragma endregion


#pragma region ===== モデル / UI画像 =====

extern int modelHandles[MODEL_MAX];
extern int paletteTex[MODEL_MAX];

#pragma endregion


#pragma region ===== UI用モデル一覧 =====

extern int tabModelList[TAB_MAX][16];
extern int tabModelCount[TAB_MAX];

#pragma endregion

#pragma region ===== 当たり判定 =====

bool CanMoveCellToCell(int y, int fromX, int fromZ, int toX, int toZ);
bool IsCellBlocked(int y, int z, int x);

#pragma endregion

#pragma region ===== 表示設定 =====

extern bool showGrid;
extern bool brushMode;
// 2026-05-11: 当たり判定の編集モードとデバッグ表示をエディターで切り替えるため追加。
extern bool showCollisionDebug;
extern bool collisionEditMode;
// 2026-05-11: 当たり判定編集をセル単位/辺単位で切り替えるため追加。
extern bool collisionEdgeEditMode;
// 2026-05-13: ホイール編集を長さモード/厚みモードで切り替えるため追加。
extern bool collisionDepthEditMode;
// 2026-05-20: 右側UIから選択中のBOXコライダーを調整するため共有する。
extern int selectedCollisionLayer;
extern int selectedCollisionX;
extern int selectedCollisionZ;
// 2026-06-02: コライダー専用画面から通常画面へ戻る時にドラッグ状態を確実に止めるため追加。
extern bool collisionBoxDragging;

#pragma endregion


#pragma region ===== 開始地点 =====

extern int startX;
extern int startY;
extern int startZ;

#pragma endregion


#pragma region ===== ゴール地点 =====

extern int goalX;
extern int goalY;
extern int goalZ;

#pragma endregion


#pragma region ===== 共通関数 =====

int GetSelectedModel();

#pragma endregion
