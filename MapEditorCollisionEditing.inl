static void ResetCollisionBox(int layer, int z, int x)
{
    // 2026-05-20: 新しく置いた手動当たり判定を、セルいっぱいのBoxColliderとして初期化する。
    if (!IsMapPosValid(layer, z, x))
        return;

    CollisionBoxOffsetXMap[layer][z][x] = 0;
    CollisionBoxOffsetZMap[layer][z][x] = 0;
    CollisionBoxSizeXMap[layer][z][x] = (int)BLOCK_SIZE;
    CollisionBoxSizeZMap[layer][z][x] = (int)BLOCK_SIZE;
}
/*static void RefreshMapNameList()
{
    mapNameCount = 0;

    WIN32_FIND_DATA fd;

    HANDLE h = FindFirstFile("*.csv", &fd);

    if (h == INVALID_HANDLE_VALUE)
        return;

    do
    {
        char name[64];

        strcpy_s(name, fd.cFileName);

        char* dot = strstr(name, ".csv");

        if (dot)
        {
            *dot = '\0';
        }

        strcpy_s(
            mapNameList[mapNameCount],
            name
        );

        mapNameCount++;

    } while (FindNextFile(h, &fd));

    FindClose(h);
}*/
static void GetCollisionBoxBounds(int layer, int z, int x, float& left, float& top, float& right, float& bottom)
{
    // 2026-05-20: 保存した中心オフセット/サイズから、編集表示と実当たり判定で使うBOX範囲を復元する。
    float cx = x * BLOCK_SIZE + BLOCK_SIZE * 0.5f + CollisionBoxOffsetXMap[layer][z][x];
    float cz = z * BLOCK_SIZE + BLOCK_SIZE * 0.5f + CollisionBoxOffsetZMap[layer][z][x];
    float halfX = max(20.0f, CollisionBoxSizeXMap[layer][z][x] * 0.5f);
    float halfZ = max(20.0f, CollisionBoxSizeZMap[layer][z][x] * 0.5f);

    left = cx - halfX;
    right = cx + halfX;
    top = cz - halfZ;
    bottom = cz + halfZ;
}

static int GetCollisionBoxDragMode(int layer, int z, int x, float worldX, float worldZ)
{
    // 2026-06-02: BOX編集を直感的にするため、角・辺・内側のつかみ判定を大きくして追加。
    float left, top, right, bottom;
    GetCollisionBoxBounds(layer, z, x, left, top, right, bottom);

    float cornerHandle = BLOCK_SIZE * 0.16f;
    float edgeHandle = BLOCK_SIZE * 0.12f;

    bool nearLeft = fabsf(worldX - left) <= cornerHandle;
    bool nearRight = fabsf(worldX - right) <= cornerHandle;
    bool nearTop = fabsf(worldZ - top) <= cornerHandle;
    bool nearBottom = fabsf(worldZ - bottom) <= cornerHandle;

    if (nearLeft && nearTop) return 6;
    if (nearRight && nearTop) return 7;
    if (nearRight && nearBottom) return 8;
    if (nearLeft && nearBottom) return 9;

    bool insideWide = worldX >= left - edgeHandle && worldX <= right + edgeHandle && worldZ >= top - edgeHandle && worldZ <= bottom + edgeHandle;
    if (!insideWide)
        return 0;

    if (fabsf(worldX - left) <= edgeHandle) return 1;
    if (fabsf(worldX - right) <= edgeHandle) return 2;
    if (fabsf(worldZ - top) <= edgeHandle) return 3;
    if (fabsf(worldZ - bottom) <= edgeHandle) return 4;
    if (worldX >= left && worldX <= right && worldZ >= top && worldZ <= bottom) return 5;

    return 0;
}

