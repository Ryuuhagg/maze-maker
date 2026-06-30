// 2026-06-02: EVENTタブで置いたゲーム用ギミックをエディター3Dビュー上でも識別できるよう追加。
static const char* GetEditorEventMarkerName(int eventId)
{
    if (eventId == 0) return "START";
    if (eventId == 1) return "GOAL";
    if (eventId == 2) return "ITEM";
    if (eventId == 3) return "DANGER";
    if (eventId == 4) return "KEY";
    if (eventId == 5) return "DOOR";
    if (eventId == 6) return "ENEMY";
    // 2026-06-15: チェックポイントを3Dビュー上で見分けるため追加。
    if (eventId == 7) return "CHECKPOINT";
    return "EVENT";
}

// 2026-06-02: EVENTタブで置いた場所を色と形で見分けられるよう、種類ごとの表示色を追加。
static int GetEditorEventMarkerColor(int eventId)
{
    if (eventId == 0) return GetColor(80, 255, 80);
    if (eventId == 1) return GetColor(255, 80, 80);
    if (eventId == 2) return GetColor(80, 220, 255);
    if (eventId == 3) return GetColor(255, 70, 70);
    if (eventId == 4) return GetColor(255, 220, 50);
    if (eventId == 5) return GetColor(190, 120, 255);
    if (eventId == 6) return GetColor(255, 140, 40);
    if (eventId == 7) return GetColor(80, 255, 170);
    return GetColor(230, 230, 230);
}

// 2026-06-02: エディター側で「どのセルに何を置いたか」が一目で分かるよう、EVENTマーカーとラベルを描くため追加。
static void DrawEditorEventMarker(int layer, int z, int x, int eventId)
{
    float cx = x * BLOCK_SIZE + BLOCK_SIZE * 0.5f;
    float cz = z * BLOCK_SIZE + BLOCK_SIZE * 0.5f;
    float baseY = layer * BLOCK_SIZE + 30.0f;
    float topY = layer * BLOCK_SIZE + 260.0f;
    int color = GetEditorEventMarkerColor(eventId);

    DrawLine3D(VGet(cx, baseY, cz), VGet(cx, topY, cz), color);
    DrawSphere3D(VGet(cx, topY, cz), eventId == 5 ? 70.0f : 52.0f, 16, color, GetColor(255, 255, 255), TRUE);

    if (eventId == 3)
    {
        DrawLine3D(VGet(cx - 90.0f, topY, cz - 90.0f), VGet(cx + 90.0f, topY, cz + 90.0f), color);
        DrawLine3D(VGet(cx + 90.0f, topY, cz - 90.0f), VGet(cx - 90.0f, topY, cz + 90.0f), color);
    }
    else if (eventId == 4)
    {
        DrawLine3D(VGet(cx - 90.0f, topY, cz), VGet(cx + 90.0f, topY, cz), color);
        DrawLine3D(VGet(cx + 40.0f, topY, cz), VGet(cx + 40.0f, topY, cz + 55.0f), color);
    }
    else if (eventId == 5)
    {
        DrawCube3D(
            VGet(cx - 95.0f, layer * BLOCK_SIZE + 20.0f, cz - 35.0f),
            VGet(cx + 95.0f, layer * BLOCK_SIZE + 220.0f, cz + 35.0f),
            color,
            GetColor(60, 30, 100),
            FALSE
        );
    }
    else if (eventId == 6)
    {
        DrawLine3D(VGet(cx - 90.0f, topY, cz), VGet(cx + 90.0f, topY, cz), color);
        DrawLine3D(VGet(cx + 90.0f, topY, cz), VGet(cx + 45.0f, topY, cz - 45.0f), color);
        DrawLine3D(VGet(cx + 90.0f, topY, cz), VGet(cx + 45.0f, topY, cz + 45.0f), color);
    }
    else if (eventId == 7)
    {
        DrawLine3D(VGet(cx, baseY, cz), VGet(cx, topY + 70.0f, cz), color);
        DrawLine3D(VGet(cx, topY, cz), VGet(cx + 110.0f, topY - 65.0f, cz), color);
        DrawLine3D(VGet(cx + 110.0f, topY - 65.0f, cz), VGet(cx, topY - 130.0f, cz), color);
    }

    VECTOR screen = ConvWorldPosToScreenPos(VGet(cx, topY + 70.0f, cz));
    if (screen.z >= 0.0f && screen.z <= 1.0f)
    {
        DrawFormatString((int)screen.x - 28, (int)screen.y - 8, color, "%s", GetEditorEventMarkerName(eventId));
    }
}

