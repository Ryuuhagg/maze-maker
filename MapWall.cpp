#include "MapEditor.h"

#pragma region ===== 壁1マス2個配置 =====

void UpdateWallTile(int y, int z, int x)
{
    if (x < 0 || x >= BLOCK_NUM_X) return;
    if (y < 0 || y >= BLOCK_NUM_Y) return;
    if (z < 0 || z >= BLOCK_NUM_Z) return;

    // 壁A
    if (WallMapA[y][z][x] >= 0)
    {
        bool up = (z > 0 && WallMapA[y][z - 1][x] >= 0);
        bool down = (z < BLOCK_NUM_Z - 1 && WallMapA[y][z + 1][x] >= 0);
        bool left = (x > 0 && WallMapA[y][z][x - 1] >= 0);
        bool right = (x < BLOCK_NUM_X - 1 && WallMapA[y][z][x + 1] >= 0);

        if (left || right)
            WallRotA[y][z][x] = 1;
        else
            WallRotA[y][z][x] = 0;
    }

    // 壁B
    if (WallMapB[y][z][x] >= 0)
    {
        bool up = (z > 0 && WallMapB[y][z - 1][x] >= 0);
        bool down = (z < BLOCK_NUM_Z - 1 && WallMapB[y][z + 1][x] >= 0);
        bool left = (x > 0 && WallMapB[y][z][x - 1] >= 0);
        bool right = (x < BLOCK_NUM_X - 1 && WallMapB[y][z][x + 1] >= 0);

        if (left || right)
            WallRotB[y][z][x] = 1;
        else
            WallRotB[y][z][x] = 0;
    }
}

#pragma endregion



#pragma region ===== 全壁更新 =====

void AutoConnectWalls()
{
    for (int y = 0; y < BLOCK_NUM_Y; y++)
    {
        for (int z = 0; z < BLOCK_NUM_Z; z++)
        {
            for (int x = 0; x < BLOCK_NUM_X; x++)
            {
                UpdateWallTile(y, z, x);
            }
        }
    }
}

#pragma endregion