static void ApplyCollisionBoxDrag(int layer, int z, int x, int mode, float worldX, float worldZ)
{
    // 2026-06-02: 角ドラッグで縦横同時リサイズ、内側ドラッグで自然に移動できるよう編集操作を拡張。
    if (!IsMapPosValid(layer, z, x) || mode <= 0)
        return;

    const float minSize = 40.0f;
    float left, top, right, bottom;
    GetCollisionBoxBounds(layer, z, x, left, top, right, bottom);

    if (mode == 1 || mode == 6 || mode == 9) left = min(worldX, right - minSize);
    if (mode == 2 || mode == 7 || mode == 8) right = max(worldX, left + minSize);
    if (mode == 3 || mode == 6 || mode == 7) top = min(worldZ, bottom - minSize);
    if (mode == 4 || mode == 8 || mode == 9) bottom = max(worldZ, top + minSize);

    if (mode == 5)
    {
        float halfX = (right - left) * 0.5f;
        float halfZ = (bottom - top) * 0.5f;
        float centerX = worldX + collisionBoxDragOffsetX;
        float centerZ = worldZ + collisionBoxDragOffsetZ;
        left = centerX - halfX;
        right = centerX + halfX;
        top = centerZ - halfZ;
        bottom = centerZ + halfZ;
    }

    float cellCenterX = x * BLOCK_SIZE + BLOCK_SIZE * 0.5f;
    float cellCenterZ = z * BLOCK_SIZE + BLOCK_SIZE * 0.5f;
    CollisionBoxOffsetXMap[layer][z][x] = (int)(((left + right) * 0.5f) - cellCenterX);
    CollisionBoxOffsetZMap[layer][z][x] = (int)(((top + bottom) * 0.5f) - cellCenterZ);
    CollisionBoxSizeXMap[layer][z][x] = (int)max(minSize, right - left);
    CollisionBoxSizeZMap[layer][z][x] = (int)max(minSize, bottom - top);
}

static void DrawCollisionBoxHandle(float x, float y, float z, int color)
{
    // 2026-06-02: BOXのつかめる場所を見た目で分かりやすくするため、大きめの立体ハンドルに変更。
    float s = BLOCK_SIZE * 0.055f;
    DrawCube3D(
        VGet(x - s, y - s, z - s),
        VGet(x + s, y + s, z + s),
        color,
        GetColor(255, 255, 255),
        TRUE
    );
}

static void DrawCollisionBoxMoveHandle(float x, float y, float z, int color)
{
    // 2026-06-02: BOX本体をつかんで動かせることを示すため、中心ハンドルを別色で表示する。
    DrawSphere3D(VGet(x, y, z), BLOCK_SIZE * 0.07f, 16, color, GetColor(255, 255, 255), TRUE);
    DrawLine3D(VGet(x - BLOCK_SIZE * 0.12f, y, z), VGet(x + BLOCK_SIZE * 0.12f, y, z), color);
    DrawLine3D(VGet(x, y, z - BLOCK_SIZE * 0.12f), VGet(x, y, z + BLOCK_SIZE * 0.12f), color);
}
static void DrawManualCollisionBox(int layer, int z, int x, int color)
{
    // 2026-05-20: 手動当たり判定を平面の×印ではなく、UnityのBoxColliderのような箱として表示する。
    float left, top, right, bottom;
    GetCollisionBoxBounds(layer, z, x, left, top, right, bottom);
    float y1 = layer * BLOCK_SIZE + 5.0f;
    float y2 = layer * BLOCK_SIZE + 150.0f;

    DrawLine3D(VGet(left, y1, top), VGet(right, y1, top), color);
    DrawLine3D(VGet(right, y1, top), VGet(right, y1, bottom), color);
    DrawLine3D(VGet(right, y1, bottom), VGet(left, y1, bottom), color);
    DrawLine3D(VGet(left, y1, bottom), VGet(left, y1, top), color);
    DrawLine3D(VGet(left, y2, top), VGet(right, y2, top), color);
    DrawLine3D(VGet(right, y2, top), VGet(right, y2, bottom), color);
    DrawLine3D(VGet(right, y2, bottom), VGet(left, y2, bottom), color);
    DrawLine3D(VGet(left, y2, bottom), VGet(left, y2, top), color);
    DrawLine3D(VGet(left, y1, top), VGet(left, y2, top), color);
    DrawLine3D(VGet(right, y1, top), VGet(right, y2, top), color);
    DrawLine3D(VGet(right, y1, bottom), VGet(right, y2, bottom), color);
    DrawLine3D(VGet(left, y1, bottom), VGet(left, y2, bottom), color);

    float hy = y2 + 10.0f;
    int handleColor = GetColor(255, 255, 80);
    DrawCollisionBoxHandle(left, hy, (top + bottom) * 0.5f, handleColor);
    DrawCollisionBoxHandle(right, hy, (top + bottom) * 0.5f, handleColor);
    DrawCollisionBoxHandle((left + right) * 0.5f, hy, top, handleColor);
    DrawCollisionBoxHandle((left + right) * 0.5f, hy, bottom, handleColor);
    DrawCollisionBoxHandle(left, hy, top, handleColor);
    DrawCollisionBoxHandle(right, hy, top, handleColor);
    DrawCollisionBoxHandle(right, hy, bottom, handleColor);
    DrawCollisionBoxHandle(left, hy, bottom, handleColor);
    DrawCollisionBoxMoveHandle((left + right) * 0.5f, hy + 16.0f, (top + bottom) * 0.5f, GetColor(80, 255, 120));
}
#pragma region ===== イベント配置 =====

