#include "MapLoader.h"
// #include "GameEnemies.h"
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <math.h>

using namespace std;

/*
static bool IsLegacyEnemyId(int id)
{
    return id == 6;
}

static int NormalizeGameEnemyId(int id)
{
    return IsLegacyEnemyId(id) ? GAME_ENEMY_THROW : id;
}
*/
#pragma region ===== マップ配列 =====

int GameFloorMap[BLOCK_NUM_Y][BLOCK_NUM_Z][BLOCK_NUM_X];
int GameFloorRot[BLOCK_NUM_Y][BLOCK_NUM_Z][BLOCK_NUM_X];

int GameWallMapA[BLOCK_NUM_Y][BLOCK_NUM_Z][BLOCK_NUM_X];
int GameWallMapB[BLOCK_NUM_Y][BLOCK_NUM_Z][BLOCK_NUM_X];
int GameWallRotA[BLOCK_NUM_Y][BLOCK_NUM_Z][BLOCK_NUM_X];
int GameWallRotB[BLOCK_NUM_Y][BLOCK_NUM_Z][BLOCK_NUM_X];

int GameCornerMap[BLOCK_NUM_Y][BLOCK_NUM_Z][BLOCK_NUM_X];
int GameCornerRot[BLOCK_NUM_Y][BLOCK_NUM_Z][BLOCK_NUM_X];

int GameDecoMap[BLOCK_NUM_Y][BLOCK_NUM_Z][BLOCK_NUM_X];
int GameDecoRot[BLOCK_NUM_Y][BLOCK_NUM_Z][BLOCK_NUM_X];

int GameEventMap[BLOCK_NUM_Y][BLOCK_NUM_Z][BLOCK_NUM_X];
// 2026-06-11: ゲーム側でも敵配置をEVENTとは別に保持するため追加。
// int GameEnemyMap[BLOCK_NUM_Y][BLOCK_NUM_Z][BLOCK_NUM_X];
// 2026-05-13: エディターで保存した手動当たり判定をゲーム側で使うため追加。
int GameCollisionMap[BLOCK_NUM_Y][BLOCK_NUM_Z][BLOCK_NUM_X];
// 2026-05-20: エディターで直接調整したBoxCollider風の手動当たり判定形状をゲーム側でも使う。
int GameCollisionBoxOffsetXMap[BLOCK_NUM_Y][BLOCK_NUM_Z][BLOCK_NUM_X];
int GameCollisionBoxOffsetZMap[BLOCK_NUM_Y][BLOCK_NUM_Z][BLOCK_NUM_X];
int GameCollisionBoxSizeXMap[BLOCK_NUM_Y][BLOCK_NUM_Z][BLOCK_NUM_X];
int GameCollisionBoxSizeZMap[BLOCK_NUM_Y][BLOCK_NUM_Z][BLOCK_NUM_X];
// 2026-05-13: エディターで保存した辺当たり判定をゲーム側で使うため追加。
int GameCollisionEdgeMap[BLOCK_NUM_Y][BLOCK_NUM_Z][BLOCK_NUM_X];
// 2026-05-13: 辺当たり判定の長さ/厚みをゲーム側で再現するため追加。
int GameCollisionEdgeScaleMap[BLOCK_NUM_Y][BLOCK_NUM_Z][BLOCK_NUM_X][4];
int GameCollisionEdgeThicknessMap[BLOCK_NUM_Y][BLOCK_NUM_Z][BLOCK_NUM_X][4];
// 2026-05-13: コーナー当たり判定の長さ/厚み/奥行をゲーム側で再現するため追加。
int GameCollisionCornerScaleMap[BLOCK_NUM_Y][BLOCK_NUM_Z][BLOCK_NUM_X];
int GameCollisionCornerThicknessMap[BLOCK_NUM_Y][BLOCK_NUM_Z][BLOCK_NUM_X];
int GameCollisionCornerOffsetMap[BLOCK_NUM_Y][BLOCK_NUM_Z][BLOCK_NUM_X];
// 2026-05-18: 迷路ゲームらしく、プレイヤーが通ったセルだけミニマップへ広げて表示するため追加。
bool GameMiniMapVisited[BLOCK_NUM_Y][BLOCK_NUM_Z][BLOCK_NUM_X];
// 2026-05-11: エディターで保存した手動当たり判定をゲーム側で使うため追加。`r`n//int GameCollisionMap[BLOCK_NUM_Y][BLOCK_NUM_Z][BLOCK_NUM_X];
// 2026-05-11: エディターで保存した辺単位の当たり判定をゲーム側で使うため追加。
//int GameCollisionEdgeMap[BLOCK_NUM_Y][BLOCK_NUM_Z][BLOCK_NUM_X];
// 2026-05-11: エディターで伸縮した辺当たり判定の長さ倍率をゲーム側で使うため追加。
//int GameCollisionEdgeScaleMap[BLOCK_NUM_Y][BLOCK_NUM_Z][BLOCK_NUM_X][4];
// 2026-05-11: エディターで編集した辺当たり判定の厚さ倍率をゲーム側で使うため追加。
//int GameCollisionEdgeThicknessMap[BLOCK_NUM_Y][BLOCK_NUM_Z][BLOCK_NUM_X][4];
// 2026-05-13: エディターで編集したコーナー当たり判定の長さ・厚み・奥行をゲーム側で使うため追加。
//int GameCollisionCornerScaleMap[BLOCK_NUM_Y][BLOCK_NUM_Z][BLOCK_NUM_X];
//int GameCollisionCornerThicknessMap[BLOCK_NUM_Y][BLOCK_NUM_Z][BLOCK_NUM_X];
//int GameCollisionCornerOffsetMap[BLOCK_NUM_Y][BLOCK_NUM_Z][BLOCK_NUM_X];
/*// 2026-05-11: エディターで編集した角用の辺当たり判定の厚さ倍率をゲーム側で使うため追加。
int CollisionCornerScaleMap[BLOCK_NUM_Y][BLOCK_NUM_Z][BLOCK_NUM_X];
// 2026-05-11: エディターで編集した角用の辺当たり判定の厚さ倍率をゲーム側で使うため追加。
int CollisionCornerThicknessMap[BLOCK_NUM_Y][BLOCK_NUM_Z][BLOCK_NUM_X];*/
#pragma endregion

