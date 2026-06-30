static bool IsWalkableCell(int y, int z, int x)
{
    if (!IsGameMapPosValid(y, z, x))
        return false;

    if (GameFloorMap[y][z][x] >= 0)
        return true;

    if (IsStairsId(GameDecoMap[y][z][x]))
        return true;

    return false;
}

bool HasWallEdge(int y, int z, int x, int edge)
{
    if (!IsGameMapPosValid(y, z, x))
        return true;

    auto HasLocalWall = [&](int cy, int cz, int cx, int rot)
        {
            if (!IsGameMapPosValid(cy, cz, cx))
                return false;

            if (GameWallMapA[cy][cz][cx] >= 0 &&
                GameWallRotA[cy][cz][cx] == rot)
            {
                return true;
            }

            if (GameWallMapB[cy][cz][cx] >= 0 &&
                GameWallRotB[cy][cz][cx] == rot)
            {
                return true;
            }

            if (HasManualCollisionEdge(cy, cz, cx, rot))
            {
                return true;
            }

            return false;
        };

    // 自セル
    if (HasLocalWall(y, z, x, edge))
        return true;

    // 隣セル側も確認
    switch (edge)
    {
    case 0: // 上
        return HasLocalWall(y, z - 1, x, 2);

    case 1: // 右
        return HasLocalWall(y, z, x + 1, 3);

    case 2: // 下
        return HasLocalWall(y, z + 1, x, 0);

    case 3: // 左
        return HasLocalWall(y, z, x - 1, 1);
    }

    return false;
}

bool CanMoveCellToCell(int y, int fromX, int fromZ, int toX, int toZ)
{
    if (!IsWalkableCell(y, toZ, toX))
        return false;

    int dx = toX - fromX;
    int dz = toZ - fromZ;

    if (dx == 0 && dz == -1)
    {
        // if (HasWallEdge(y, fromZ, fromX, 0)) return false;
        //if (HasWallEdge(y, toZ, toX, 2)) return false;
    }
    else if (dx == 1 && dz == 0)
    {
        //if (HasWallEdge(y, fromZ, fromX, 1)) return false;
        //if (HasWallEdge(y, toZ, toX, 3)) return false;
    }
    else if (dx == 0 && dz == 1)
    {
        // if (HasWallEdge(y, fromZ, fromX, 2)) return false;
        // if (HasWallEdge(y, toZ, toX, 0)) return false;
    }
    else if (dx == -1 && dz == 0)
    {
        // if (HasWallEdge(y, fromZ, fromX, 3)) return false;
        // if (HasWallEdge(y, toZ, toX, 1)) return false;
    }
    else
    {
        return false;
    }

    return true;
}

bool IsGoalCell(int y, int z, int x)
{
    return
        x == gameGoalX &&
        y == gameGoalY &&
        z == gameGoalZ;
}

#pragma endregion


#pragma region ===== ワールド判定 共通 =====

static bool IsStairsAtWorldInLayer(int y, float worldX, float worldZ)
{
    int x = WorldToCell(worldX);
    int z = WorldToCell(worldZ);

    if (!IsGameMapPosValid(y, z, x))
        return false;

    return IsStairsId(GameDecoMap[y][z][x]);
}

static bool CircleHitAABB(float cx, float cz, float radius, float left, float top, float right, float bottom)
{
    float nearestX = cx;
    float nearestZ = cz;

    if (nearestX < left) nearestX = left;
    if (nearestX > right) nearestX = right;
    if (nearestZ < top) nearestZ = top;
    if (nearestZ > bottom) nearestZ = bottom;

    float dx = cx - nearestX;
    float dz = cz - nearestZ;

    return (dx * dx + dz * dz) <= radius * radius;
}

static bool CircleHitCircle(float x1, float z1, float r1, float x2, float z2, float r2)
{
    float dx = x1 - x2;
    float dz = z1 - z2;
    float rr = r1 + r2;

    return dx * dx + dz * dz <= rr * rr;
}