static void ClearEventId(int eventId)
{
    for (int y = 0; y < BLOCK_NUM_Y; y++)
    {
        for (int z = 0; z < BLOCK_NUM_Z; z++)
        {
            for (int x = 0; x < BLOCK_NUM_X; x++)
            {
                if (EventMap[y][z][x] == eventId)
                {
                    EventMap[y][z][x] = -1;
                }
            }
        }
    }
}

/*
static bool IsEditorEnemyModelId(int modelId)
{
    return modelId == 6;
}

static int GetEditorEnemyIdFromModel(int modelId)
{
    if (modelId == 6)
        return GAME_ENEMY_THROW;

    return -1;
}

static bool IsLegacyEnemyEventId(int eventId)
{
    return eventId == 6;
}

static int NormalizeEditorEnemyId(int enemyId)
{
    return IsLegacyEnemyEventId(enemyId) ? GAME_ENEMY_THROW : enemyId;
}
*/
static void PlaceEvent(int layer, int z, int x, int eventId)
{
    if (!IsMapPosValid(layer, z, x))
        return;

    // スタートとゴールのみ1個に制限
    if (eventId == 0 || eventId == 1)
    {
        ClearEventId(eventId);
    }

    EventMap[layer][z][x] = eventId;

    if (eventId == 0)
    {
        startX = x;
        startY = layer;
        startZ = z;
    }
    else if (eventId == 1)
    {
        goalX = x;
        goalY = layer;
        goalZ = z;
    }
}

#pragma endregion

#pragma region ===== 座標・回転 =====

static float RotToRad(int rot)
{
    return rot * 3.14159265f * 0.5f;
}

static bool IsEditorStairsId(int id)
{
    return id == 5;
}

static int GetEditorStairsDrawRot(int rot)
{
    // 2026-05-19: ゲーム側と同じく、階段モデルだけ見た目の向きを180度反転して表示する。
    return (rot + 2) % 4;
}

static int GetEditorModelDrawRot(int tab, int id, int rot)
{
    // 2026-06-29: 壁モデルID 1/3は横向きの時だけ外側を向くため、回転1/3だけ補正する。
    if (tab == WALL && (id == 1 || id == 3) && (rot & 1))
        return (rot + 2) & 3;

    if (tab == DECO && IsEditorStairsId(id))
        return GetEditorStairsDrawRot(rot);

    return rot;
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
    // 2026-06-29: 壁は選択したセルの辺に置き、内向きになる見た目は描画回転で補正する。

    switch (rot % 4)
    {
    case 0: pz = z * BLOCK_SIZE; break;
    case 1: px = (x + 1) * BLOCK_SIZE; break;
    case 2: pz = (z + 1) * BLOCK_SIZE; break;
    case 3: px = x * BLOCK_SIZE; break;
    }

    return VGet(px, py, pz);
}