#pragma region ===== モデル別当たり判定 =====

enum CollisionType
{
    COLL_NONE,
    COLL_CIRCLE,
    COLL_BOX,
    COLL_ARC,
    COLL_WALL,
    COLL_STAIRS
};

struct CollisionInfo
{
    CollisionType type;

    float radius;

    float width;
    float depth;


};

CollisionInfo collisionTable[MODEL_MAX];

#pragma endregion

#pragma region ===== 状態 / リソース =====

int gameModelHandles[MODEL_MAX];

int gameCurrentMapIndex = 1;
// 2026-05-27: タイトルで選んだ保存名CSVを読み込むため保持する。空なら従来のmap番号を使う。
//char gameCurrentMapName[64] = "";
// 2026-05-20: 階段で層をまたぐたびに、生成・描画・当たり判定の対象にする層を切り替えるため保持する。
int gameLoadedLayer = 0;

int gameStartX = 0;
int gameStartY = 0;
int gameStartZ = 0;

int gameGoalX = 5;
int gameGoalY = 0;
int gameGoalZ = 5;

#pragma endregion


#pragma region ===== 共通処理 =====

static float RotToRad(int rot)
{
    return rot * 3.14159265f * 0.5f;
}

static vector<string> SplitCSV(const string& line)
{
    vector<string> result;
    string item;
    stringstream ss(line);

    while (getline(ss, item, ','))
    {
        result.push_back(item);
    }

    return result;
}

bool IsGameMapPosValid(int y, int z, int x)
{
    return
        y >= 0 && y < BLOCK_NUM_Y &&
        z >= 0 && z < BLOCK_NUM_Z &&
        x >= 0 && x < BLOCK_NUM_X;
}

static int WorldToCell(float value)
{
    return (int)floorf(value / BLOCK_SIZE);
}

