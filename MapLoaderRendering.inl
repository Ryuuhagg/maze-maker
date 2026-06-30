static void DrawModelById(int id, VECTOR pos, int rot)
{
    if (id < 0 || id >= MODEL_MAX)
        return;

    if (gameModelHandles[id] == -1)
        return;

    MV1SetPosition(gameModelHandles[id], pos);
    int drawRot = GetGameModelDrawRot(id, rot);
    MV1SetRotationXYZ(gameModelHandles[id], VGet(0.0f, RotToRad(drawRot), 0.0f));
    MV1DrawModel(gameModelHandles[id]);
}

void DrawGameMap()
{
    SetUseZBuffer3D(TRUE);
    SetWriteZBuffer3D(TRUE);

    for (int y = 0; y < BLOCK_NUM_Y; y++)
    {
        // 2026-05-20: ŠK’iƒ[ƒhŒã‚ÍŒ»Ý‘w‚ð’†S‚ÉAŒ©’Ê‚µ—p‚Æ‚µ‚Äã‰º1ŠK‘w‚¾‚¯•`‰æ‚·‚éB
        if (y < gameLoadedLayer - 1 || y > gameLoadedLayer + 1)
            continue;

        for (int z = 0; z < BLOCK_NUM_Z; z++)
        {
            for (int x = 0; x < BLOCK_NUM_X; x++)
            {
                if (GameFloorMap[y][z][x] >= 0)
                {
                    DrawModelById(
                        GameFloorMap[y][z][x],
                        GetModelDrawPosition(FLOOR, x, y, z, GameFloorRot[y][z][x]),
                        GameFloorRot[y][z][x]
                    );
                }

                if (GameWallMapA[y][z][x] >= 0)
                {
                    DrawModelById(
                        GameWallMapA[y][z][x],
                        GetModelDrawPosition(WALL, x, y, z, GameWallRotA[y][z][x]),
                        GameWallRotA[y][z][x]
                    );
                }

                if (GameWallMapB[y][z][x] >= 0)
                {
                    DrawModelById(
                        GameWallMapB[y][z][x],
                        GetModelDrawPosition(WALL, x, y, z, GameWallRotB[y][z][x]),
                        GameWallRotB[y][z][x]
                    );
                }

                if (GameCornerMap[y][z][x] >= 0)
                {
                    DrawModelById(
                        GameCornerMap[y][z][x],
                        GetModelDrawPosition(CORNER, x, y, z, GameCornerRot[y][z][x]),
                        GameCornerRot[y][z][x]
                    );
                }

                if (GameDecoMap[y][z][x] >= 0)
                {
                    DrawModelById(
                        GameDecoMap[y][z][x],
                        GetModelDrawPosition(DECO, x, y, z, GameDecoRot[y][z][x]),
                        GameDecoRot[y][z][x]
                    );
                }
            }
        }
    }
    /*debug—p
    DrawFormatString(
        20,
        120,
        GetColor(255, 255, 255),
        "LoadedLayer:%d",
        gameLoadedLayer
    );
    */
    DrawFormatString(
        10, 10,
        GetColor(255, 255, 0),
        "Map:%d  Start:%d,%d,%d  Goal:%d,%d,%d",
        gameCurrentMapIndex,
        gameStartX, gameStartY, gameStartZ,
        gameGoalX, gameGoalY, gameGoalZ
    );
}

#include "MapLoaderMiniMap.inl"

#pragma endregion


#pragma region ===== ƒZƒ‹’PˆÊ”»’è =====