static VECTOR GetModelDrawPosition(int tab, int x, int y, int z, int rot)
{
    if (tab == WALL)
        return GetWallPosition(x, y, z, rot);

    VECTOR pos = GetBlockCenterPosition(x, y, z);

    // 階段だけ位置補正
   /* if (tab == DECO && DecoMap[y][z][x] == 5)
    {
        float offset = BLOCK_SIZE * -0.25f;

        switch (rot % 4)
        {
        case 0:
            pos.z += offset;
            break;

        case 1:
            pos.x += offset;
            break;

        case 2:
            pos.z -= offset;
            break;

        case 3:
            pos.x -= offset;
            break;
        }
    }*/

    return pos;
}

#pragma endregion


#pragma region ===== 当たり判定設定 =====

static vector<string> SplitCollisionCSV(const string& line)
{
    // 2026-05-11: 新しいオブジェクトを追加した時に、CSVで既定当たり判定を増やせるよう追加。
    vector<string> result;
    string item;
    stringstream ss(line);

    while (getline(ss, item, ','))
    {
        result.push_back(item);
    }

    return result;
}

static EditorCollisionType ParseEditorCollisionType(const string& text)
{
    // 2026-05-11: collision_config.csv の文字列から当たり判定タイプを選べるよう追加。
    if (text == "CIRCLE") return EDITOR_COLL_CIRCLE;
    if (text == "BOX") return EDITOR_COLL_BOX;
    if (text == "ARC") return EDITOR_COLL_ARC;
    if (text == "WALL") return EDITOR_COLL_WALL;
    if (text == "STAIRS") return EDITOR_COLL_STAIRS;
    return EDITOR_COLL_NONE;
}

static void InitEditorCollisionTable()
{
    // 2026-05-11: Loader側の既定当たり判定をエディターでも見えるようにするため追加。
    for (int i = 0; i < MODEL_MAX; i++)
    {
        editorCollisionTable[i].type = EDITOR_COLL_NONE;
        editorCollisionTable[i].radius = 0.0f;
        editorCollisionTable[i].width = 0.0f;
        editorCollisionTable[i].depth = 0.0f;
    }

    editorCollisionTable[1].type = EDITOR_COLL_WALL;

    editorCollisionTable[2].type = EDITOR_COLL_CIRCLE;
    editorCollisionTable[2].radius = BLOCK_SIZE * 0.3f;

    editorCollisionTable[5].type = EDITOR_COLL_STAIRS;

    editorCollisionTable[6].type = EDITOR_COLL_BOX;
    editorCollisionTable[6].width = BLOCK_SIZE * 0.35f;
    editorCollisionTable[6].depth = BLOCK_SIZE * 0.35f;

    editorCollisionTable[7].type = EDITOR_COLL_NONE;

    ifstream ifs("collision_config.csv");
    if (!ifs)
        return;

    string line;
    while (getline(ifs, line))
    {
        if (line.empty() || line[0] == '#')
            continue;

        vector<string> cols = SplitCollisionCSV(line);
        if (cols.size() < 2 || cols[0] == "id")
            continue;

        int id = stoi(cols[0]);
        if (id < 0 || id >= MODEL_MAX)
            continue;

        editorCollisionTable[id].type = ParseEditorCollisionType(cols[1]);
        editorCollisionTable[id].radius = cols.size() >= 3 ? stof(cols[2]) : 0.0f;
        editorCollisionTable[id].width = cols.size() >= 4 ? stof(cols[3]) : 0.0f;
        editorCollisionTable[id].depth = cols.size() >= 5 ? stof(cols[4]) : 0.0f;
    }
}

#pragma endregion
static int ParseModelTab(const string& text)
{
    // 2026-05-20: model_config.csv のタブ名から、エディターの配置タブへ変換するため追加。
    if (text == "FLOOR") return FLOOR;
    if (text == "WALL") return WALL;
    if (text == "CORNER") return CORNER;
    if (text == "DECO") return DECO;
    if (text == "ENEMY") return ENEMY;
    if (text == "EVENT") return EVENT;
    return -1;
}

static void AddModelToEditorTab(int tab, int id)
{
    if (tab < 0 || tab >= TAB_MAX)
        return;

    if (tabModelCount[tab] >= 16)
        return;

    tabModelList[tab][tabModelCount[tab]] = id;
    tabModelCount[tab]++;
}