static float Clamp01(float v)
{
    if (v < 0.0f) return 0.0f;
    if (v > 1.0f) return 1.0f;
    return v;
}

int GetMapLayerFromWorldY(float worldY)
{
    int layer = (int)floorf((worldY + 1.0f) / BLOCK_SIZE);

    if (layer < 0) layer = 0;
    if (layer >= BLOCK_NUM_Y) layer = BLOCK_NUM_Y - 1;

    return layer;
    
}

void SetGameLoadedLayer(int layer)
{
    if (layer < 0) layer = 0;
    if (layer >= BLOCK_NUM_Y) layer = BLOCK_NUM_Y - 1;

    gameLoadedLayer = layer;
}

int GetGameLoadedLayer()
{
    return gameLoadedLayer;
}

static CollisionType ParseCollisionType(const string& text)
{
    // 2026-05-11: 新しいモデルを追加した時に、collision_config.csv から既定当たり判定を設定できるよう追加。
    if (text == "CIRCLE") return COLL_CIRCLE;
    if (text == "BOX") return COLL_BOX;
    if (text == "ARC") return COLL_ARC;
    if (text == "WALL") return COLL_WALL;
    if (text == "STAIRS") return COLL_STAIRS;
    return COLL_NONE;
}


static void LoadGameModelConfig()
{
    // 2026-05-20: 今後モデルを増やしやすくするため、ゲーム側のモデル読み込みも model_config.csv から行う。
    ifstream ifs("model_config.csv");
    if (!ifs)
    {
        printfDx("model_config.csv load failed\n");
        return;
    }

    string line;
    while (getline(ifs, line))
    {
        if (line.empty() || line[0] == '#')
            continue;

        vector<string> cols = SplitCSV(line);
        if (cols.size() < 3 || cols[0] == "id")
            continue;

        int id = stoi(cols[0]);
        if (id < 0 || id >= MODEL_MAX)
            continue;

        string modelPath = cols[2];
        if (modelPath == "-" || gameModelHandles[id] != -1)
            continue;

        float scaleX = cols.size() >= 5 ? stof(cols[4]) : 1.0f;
        float scaleY = cols.size() >= 6 ? stof(cols[5]) : scaleX;
        float scaleZ = cols.size() >= 7 ? stof(cols[6]) : scaleX;

        gameModelHandles[id] = MV1LoadModel(modelPath.c_str());
        if (gameModelHandles[id] != -1)
        {
            MV1SetScale(gameModelHandles[id], VGet(scaleX, scaleY, scaleZ));
        }
    }
}
static bool IsStairsId(int id)
{
    return id == 5;
}

static bool IsLowPlatformDecoId(int id)
{
    return id == 6;
}

static bool IsOverheadDecoId(int id)
{
    return id == 7;
}

static float GetLowPlatformTopOffset()
{
    // 装飾6は低い足場なので、通常床より少し高い位置を歩ける上面にする。
    return BLOCK_SIZE * 0.18f;
}

static float GetHighPlatformPostOffset()
{
    // 装飾7は中央をくぐれるようにして、四つ角だけ柱として当たり判定を置く。
    return BLOCK_SIZE * 0.12f;
}

static float GetHighPlatformPostRadius()
{
    // 柱が太すぎると中央通路を邪魔するので、角だけ止まる程度の半径に抑える。
    return BLOCK_SIZE * 0.08f;
}

static int GetStairsDrawRot(int rot)
{
    // 2026-05-19: 階段モデルの見た目の向きが配置向きと逆だったため、階段だけ180度反転して描画する。
    return (rot + 2) % 4;
}

static int GetGameModelDrawRot(int id, int rot)
{
    // 2026-06-29: 壁モデルID 1/3は横向きの時だけ外側を向くため、回転1/3だけ補正する。
    if ((id == 1 || id == 3) && (rot & 1))
        return (rot + 2) & 3;

    if (IsStairsId(id))
        return GetStairsDrawRot(rot);

    return rot;
}