static const char* GetEditorEnemyMarkerName(int enemyId)
{
    // 2026-06-12: EnemyMapのID 0をSTARTではなく敵名として表示するため追加。
    if (enemyId == 0) return "ENEMY";
    return "ENEMY";
}

static void DrawEditorEnemyMarker(int layer, int z, int x, int enemyId)
{
    // 2026-06-12: ENEMY配置をEVENTマーカーとは別の見た目で確認できるよう追加。
    float cx = x * BLOCK_SIZE + BLOCK_SIZE * 0.5f;
    float cz = z * BLOCK_SIZE + BLOCK_SIZE * 0.5f;
    float baseY = layer * BLOCK_SIZE + 30.0f;
    float topY = layer * BLOCK_SIZE + 260.0f;
    int color = GetColor(255, 140, 40);

    DrawLine3D(VGet(cx, baseY, cz), VGet(cx, topY, cz), color);
    DrawSphere3D(VGet(cx, topY, cz), 62.0f, 16, color, GetColor(255, 255, 255), TRUE);
    DrawLine3D(VGet(cx - 90.0f, topY, cz), VGet(cx + 90.0f, topY, cz), color);
    DrawLine3D(VGet(cx + 90.0f, topY, cz), VGet(cx + 45.0f, topY, cz - 45.0f), color);
    DrawLine3D(VGet(cx + 90.0f, topY, cz), VGet(cx + 45.0f, topY, cz + 45.0f), color);

    VECTOR screen = ConvWorldPosToScreenPos(VGet(cx, topY + 70.0f, cz));
    if (screen.z >= 0.0f && screen.z <= 1.0f)
    {
        DrawFormatString((int)screen.x - 28, (int)screen.y - 8, color, "%s", GetEditorEnemyMarkerName(enemyId));
    }
}

// 2026-06-24: コピー範囲を確定前に半透明表示するため追加。
// 実データへ一度書き込んで戻す方式を避け、プレビュー中のUndo汚染を防ぐ。
static void DrawCopyPreviewModel(int tab, int id, int x, int y, int z, int rot)
{
    if (id < 0 || id >= MODEL_MAX || modelHandles[id] == -1)
        return;

    VECTOR pos = GetModelDrawPosition(tab, x, y, z, rot);
    pos.y += 12.0f;

    int drawRot = GetEditorModelDrawRot(tab, id, rot);
    MV1SetPosition(modelHandles[id], pos);
    MV1SetRotationXYZ(modelHandles[id], VGet(0.0f, RotToRad(drawRot), 0.0f));
    MV1SetOpacityRate(modelHandles[id], 0.45f);
    MV1DrawModel(modelHandles[id]);
    MV1SetOpacityRate(modelHandles[id], 1.0f);
}