static bool CircleHitBox(float cx, float cz, float radius, float bx, float bz, float halfW, float halfD)
{
    float left = bx - halfW;
    float right = bx + halfW;
    float top = bz - halfD;
    float bottom = bz + halfD;

    return CircleHitAABB(cx, cz, radius, left, top, right, bottom);
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

static bool IsAngleOnArc(float angle, float start, float end)
{
    // 2026-05-20: 円弧の角度範囲内か調べる。
    if (angle < 0.0f)
        angle += DX_PI_F * 2.0f;

    return angle >= start && angle <= end;
}

static bool CircleHitArc(float worldX, float worldZ, float playerRadius, int x, int z, int rot, float arcRadius, float thickness)
{
    // 2026-05-20: コーナー用90度円弧の帯とプレイヤー円の接触を判定する。
    float cx, cz, start, end;
    GetCornerArcData(x, z, rot, arcRadius, cx, cz, start, end);
    float dx = worldX - cx;
    float dz = worldZ - cz;
    float angle = atan2f(dz, dx);
    if (angle < 0.0f)
        angle += DX_PI_F * 2.0f;

    float inner = max(10.0f, arcRadius - thickness);
    float outer = arcRadius + thickness;
    float dist = sqrtf(dx * dx + dz * dz);

    if (IsAngleOnArc(angle, start, end) && dist >= inner - playerRadius && dist <= outer + playerRadius)
    {
        // 2026-05-20: 内側全体はふさがず、エディターで見えている厚い円弧の範囲だけを実判定にする。
        return true;
    }

    auto HitCapLine = [&](float a)
    {
        float ix = cx + cosf(a) * inner;
        float iz = cz + sinf(a) * inner;
        float ox = cx + cosf(a) * outer;
        float oz = cz + sinf(a) * outer;

        float vx = ox - ix;
        float vz = oz - iz;
        float wx = worldX - ix;
        float wz = worldZ - iz;
        float lenSq = vx * vx + vz * vz;
        float t = lenSq > 0.0f ? (wx * vx + wz * vz) / lenSq : 0.0f;
        if (t < 0.0f) t = 0.0f;
        if (t > 1.0f) t = 1.0f;

        float nearestX = ix + vx * t;
        float nearestZ = iz + vz * t;
        float capDx = worldX - nearestX;
        float capDz = worldZ - nearestZ;
        return capDx * capDx + capDz * capDz <= playerRadius * playerRadius;
    };

    // 2026-05-20: 円弧の端から裏側へ入れる隙間だけを、見た目の端線と同じ位置で止める。
    return HitCapLine(start) || HitCapLine(end);
}
static bool IsPointInsideLowPlatformCell(int y, int z, int x, float worldX, float worldZ)
{
    if (!IsGameMapPosValid(y, z, x))
        return false;

    int decoId = GameDecoMap[y][z][x];
    if (!IsLowPlatformDecoId(decoId))
        return false;

    CollisionInfo& info = collisionTable[decoId];
    if (info.type != COLL_BOX)
        return false;

    float bx = x * BLOCK_SIZE + BLOCK_SIZE * 0.5f;
    float bz = z * BLOCK_SIZE + BLOCK_SIZE * 0.5f;

    return
        worldX >= bx - info.width &&
        worldX <= bx + info.width &&
        worldZ >= bz - info.depth &&
        worldZ <= bz + info.depth;
}

static bool FindLowPlatformAtWorld(int y, float worldX, float worldZ)
{
    int cellX = WorldToCell(worldX);
    int cellZ = WorldToCell(worldZ);

    // マス境界で足場が消えないよう、周囲セルの装飾6も確認する。
    for (int dz = -1; dz <= 1; dz++)
    {
        for (int dx = -1; dx <= 1; dx++)
        {
            if (IsPointInsideLowPlatformCell(y, cellZ + dz, cellX + dx, worldX, worldZ))
                return true;
        }
    }

    return false;
}

static bool IsPointInsideCellRect(int z, int x, float worldX, float worldZ)
{
    const float edgeEpsilon = 1.0f;
    float left = x * BLOCK_SIZE;
    float right = (x + 1) * BLOCK_SIZE;
    float top = z * BLOCK_SIZE;
    float bottom = (z + 1) * BLOCK_SIZE;

    return
        worldX >= left - edgeEpsilon &&
        worldX <= right + edgeEpsilon &&
        worldZ >= top - edgeEpsilon &&
        worldZ <= bottom + edgeEpsilon;
}

static bool FindOverheadFloorAtWorld(int y, float worldX, float worldZ)
{
    if (y <= 0)
        return false;

    int cellX = WorldToCell(worldX);
    int cellZ = WorldToCell(worldZ);

    // 装飾7は1つ上の階の床になるため、境界の抜け防止で周囲セルも確認する。
    for (int dz = -1; dz <= 1; dz++)
    {
        for (int dx = -1; dx <= 1; dx++)
        {
            int x = cellX + dx;
            int z = cellZ + dz;

            if (!IsGameMapPosValid(y - 1, z, x))
                continue;

            if (IsOverheadDecoId(GameDecoMap[y - 1][z][x]) &&
                IsPointInsideCellRect(z, x, worldX, worldZ))
            {
                return true;
            }
        }
    }

    return false;
}

static bool IsWalkableAtWorld(int y, float worldX, float worldZ)
{
    int x = WorldToCell(worldX);
    int z = WorldToCell(worldZ);

    if (IsWalkableCell(y, z, x))
        return true;

    if (!IsGameMapPosValid(y, z, x))
        return false;

    // 1つ下の階に置いた装飾7は、今の階の床として扱う。
    // マス境界で床が抜けないよう、周囲セルも確認する。
    if (FindOverheadFloorAtWorld(y, worldX, worldZ))
        return true;

    // 装飾6の上面も周囲セルから探し、マス境界で足元が埋まらないようにする。
    return FindLowPlatformAtWorld(y, worldX, worldZ);
}
static bool IsWalkableAtWorldForStairsTransition(int y, float worldX, float worldZ)
{
    // 2026-05-13: 階段でレイヤーが切り替わった直後も、半径の一部が下階段側に残って進めるよう追加。
    if (IsWalkableAtWorld(y, worldX, worldZ))
        return true;

    // 2026-05-25: 階段移動中だけは同じ層/一つ下の階段セルを歩ける場所として扱い、関数未呼び出しの警告も消す。
    if (IsStairsAtWorldInLayer(y, worldX, worldZ))
        return true;

    // 2026-06-29: 階段の途中で上階扱いになっても、足元の下階段を歩ける場所として残す。
    if (IsStairsAtWorldInLayer(y - 1, worldX, worldZ))
        return true;

    return false;
}

static bool HasWalkableSurfaceForCircle(int y, float worldX, float worldZ, float radius)
{
    // 階段は1マス内で高さが変わるので、足元中心が階段なら通す。
    // 周囲5点まで見ると階段セルの端で弾かれて登れなくなる。
    if (IsStairsAtWorldInLayer(y, worldX, worldZ))
        return true;

    // 2026-06-29: 階段の途中で上階扱いになっても、足元の下階段を歩ける場所として残す。
    if (IsStairsAtWorldInLayer(y - 1, worldX, worldZ))
        return true;

    // 2026-05-13: レイヤー切り替え直後の上階判定でも、下の階段にかかっている足元を歩ける扱いにするため変更。
    if (!IsWalkableAtWorldForStairsTransition(y, worldX, worldZ)) return false;
    if (!IsWalkableAtWorldForStairsTransition(y, worldX - radius, worldZ)) return false;
    if (!IsWalkableAtWorldForStairsTransition(y, worldX + radius, worldZ)) return false;
    if (!IsWalkableAtWorldForStairsTransition(y, worldX, worldZ - radius)) return false;
    if (!IsWalkableAtWorldForStairsTransition(y, worldX, worldZ + radius)) return false;

    return true;
}
#pragma endregion


#pragma region ===== 壁 / 物 判定 =====

static bool IsSolidDecoId(int id)
{
    if (id < 0)
        return false;

    if (IsStairsId(id))
        return false;

    if (IsLowPlatformDecoId(id) || IsOverheadDecoId(id))
        return false;

    if (id >= MODEL_MAX)
        return false;

    return collisionTable[id].type != COLL_NONE;
}

static bool IsSolidCell(int y, int z, int x)
{
    if (!IsGameMapPosValid(y, z, x))
        return true;

    // 2026-05-11: エディターで手動設定したセル当たり判定を移動判定へ反映するため追加。
    if (GameCollisionMap[y][z][x] >= 0)
        return true;

    //if (GameCornerMap[y][z][x] >= 0)
    //    return true;

    if (IsSolidDecoId(GameDecoMap[y][z][x]))
        return true;

    return false;
}
bool IsCellBlocked(int y, int z, int x)
{
    // 2026-05-11: エディター/デバッグ側からセルの当たり判定状態を確認できるよう追加。
    return IsSolidCell(y, z, x);
}
static float PointLineDistance(
    float px,
    float pz,
    float x1,
    float z1,
    float x2,
    float z2)
{
    float vx = x2 - x1;
    float vz = z2 - z1;

    float wx = px - x1;
    float wz = pz - z1;

    float lenSq = vx * vx + vz * vz;

    float t = 0.0f;

    if (lenSq > 0.0f)
    {
        t = (wx * vx + wz * vz) / lenSq;
    }

    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;

    float nearestX = x1 + vx * t;
    float nearestZ = z1 + vz * t;

    float dx = px - nearestX;
    float dz = pz - nearestZ;

    return sqrtf(dx * dx + dz * dz);
}
static bool HitMapObjects(
    int y,
    float worldX,
    float worldZ,
    float radius)
{
    int cellX = WorldToCell(worldX);
    int cellZ = WorldToCell(worldZ);

    for (int dz = -1; dz <= 1; dz++)
    {
        for (int dx = -1; dx <= 1; dx++)
        {
            int x = cellX + dx;
            int z = cellZ + dz;

            if (!IsGameMapPosValid(y, z, x))
                continue;

            // 2026-05-11: 手動当たり判定セルは、プレイヤー半径とセル矩形の接触で止めるため追加。
            if (GameCollisionMap[y][z][x] >= 0)
            {
                // 2026-05-20: セル全体ではなく、エディターで直接触って調整したBoxCollider風の範囲で止める。
                float boxCenterX = x * BLOCK_SIZE + BLOCK_SIZE * 0.5f + GameCollisionBoxOffsetXMap[y][z][x];
                float boxCenterZ = z * BLOCK_SIZE + BLOCK_SIZE * 0.5f + GameCollisionBoxOffsetZMap[y][z][x];
                float halfX = max(20.0f, GameCollisionBoxSizeXMap[y][z][x] * 0.5f);
                float halfZ = max(20.0f, GameCollisionBoxSizeZMap[y][z][x] * 0.5f);

                if (CircleHitAABB(worldX, worldZ, radius, boxCenterX - halfX, boxCenterZ - halfZ, boxCenterX + halfX, boxCenterZ + halfZ))
                    return true;
            }

            float left = x * BLOCK_SIZE;
            float right = (x + 1) * BLOCK_SIZE;
            float top = z * BLOCK_SIZE;
            float bottom = (z + 1) * BLOCK_SIZE;


            auto CheckWall = [&](int rot, int scale, int thicknessScale)
                {
                    float x1, z1;
                    float x2, z2;

                    // 各方向ごとの線位置
                    switch (rot % 4)
                    {
                    case 0: // 上壁
                        x1 = left + BLOCK_SIZE * 0.0f;
                        z1 = top;

                        x2 = right - BLOCK_SIZE * 0.0f;
                        z2 = top;
                        break;

                    case 1: // 右
                        x1 = right;
                        z1 = top + BLOCK_SIZE * 0.00f;

                        x2 = right;
                        z2 = bottom - BLOCK_SIZE * 0.00f;
                        break;
                    case 2: // 下壁
                        x1 = left + BLOCK_SIZE * 0.0f;
                        z1 = bottom;

                        x2 = right - BLOCK_SIZE * 0.0f;
                        z2 = bottom;
                        break;
                    case 3: // 左壁
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

                    float lineDX = x2 - x1;
                    float lineDZ = z2 - z1;
                    float lineLenSq = lineDX * lineDX + lineDZ * lineDZ;
                    float along = 0.0f;
                    if (lineLenSq > 0.0001f)
                    {
                        along = ((worldX - x1) * lineDX + (worldZ - z1) * lineDZ) / lineLenSq;
                    }

                    // 2026-06-29: 壁端へ半分めり込めないよう、端側に少しだけ余白判定を残す。
                    float lineLen = sqrtf(lineLenSq);
                    float endMarginRate = lineLen > 0.0001f ? (radius * 0.45f) / lineLen : 0.0f;
                    if (along < -endMarginRate || along > 1.0f + endMarginRate)
                        return false;

                    float dist = PointLineDistance(
                        worldX,
                        worldZ,
                        x1,
                        z1,
                        x2,
                        z2);

                    // 2026-06-29: 壁の線は見えている辺に置き、横へ広げず正面側の厚みで見た目の本体を埋める。
                    float signedDist = 0.0f;
                    switch (rot % 4)
                    {
                    case 0: signedDist = worldZ - z1; break;
                    case 1: signedDist = x1 - worldX; break;
                    case 2: signedDist = z1 - worldZ; break;
                    case 3: signedDist = worldX - x1; break;
                    }

                    float frontThickness = BLOCK_SIZE * 0.19f;
                    float backThickness = BLOCK_SIZE * 0.0f;
                    float rateThickness = thicknessScale / 100.0f;
                    frontThickness *= rateThickness;
                    backThickness *= rateThickness;

                    if (signedDist >= 0.0f)
                        return dist <= radius + frontThickness;

                    float backRadius = radius * 0.35f;
                    return dist <= backRadius + backThickness;
                };

            auto HasLocalPlacedWallEdge = [&](int rot)
                {
                    return
                        (GameWallMapA[y][z][x] >= 0 && GameWallRotA[y][z][x] == rot) ||
                        (GameWallMapB[y][z][x] >= 0 && GameWallRotB[y][z][x] == rot);
                };

            // 2026-06-29: 隣セルの壁辺をモデル判定へ取り込むと裏側が広くなりすぎるため、自セルの壁だけを見る。
            if (HasLocalPlacedWallEdge(0) && CheckWall(0, 100, 100))
            {
                return true;
            }

            if (HasLocalPlacedWallEdge(1) && CheckWall(1, 100, 100))
            {
                return true;
            }

            if (HasLocalPlacedWallEdge(2) && CheckWall(2, 100, 100))
            {
                return true;
            }

            if (HasLocalPlacedWallEdge(3) && CheckWall(3, 100, 100))
            {
                return true;
            }


            if (HasManualCollisionEdge(y, z, x, 0) && CheckWall(0, GetManualCollisionEdgeScale(y, z, x, 0), GetManualCollisionEdgeThickness(y, z, x, 0)))
            {
                return true;
            }

            if (HasManualCollisionEdge(y, z, x, 1) && CheckWall(1, GetManualCollisionEdgeScale(y, z, x, 1), GetManualCollisionEdgeThickness(y, z, x, 1)))
            {
                return true;
            }

            if (HasManualCollisionEdge(y, z, x, 2) && CheckWall(2, GetManualCollisionEdgeScale(y, z, x, 2), GetManualCollisionEdgeThickness(y, z, x, 2)))
            {
                return true;
            }

            if (HasManualCollisionEdge(y, z, x, 3) && CheckWall(3, GetManualCollisionEdgeScale(y, z, x, 3), GetManualCollisionEdgeThickness(y, z, x, 3)))
            {
                return true;
            }
            int cornerId = GameCornerMap[y][z][x];

            if (cornerId >= 0)
            {
                CollisionInfo& cornerInfo = collisionTable[cornerId];
                // 2026-05-20: コーナーもモデルごとのBOX/CIRCLE指定を優先し、未指定の時だけ従来の斜め線判定を使う。
                if (cornerInfo.type == COLL_CIRCLE)
                {
                    float cx = x * BLOCK_SIZE + BLOCK_SIZE * 0.5f;
                    float cz = z * BLOCK_SIZE + BLOCK_SIZE * 0.5f;
                    if (CircleHitCircle(worldX, worldZ, radius, cx, cz, cornerInfo.radius))
                        return true;

                    continue;
                }
                else if (cornerInfo.type == COLL_BOX)
                {
                    float bx = x * BLOCK_SIZE + BLOCK_SIZE * 0.5f;
                    float bz = z * BLOCK_SIZE + BLOCK_SIZE * 0.5f;
                    if (CircleHitBox(worldX, worldZ, radius, bx, bz, cornerInfo.width, cornerInfo.depth))
                        return true;

                    continue;
                }
                else if (cornerInfo.type == COLL_ARC)
                {
                                        // 2026-05-20: エディター表示と同じく、道側に寄りすぎたコーナーARC判定を弧側へ一段外へ出す。
                    if (CircleHitArc(worldX, worldZ, radius, x, z, GameCornerRot[y][z][x], cornerInfo.radius + 40.0f, max(10.0f, cornerInfo.width)))
                        return true;

                    continue;
                }

                float x1, z1;
                float x2, z2;

                float inset = BLOCK_SIZE * 0.12f;

                switch (GameCornerRot[y][z][x] % 4)
                {
                case 0: // ＼
                    x1 = left + inset;
                    z1 = top + inset;

                    x2 = right - inset;
                    z2 = bottom - inset;
                    break;

                case 1: // ／
                    x1 = right - inset;
                    z1 = top + inset;

                    x2 = left + inset;
                    z2 = bottom - inset;
                    break;

                case 2: // ＼
                    x1 = right - inset;
                    z1 = bottom - inset;

                    x2 = left + inset;
                    z2 = top + inset;
                    break;

                case 3: // ／
                    x1 = left + inset;
                    z1 = bottom - inset;

                    x2 = right - inset;
                    z2 = top + inset;
                    break;
                }

                // 2026-05-13: エディターで調整したコーナー当たり判定の長さ・厚み・奥行を実判定へ反映するため追加。
                float centerX = (x1 + x2) * 0.5f;
                float centerZ = (z1 + z2) * 0.5f;
                float rate = GameCollisionCornerScaleMap[y][z][x] / 100.0f;

                x1 = centerX + (x1 - centerX) * rate;
                z1 = centerZ + (z1 - centerZ) * rate;
                x2 = centerX + (x2 - centerX) * rate;
                z2 = centerZ + (z2 - centerZ) * rate;

                float lineDX = x2 - x1;
                float lineDZ = z2 - z1;
                float lineLen = sqrtf(lineDX * lineDX + lineDZ * lineDZ);
                if (lineLen > 0.0001f)
                {
                    float nx = -lineDZ / lineLen;
                    float nz = lineDX / lineLen;
                    float offset = (float)GameCollisionCornerOffsetMap[y][z][x];

                    x1 += nx * offset;
                    z1 += nz * offset;
                    x2 += nx * offset;
                    z2 += nz * offset;
                }

                float dist = PointLineDistance(
                    worldX,
                    worldZ,
                    x1,
                    z1,
                    x2,
                    z2);

                float wallThickness = BLOCK_SIZE * 0.07f;
                wallThickness *= GameCollisionCornerThicknessMap[y][z][x] / 100.0f;
                if (dist <= radius + wallThickness)
                {
                    return true;
                }
            }




            int decoId = GameDecoMap[y][z][x];

            if (decoId >= 0)
            {
                if (IsLowPlatformDecoId(decoId))
                    continue;

                if (IsOverheadDecoId(decoId))
                {
                    // 全体を箱判定にすると下をくぐれないので、装飾7は四隅の柱だけ判定する。
                    float left = x * BLOCK_SIZE;
                    float right = (x + 1) * BLOCK_SIZE;
                    float top = z * BLOCK_SIZE;
                    float bottom = (z + 1) * BLOCK_SIZE;
                    float offset = GetHighPlatformPostOffset();
                    float postRadius = GetHighPlatformPostRadius();

                    if (CircleHitCircle(worldX, worldZ, radius, left + offset, top + offset, postRadius) ||
                        CircleHitCircle(worldX, worldZ, radius, right - offset, top + offset, postRadius) ||
                        CircleHitCircle(worldX, worldZ, radius, left + offset, bottom - offset, postRadius) ||
                        CircleHitCircle(worldX, worldZ, radius, right - offset, bottom - offset, postRadius))
                    {
                        return true;
                    }

                    continue;
                }

                CollisionInfo& info = collisionTable[decoId];

                if (info.type == COLL_CIRCLE)
                {
                    float cx =
                        x * BLOCK_SIZE + BLOCK_SIZE * 0.5f;

                    float cz =
                        z * BLOCK_SIZE + BLOCK_SIZE * 0.5f;

                    if (CircleHitCircle(
                        worldX,
                        worldZ,
                        radius,
                        cx,
                        cz,
                        info.radius))
                    {
                        return true;
                    }
                }
                else if (info.type == COLL_BOX)
                {
                    float bx =
                        x * BLOCK_SIZE + BLOCK_SIZE * 0.5f;

                    float bz =
                        z * BLOCK_SIZE + BLOCK_SIZE * 0.5f;

                    if (CircleHitBox(
                        worldX,
                        worldZ,
                        radius,
                        bx,
                        bz,
                        info.width,
                        info.depth))
                    {
                        return true;
                    }
                }
                else if (info.type == COLL_STAIRS)
                {
                    // 2026-05-20: 階段の横壁判定は不要なので、階段デコ自体では横を塞がない。
                    continue;
                }
            }
        }
    }

    return false;
}
/*
static bool HitWallEdges(int y, float worldX, float worldZ, float radius)
{
    int cellX = WorldToCell(worldX);
    int cellZ = WorldToCell(worldZ);

    for (int dz = -1; dz <= 1; dz++)
    {
        for (int dx = -1; dx <= 1; dx++)
        {
            int x = cellX + dx;
            int z = cellZ + dz;

            if (!IsGameMapPosValid(y, z, x))
                continue;

            float left = x * BLOCK_SIZE;
            float right = (x + 1) * BLOCK_SIZE;
            float top = z * BLOCK_SIZE;
            float bottom = (z + 1) * BLOCK_SIZE;

            if (HasWallEdge(y, z, x, 0) && HitHorizontalWallLine(worldX, worldZ, radius, top, left, right))
                return true;

            if (HasWallEdge(y, z, x, 1) && HitVerticalWallLine(worldX, worldZ, radius, right, top, bottom))
                return true;

            if (HasWallEdge(y, z, x, 2) && HitHorizontalWallLine(worldX, worldZ, radius, bottom, left, right))
                return true;

            if (HasWallEdge(y, z, x, 3) && HitVerticalWallLine(worldX, worldZ, radius, left, top, bottom))
                return true;
        }
    }

    return false;
}
*/

#pragma endregion




#pragma region ===== 形状判定 =====

static bool IsCollisionModelId(int id)
{
    if (id < 0)
        return false;

    if (id == 0)
        return false;

    if (IsStairsId(id))
        return false;

    if (IsLowPlatformDecoId(id) || IsOverheadDecoId(id))
        return false;

    if (id >= MODEL_MAX)
        return false;

    return collisionTable[id].type != COLL_NONE;
}

static bool HitPlacedModelSphere(int id, int tab, int x, int y, int z, int rot, VECTOR center, float radius)
{
    if (!IsCollisionModelId(id))
        return false;

    if (id >= MODEL_MAX || gameModelHandles[id] == -1)
        return false;

    MV1SetPosition(gameModelHandles[id], GetModelDrawPosition(tab, x, y, z, rot));
    MV1SetRotationXYZ(gameModelHandles[id], VGet(0.0f, RotToRad(rot), 0.0f));

    MV1_COLL_RESULT_POLY_DIM hit = MV1CollCheck_Sphere(gameModelHandles[id], -1, center, radius);
    bool result = hit.HitNum > 0;
    MV1CollResultPolyDimTerminate(hit);

    return result;
}
/*
static bool HitCollisionModels(int y, float worldX, float worldZ, float radius)
{
    int cellX = WorldToCell(worldX);
    int cellZ = WorldToCell(worldZ);
    VECTOR center = VGet(worldX, y * BLOCK_SIZE + radius, worldZ);

    for (int dz = -1; dz <= 1; dz++)
    {
        for (int dx = -1; dx <= 1; dx++)
        {
            int x = cellX + dx;
            int z = cellZ + dz;

            if (!IsGameMapPosValid(y, z, x))
                continue;

            // 壁はエディターと同じ端ライン判定を使う。
            // モデル判定も重ねると、モデル原点や当たり情報の差で見た目よりズレることがある。
            //if (HitPlacedModelSphere(GameCornerMap[y][z][x], CORNER, x, y, z, GameCornerRot[y][z][x], center, radius))
            //    return true;

            if (HitPlacedModelSphere(GameDecoMap[y][z][x], DECO, x, y, z, GameDecoRot[y][z][x], center, radius))
                return true;
        }
    }

    return false;
}
*/
#pragma endregion


#pragma region ===== 高さ / 階段 =====

static bool GetStairsProgressInLayer(int y, float worldX, float worldZ, float& progress)
{
    // 2026-05-25: 階段自体に上る場所/下がる場所を持たせるため、配置回転から階段上の進行率を計算する。
    int x = WorldToCell(worldX);
    int z = WorldToCell(worldZ);

    if (!IsGameMapPosValid(y, z, x))
        return false;

    if (!IsStairsId(GameDecoMap[y][z][x]))
        return false;

    float localX = Clamp01((worldX - x * BLOCK_SIZE) / BLOCK_SIZE);
    float localZ = Clamp01((worldZ - z * BLOCK_SIZE) / BLOCK_SIZE);

    switch (GameDecoRot[y][z][x] % 4)
    {
    case 0: progress = 1.0f - localZ; break;
    case 1: progress = 1.0f - localX; break;
    case 2: progress = localZ; break;
    case 3: progress = localX; break;
    default: progress = 0.0f; break;
    }

    return true;
}

bool IsStairsAtWorldLayer(int layer, float worldX, float worldZ)
{
    return IsStairsAtWorldInLayer(layer, worldX, worldZ);
}

static bool IsStairsLowerEntry(int y, float worldX, float worldZ)
{
    float progress = 0.0f;
    return GetStairsProgressInLayer(y, worldX, worldZ, progress) && progress <= 0.28f;
}

static bool IsStairsUpperEntry(int y, float worldX, float worldZ)
{
    float progress = 0.0f;
    return GetStairsProgressInLayer(y, worldX, worldZ, progress) && progress >= 0.72f;
}

bool TryGetStairsLoadLayer(int currentLayer, float worldX, float worldZ, int& loadLayer)
{
    loadLayer = currentLayer;

    float progress = 0.0f;
    if (GetStairsProgressInLayer(currentLayer, worldX, worldZ, progress))
    {
        // 2026-06-29: 表示だけは階段に乗った時点で上階を先読みし、上がり際に真っ暗にならないようにする。
        if (currentLayer + 1 < BLOCK_NUM_Y)
        {
            loadLayer = currentLayer + 1;
            return true;
        }

        loadLayer = currentLayer;
        return true;
    }

    if (currentLayer - 1 >= 0 && GetStairsProgressInLayer(currentLayer - 1, worldX, worldZ, progress))
    {
        // 2026-05-20: 上階側から階段へ戻る時は、下端側へ入ったら下階をロード対象に戻す。
        loadLayer = progress <= 0.42f ? currentLayer - 1 : currentLayer;
        return true;
    }

    return false;
}
bool TryMoveLayerByStairs(int& layer, float worldX, float worldZ)
{
    float progress = 0.0f;

    // 2026-06-29: 真っ黒になる前の挙動へ戻し、階段に触れた時点で上階扱いにする。
    if (GetStairsProgressInLayer(layer, worldX, worldZ, progress))
    {
        if (layer + 1 < BLOCK_NUM_Y)
        {
            layer++;
            return true;
        }

        return false;
    }

    if (layer - 1 >= 0 && GetStairsProgressInLayer(layer - 1, worldX, worldZ, progress))
    {
        if (progress <= 0.28f)
        {
            layer--;
            return true;
        }
    }

    return false;
}
bool IsStairsAtWorld(float worldX, float worldZ)
{
    int x = WorldToCell(worldX);
    int z = WorldToCell(worldZ);

    for (int y = BLOCK_NUM_Y - 1; y >= 0; y--)
    {
        if (!IsGameMapPosValid(y, z, x))
            continue;

        if (IsStairsId(GameDecoMap[y][z][x]))
            return true;
    }

    return false;
}

float GetMapGroundY(float worldX, float worldZ, float currentY)
{
    int x = WorldToCell(worldX);
    int z = WorldToCell(worldZ);

    if (x < 0 || x >= BLOCK_NUM_X || z < 0 || z >= BLOCK_NUM_Z)
        return 0.0f;

    for (int y = BLOCK_NUM_Y - 1; y >= 0; y--)
    {
        float baseY = y * BLOCK_SIZE;

        // 2026-05-25: 階層ロード後に別階層の床へ吸われないよう、現在の高さから遠い上階は地面候補から外す。
        if (baseY > currentY + BLOCK_SIZE * 0.75f)
            continue;

        // 装飾7の上面は1つ上の階の床。ただし下をくぐっている時に上へ吸い上がらないよう、
        // 現在Yが上面に近い時だけ床として返す。
        if (FindOverheadFloorAtWorld(y, worldX, worldZ) &&
            currentY >= baseY - BLOCK_SIZE * 0.5f)
        {
            return baseY;
        }

        if (IsStairsId(GameDecoMap[y][z][x]))
        {
            float localX = Clamp01((worldX - x * BLOCK_SIZE) / BLOCK_SIZE);
            float localZ = Clamp01((worldZ - z * BLOCK_SIZE) / BLOCK_SIZE);

            float t = 1.3f;

            // 2026-05-19: 階段の当たり判定と高さ方向は前のまま使い、見た目だけを反転する。
            switch (GameDecoRot[y][z][x] % 4)
            {
            case 0: // 奥→手前へ上る
                t = 1.0f - localZ;
                break;

            case 1: // 左→右へ上る
                t = 1.0f - localX;
                break;

            case 2: // 手前→奥へ上る
                t = localZ;
                break;

            case 3: // 右→左へ上る
                t = localX;
                break;
            }

            // なめらか補正
            t = sqrtf(t);

            // 全方向同じ高さ補正
            return baseY + t * BLOCK_SIZE;
        }

        if (FindLowPlatformAtWorld(y, worldX, worldZ))
        {
            return baseY + GetLowPlatformTopOffset();
        }

        if (GameFloorMap[y][z][x] >= 0)
            return baseY;
    }

    return 0.0f;
}
float GetMapGroundY(float worldX, float worldZ)
{
    return GetMapGroundY(worldX, worldZ, BLOCK_SIZE * (BLOCK_NUM_Y + 1));
}
#pragma endregion


#pragma region ===== 連続移動判定 =====

static bool CanStandPlayerPosition(
    int y,
    float worldX,
    float worldZ,
    float radius)
{
    if (!HasWalkableSurfaceForCircle(y, worldX, worldZ, radius))
        return false;

    if (HitMapObjects(y, worldX, worldZ, radius))
        return false;

    return true;
}

bool CanMoveWorldPosition(int y, float worldX, float worldZ, float radius)
{
    // 2026-05-11: デバッグや他クラスから、ワールド座標が移動可能か直接確認できるよう追加。
    return CanStandPlayerPosition(y, worldX, worldZ, radius);
}
static bool CanStandPlayerPositionByWorld(float worldX, float worldZ, float radius, int currentLayer)
{
    // 2026-05-25: 通常移動では今いる層だけの当たり判定を使い、階段端イベントでだけ隣の層をロード扱いで許可する。
    if (CanStandPlayerPosition(currentLayer, worldX, worldZ, radius))
        return true;

    if (currentLayer + 1 < BLOCK_NUM_Y &&
        IsStairsUpperEntry(currentLayer, worldX, worldZ) &&
        CanStandPlayerPosition(currentLayer + 1, worldX, worldZ, radius))
    {
        return true;
    }

    if (currentLayer - 1 >= 0 &&
        IsStairsLowerEntry(currentLayer - 1, worldX, worldZ) &&
        CanStandPlayerPosition(currentLayer - 1, worldX, worldZ, radius))
    {
        return true;
    }

    return false;
}

VECTOR ResolvePlayerMapCollision(
    VECTOR currentPos,
    VECTOR nextPos,
    float radius,
    int currentLayer)
{
    VECTOR result = currentPos;

    /// 移動量
    float moveX = nextPos.x - currentPos.x;
    float moveZ = nextPos.z - currentPos.z;

    if (CanStandPlayerPositionByWorld(
        nextPos.x,
        nextPos.z,
        radius,
        currentLayer))
    {
        result.x = nextPos.x;
        result.z = nextPos.z;
    }
    else
    {
        if (CanStandPlayerPositionByWorld(
            currentPos.x + moveX,
            currentPos.z,
            radius,
            currentLayer))
        {
            result.x = currentPos.x + moveX;
        }

        if (CanStandPlayerPositionByWorld(
            result.x,
            currentPos.z + moveZ,
            radius,
            currentLayer))
        {
            result.z = currentPos.z + moveZ;
        }
    }

    /// 高さ
    result.y = nextPos.y;

    return result;
}

bool IsGoalWorldPosition(int y, float worldX, float worldZ)
{
    int x = WorldToCell(worldX);
    int z = WorldToCell(worldZ);

    return IsGoalCell(y, z, x);
}

#pragma endregion