static int GetCollisionEdgeBit(int edge)
{
    // 2026-05-11: エディターで保存した辺当たり判定のビットをLoader側で読むため追加。
    return 1 << (edge % 4);
}

static int GetManualCollisionEdgeScale(int y, int z, int x, int edge)
{
    // 2026-05-11: 伸縮編集した辺当たり判定の長さ倍率を取得するため追加。
    if (!IsGameMapPosValid(y, z, x))
        return 100;

    return GameCollisionEdgeScaleMap[y][z][x][edge % 4];
}

static int GetManualCollisionEdgeThickness(int y, int z, int x, int edge)
{
    // 2026-05-11: 厚さ編集した辺当たり判定の倍率を取得するため追加。
    if (!IsGameMapPosValid(y, z, x))
        return 100;

    return GameCollisionEdgeThicknessMap[y][z][x][edge % 4];
}

static bool HasManualCollisionEdge(int y, int z, int x, int edge)
{
    // 2026-05-11: 壁モデルとは別に、手動で置いた辺当たり判定を移動判定へ反映するため追加。
    if (!IsGameMapPosValid(y, z, x))
        return true;

    if (GameCollisionEdgeMap[y][z][x] < 0)
        return false;

    return (GameCollisionEdgeMap[y][z][x] & GetCollisionEdgeBit(edge)) != 0;
}

#pragma endregion


#pragma region ===== 座標変換 =====

VECTOR GetStartPosition()
{
    return VGet(
        gameStartX * BLOCK_SIZE + BLOCK_SIZE * 0.5f,
        gameStartY * BLOCK_SIZE,
        gameStartZ * BLOCK_SIZE + BLOCK_SIZE * 0.5f
    );
}

static VECTOR GetBlockCenterPosition(int x, int y, int z)
{
    return VGet(
        x * BLOCK_SIZE + BLOCK_SIZE * 0.5f,
        y * BLOCK_SIZE,
        z * BLOCK_SIZE + BLOCK_SIZE * 0.5f
    );
}

static VECTOR GetWallPosition(int x, int y, int z, int rot)
{
    float px = x * BLOCK_SIZE + BLOCK_SIZE * 0.5f;
    float py = y * BLOCK_SIZE;
    float pz = z * BLOCK_SIZE + BLOCK_SIZE * 0.5f;
    // 2026-06-29: エディターの配置感に合わせ、壁モデルは選択したセルの辺に置く。

    switch (rot % 4)
    {
    case 0:
        pz = z * BLOCK_SIZE;
        break;
    case 1:
        px = (x + 1) * BLOCK_SIZE;
        break;
    case 2:
        pz = (z + 1) * BLOCK_SIZE;
        break;
    case 3:
        px = x * BLOCK_SIZE;
        break;
    }

    return VGet(px, py, pz);
}

static VECTOR GetModelDrawPosition(int tab, int x, int y, int z, int rot)
{
    if (tab == WALL)
        return GetWallPosition(x, y, z, rot);

    VECTOR pos = GetBlockCenterPosition(x, y, z);

    // 階段モデルだけ位置補正
   /* if (tab == DECO && GameDecoMap[y][z][x] == 5)
    {
        float offset = BLOCK_SIZE * -0.25f;

        switch (rot % 4)
        {
        case 0: // 上向き
            pos.z += offset;
            break;

        case 1: // 右向き
            pos.x += offset;
            break;

        case 2: // 下向き
            pos.z -= offset;
            break;

        case 3: // 左向き
            pos.x -= offset;
            break;
        }
    }*/

    return pos;
}
#pragma endregion

//カメラ当たり判定用のやつ追加
//佐藤龍波
#include "MapLoaderCameraCollision.inl"