static void LoadEditorModelConfig()
{
    // 2026-05-20: 今後モデルを増やしやすくするため、エディターのモデル一覧をCSVから読み込む。
    for (int tab = 0; tab < TAB_MAX; tab++)
    {
        tabModelCount[tab] = 0;
        for (int i = 0; i < 16; i++)
        {
            tabModelList[tab][i] = 0;
        }
    }

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

        vector<string> cols = SplitCollisionCSV(line);
        if (cols.size() < 3 || cols[0] == "id")
            continue;

        int id = stoi(cols[0]);
        int tab = ParseModelTab(cols[1]);
        if (id < 0 || id >= MODEL_MAX || tab < 0)
            continue;

        string modelPath = cols[2];
        string palettePath = cols.size() >= 4 ? cols[3] : "-";
        float scaleX = cols.size() >= 5 ? stof(cols[4]) : 1.0f;
        float scaleY = cols.size() >= 6 ? stof(cols[5]) : scaleX;
        float scaleZ = cols.size() >= 7 ? stof(cols[6]) : scaleX;

        if (modelPath != "-" && modelHandles[id] == -1)
        {
            modelHandles[id] = MV1LoadModel(modelPath.c_str());
            if (modelHandles[id] != -1)
            {
                MV1SetScale(modelHandles[id], VGet(scaleX, scaleY, scaleZ));
            }
        }

        if (palettePath != "-" && paletteTex[id] == -1)
        {
            paletteTex[id] = LoadGraph(palettePath.c_str());
        }

        AddModelToEditorTab(tab, id);
    }
}

#pragma region ===== 当たり判定表示補助 =====

static int GetCollisionEdgeBit(int edge)
{
    // 2026-05-11: 1セルに複数の辺当たり判定を持たせるため、方向をビットに変換する処理を追加。
    return 1 << (edge % 4);
}

static void GetCollisionEdgeLine(int x, int z, int edge, int scale, float& x1, float& z1, float& x2, float& z2)
{
    // 2026-05-11: MapLoader.cppの壁ライン判定と同じ位置を、伸縮値込みでエディター可視化するため追加。
    float left = x * BLOCK_SIZE;
    float right = (x + 1) * BLOCK_SIZE;
    float top = z * BLOCK_SIZE;
    float bottom = (z + 1) * BLOCK_SIZE;

    switch (edge % 4)
    {
    case 0:
        x1 = left + BLOCK_SIZE * 0.0f;
        z1 = top;
        x2 = right - BLOCK_SIZE * 0.0f;
        z2 = top;
        break;

    case 1:
        x1 = right;
        z1 = top + BLOCK_SIZE * 0.00f;
        x2 = right;
        z2 = bottom - BLOCK_SIZE * 0.00f;
        break;

    case 2:
        x1 = left + BLOCK_SIZE * 0.0f;
        z1 = bottom;
        x2 = right - BLOCK_SIZE * 0.0f;
        z2 = bottom;
        break;

    case 3:
        x1 = left;
        z1 = top + BLOCK_SIZE * 0.0f;
        x2 = left;
        z2 = bottom - BLOCK_SIZE * 0.0f;
        break;
    }

    float centerX = (x1 + x2) * 0.5f;
    float centerZ = (z1 + z2) * 0.5f;
    float rate = scale / 100.0f;

    x1 = centerX + (x1 - centerX) * rate;
    z1 = centerZ + (z1 - centerZ) * rate;
    x2 = centerX + (x2 - centerX) * rate;
    z2 = centerZ + (z2 - centerZ) * rate;
}