static void DrawCopyPreview()
{
    if (!pasteMode || !hasCopyData || !IsHoverValid())
        return;

    int rotation = copyRotation & 3;

    for (int srcZ = 0; srcZ < copySizeZ; srcZ++)
    {
        for (int srcX = 0; srcX < copySizeX; srcX++)
        {
            int rotatedX = 0;
            int rotatedZ = 0;
            GetRotatedCopyPosition(srcX, srcZ, rotation, rotatedX, rotatedZ);

            int previewX = hoverX + rotatedX;
            int previewZ = hoverZ + rotatedZ;

            DrawCopyPreviewModel(
                FLOOR, CopyFloorMap[srcZ][srcX], previewX, currentLayer, previewZ,
                (CopyFloorRot[srcZ][srcX] + rotation) & 3);
            DrawCopyPreviewModel(
                WALL, CopyWallMapA[srcZ][srcX], previewX, currentLayer, previewZ,
                (CopyWallRotA[srcZ][srcX] + rotation) & 3);
            DrawCopyPreviewModel(
                WALL, CopyWallMapB[srcZ][srcX], previewX, currentLayer, previewZ,
                (CopyWallRotB[srcZ][srcX] + rotation) & 3);
            DrawCopyPreviewModel(
                CORNER, CopyCornerMap[srcZ][srcX], previewX, currentLayer, previewZ,
                (CopyCornerRot[srcZ][srcX] + rotation) & 3);
            DrawCopyPreviewModel(
                DECO, CopyDecoMap[srcZ][srcX], previewX, currentLayer, previewZ,
                (CopyDecoRot[srcZ][srcX] + rotation) & 3);

            if (CopyEventMap[srcZ][srcX] >= 0)
                DrawEditorEventMarker(currentLayer, previewZ, previewX, CopyEventMap[srcZ][srcX]);
        }
    }

    int previewSizeX = 0;
    int previewSizeZ = 0;
    GetRotatedCopySize(rotation, previewSizeX, previewSizeZ);

    float left = hoverX * BLOCK_SIZE;
    float top = hoverZ * BLOCK_SIZE;
    float right = (hoverX + previewSizeX) * BLOCK_SIZE;
    float bottom = (hoverZ + previewSizeZ) * BLOCK_SIZE;
    float y = currentLayer * BLOCK_SIZE + 24.0f;
    bool fitsMap =
        hoverX >= 0 && hoverZ >= 0 &&
        hoverX + previewSizeX <= BLOCK_NUM_X &&
        hoverZ + previewSizeZ <= BLOCK_NUM_Z;
    int color = fitsMap ? GetColor(80, 255, 255) : GetColor(255, 80, 80);

    SetUseZBuffer3D(FALSE);
    SetWriteZBuffer3D(FALSE);
    DrawLine3D(VGet(left, y, top), VGet(right, y, top), color);
    DrawLine3D(VGet(right, y, top), VGet(right, y, bottom), color);
    DrawLine3D(VGet(right, y, bottom), VGet(left, y, bottom), color);
    DrawLine3D(VGet(left, y, bottom), VGet(left, y, top), color);
    SetUseZBuffer3D(TRUE);
    SetWriteZBuffer3D(TRUE);

    // 2026-06-25: コピー配置中の案内は左素材欄と被るため、MapUI.cppの下部UIへ移動。
}