static void ClearGameMap()
{
    for (int y = 0; y < BLOCK_NUM_Y; y++)
    {
        for (int z = 0; z < BLOCK_NUM_Z; z++)
        {
            for (int x = 0; x < BLOCK_NUM_X; x++)
            {
                GameFloorMap[y][z][x] = -1;
                GameWallMapA[y][z][x] = -1;
                GameWallMapB[y][z][x] = -1;
                GameCornerMap[y][z][x] = -1;
                GameDecoMap[y][z][x] = -1;
                GameEventMap[y][z][x] = -1;
                // 2026-06-11: マップ再読込時に前の敵配置を残さないため追加。
                //GameEnemyMap[y][z][x] = -1;
                // 2026-05-11: マップ再読込時に前の手動当たり判定を残さないため追加。
                GameCollisionMap[y][z][x] = -1;
                // 2026-05-20: マップ再読込時にBoxCollider風の調整値も標準値へ戻す。
                GameCollisionBoxOffsetXMap[y][z][x] = 0;
                GameCollisionBoxOffsetZMap[y][z][x] = 0;
                GameCollisionBoxSizeXMap[y][z][x] = (int)BLOCK_SIZE;
                GameCollisionBoxSizeZMap[y][z][x] = (int)BLOCK_SIZE;
                // 2026-05-11: マップ再読込時に前の辺当たり判定を残さないため追加。
                GameCollisionEdgeMap[y][z][x] = -1;
                // 2026-05-13: マップ未読込時にコーナー当たり調整値を標準値へ戻すため追加。
                GameCollisionCornerScaleMap[y][z][x] = 100;
                GameCollisionCornerThicknessMap[y][z][x] = 100;
                GameCollisionCornerOffsetMap[y][z][x] = 0;
                // 2026-06-28: 別マップや再プレイ時に前回の探索済み表示が残り、ミニマップが壊れて見えるため再読込でも初期化する。
                GameMiniMapVisited[y][z][x] = false;
                for (int edge = 0; edge < 4; edge++)
                {
                    GameCollisionEdgeScaleMap[y][z][x][edge] = 100;
                    GameCollisionEdgeThicknessMap[y][z][x][edge] = 100;
                }

                GameFloorRot[y][z][x] = 0;
                GameWallRotA[y][z][x] = 0;
                GameWallRotB[y][z][x] = 0;
                GameCornerRot[y][z][x] = 0;
                GameDecoRot[y][z][x] = 0;
            }
        }
    }
}

#pragma endregion


#pragma region ===== CSV読込 =====