static void DrawCollisionEdgeLine(int x, int y, int z, int edge, int scale, int thickness, int color)
{
    // 2026-05-11: 壁モデル由来/手動設定由来の辺当たり判定を線で確認するため追加。
    float x1 = 0.0f;
    float z1 = 0.0f;
    float x2 = 0.0f;
    float z2 = 0.0f;
    float yy = y * BLOCK_SIZE + 70.0f;

    GetCollisionEdgeLine(x, z, edge, scale, x1, z1, x2, z2);
    DrawLine3D(VGet(x1, yy, z1), VGet(x2, yy, z2), color);
    DrawLine3D(VGet(x1, yy + 8.0f, z1), VGet(x2, yy + 8.0f, z2), color);

    // 2026-05-11: 辺当たり判定の厚さを、線に直交する短い目印として見えるよう追加。
    float dx = x2 - x1;
    float dz = z2 - z1;
    float len = sqrtf(dx * dx + dz * dz);
    if (len > 0.0001f)
    {
        float nx = -dz / len;
        float nz = dx / len;
        float half = BLOCK_SIZE * 0.08f * (thickness / 100.0f);
        float cx = (x1 + x2) * 0.5f;
        float cz = (z1 + z2) * 0.5f;
        DrawLine3D(VGet(cx - nx * half, yy + 16.0f, cz - nz * half), VGet(cx + nx * half, yy + 16.0f, cz + nz * half), color);
    }
}

static void DrawCollisionCircle(int x, int y, int z, float radius, int color)
{
    // 2026-05-11: 円形の既定当たり判定を、オブジェクトを置いた時点で直接見えるよう追加。
    const int DIV = 32;
    float cx = x * BLOCK_SIZE + BLOCK_SIZE * 0.5f;
    float cz = z * BLOCK_SIZE + BLOCK_SIZE * 0.5f;
    float yy = y * BLOCK_SIZE + 72.0f;

    for (int i = 0; i < DIV; i++)
    {
        float a1 = DX_PI_F * 2.0f * i / DIV;
        float a2 = DX_PI_F * 2.0f * (i + 1) / DIV;
        DrawLine3D(
            VGet(cx + cosf(a1) * radius, yy, cz + sinf(a1) * radius),
            VGet(cx + cosf(a2) * radius, yy, cz + sinf(a2) * radius),
            color);
    }
}

static void DrawCollisionCircleAt(float cx, float yy, float cz, float radius, int color)
{
    // 装飾7は中央ではなく四隅に小さい柱判定を置くので、任意位置に円を描ける補助を使う。
    const int DIV = 32;

    for (int i = 0; i < DIV; i++)
    {
        float a1 = DX_PI_F * 2.0f * i / DIV;
        float a2 = DX_PI_F * 2.0f * (i + 1) / DIV;
        DrawLine3D(
            VGet(cx + cosf(a1) * radius, yy, cz + sinf(a1) * radius),
            VGet(cx + cosf(a2) * radius, yy, cz + sinf(a2) * radius),
            color);
    }
}

static void DrawCollisionBox(int x, int y, int z, float halfW, float halfD, int color)
{
    // 2026-05-11: 箱形の既定当たり判定を、オブジェクトを置いた時点で直接見えるよう追加。
    float cx = x * BLOCK_SIZE + BLOCK_SIZE * 0.5f;
    float cz = z * BLOCK_SIZE + BLOCK_SIZE * 0.5f;
    float yy = y * BLOCK_SIZE + 72.0f;

    float left = cx - halfW;
    float right = cx + halfW;
    float top = cz - halfD;
    float bottom = cz + halfD;

    DrawLine3D(VGet(left, yy, top), VGet(right, yy, top), color);
    DrawLine3D(VGet(right, yy, top), VGet(right, yy, bottom), color);
    DrawLine3D(VGet(right, yy, bottom), VGet(left, yy, bottom), color);
    DrawLine3D(VGet(left, yy, bottom), VGet(left, yy, top), color);
}

static void GetCornerArcData(int x, int z, int rot, float radius, float& cx, float& cz, float& start, float& end)
{
    // 2026-05-20: 画像のようなコーナー用90度円弧を、rotごとにセル角へ合わせて配置する。
    // 2026-05-20: モデルのカーブより左下へズレたため、内側補正を外してセル角を中心にする。
    float left = x * BLOCK_SIZE;
    float right = (x + 1) * BLOCK_SIZE;
    float top = z * BLOCK_SIZE;
    float bottom = (z + 1) * BLOCK_SIZE;

    // 2026-05-20: 回転2を正解にして、逆だった回転1と回転3だけを入れ替える。
    switch (rot % 4)
    {
    case 0: cx = left; cz = bottom; start = DX_PI_F * 1.5f; end = DX_PI_F * 2.0f; break;
    case 1: cx = right; cz = bottom; start = DX_PI_F; end = DX_PI_F * 1.5f; break;
    case 2: cx = right; cz = top; start = DX_PI_F * 0.5f; end = DX_PI_F; break;
    default: cx = left; cz = top; start = 0.0f; end = DX_PI_F * 0.5f; break;
    }
}