void DrawEditor()
{
    SetUseZBuffer3D(TRUE);
    SetWriteZBuffer3D(TRUE);

    // 2026-05-11: 当たり判定編集中に線を見やすくするため、モデルを少し透かす値を追加。
    float editorModelOpacity = (showCollisionDebug || collisionEditMode) ? 0.35f : 1.0f;

    float mapCenterX = BLOCK_NUM_X * BLOCK_SIZE * 0.5f;
    float mapCenterZ = BLOCK_NUM_Z * BLOCK_SIZE * 0.5f;

    float camX = sinf(camRotY) * cosf(camRotX) * camDist + mapCenterX;
    float camZ = cosf(camRotY) * cosf(camRotX) * camDist + mapCenterZ;
    float camY = 2000.0f + sinf(camRotX) * camDist;

    SetCameraPositionAndTarget_UpVecY(
        VGet(camX, camY, camZ),
        VGet(mapCenterX, 0.0f, mapCenterZ));

#pragma region グリッド描画

    if (showGrid)
    {
        for (int z = 0; z <= BLOCK_NUM_Z; z++)
        {
            DrawLine3D(
                VGet(0.0f, 0.0f, z * BLOCK_SIZE),
                VGet(BLOCK_NUM_X * BLOCK_SIZE, 0.0f, z * BLOCK_SIZE),
                GetColor(80, 80, 80));
        }

        for (int x = 0; x <= BLOCK_NUM_X; x++)
        {
            DrawLine3D(
                VGet(x * BLOCK_SIZE, 0.0f, 0.0f),
                VGet(x * BLOCK_SIZE, 0.0f, BLOCK_NUM_Z * BLOCK_SIZE),
                GetColor(80, 80, 80));
        }
    }

#pragma endregion


#pragma region モデル描画

    for (int y = 0; y < BLOCK_NUM_Y; y++)
    {
        for (int z = 0; z < BLOCK_NUM_Z; z++)
        {
            for (int x = 0; x < BLOCK_NUM_X; x++)
            {
                if (FloorMap[y][z][x] >= 0)
                {
                    int id = FloorMap[y][z][x];

                    if (id >= 0 && id < MODEL_MAX && modelHandles[id] != -1)
                    {
                        MV1SetPosition(modelHandles[id], GetModelDrawPosition(FLOOR, x, y, z, FloorRot[y][z][x]));
                        MV1SetRotationXYZ(modelHandles[id], VGet(0.0f, RotToRad(FloorRot[y][z][x]), 0.0f));
                        MV1SetOpacityRate(modelHandles[id], editorModelOpacity);
                        MV1DrawModel(modelHandles[id]);
                        MV1SetOpacityRate(modelHandles[id], 1.0f);
                    }
                }

                if (WallMapA[y][z][x] >= 0)
                {
                    int id = WallMapA[y][z][x];

                    if (id >= 0 && id < MODEL_MAX && modelHandles[id] != -1)
                    {
                        MV1SetPosition(modelHandles[id], GetModelDrawPosition(WALL, x, y, z, WallRotA[y][z][x]));
                        MV1SetRotationXYZ(modelHandles[id], VGet(0.0f, RotToRad(GetEditorModelDrawRot(WALL, id, WallRotA[y][z][x])), 0.0f));
                        MV1SetOpacityRate(modelHandles[id], editorModelOpacity);
                        MV1DrawModel(modelHandles[id]);
                        MV1SetOpacityRate(modelHandles[id], 1.0f);
                    }
                }

                if (WallMapB[y][z][x] >= 0)
                {
                    int id = WallMapB[y][z][x];

                    if (id >= 0 && id < MODEL_MAX && modelHandles[id] != -1)
                    {
                        MV1SetPosition(modelHandles[id], GetModelDrawPosition(WALL, x, y, z, WallRotB[y][z][x]));
                        MV1SetRotationXYZ(modelHandles[id], VGet(0.0f, RotToRad(GetEditorModelDrawRot(WALL, id, WallRotB[y][z][x])), 0.0f));
                        MV1SetOpacityRate(modelHandles[id], editorModelOpacity);
                        MV1DrawModel(modelHandles[id]);
                        MV1SetOpacityRate(modelHandles[id], 1.0f);
                    }
                }

                if (CornerMap[y][z][x] >= 0)
                {
                    int id = CornerMap[y][z][x];

                    if (id >= 0 && id < MODEL_MAX && modelHandles[id] != -1)
                    {
                        MV1SetPosition(modelHandles[id], GetModelDrawPosition(CORNER, x, y, z, CornerRot[y][z][x]));
                        MV1SetRotationXYZ(modelHandles[id], VGet(0.0f, RotToRad(CornerRot[y][z][x]), 0.0f));
                        MV1SetOpacityRate(modelHandles[id], editorModelOpacity);
                        MV1DrawModel(modelHandles[id]);
                        MV1SetOpacityRate(modelHandles[id], 1.0f);
                    }
                }

                if (DecoMap[y][z][x] >= 0)
                {
                    int id = DecoMap[y][z][x];

                    if (id >= 0 && id < MODEL_MAX && modelHandles[id] != -1)
                    {
                        MV1SetPosition(modelHandles[id], GetModelDrawPosition(DECO, x, y, z, DecoRot[y][z][x]));
                        MV1SetRotationXYZ(modelHandles[id], VGet(0.0f, RotToRad(IsEditorStairsId(id) ? GetEditorStairsDrawRot(DecoRot[y][z][x]) : DecoRot[y][z][x]), 0.0f));
                        MV1SetOpacityRate(modelHandles[id], editorModelOpacity);
                        MV1DrawModel(modelHandles[id]);
                        MV1SetOpacityRate(modelHandles[id], 1.0f);
                    }
                }
            }
        }
    }

#pragma endregion


#pragma region EVENT / ENEMY配置マーカー描画

    // 2026-06-02: ゲーム用EVENT/ENEMYの配置場所をエディター上で確認できるよう、全階層のマーカーを描画するため追加。
    SetUseZBuffer3D(FALSE);
    SetWriteZBuffer3D(FALSE);
    for (int y = 0; y < BLOCK_NUM_Y; y++)
    {
        for (int z = 0; z < BLOCK_NUM_Z; z++)
        {
            for (int x = 0; x < BLOCK_NUM_X; x++)
            {
                if (EventMap[y][z][x] >= 0)
                {
                    DrawEditorEventMarker(y, z, x, EventMap[y][z][x]);
                }

                /*
                if (EnemyMap[y][z][x] >= 0)
                {
                    DrawEditorEnemyMarker(y, z, x, EnemyMap[y][z][x]);
                }
                */
            }
        }
    }
    SetUseZBuffer3D(TRUE);
    SetWriteZBuffer3D(TRUE);

#pragma endregion


#pragma region コピー配置プレビュー

    // 2026-06-24: 通常モデルより後に描き、コピー中であることを見分けやすくする。
    DrawCopyPreview();

#pragma endregion

#pragma region プレビュー描画

    if (IsHoverValid() && !pasteMode)
    {
        int previewId = GetSelectedModel();

        if (previewId >= 0 && previewId < MODEL_MAX && modelHandles[previewId] != -1)
        {
            VECTOR pos = GetModelDrawPosition(
                currentTab,
                hoverX,
                currentLayer,
                hoverZ,
                GetEditorPlacementRot(currentTab, hoverZ, hoverX)
            );

            pos.y += 8.0f;

            MV1SetPosition(modelHandles[previewId], pos);
            int previewRot = GetEditorPlacementRot(currentTab, hoverZ, hoverX);
            int previewDrawRot = GetEditorModelDrawRot(currentTab, previewId, previewRot);
            // 2026-06-29: 壁データは選択した辺のまま保持し、モデルだけセル中心側を向くように補正する。
            MV1SetRotationXYZ(modelHandles[previewId], VGet(0.0f, RotToRad(previewDrawRot), 0.0f));
            // 2026-05-11: 当たり判定編集中に配置プレビューも線を隠さないよう透過するため追加。
            MV1SetOpacityRate(modelHandles[previewId], editorModelOpacity);
            MV1DrawModel(modelHandles[previewId]);
            MV1SetOpacityRate(modelHandles[previewId], 1.0f);
        }
    }

#pragma endregion


#pragma region 当たり判定デバッグ描画

    // 2026-05-11: 当たり判定をモデルや配置プレビューに隠されず直接見えるよう、最後にZバッファなしで描画するため追加。
    if (showCollisionDebug || collisionEditMode)
    {
        SetUseZBuffer3D(FALSE);
        SetWriteZBuffer3D(FALSE);

        // 2026-05-25: 階段で層をまたいだ後に前階層の当たり判定が残って見えないよう、現在編集中の層だけ表示する。
        int y = currentLayer;
        if (IsMapPosValid(y, 0, 0))
        {
            for (int z = 0; z < BLOCK_NUM_Z; z++)
            {
                for (int x = 0; x < BLOCK_NUM_X; x++)
                {
                    if (CollisionMap[y][z][x] >= 0)
                    {
                        DrawManualCollisionBox(y, z, x, GetColor(255, 60, 60));
                    }

                    if (WallMapA[y][z][x] >= 0)
                        DrawDefaultObjectCollision(WALL, WallMapA[y][z][x], x, y, z, WallRotA[y][z][x], GetColor(255, 170, 40));

                    if (WallMapB[y][z][x] >= 0)
                        DrawDefaultObjectCollision(WALL, WallMapB[y][z][x], x, y, z, WallRotB[y][z][x], GetColor(255, 170, 40));

                    if (CornerMap[y][z][x] >= 0)
                        DrawDefaultObjectCollision(CORNER, CornerMap[y][z][x], x, y, z, CornerRot[y][z][x], GetColor(255, 170, 40));

                    if (DecoMap[y][z][x] >= 0)
                        DrawDefaultObjectCollision(DECO, DecoMap[y][z][x], x, y, z, DecoRot[y][z][x], GetColor(255, 170, 40));

                    int edgeMask = CollisionEdgeMap[y][z][x] >= 0 ? CollisionEdgeMap[y][z][x] : 0;
                    for (int edge = 0; edge < 4; edge++)
                    {
                        if (edgeMask & GetCollisionEdgeBit(edge))
                            DrawCollisionEdgeLine(x, y, z, edge, CollisionEdgeScaleMap[y][z][x][edge], CollisionEdgeThicknessMap[y][z][x][edge], GetColor(40, 220, 255));
                    }
                }
            }
        }

        // 2026-05-11: いま持っている配置予定オブジェクトの既定当たり判定も、置く前から直接見えるよう追加。
        if (IsHoverValid())
        {
            DrawDefaultObjectCollision(currentTab, GetSelectedModel(), hoverX, currentLayer, hoverZ, currentRot, GetColor(255, 255, 0));
        }
    }

    // 2026-05-11: 辺編集時に、いま置こうとしている辺を黄色でプレビューするため追加。
    if (collisionEditMode && collisionEdgeEditMode && IsHoverValid())
    {
        DrawCollisionEdgeLine(hoverX, currentLayer, hoverZ, currentRot, CollisionEdgeScaleMap[currentLayer][hoverZ][hoverX][currentRot], CollisionEdgeThicknessMap[currentLayer][hoverZ][hoverX][currentRot], GetColor(255, 255, 0));
    }

#pragma endregion
#pragma region ハイライト描画

    if (IsHoverValid())
    {
        float x = hoverX * BLOCK_SIZE;
        float z = hoverZ * BLOCK_SIZE;
        float y = currentLayer * BLOCK_SIZE + 5.0f;

        DrawLine3D(VGet(x, y, z), VGet(x + BLOCK_SIZE, y, z), GetColor(255, 255, 0));
        DrawLine3D(VGet(x + BLOCK_SIZE, y, z), VGet(x + BLOCK_SIZE, y, z + BLOCK_SIZE), GetColor(255, 255, 0));
        DrawLine3D(VGet(x + BLOCK_SIZE, y, z + BLOCK_SIZE), VGet(x, y, z + BLOCK_SIZE), GetColor(255, 255, 0));
        DrawLine3D(VGet(x, y, z + BLOCK_SIZE), VGet(x, y, z), GetColor(255, 255, 0));
    }

#pragma endregion


#pragma region 複数選択描画

    // 2026-06-25: Shift+左クリックで選んだ離れたマスを、範囲選択の青枠と区別して紫枠で表示する。
    if (!pasteMode && multiSelectCount > 0)
    {
        int color = GetColor(210, 80, 255);
        for (int layer = 0; layer < BLOCK_NUM_Y; layer++)
        {
            for (int z = 0; z < BLOCK_NUM_Z; z++)
            {
                for (int x = 0; x < BLOCK_NUM_X; x++)
                {
                    if (!multiSelectMap[layer][z][x])
                        continue;

                    float x1 = x * BLOCK_SIZE;
                    float z1 = z * BLOCK_SIZE;
                    float x2 = (x + 1) * BLOCK_SIZE;
                    float z2 = (z + 1) * BLOCK_SIZE;
                    float y = layer * BLOCK_SIZE + 22.0f;

                    DrawLine3D(VGet(x1, y, z1), VGet(x2, y, z1), color);
                    DrawLine3D(VGet(x2, y, z1), VGet(x2, y, z2), color);
                    DrawLine3D(VGet(x2, y, z2), VGet(x1, y, z2), color);
                    DrawLine3D(VGet(x1, y, z2), VGet(x1, y, z1), color);
                }
            }
        }
    }

#pragma endregion


#pragma region 選択範囲描画

    // 2026-06-24: 青い枠は範囲選択中だけ表示する。コピー後は移動する
    // 貼り付けプレビュー枠へ役割を引き継ぎ、元位置へ固定表示しない。
    if (selectMode && !pasteMode && HasValidSelection())
    {
        int minX = min(selectStartX, selectEndX);
        int maxX = max(selectStartX, selectEndX);
        int minZ = min(selectStartZ, selectEndZ);
        int maxZ = max(selectStartZ, selectEndZ);

        float x1 = minX * BLOCK_SIZE;
        float z1 = minZ * BLOCK_SIZE;
        float x2 = (maxX + 1) * BLOCK_SIZE;
        float z2 = (maxZ + 1) * BLOCK_SIZE;
        float y = selectLayer * BLOCK_SIZE + 15.0f;

        DrawLine3D(VGet(x1, y, z1), VGet(x2, y, z1), GetColor(0, 255, 255));
        DrawLine3D(VGet(x2, y, z1), VGet(x2, y, z2), GetColor(0, 255, 255));
        DrawLine3D(VGet(x2, y, z2), VGet(x1, y, z2), GetColor(0, 255, 255));
        DrawLine3D(VGet(x1, y, z2), VGet(x1, y, z1), GetColor(0, 255, 255));
    }

#pragma endregion
#pragma region 開始 / ゴール描画

    if (IsMapPosValid(startY, startZ, startX))
    {
        float x = startX * BLOCK_SIZE;
        float z = startZ * BLOCK_SIZE;
        float y = startY * BLOCK_SIZE + 25.0f;

        DrawLine3D(VGet(x, y, z), VGet(x + BLOCK_SIZE, y, z), GetColor(0, 255, 0));
        DrawLine3D(VGet(x + BLOCK_SIZE, y, z), VGet(x + BLOCK_SIZE, y, z + BLOCK_SIZE), GetColor(0, 255, 0));
        DrawLine3D(VGet(x + BLOCK_SIZE, y, z + BLOCK_SIZE), VGet(x, y, z + BLOCK_SIZE), GetColor(0, 255, 0));
        DrawLine3D(VGet(x, y, z + BLOCK_SIZE), VGet(x, y, z), GetColor(0, 255, 0));
    }

    if (IsMapPosValid(goalY, goalZ, goalX))
    {
        float x = goalX * BLOCK_SIZE;
        float z = goalZ * BLOCK_SIZE;
        float y = goalY * BLOCK_SIZE + 35.0f;

        DrawLine3D(VGet(x, y, z), VGet(x + BLOCK_SIZE, y, z), GetColor(255, 0, 0));
        DrawLine3D(VGet(x + BLOCK_SIZE, y, z), VGet(x + BLOCK_SIZE, y, z + BLOCK_SIZE), GetColor(255, 0, 0));
        DrawLine3D(VGet(x + BLOCK_SIZE, y, z + BLOCK_SIZE), VGet(x, y, z + BLOCK_SIZE), GetColor(255, 0, 0));
        DrawLine3D(VGet(x, y, z + BLOCK_SIZE), VGet(x, y, z), GetColor(255, 0, 0));
    }

#pragma endregion



#pragma region UI / デバッグ描画

    SavePendingEditorKitThumbnail();
    DrawEditorUI();
    DrawSaveNameInput();
    DrawKitNameInput();
    // 2026-06-12: 未保存のままマップ切替しようとした時の確認を、UIの上に表示するため追加。
    DrawLoadMapConfirm();

    /* DrawFormatString(0, 40, GetColor(255, 255, 255),
         "map=%d layer=%d rot=%d tab=%d model=%d brush=%s",
         currentMapIndex, currentLayer, currentRot, currentTab, GetSelectedModel(),
         brushMode ? "ON" : "OFF");

     DrawFormatString(0, 60, GetColor(255, 255, 0),
         "hoverX=%d hoverZ=%d  F5:Save F9:Load",
         hoverX, hoverZ);

     DrawFormatString(0, 80, GetColor(0, 255, 255),
         "select=%s copy=%s size=%d,%d  V:Select Ctrl+C/V/Z",
         selectMode ? "ON" : "OFF",
         hasCopyData ? "YES" : "NO",
         copySizeX, copySizeZ);
         */
#pragma endregion
}

#pragma endregion