void LoadGameMap()
{
    ClearGameMap();

    char fileName[128];
    if (gameCurrentMapName[0] != '\0')
        sprintf_s(fileName, sizeof(fileName), "%s.csv", gameCurrentMapName);
    else
        sprintf_s(fileName, sizeof(fileName), "map%d.csv", gameCurrentMapIndex);

    ifstream ifs(fileName);

    if (!ifs)
    {
        printfDx("game map load failed: %s\n", fileName);
        return;
    }

    string section;
    string line;

    while (getline(ifs, line))
    {
        if (line.empty())
            continue;

        if (line[0] == '#')
            continue;

        if (line[0] == '[')
        {
            section = line;
            continue;
        }

        vector<string> cols = SplitCSV(line);

        if (cols.empty())
            continue;

        if (cols[0] == "SIZE")
            continue;

        if (cols[0] == "START" && cols.size() >= 4)
        {
            gameStartX = stoi(cols[1]);
            gameStartY = stoi(cols[2]);
            gameStartZ = stoi(cols[3]);
            continue;
        }

        if (cols[0] == "GOAL" && cols.size() >= 4)
        {
            gameGoalX = stoi(cols[1]);
            gameGoalY = stoi(cols[2]);
            gameGoalZ = stoi(cols[3]);
            continue;
        }

        if (cols[0] == "y")
            continue;

        if (section == "[FLOOR]" && cols.size() >= 5)
        {
            int y = stoi(cols[0]);
            int z = stoi(cols[1]);
            int x = stoi(cols[2]);

            if (IsGameMapPosValid(y, z, x))
            {
                GameFloorMap[y][z][x] = stoi(cols[3]);
                GameFloorRot[y][z][x] = stoi(cols[4]);
            }
        }
        else if (section == "[WALL_A]" && cols.size() >= 5)
        {
            int y = stoi(cols[0]);
            int z = stoi(cols[1]);
            int x = stoi(cols[2]);

            if (IsGameMapPosValid(y, z, x))
            {
                GameWallMapA[y][z][x] = stoi(cols[3]);
                GameWallRotA[y][z][x] = stoi(cols[4]);
            }
        }
        else if (section == "[WALL_B]" && cols.size() >= 5)
        {
            int y = stoi(cols[0]);
            int z = stoi(cols[1]);
            int x = stoi(cols[2]);

            if (IsGameMapPosValid(y, z, x))
            {
                GameWallMapB[y][z][x] = stoi(cols[3]);
                GameWallRotB[y][z][x] = stoi(cols[4]);
            }
        }
        else if (section == "[CORNER]" && cols.size() >= 5)
        {
            int y = stoi(cols[0]);
            int z = stoi(cols[1]);
            int x = stoi(cols[2]);

            if (IsGameMapPosValid(y, z, x))
            {
                GameCornerMap[y][z][x] = stoi(cols[3]);
                GameCornerRot[y][z][x] = stoi(cols[4]);
                // 2026-05-13: コーナー当たり判定の調整値をLoaderでも読むため追加。
                GameCollisionCornerScaleMap[y][z][x] = cols.size() > 5 ? stoi(cols[5]) : 100;
                GameCollisionCornerThicknessMap[y][z][x] = cols.size() > 6 ? stoi(cols[6]) : 100;
                GameCollisionCornerOffsetMap[y][z][x] = cols.size() > 7 ? stoi(cols[7]) : 0;
            }
        }
        else if (section == "[DECO]" && cols.size() >= 5)
        {
            int y = stoi(cols[0]);
            int z = stoi(cols[1]);
            int x = stoi(cols[2]);

            if (IsGameMapPosValid(y, z, x))
            {
                GameDecoMap[y][z][x] = stoi(cols[3]);
                GameDecoRot[y][z][x] = stoi(cols[4]);
            }
        }
        else if (section == "[COLLISION]" && cols.size() >= 4)
        {
            int y = stoi(cols[0]);
            int z = stoi(cols[1]);
            int x = stoi(cols[2]);

            // 2026-05-11: エディターで置いた手動当たり判定をLoaderでも読むため追加。
            if (IsGameMapPosValid(y, z, x))
            {
                GameCollisionMap[y][z][x] = stoi(cols[3]);
                // 2026-05-20: 古いCSVではセルいっぱい、新しいCSVでは保存したBoxCollider風の形で復元する。
                GameCollisionBoxOffsetXMap[y][z][x] = cols.size() > 4 ? stoi(cols[4]) : 0;
                GameCollisionBoxOffsetZMap[y][z][x] = cols.size() > 5 ? stoi(cols[5]) : 0;
                GameCollisionBoxSizeXMap[y][z][x] = cols.size() > 6 ? stoi(cols[6]) : (int)BLOCK_SIZE;
                GameCollisionBoxSizeZMap[y][z][x] = cols.size() > 7 ? stoi(cols[7]) : (int)BLOCK_SIZE;
            }
        }
        else if (section == "[COLLISION_EDGE]" && cols.size() >= 4)
        {
            int y = stoi(cols[0]);
            int z = stoi(cols[1]);
            int x = stoi(cols[2]);

            // 2026-05-11: 保存済みの辺単位当たり判定をLoaderでも読むため追加。
            if (IsGameMapPosValid(y, z, x))
            {
                GameCollisionEdgeMap[y][z][x] = stoi(cols[3]);
                // 2026-05-11: 辺当たり判定の伸縮値もLoaderで読むため追加。
                for (int edge = 0; edge < 4; edge++)
                {
                    GameCollisionEdgeScaleMap[y][z][x][edge] = cols.size() >= 8 ? stoi(cols[4 + edge]) : 100;
                    GameCollisionEdgeThicknessMap[y][z][x][edge] = cols.size() >= 12 ? stoi(cols[8 + edge]) : 100;
                }
            }
        }
        else if (section == "[EVENT]" && cols.size() >= 4)
        {
            int y = stoi(cols[0]);
            int z = stoi(cols[1]);
            int x = stoi(cols[2]);

            if (IsGameMapPosValid(y, z, x))
            {
                int id = stoi(cols[3]);
                // エネミー関連処理を一時停止中。旧EVENT ID 6は敵扱いなので読み込まない。
                if (id != 6)
                {
                    GameEventMap[y][z][x] = id;
                }
            }
        }
        /*else if (section == "[ENEMY]" && cols.size() >= 4)
        {
            int y = stoi(cols[0]);
            int z = stoi(cols[1]);
            int x = stoi(cols[2]);

            if (IsGameMapPosValid(y, z, x))
            {
                // 2026-06-12: 新旧どちらのENEMY IDでもゲーム内では敵専用IDへ正規化する。
                GameEnemyMap[y][z][x] = NormalizeGameEnemyId(stoi(cols[3]));
            }
        }*/
    }

    // 2026-05-20: マップロード直後は、スタート地点の層だけを生成対象として扱う。
    SetGameLoadedLayer(gameStartY);
}