static void DrawCollisionArc(int x, int y, int z, int rot, float radius, float thickness, int color)
{
    // 2026-05-20: 半円ではなくコーナーの向きに合う90度円弧コライダーとして表示する。
    const int DIV = 24;
    float cx, cz, start, end;
    GetCornerArcData(x, z, rot, radius, cx, cz, start, end);
    float yy = y * BLOCK_SIZE + 82.0f;
    float inner = max(10.0f, radius - thickness);
    float outer = radius + thickness;

    for (int i = 0; i < DIV; i++)
    {
        float a1 = start + (end - start) * i / DIV;
        float a2 = start + (end - start) * (i + 1) / DIV;

        // 2026-05-20: 円弧コライダーに厚みがあることを分かるよう、内側線・中心線・外側線で表示する。
        DrawLine3D(VGet(cx + cosf(a1) * inner, yy, cz + sinf(a1) * inner), VGet(cx + cosf(a2) * inner, yy, cz + sinf(a2) * inner), color);
        DrawLine3D(VGet(cx + cosf(a1) * radius, yy + 8.0f, cz + sinf(a1) * radius), VGet(cx + cosf(a2) * radius, yy + 8.0f, cz + sinf(a2) * radius), color);
        DrawLine3D(VGet(cx + cosf(a1) * outer, yy, cz + sinf(a1) * outer), VGet(cx + cosf(a2) * outer, yy, cz + sinf(a2) * outer), color);
    }

    DrawLine3D(VGet(cx + cosf(start) * inner, yy + 16.0f, cz + sinf(start) * inner), VGet(cx + cosf(start) * outer, yy + 16.0f, cz + sinf(start) * outer), color);
    DrawLine3D(VGet(cx + cosf(end) * inner, yy + 16.0f, cz + sinf(end) * inner), VGet(cx + cosf(end) * outer, yy + 16.0f, cz + sinf(end) * outer), color);
}
static void DrawCollisionCornerLine(
    int x,
    int y,
    int z,
    int rot,
    int scale,
    int thickness,
    int offset,
    int color)
{
    //2026-5-25 半円コーナー当たり判定

    const int DIV = 32;

    float cx = x * BLOCK_SIZE + BLOCK_SIZE * -0.5f;
    float cz = z * BLOCK_SIZE + BLOCK_SIZE * -0.5f;

    float yy = y * BLOCK_SIZE + 74.0f;

    // 半径
    float radius = BLOCK_SIZE * 0.5f * (scale / 100.0f);

    // offsetで前後移動
    float dirX = 0.0f;
    float dirZ = 0.0f;

    float startAngle = 0.0f;
    float endAngle = DX_PI_F;

    switch (rot % 4)
    {
    case 0:
        // 上向き
        startAngle = DX_PI_F;
        endAngle = DX_PI_F * 2.0f;
        dirZ = -1.0f;
        break;

    case 1:
        // 右向き
        startAngle = -DX_PI_F * 0.5f;
        endAngle = DX_PI_F * 0.5f;
        dirX = 1.0f;
        break;

    case 2:
        // 下向き
        startAngle = 0.0f;
        endAngle = DX_PI_F;
        dirZ = 1.0f;
        break;

    case 3:
        // 左向き

        startAngle = DX_PI_F * 0.5f;
        endAngle = DX_PI_F * 1.5f;
        dirX = -1.0f;
        break;
    }

    cx += dirX * offset;
    cz += dirZ * offset;

    // 半円描画
    for (int i = 0; i < DIV; i++)
    {
        float t1 = (float)i / DIV;
        float t2 = (float)(i + 1) / DIV;

        float a1 = startAngle + (endAngle - startAngle) * t1;
        float a2 = startAngle + (endAngle - startAngle) * t2;

        float x1 = cx + cosf(a1) * radius;
        float z1 = cz + sinf(a1) * radius;

        float x2 = cx + cosf(a2) * radius;
        float z2 = cz + sinf(a2) * radius;

        DrawLine3D(
            VGet(x1, yy, z1),
            VGet(x2, yy, z2),
            color);

        DrawLine3D(
            VGet(x1, yy + 8.0f, z1),
            VGet(x2, yy + 8.0f, z2),
            color);
    }

    // thickness表示
    float thick = BLOCK_SIZE * 0.08f * (thickness / 100.0f);

    DrawLine3D(
        VGet(cx - thick, yy + 16.0f, cz),
        VGet(cx + thick, yy + 16.0f, cz),
        color);

    DrawLine3D(
        VGet(cx, yy + 16.0f, cz - thick),
        VGet(cx, yy + 16.0f, cz + thick),
        color);
}
static void DrawHighPlatformCollision(int x, int y, int z, int color)
{
    // 装飾7は下をくぐれるため、配置した階では四隅の柱だけを表示する。
    float left = x * BLOCK_SIZE;
    float right = (x + 1) * BLOCK_SIZE;
    float top = z * BLOCK_SIZE;
    float bottom = (z + 1) * BLOCK_SIZE;
    float postY = y * BLOCK_SIZE + 72.0f;
    float offset = BLOCK_SIZE * 0.12f;
    float postRadius = BLOCK_SIZE * 0.08f;

    DrawCollisionCircleAt(left + offset, postY, top + offset, postRadius, color);
    DrawCollisionCircleAt(right - offset, postY, top + offset, postRadius, color);
    DrawCollisionCircleAt(left + offset, postY, bottom - offset, postRadius, color);
    DrawCollisionCircleAt(right - offset, postY, bottom - offset, postRadius, color);

    // 上面は、配置した階の1つ上で歩ける床として表示する。
    float floorY = (y + 1) * BLOCK_SIZE + 10.0f;
    DrawLine3D(VGet(left, floorY, top), VGet(right, floorY, top), color);
    DrawLine3D(VGet(right, floorY, top), VGet(right, floorY, bottom), color);
    DrawLine3D(VGet(right, floorY, bottom), VGet(left, floorY, bottom), color);
    DrawLine3D(VGet(left, floorY, bottom), VGet(left, floorY, top), color);
}

static void DrawDefaultObjectCollision(int tab, int id, int x, int y, int z, int rot, int color)
{
    // 2026-05-11: オブジェクトが最初から持っている既定当たり判定を、配置後すぐ見えるよう追加。
    if (id < 0 || id >= MODEL_MAX)
        return;

    if (tab == WALL || editorCollisionTable[id].type == EDITOR_COLL_WALL)
    {
        DrawCollisionEdgeLine(x, y, z, rot, 100, 100, color);
        return;
    }

    if (tab == DECO && id == 7)
    {
        DrawHighPlatformCollision(x, y, z, color);
        return;
    }

    if (tab == CORNER && editorCollisionTable[id].type == EDITOR_COLL_NONE)
    {
        // 2026-05-20: コーナーもモデルごとのBOX指定へ切り替えられるよう、未指定の時だけ従来の斜め線を使う。
        DrawCollisionCornerLine(
            x,
            y,
            z,
            rot,
            CollisionCornerScaleMap[y][z][x],
            CollisionCornerThicknessMap[y][z][x],
            CollisionCornerOffsetMap[y][z][x],
            color);
        return;
    }

    if (editorCollisionTable[id].type == EDITOR_COLL_CIRCLE)
    {
        DrawCollisionCircle(x, y, z, editorCollisionTable[id].radius, color);
    }
    else if (editorCollisionTable[id].type == EDITOR_COLL_BOX)
    {
        DrawCollisionBox(x, y, z, editorCollisionTable[id].width, editorCollisionTable[id].depth, color);
    }
    else if (editorCollisionTable[id].type == EDITOR_COLL_ARC)
    {
        // 2026-05-20: 道側に寄りすぎたため、コーナーの弧側へ一段外へ出して表示する。
        DrawCollisionArc(x, y, z, rot, editorCollisionTable[id].radius + 40.0f, max(10.0f, editorCollisionTable[id].width), color);
    }
}

#pragma endregion