#pragma endregion


#pragma region ===== 初期化 / 更新 =====


void UpdateGameMap()
{
    /*if (CheckHitKey(KEY_INPUT_RIGHT))
    {
        gameCurrentMapIndex++;

        LoadGameMap();
    }

    if (CheckHitKey(KEY_INPUT_LEFT))
    {
        if (gameCurrentMapIndex > 1)
        {
            gameCurrentMapIndex--;
        }

        LoadGameMap();
    }*/
}

#pragma endregion
static void InitCollisionTable()
{
    for (int i = 0; i < MODEL_MAX; i++)
    {
        collisionTable[i].type = COLL_NONE;

        collisionTable[i].radius = 0.0f;

        collisionTable[i].width = 0.0f;
        collisionTable[i].depth = 0.0f;
    }

    collisionTable[1].type = COLL_WALL;

    collisionTable[2].type = COLL_CIRCLE;
    collisionTable[2].radius = BLOCK_SIZE * 0.3f;

    collisionTable[5].type = COLL_STAIRS;

    collisionTable[6].type = COLL_BOX;
    collisionTable[6].width = BLOCK_SIZE * 0.35f;
    collisionTable[6].depth = BLOCK_SIZE * 0.35f;

    collisionTable[7].type = COLL_NONE;

    ifstream ifs("collision_config.csv");
    if (!ifs)
        return;

    string line;
    while (getline(ifs, line))
    {
        if (line.empty() || line[0] == '#')
            continue;

        vector<string> cols = SplitCSV(line);
        if (cols.size() < 2 || cols[0] == "id")
            continue;

        int id = stoi(cols[0]);
        if (id < 0 || id >= MODEL_MAX)
            continue;

        // 2026-05-11: 追加モデルの既定当たり判定を、コード変更なしでLoaderへ反映するため追加。
        collisionTable[id].type = ParseCollisionType(cols[1]);
        collisionTable[id].radius = cols.size() >= 3 ? stof(cols[2]) : 0.0f;
        collisionTable[id].width = cols.size() >= 4 ? stof(cols[3]) : 0.0f;
        collisionTable[id].depth = cols.size() >= 5 ? stof(cols[4]) : 0.0f;
    }
}
void InitGameMap()
{
    // ミニマップ探索状態初期化
    for (int y = 0; y < BLOCK_NUM_Y; y++)
    {
        for (int z = 0; z < BLOCK_NUM_Z; z++)
        {
            for (int x = 0; x < BLOCK_NUM_X; x++)
            {
                GameMiniMapVisited[y][z][x] = false;
            }
        }
    }

    for (int i = 0; i < MODEL_MAX; i++)
    {
        gameModelHandles[i] = -1;
    }

    LoadGameModelConfig();

    for (int i = 0; i < MODEL_MAX; i++)
    {
        if (gameModelHandles[i] != -1)
        {
            MV1SetupCollInfo(gameModelHandles[i], -1);
        }
    }

    LoadGameMap();

    InitCollisionTable();
}
#pragma region ===== 描画 =====

#include "MapLoaderRendering.inl"

#include "MapLoaderMovementCollision.inl"

