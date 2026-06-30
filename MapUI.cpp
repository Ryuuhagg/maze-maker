#include "MapEditor.h"
#include "DxLib.h"
#include <windows.h>
#include <math.h>
#include <cstring>

#pragma region ===== UI定数 =====

static const int TAB_W = 76;
static const int TAB_H = 28;
static const int PAD = 12;

static const int PALETTE_ICON = 48;
static const int PALETTE_STEP = 62;

#pragma endregion


#pragma region ===== フォント =====

static const char* EDITOR_FONT_PATH = "font/LightNovelPOPv2.otf";
static const char* TITLE_FONT_NAME = "ラノベPOP V2";
static const char* BODY_FONT_NAME = "メイリオ";

static int uiFont = -1;
static int uiSmallFont = -1;
static int uiTitleFont = -1;

static void InitEditorUIFont()
{
    if (uiFont != -1)
        return;

    AddFontResourceExA(EDITOR_FONT_PATH, FR_PRIVATE, NULL);

    uiFont = CreateFontToHandle(
        BODY_FONT_NAME,
        15,
        2,
        DX_FONTTYPE_ANTIALIASING
    );

    uiSmallFont = CreateFontToHandle(
        BODY_FONT_NAME,
        13,
        1,
        DX_FONTTYPE_ANTIALIASING
    );

    uiTitleFont = CreateFontToHandle(
        TITLE_FONT_NAME,
        17,
        2,
        DX_FONTTYPE_ANTIALIASING
    );
}

static void DrawText(int x, int y, const char* text, int color)
{
    InitEditorUIFont();

    if (uiFont != -1)
        DrawStringToHandle(x, y, text, color, uiFont);
    else
        DrawString(x, y, text, color);
}

static void DrawSmallText(int x, int y, const char* text, int color)
{
    InitEditorUIFont();

    if (uiSmallFont != -1)
        DrawStringToHandle(x, y, text, color, uiSmallFont);
    else
        DrawString(x, y, text, color);
}

static void DrawTitleText(int x, int y, const char* text, int color)
{
    InitEditorUIFont();

    if (uiTitleFont != -1)
        DrawStringToHandle(x, y, text, color, uiTitleFont);
    else
        DrawString(x, y, text, color);
}

#pragma endregion


#pragma region ===== UI補助 =====

static int GetUIX()
{
    return SCREEN_W - UI_WIDTH;
}

static void DrawPanelTitle(int x, int y, const char* text)
{
    DrawTitleText(x, y, text, GetColor(190, 190, 190));
    DrawLine(x, y + 22, x + UI_WIDTH - 24, y + 22, GetColor(70, 70, 70));
}

static void DrawButton(int x1, int y1, int x2, int y2, const char* text, bool active)
{
    int bg = active ? GetColor(220, 220, 80) : GetColor(70, 70, 70);
    int fg = active ? GetColor(0, 0, 0) : GetColor(235, 235, 235);

    DrawBox(x1, y1, x2, y2, bg, TRUE);
    DrawBox(x1, y1, x2, y2, GetColor(25, 25, 25), FALSE);
    DrawSmallText(x1 + 7, y1 + 6, text, fg);
}

static void DrawSmallTextFit(int x, int y, int maxWidth, const char* text, int color)
{
    // 2026-06-28: 長いキット名が左パネルから飛び出さないよう、表示幅に収まるところで省略する。
    InitEditorUIFont();

    if (uiSmallFont == -1 || GetDrawStringWidthToHandle(text, (int)strlen(text), uiSmallFont) <= maxWidth)
    {
        DrawSmallText(x, y, text, color);
        return;
    }

    char buf[128];
    strcpy_s(buf, sizeof(buf), text);
    const char* suffix = "...";
    int suffixW = GetDrawStringWidthToHandle(suffix, 3, uiSmallFont);
    int len = (int)strlen(buf);

    while (len > 0)
    {
        buf[--len] = '\0';
        if ((unsigned char)buf[len] >= 0x80 && len > 0)
            buf[--len] = '\0';

        int width = GetDrawStringWidthToHandle(buf, (int)strlen(buf), uiSmallFont) + suffixW;
        if (width <= maxWidth)
            break;
    }

    strcat_s(buf, sizeof(buf), suffix);
    DrawSmallText(x, y, buf, color);
}

#pragma endregion


// 2026-06-02: EVENT素材が増えても右側一覧で役割名を一貫して表示するため追加。
static const char* GetEditorEventDisplayName(int id)
{
    if (id == 0) return "開始";
    if (id == 1) return "ゴール";
    if (id == 2) return "取るもの";
    if (id == 3) return "避けるもの";
    if (id == 4) return "カギ";
    if (id == 5) return "カギ扉";
    if (id == 6) return "投げ敵";
    // 2026-
    // : EVENT ID 7をエディターから選べるよう、表示名を追加。
    if (id == 7) return "チェックポイント";
    return "イベント";
}

// 2026-06-25: 選択中アイテム名をUIへ出すため、ENEMY側も表示名で扱えるよう戻した。
static const char* GetEditorEnemyDisplayName(int id)
{
    if (id == 6) return "投げ敵";
    return "敵";
}

// 2026-06-25: 配置前に「何を選んでいるか」を確認できるよう、現在タブとIDから表示名を返す。
static const char* GetEditorSelectedItemDisplayName()
{
    if (selectedModel < 0 || selectedModel >= tabModelCount[currentTab])
        return "未選択";

    int id = tabModelList[currentTab][selectedModel];
    if (currentTab == FLOOR)
    {
        if (id == 0) return "床";
        return "床パーツ";
    }
    if (currentTab == WALL)
    {
        if (id == 1) return "壁";
        if (id == 3) return "上壁";
        if (id == 4) return "装飾壁";
        return "壁パーツ";
    }
    if (currentTab == CORNER)
    {
        if (id == 2) return "角";
        return "角パーツ";
    }
    if (currentTab == DECO)
    {
        if (id == 5) return "階段";
        if (id == 6) return "床レイヤー";
        if (id == 7) return "高床レイヤー";
        return "装飾";
    }
    if (currentTab == ENEMY)
        return GetEditorEnemyDisplayName(id);
    if (currentTab == EVENT)
        return GetEditorEventDisplayName(id);

    return "未選択";
}

// 2026-06-02: 3Dモデル配置の階層/回転/素材切替を右側パネル最下部から直接触れるよう追加。
static void DrawModelOperationPanel(int uiX)
{
    DrawPanelTitle(uiX + PAD, 622, "3Dモデル操作");
    DrawButton(uiX + PAD, 650, uiX + 80, 676, "階層-", false);
    DrawButton(uiX + 92, 650, uiX + 160, 676, "階層+", false);
    DrawButton(uiX + 172, 650, uiX + 248, 676, "回転", false);
    DrawButton(uiX + PAD, 684, uiX + 80, 710, "素材<", false);
    DrawButton(uiX + 92, 684, uiX + 160, 710, "素材>", false);
    DrawFormatStringToHandle(uiX + 172, 690, GetColor(255, 230, 80), uiSmallFont, "L%d R%d", currentLayer, currentRot);
}
static bool IsSelectedCollisionValid()
{
    // 2026-05-20: 右側パネルで操作できるBOXコライダーが選択されているか確認する。
    return
        selectedCollisionLayer >= 0 && selectedCollisionLayer < BLOCK_NUM_Y &&
        selectedCollisionZ >= 0 && selectedCollisionZ < BLOCK_NUM_Z &&
        selectedCollisionX >= 0 && selectedCollisionX < BLOCK_NUM_X &&
        CollisionMap[selectedCollisionLayer][selectedCollisionZ][selectedCollisionX] >= 0;
}

static void AdjustSelectedCollisionBox(int offsetX, int offsetZ, int sizeX, int sizeZ)
{
    // 2026-05-20: 3D上で細かくつかまず、右側パネルのボタンで選択中BOXを調整できるようにする。
    if (!IsSelectedCollisionValid())
        return;

    PushUndo();

    int y = selectedCollisionLayer;
    int z = selectedCollisionZ;
    int x = selectedCollisionX;

    CollisionBoxOffsetXMap[y][z][x] += offsetX;
    CollisionBoxOffsetZMap[y][z][x] += offsetZ;
    CollisionBoxSizeXMap[y][z][x] = max(40, CollisionBoxSizeXMap[y][z][x] + sizeX);
    CollisionBoxSizeZMap[y][z][x] = max(40, CollisionBoxSizeZMap[y][z][x] + sizeZ);
}
// 2026-06-02: 右側の簡易ビューから直接BOXを触れるよう、クリック位置をコライダー値へ反映するため追加。
static void AdjustSelectedCollisionBoxFromPreview(int mx, int my)
{
    if (!IsSelectedCollisionValid())
        return;

    const int uiX = SCREEN_W - UI_WIDTH;
    const int previewX = uiX + PAD;
    const int previewY = 212;
    const int previewSize = 220;
    if (mx < previewX || mx > previewX + previewSize || my < previewY || my > previewY + previewSize)
        return;

    int y = selectedCollisionLayer;
    int z = selectedCollisionZ;
    int x = selectedCollisionX;
    float localX = (mx - previewX) / (float)previewSize;
    float localZ = (my - previewY) / (float)previewSize;
    float worldX = x * BLOCK_SIZE + localX * BLOCK_SIZE;
    float worldZ = z * BLOCK_SIZE + localZ * BLOCK_SIZE;

    float centerX = x * BLOCK_SIZE + BLOCK_SIZE * 0.5f + CollisionBoxOffsetXMap[y][z][x];
    float centerZ = z * BLOCK_SIZE + BLOCK_SIZE * 0.5f + CollisionBoxOffsetZMap[y][z][x];
    float halfX = max(20.0f, CollisionBoxSizeXMap[y][z][x] * 0.5f);
    float halfZ = max(20.0f, CollisionBoxSizeZMap[y][z][x] * 0.5f);
    float left = centerX - halfX;
    float right = centerX + halfX;
    float top = centerZ - halfZ;
    float bottom = centerZ + halfZ;
    float handle = BLOCK_SIZE * 0.14f;

    PushUndo();

    if (fabsf(worldX - left) <= handle)
    {
        left = min(worldX, right - 40.0f);
    }
    else if (fabsf(worldX - right) <= handle)
    {
        right = max(worldX, left + 40.0f);
    }
    else if (fabsf(worldZ - top) <= handle)
    {
        top = min(worldZ, bottom - 40.0f);
    }
    else if (fabsf(worldZ - bottom) <= handle)
    {
        bottom = max(worldZ, top + 40.0f);
    }
    else
    {
        left = worldX - halfX;
        right = worldX + halfX;
        top = worldZ - halfZ;
        bottom = worldZ + halfZ;
    }

    float cellCenterX = x * BLOCK_SIZE + BLOCK_SIZE * 0.5f;
    float cellCenterZ = z * BLOCK_SIZE + BLOCK_SIZE * 0.5f;
    CollisionBoxOffsetXMap[y][z][x] = (int)(((left + right) * 0.5f) - cellCenterX);
    CollisionBoxOffsetZMap[y][z][x] = (int)(((top + bottom) * 0.5f) - cellCenterZ);
    CollisionBoxSizeXMap[y][z][x] = max(40, (int)(right - left));
    CollisionBoxSizeZMap[y][z][x] = max(40, (int)(bottom - top));
}

// 2026-06-02: 右側パネルで現在のBOX形状を見ながら直接触れるよう、Unity風の簡易トップビューを追加。
static void DrawSelectedCollisionPreview(int uiX)
{
    const int previewX = uiX + PAD;
    const int previewY = 212;
    const int previewSize = 220;

    DrawPanelTitle(uiX + PAD, 184, "BOXプレビュー");
    DrawBox(previewX, previewY, previewX + previewSize, previewY + previewSize, GetColor(38, 38, 42), TRUE);
    DrawBox(previewX, previewY, previewX + previewSize, previewY + previewSize, GetColor(95, 95, 100), FALSE);

    for (int i = 1; i < 4; i++)
    {
        int gx = previewX + previewSize * i / 4;
        int gy = previewY + previewSize * i / 4;
        DrawLine(gx, previewY, gx, previewY + previewSize, GetColor(58, 58, 64));
        DrawLine(previewX, gy, previewX + previewSize, gy, GetColor(58, 58, 64));
    }

    if (!IsSelectedCollisionValid())
    {
        DrawSmallText(previewX + 58, previewY + 96, "BOXを選択", GetColor(210, 210, 210));
        DrawSmallText(previewX + 16, previewY + 122, "左クリックで作成/選択", GetColor(180, 180, 180));
        return;
    }

    int y = selectedCollisionLayer;
    int z = selectedCollisionZ;
    int x = selectedCollisionX;
    float left = 0.5f + CollisionBoxOffsetXMap[y][z][x] / BLOCK_SIZE - CollisionBoxSizeXMap[y][z][x] / (BLOCK_SIZE * 2.0f);
    float right = left + CollisionBoxSizeXMap[y][z][x] / BLOCK_SIZE;
    float top = 0.5f + CollisionBoxOffsetZMap[y][z][x] / BLOCK_SIZE - CollisionBoxSizeZMap[y][z][x] / (BLOCK_SIZE * 2.0f);
    float bottom = top + CollisionBoxSizeZMap[y][z][x] / BLOCK_SIZE;

    int l = previewX + (int)(left * previewSize);
    int r = previewX + (int)(right * previewSize);
    int t = previewY + (int)(top * previewSize);
    int b = previewY + (int)(bottom * previewSize);
    int color = GetColor(120, 255, 150);

    DrawBox(l, t, r, b, GetColor(28, 92, 54), TRUE);
    DrawBox(l, t, r, b, color, FALSE);
    DrawLine(l, (t + b) / 2, r, (t + b) / 2, GetColor(80, 180, 110));
    DrawLine((l + r) / 2, t, (l + r) / 2, b, GetColor(80, 180, 110));
    DrawCircle((l + r) / 2, (t + b) / 2, 8, GetColor(80, 255, 120), TRUE);
    DrawBox(l - 5, t - 5, l + 5, t + 5, GetColor(255, 255, 80), TRUE);
    DrawBox(r - 5, t - 5, r + 5, t + 5, GetColor(255, 255, 80), TRUE);
    DrawBox(r - 5, b - 5, r + 5, b + 5, GetColor(255, 255, 80), TRUE);
    DrawBox(l - 5, b - 5, l + 5, b + 5, GetColor(255, 255, 80), TRUE);
    DrawSmallText(previewX, previewY + previewSize + 8, "中央:移動  辺:サイズ", GetColor(220, 220, 220));
}
// 2026-06-02: コライダー専用画面の右パネルを通常編集と分けて表示するため追加。
static void DrawColliderEditorPanel(int uiX)
{
    DrawPanelTitle(uiX + PAD, 18, "コライダー編集");

    DrawSmallText(uiX + PAD, 50, "左ドラッグ : 角/辺/中心を直接編集", GetColor(220, 220, 220));
    DrawSmallText(uiX + PAD, 70, "右クリック : コライダー削除", GetColor(220, 220, 220));
    DrawSmallText(uiX + PAD, 90, "通常配置とは別画面", GetColor(255, 230, 80));

    DrawPanelTitle(uiX + PAD, 126, "表示");
    DrawFormatStringToHandle(
        uiX + PAD, 152,
        GetColor(230, 230, 230),
        uiSmallFont,
        "階層 : %d\n編集 : %s\n辺調整 : %s",
        currentLayer,
        collisionEdgeEditMode ? "辺" : "BOX",
        collisionDepthEditMode ? "厚み" : "長さ"
    );

    DrawSelectedCollisionPreview(uiX);

    DrawPanelTitle(uiX + PAD, 506, "モード");
    DrawButton(uiX + PAD, 568, uiX + 116, 594, "BOX編集", !collisionEdgeEditMode);
    DrawButton(uiX + 128, 568, uiX + 248, 594, "辺編集", collisionEdgeEditMode);
    DrawButton(uiX + PAD, 600, uiX + 116, 626, "通常へ", false);
    DrawButton(uiX + 128, 600, uiX + 248, 626, collisionDepthEditMode ? "辺 厚み" : "辺 長さ", collisionEdgeEditMode && collisionDepthEditMode);

    DrawPanelTitle(uiX + PAD, 632, "選択BOX");

    if (IsSelectedCollisionValid())
    {
        int y = selectedCollisionLayer;
        int z = selectedCollisionZ;
        int x = selectedCollisionX;

        DrawFormatStringToHandle(
            uiX + PAD, 654,
            GetColor(255, 230, 40),
            uiSmallFont,
            "%d,%d,%d  位置 %d,%d  サイズ %d,%d",
            x, y, z,
            CollisionBoxOffsetXMap[y][z][x],
            CollisionBoxOffsetZMap[y][z][x],
            CollisionBoxSizeXMap[y][z][x],
            CollisionBoxSizeZMap[y][z][x]
        );

        int bx = uiX + PAD;
        int bw = 52;
        int gap = 6;
        DrawButton(bx, 674, bx + bw, 696, "X-", false);
        DrawButton(bx + (bw + gap), 674, bx + (bw + gap) + bw, 696, "X+", false);
        DrawButton(bx + (bw + gap) * 2, 674, bx + (bw + gap) * 2 + bw, 696, "Z-", false);
        DrawButton(bx + (bw + gap) * 3, 674, bx + (bw + gap) * 3 + bw, 696, "Z+", false);

        DrawButton(bx, 698, bx + bw, 720, "幅-", false);
        DrawButton(bx + (bw + gap), 698, bx + (bw + gap) + bw, 720, "幅+", false);
        DrawButton(bx + (bw + gap) * 2, 698, bx + (bw + gap) * 2 + bw, 720, "奥-", false);
        DrawButton(bx + (bw + gap) * 3, 698, bx + (bw + gap) * 3 + bw, 720, "奥+", false);
    }
    else
    {
        DrawSmallText(uiX + PAD, 656, "BOXをクリックして選択", GetColor(210, 210, 210));
        DrawSmallText(uiX + PAD, 676, "何もない場所をクリックすると作成", GetColor(210, 210, 210));
    }
}
static int GetPaletteMaxScrollY()
{
    const int visibleHeight = 286 - 112;
    int contentHeight = tabModelCount[currentTab] * PALETTE_STEP;
    return max(0, contentHeight - visibleHeight);
}

static void ClampEditorPaletteScroll()
{
    int maxScroll = GetPaletteMaxScrollY();
    editorPaletteScrollY = max(0, min(editorPaletteScrollY, maxScroll));
}

static bool UpdateLegacyEditorUIWheel(int mx, int my, int wheel)
{
    if (wheel == 0 || editorScreenMode == 1)
        return false;

    int uiX = GetUIX();
    bool inPalette =
        mx >= uiX + PAD && mx <= SCREEN_W - PAD &&
        my >= 112 && my <= 286;

    if (!inPalette)
        return false;

    // 2026-06-15: EVENT素材が増えてきたため、素材一覧上のホイールだけ欄スクロールに使う。
    editorPaletteScrollY -= wheel * 28;
    ClampEditorPaletteScroll();
    return true;
}

#pragma region ===== UIクリック更新 =====

static void UpdateLegacyEditorUI(int mx, int my, int lClick)
{
    if (!lClick)
        return;

    int uiX = GetUIX();
    ClampEditorPaletteScroll();

    // 2026-06-02: コライダー専用画面では通常タブ/素材選択を触らず、コライダー操作だけ受け付けるため追加。
    if (editorScreenMode == 1)
    {
        // 2026-06-02: 右側BOXプレビュー上をクリックして、中心移動やサイズ変更を直接できるよう追加。
        if (mx >= uiX + PAD && mx <= uiX + PAD + 220 && my >= 212 && my <= 432)
        {
            AdjustSelectedCollisionBoxFromPreview(mx, my);
            return;
        }

        // 既存のボタン座標を使い、表示とクリック位置を一致させる。
        if (mx >= uiX + PAD && mx <= uiX + 116 && my >= 568 && my <= 594)
        {
            collisionEditMode = true;
            collisionEdgeEditMode = false;
            collisionDepthEditMode = false;
            showCollisionDebug = true;
            return;
        }

        if (mx >= uiX + 128 && mx <= uiX + 248 && my >= 568 && my <= 594)
        {
            collisionEditMode = true;
            collisionEdgeEditMode = true;
            collisionDepthEditMode = false;
            showCollisionDebug = true;
            return;
        }

        if (mx >= uiX + PAD && mx <= uiX + 116 && my >= 600 && my <= 626)
        {
            editorScreenMode = 0;
            collisionEditMode = false;
            collisionBoxDragging = false;
            return;
        }

        if (mx >= uiX + 128 && mx <= uiX + 248 && my >= 600 && my <= 626)
        {
            collisionDepthEditMode = !collisionDepthEditMode;
            return;
        }

        if (IsSelectedCollisionValid())
        {
            const int step = 20;
            int bx = uiX + PAD;
            int by = 674;
            int bw = 52;
            int bh = 22;
            int gap = 6;

            if (my >= by && my <= by + bh)
            {
                if (mx >= bx && mx <= bx + bw) { AdjustSelectedCollisionBox(-step, 0, 0, 0); return; }
                if (mx >= bx + (bw + gap) && mx <= bx + (bw + gap) + bw) { AdjustSelectedCollisionBox(step, 0, 0, 0); return; }
                if (mx >= bx + (bw + gap) * 2 && mx <= bx + (bw + gap) * 2 + bw) { AdjustSelectedCollisionBox(0, -step, 0, 0); return; }
                if (mx >= bx + (bw + gap) * 3 && mx <= bx + (bw + gap) * 3 + bw) { AdjustSelectedCollisionBox(0, step, 0, 0); return; }
            }

            by = 698;
            if (my >= by && my <= by + bh)
            {
                if (mx >= bx && mx <= bx + bw) { AdjustSelectedCollisionBox(0, 0, -step, 0); return; }
                if (mx >= bx + (bw + gap) && mx <= bx + (bw + gap) + bw) { AdjustSelectedCollisionBox(0, 0, step, 0); return; }
                if (mx >= bx + (bw + gap) * 2 && mx <= bx + (bw + gap) * 2 + bw) { AdjustSelectedCollisionBox(0, 0, 0, -step); return; }
                if (mx >= bx + (bw + gap) * 3 && mx <= bx + (bw + gap) * 3 + bw) { AdjustSelectedCollisionBox(0, 0, 0, step); return; }
            }
        }

        return;
    }

#pragma region タブ切替

    for (int i = 0; i < TAB_MAX; i++)
    {
        int col = i % 3;
        int row = i / 3;

        int x1 = uiX + PAD + col * (TAB_W + 6);
        int y1 = 32 + row * (TAB_H + 6);
        int x2 = x1 + TAB_W;
        int y2 = y1 + TAB_H;

        if (mx >= x1 && mx <= x2 &&
            my >= y1 && my <= y2)
        {
            currentTab = i;
            selectedModel = 0;
            editorPaletteScrollY = 0;
            return;
        }
    }

#pragma endregion


#pragma region 素材選択

    int px = uiX + PAD;
    int py = 112;

    for (int i = 0; i < tabModelCount[currentTab]; i++)
    {
        int y = py + i * PALETTE_STEP - editorPaletteScrollY;

        if (y >= py && y <= 286 && mx >= px && mx <= px + PALETTE_ICON &&
            my >= y && my <= y + PALETTE_ICON)
        {
            selectedModel = i;
            return;
        }
    }

#pragma endregion


#pragma region ボタン

    if (mx >= uiX + PAD && mx <= uiX + 116 &&
        my >= 536 && my <= 562)
    {
        showGrid = !showGrid;
        return;
    }

    if (mx >= uiX + 128 && mx <= uiX + 248 &&
        my >= 536 && my <= 562)
    {
        brushMode = !brushMode;
        return;
    }

    // 2026-06-02: 右側パネル下部の3Dモデル操作から、階層/回転/素材を直接変更できるよう追加。
    if (my >= 650 && my <= 676)
    {
        if (mx >= uiX + PAD && mx <= uiX + 80) { currentLayer = max(0, currentLayer - 1); return; }
        if (mx >= uiX + 92 && mx <= uiX + 160) { currentLayer = min(BLOCK_NUM_Y - 1, currentLayer + 1); return; }
        if (mx >= uiX + 172 && mx <= uiX + 248) { currentRot = (currentRot + 1) % 4; return; }
    }
    if (my >= 684 && my <= 710)
    {
        if (mx >= uiX + PAD && mx <= uiX + 80 && tabModelCount[currentTab] > 0)
        {
            selectedModel--;
            if (selectedModel < 0) selectedModel = tabModelCount[currentTab] - 1;
            return;
        }
        if (mx >= uiX + 92 && mx <= uiX + 160 && tabModelCount[currentTab] > 0)
        {
            selectedModel++;
            if (selectedModel >= tabModelCount[currentTab]) selectedModel = 0;
            return;
        }
    }
    // 2026-05-20: セル編集はBoxCollider風に直接触る編集へ変え、ボタン名もBOX編集に合わせる。
    if (mx >= uiX + PAD && mx <= uiX + 116 &&
        my >= 568 && my <= 594)
    {
        // 2026-06-02: BOX編集は通常配置と切り離した専用画面で操作できるよう追加。
        editorScreenMode = 1;
        collisionEditMode = true;
        collisionEdgeEditMode = false;
        // 2026-05-20: BOX編集へ戻った時に辺用の厚みモード選択が残らないようにする。
        collisionDepthEditMode = false;
        showCollisionDebug = true;
        return;
    }

    if (mx >= uiX + 128 && mx <= uiX + 248 &&
        my >= 568 && my <= 594)
    {
        // 2026-06-02: 辺編集もコライダー専用画面へ入れて、配置操作と混ざらないよう追加。
        editorScreenMode = 1;
        collisionEditMode = true;
        collisionEdgeEditMode = true;
        // 2026-05-13: 辺編集を開始した時はまず長さモードから触れるよう追加。
        collisionDepthEditMode = false;
        showCollisionDebug = true;
        return;
    }

    if (mx >= uiX + PAD && mx <= uiX + 116 &&
        my >= 600 && my <= 626)
    {
        // 2026-06-02: コライダー専用画面から通常配置画面へ戻れるよう追加。
        editorScreenMode = 0;
        collisionEditMode = false;
        collisionBoxDragging = false;
        return;
    }

    if (mx >= uiX + 128 && mx <= uiX + 248 &&
        my >= 600 && my <= 626)
    {
    // 2026-05-13: Shift操作ではなくボタンで辺の長さ編集/厚み編集を切り替えるため変更。
        collisionDepthEditMode = !collisionDepthEditMode;
        return;
    }
    // 2026-05-20: 右側のコライダーパネルを触るだけで、選択済みBOXの位置/大きさを調整できるよう追加。
    if (IsSelectedCollisionValid())
    {
        const int step = 20;
        int bx = uiX + PAD;
        int by = 674;
        int bw = 52;
        int bh = 22;
        int gap = 6;

        if (my >= by && my <= by + bh)
        {
            if (mx >= bx && mx <= bx + bw) { AdjustSelectedCollisionBox(-step, 0, 0, 0); return; }
            if (mx >= bx + (bw + gap) && mx <= bx + (bw + gap) + bw) { AdjustSelectedCollisionBox(step, 0, 0, 0); return; }
            if (mx >= bx + (bw + gap) * 2 && mx <= bx + (bw + gap) * 2 + bw) { AdjustSelectedCollisionBox(0, -step, 0, 0); return; }
            if (mx >= bx + (bw + gap) * 3 && mx <= bx + (bw + gap) * 3 + bw) { AdjustSelectedCollisionBox(0, step, 0, 0); return; }
        }

        by = 698;
        if (my >= by && my <= by + bh)
        {
            if (mx >= bx && mx <= bx + bw) { AdjustSelectedCollisionBox(0, 0, -step, 0); return; }
            if (mx >= bx + (bw + gap) && mx <= bx + (bw + gap) + bw) { AdjustSelectedCollisionBox(0, 0, step, 0); return; }
            if (mx >= bx + (bw + gap) * 2 && mx <= bx + (bw + gap) * 2 + bw) { AdjustSelectedCollisionBox(0, 0, 0, -step); return; }
            if (mx >= bx + (bw + gap) * 3 && mx <= bx + (bw + gap) * 3 + bw) { AdjustSelectedCollisionBox(0, 0, 0, step); return; }
        }
    }
#pragma endregion
}

#pragma endregion


#pragma region ===== UI描画 =====

static void DrawLegacyEditorUI()
{
    InitEditorUIFont();

    int uiX = GetUIX();

#pragma region 背景

    DrawBox(
        uiX, 0,
        SCREEN_W, SCREEN_H,
        GetColor(18, 18, 20),
        TRUE
    );

    DrawLine(uiX, 0, uiX, SCREEN_H, GetColor(80, 80, 80));

#pragma endregion

}

// 2026-06-25: 画面構成案に合わせ、上部コマンド・左素材一覧・中央ワークスペース・右折りたたみパネルへ再構成。既存の編集データ処理は変えずUI座標だけを分離する。
static const int EDITOR_TOP_H = 56;
static const int EDITOR_LEFT_W = 220;
static const int EDITOR_RIGHT_W = 300;
static const int EDITOR_RIGHT_X = SCREEN_W - EDITOR_RIGHT_W;
static bool editorHelpPanelOpen = true;
static bool editorObjectPanelOpen = true;

// 2026-06-25: 左側を「大分類の縦タブ」と「オブジェクト内の横タブ」の
// 二段構造にする。KITは対応データ追加まで入口だけを表示する。
enum EditorPrimaryTab
{
    EDITOR_PRIMARY_OBJECT,
    EDITOR_PRIMARY_ENEMY,
    EDITOR_PRIMARY_EVENT,
    EDITOR_PRIMARY_KIT,
    EDITOR_PRIMARY_MAX
};
static int editorPrimaryTab = EDITOR_PRIMARY_OBJECT;
// 2026-06-26: キットも床/壁のように「ベース」「自作」を横タブで切り替えられるよう保持する。
static int editorKitSubTab = 0;

static bool IsEditorKitSubTabCsv()
{
    return editorKitSubTab == 0;
}

static const char* GetEditorSelectedKitDisplayName()
{
    int index = GetSelectedEditorKitIndex();
    if (index < 0)
        return "キット未選択";
    return GetEditorKitName(index);
}

static void GetEditorRightPanelLayout(int& helpBottom, int& objectTop, int& objectBottom)
{
    // 2026-06-25: 操作説明にコピー/複数選択行を追加したため、下端案内と被らない高さに広げる。
    helpBottom = editorHelpPanelOpen ? 404 : EDITOR_TOP_H + 34;
    objectTop = helpBottom;
    objectBottom = editorObjectPanelOpen ? SCREEN_H : objectTop + 34;
}

bool IsEditorWorkspacePoint(int mx, int my)
{
    if (my < EDITOR_TOP_H || mx < EDITOR_LEFT_W)
        return false;

    if (mx < EDITOR_RIGHT_X)
        return true;

    int helpBottom = 0;
    int objectTop = 0;
    int objectBottom = 0;
    GetEditorRightPanelLayout(helpBottom, objectTop, objectBottom);
    return my >= objectBottom;
}

static void DrawNewEditorPalette()
{
    if (editorPrimaryTab == EDITOR_PRIMARY_KIT)
    {
        // 2026-06-26: 選択中のキット横タブだけを表示し、ベースCSVと自作キットが混ざらないようにする。
        // 2026-06-28: 登録・サムネ更新・再読み込みを近い位置にまとめ、キット作成の手数を減らす。
        DrawButton(14, 354, 108, 384, "登録", false);
        DrawButton(112, 354, 206, 384, "サムネ", GetSelectedEditorKitIndex() >= 0);
        DrawButton(14, 392, 108, 422, "更新", false);
        DrawButton(112, 392, 206, 422, "ベース更新", false);
        DrawSmallTextFit(16, 434, 190, GetEditorSelectedKitDisplayName(), GetColor(210, 230, 255));

        int kitCount = GetEditorKitCount();
        if (kitCount <= 0)
        {
            DrawSmallText(16, 462, "まだキットがありません", GetColor(170, 180, 195));
            return;
        }

        const int listTop = 462;
        const int kitItemHeight = 92;
        const int kitItemStep = 100;
        const int kitThumbSize = 76;
        int y = listTop - editorPaletteScrollY;
        bool showCsvKit = IsEditorKitSubTabCsv();
        bool hasVisibleKit = false;
        for (int i = 0; i < kitCount; i++)
        {
            if (IsEditorKitFromCsv(i) == showCsvKit)
            {
                hasVisibleKit = true;
                break;
            }
        }

        if (!hasVisibleKit)
        {
            DrawSmallText(16, listTop, showCsvKit ? "ベースキットがありません" : "自作キットがありません", GetColor(170, 180, 195));
            return;
        }

        for (int i = 0; i < kitCount; i++)
        {
            if (IsEditorKitFromCsv(i) != showCsvKit)
                continue;

            if (y > SCREEN_H - 12)
                break;

            if (y >= listTop)
            {
                bool active = GetSelectedEditorKitIndex() == i;
                // 2026-06-28: キット内容を判別しやすくするため、一覧カードとサムネ表示を大きくする。
                DrawBox(14, y, 206, y + kitItemHeight, active ? GetColor(210, 80, 255) : GetColor(72, 78, 88), TRUE);
                DrawBox(17, y + 3, 203, y + kitItemHeight - 3, GetColor(30, 34, 42), TRUE);

                int thumb = GetEditorKitThumbnailHandle(i);
                if (thumb >= 0)
                    DrawExtendGraph(22, y + 8, 22 + kitThumbSize, y + 8 + kitThumbSize, thumb, TRUE);
                else
                    DrawBox(22, y + 8, 22 + kitThumbSize, y + 8 + kitThumbSize, GetColor(48, 52, 62), TRUE);

                DrawSmallTextFit(106, y + 14, 94, GetEditorKitName(i), GetColor(235, 238, 245));
                DrawSmallText(106, y + 42, "クリックで配置", GetColor(170, 190, 220));
            }

            y += kitItemStep;
        }
        return;
    }

    const int paletteTop = 344;
    const int iconSize = 52;
    const int stepX = 64;
    const int stepY = 68;

    for (int i = 0; i < tabModelCount[currentTab]; i++)
    {
        int col = i % 3;
        int row = i / 3;
        int x = 14 + col * stepX;
        int y = paletteTop + row * stepY - editorPaletteScrollY;
        if (y < paletteTop - iconSize || y > SCREEN_H - 12)
            continue;

        int id = tabModelList[currentTab][i];
        int border = selectedModel == i ? GetColor(255, 215, 70) : GetColor(72, 78, 88);
        DrawBox(x - 3, y - 3, x + iconSize + 3, y + iconSize + 3, border, TRUE);
        DrawBox(x, y, x + iconSize, y + iconSize, GetColor(30, 34, 42), TRUE);
        if (id >= 0 && id < MODEL_MAX && paletteTex[id] > 0)
            DrawExtendGraph(x, y, x + iconSize, y + iconSize, paletteTex[id], TRUE);
        else
            DrawFormatStringToHandle(x + 8, y + 18, GetColor(220, 225, 235), uiSmallFont, "%d", id);
    }
}

static void DrawNewEditorHelpPanel(int top, int bottom)
{
    DrawBox(EDITOR_RIGHT_X, top, SCREEN_W, bottom, GetColor(24, 28, 35), TRUE);
    DrawLine(EDITOR_RIGHT_X, top, SCREEN_W, top, GetColor(76, 84, 98));
    DrawTitleText(EDITOR_RIGHT_X + 16, top + 7, "操作説明", GetColor(235, 238, 245));
    DrawSmallText(SCREEN_W - 34, top + 10, editorHelpPanelOpen ? "－" : "＋", GetColor(255, 220, 90));

    if (!editorHelpPanelOpen)
        return;

    int x = EDITOR_RIGHT_X + 18;
    DrawSmallText(x, top + 48, "左クリック        配置 / 選択", GetColor(210, 215, 225));
    DrawSmallText(x, top + 72, "右クリック        削除 / キャンセル", GetColor(210, 215, 225));
    DrawSmallText(x, top + 96, "Shift + ドラッグ  視点回転", GetColor(210, 215, 225));
    DrawSmallText(x, top + 120, "ホイール          ズーム", GetColor(210, 215, 225));
    DrawSmallText(x, top + 144, "Q / E             階層変更", GetColor(210, 215, 225));
    DrawSmallText(x, top + 168, "R                 回転", GetColor(210, 215, 225));
    DrawSmallText(x, top + 192, "Ctrl+C            コピー", GetColor(210, 215, 225));
    DrawSmallText(x, top + 216, "Ctrl+V            貼り付け", GetColor(210, 215, 225));
    DrawSmallText(x, top + 240, "C                 一括削除", GetColor(210, 215, 225));
    DrawSmallText(x, top + 264, "Ctrl+左クリック   複数選択", GetColor(210, 215, 225));
    DrawSmallText(x, top + 288, "Shift+R           選択だけ回転", GetColor(210, 215, 225));

    DrawSmallText(x, bottom - 26, "右上の－ボタンで非表示", GetColor(150, 190, 230));
}

static void DrawNewEditorObjectPanel(int top, int bottom)
{
    DrawBox(EDITOR_RIGHT_X, top, SCREEN_W, bottom, GetColor(20, 24, 31), TRUE);
    DrawLine(EDITOR_RIGHT_X, top, SCREEN_W, top, GetColor(76, 84, 98));
    DrawTitleText(EDITOR_RIGHT_X + 16, top + 7, "3Dオブジェクト操作", GetColor(235, 238, 245));
    DrawSmallText(SCREEN_W - 34, top + 10, editorObjectPanelOpen ? "－" : "＋", GetColor(255, 220, 90));

    if (!editorObjectPanelOpen)
        return;

    int x = EDITOR_RIGHT_X + 16;
    int y = top + 48;
    DrawFormatStringToHandle(x, y, GetColor(255, 220, 90), uiFont,
        "階層 %d   回転 %d   モデル %d", currentLayer, currentRot, GetSelectedModel());
    // 2026-06-25: 右パネル操作中でも配置予定アイテムを確認できるよう、選択名を表示する。
    DrawFormatStringToHandle(x, y + 26, GetColor(210, 230, 255), uiSmallFont,
        "選択中 : %s", editorPrimaryTab == EDITOR_PRIMARY_KIT ? GetEditorSelectedKitDisplayName() : GetEditorSelectedItemDisplayName());

    DrawButton(x, y + 56, x + 80, y + 84, "階層－", false);
    DrawButton(x + 90, y + 56, x + 170, y + 84, "階層＋", false);
    DrawButton(x + 180, y + 56, x + 268, y + 84, "回転", false);
    DrawButton(x, y + 94, x + 126, y + 122, showGrid ? "グリッドON" : "グリッドOFF", showGrid);
    DrawButton(x + 136, y + 94, x + 268, y + 122, brushMode ? "ブラシON" : "ブラシOFF", brushMode);
    DrawButton(x, y + 132, x + 126, y + 160, "BOX編集", collisionEditMode && !collisionEdgeEditMode);
    DrawButton(x + 136, y + 132, x + 268, y + 160, "辺編集", collisionEditMode && collisionEdgeEditMode);
    DrawButton(x, y + 170, x + 126, y + 198, "編集終了", false);
    DrawButton(x + 136, y + 170, x + 268, y + 198,
        collisionDepthEditMode ? "辺：厚み" : "辺：長さ", collisionDepthEditMode);

    if (editorScreenMode == 1)
    {
        DrawSmallText(x, y + 218, "当たり判定編集モード", GetColor(120, 255, 170));
        DrawSmallText(x, y + 242, "Esc または編集終了で戻る", GetColor(205, 215, 225));
    }
}

bool UpdateEditorUIWheel(int mx, int my, int wheel)
{
    if (wheel == 0 || mx < 0 || mx >= EDITOR_LEFT_W || my < 344)
        return false;

    if (editorPrimaryTab == EDITOR_PRIMARY_KIT)
    {
        // 2026-06-26: キットが増えても、現在の「ベース/自作」タブ内だけをホイールで見られるようにする。
        int kitCount = GetEditorKitCount();
        int visibleKitCount = 0;
        bool showCsvKit = IsEditorKitSubTabCsv();
        for (int i = 0; i < kitCount; i++)
        {
            if (IsEditorKitFromCsv(i) == showCsvKit)
                visibleKitCount++;
        }

        int contentHeight = visibleKitCount * 100;
        int maxScroll = max(0, contentHeight - (SCREEN_H - 462));
        editorPaletteScrollY = max(0, min(maxScroll, editorPaletteScrollY - wheel * 28));
        return true;
    }

    int rows = (tabModelCount[currentTab] + 2) / 3;
    int maxScroll = max(0, rows * 68 - (SCREEN_H - 356));
    editorPaletteScrollY = max(0, min(maxScroll, editorPaletteScrollY - wheel * 28));
    return true;
}

void UpdateEditorUI(int mx, int my, int lClick)
{
    if (!lClick)
        return;

    if (my < EDITOR_TOP_H)
    {
        if (mx >= 330 && mx <= 440)
            SaveEditorFromUI();
        return;
    }

    if (mx < EDITOR_LEFT_W)
    {
        // 2026-06-25: 大分類は縦並び。オブジェクトだけが床/壁/角/装飾を内包する。
        // 2026-06-25: 縦タブの「敵/イベント/キット」も押せるよう、描画と同じ全タブ数で判定する。
        for (int i = 0; i < EDITOR_PRIMARY_MAX; i++)
        {
            int y = 78 + i * 42;
            if (my >= y && my <= y + 34)
            {
                editorPrimaryTab = i;
                if (i == EDITOR_PRIMARY_OBJECT)
                {
                    if (currentTab < FLOOR || currentTab > DECO)
                        currentTab = FLOOR;
                }
                else if (i == EDITOR_PRIMARY_ENEMY)
                    currentTab = ENEMY;
                else if (i == EDITOR_PRIMARY_EVENT)
                    currentTab = EVENT;
                else if (i == EDITOR_PRIMARY_KIT)
                    RefreshEditorKitList();
                selectedModel = 0;
                editorPaletteScrollY = 0;
                return;
            }
        }

        if (editorPrimaryTab == EDITOR_PRIMARY_OBJECT)
        {
            for (int i = 0; i < 4; i++)
            {
                int x = 14 + i * 50;
                if (mx >= x && mx <= x + 46 && my >= 264 && my <= 294)
                {
                    currentTab = FLOOR + i;
                    selectedModel = 0;
                    editorPaletteScrollY = 0;
                    return;
                }
            }
        }

        if (editorPrimaryTab == EDITOR_PRIMARY_KIT)
        {
            // 2026-06-26: オブジェクトの床/壁タブと同じ操作感で、キットの分類を切り替える。
            for (int i = 0; i < 2; i++)
            {
                int x = 14 + i * 96;
                if (mx >= x && mx <= x + 90 && my >= 264 && my <= 294)
                {
                    editorKitSubTab = i;
                    editorPaletteScrollY = 0;
                    return;
                }
            }
        }

        if (editorPrimaryTab == EDITOR_PRIMARY_KIT)
        {
            if (mx >= 14 && mx <= 108 && my >= 354 && my <= 384)
            {
                StartEditorKitRegistrationFromSelection();
                return;
            }
            if (mx >= 112 && mx <= 206 && my >= 354 && my <= 384)
            {
                // 2026-06-28: 選択したベース/自作キットのサムネをすぐ作り直せるようにする。
                RefreshSelectedEditorKitThumbnail();
                return;
            }
            if (mx >= 14 && mx <= 108 && my >= 392 && my <= 422)
            {
                RefreshEditorKitList();
                return;
            }
            if (mx >= 112 && mx <= 206 && my >= 392 && my <= 422)
            {
                // 2026-06-26: kitbase.csvを編集した後、エディタ内のボタンでベースキットへ反映できるようにする。
                if (RebuildEditorBaseKitCsv())
                {
                    editorKitSubTab = 0;
                    editorPaletteScrollY = 0;
                }
                return;
            }

            int kitCount = GetEditorKitCount();
            const int listTop = 462;
            const int kitItemHeight = 92;
            const int kitItemStep = 100;
            int y = listTop - editorPaletteScrollY;
            bool showCsvKit = IsEditorKitSubTabCsv();
            for (int i = 0; i < kitCount; i++)
            {
                if (IsEditorKitFromCsv(i) != showCsvKit)
                    continue;

                if (my >= listTop && mx >= 14 && mx <= 206 && my >= y && my <= y + kitItemHeight)
                {
                    SelectEditorKit(i);
                    return;
                }

                y += kitItemStep;
            }
            return;
        }

        const int paletteTop = 344;
        for (int i = 0; i < tabModelCount[currentTab]; i++)
        {
            int x = 14 + (i % 3) * 64;
            int y = paletteTop + (i / 3) * 68 - editorPaletteScrollY;
            if (mx >= x && mx <= x + 52 && my >= y && my <= y + 52)
            {
                selectedModel = i;
                return;
            }
        }
        return;
    }

    if (mx < EDITOR_RIGHT_X)
        return;

    int helpBottom = 0;
    int objectTop = 0;
    int objectBottom = 0;
    GetEditorRightPanelLayout(helpBottom, objectTop, objectBottom);
    if (my >= EDITOR_TOP_H && my <= EDITOR_TOP_H + 34)
    {
        editorHelpPanelOpen = !editorHelpPanelOpen;
        return;
    }
    if (my >= objectTop && my <= objectTop + 34)
    {
        editorObjectPanelOpen = !editorObjectPanelOpen;
        return;
    }
    if (!editorObjectPanelOpen || my > objectBottom)
        return;

    int x = EDITOR_RIGHT_X + 16;
    int y = objectTop + 48;
    if (my >= y + 56 && my <= y + 84)
    {
        if (mx >= x && mx <= x + 80) currentLayer = max(0, currentLayer - 1);
        else if (mx >= x + 90 && mx <= x + 170) currentLayer = min(BLOCK_NUM_Y - 1, currentLayer + 1);
        else if (mx >= x + 180 && mx <= x + 268) currentRot = (currentRot + 1) & 3;
        return;
    }
    if (my >= y + 94 && my <= y + 122)
    {
        if (mx >= x && mx <= x + 126) showGrid = !showGrid;
        else if (mx >= x + 136 && mx <= x + 268) brushMode = !brushMode;
        return;
    }
    if (my >= y + 132 && my <= y + 160)
    {
        editorScreenMode = 1;
        collisionEditMode = true;
        collisionEdgeEditMode = mx >= x + 136;
        collisionDepthEditMode = false;
        showCollisionDebug = true;
        return;
    }
    if (my >= y + 170 && my <= y + 198)
    {
        if (mx <= x + 126)
        {
            editorScreenMode = 0;
            collisionEditMode = false;
            showCollisionDebug = false;
            collisionBoxDragging = false;
        }
        else if (collisionEdgeEditMode)
        {
            collisionDepthEditMode = !collisionDepthEditMode;
        }
    }
}

void DrawEditorUI()
{
    InitEditorUIFont();

    DrawBox(0, 0, SCREEN_W, EDITOR_TOP_H, GetColor(18, 22, 29), TRUE);
    DrawLine(0, EDITOR_TOP_H, SCREEN_W, EDITOR_TOP_H, GetColor(92, 102, 118));
    DrawFormatStringToHandle(18, 18, GetColor(235, 238, 245), uiTitleFont,
        "マップ名  %s", gameCurrentMapName[0] != '\0' ? gameCurrentMapName : "名前無し");
    DrawButton(330, 10, 440, 46, "保存", false);

    DrawBox(0, EDITOR_TOP_H, EDITOR_LEFT_W, SCREEN_H, GetColor(22, 26, 33), TRUE);
    DrawLine(EDITOR_LEFT_W, EDITOR_TOP_H, EDITOR_LEFT_W, SCREEN_H, GetColor(92, 102, 118));
    DrawTitleText(14, 60, "カテゴリ", GetColor(235, 238, 245));

    const char* primaryNames[EDITOR_PRIMARY_MAX] = { "オブジェクト", "敵", "イベント", "キット" };
    for (int i = 0; i < EDITOR_PRIMARY_MAX; i++)
        DrawButton(14, 78 + i * 42, 206, 112 + i * 42, primaryNames[i], editorPrimaryTab == i);

    if (editorPrimaryTab == EDITOR_PRIMARY_OBJECT)
    {
        // 2026-06-25: オブジェクトを開いた時だけ、種類を横タブで表示する。
        const char* objectTabNames[4] = { "床", "壁", "角", "装飾" };
        for (int i = 0; i < 4; i++)
            DrawButton(14 + i * 50, 264, 60 + i * 50, 294,
                objectTabNames[i], currentTab == FLOOR + i);
    }
    else if (editorPrimaryTab == EDITOR_PRIMARY_KIT)
    {
        // 2026-06-26: キットもオブジェクト分類と同じ位置に横タブを出し、ベースと自作を分ける。
        const char* kitTabNames[2] = { "ベース", "自作" };
        for (int i = 0; i < 2; i++)
            DrawButton(14 + i * 96, 264, 104 + i * 96, 294,
                kitTabNames[i], editorKitSubTab == i);
    }

    const char* contentTitle =
        editorPrimaryTab == EDITOR_PRIMARY_OBJECT ? "オブジェクト一覧" :
        editorPrimaryTab == EDITOR_PRIMARY_ENEMY ? "敵一覧" :
        editorPrimaryTab == EDITOR_PRIMARY_EVENT ? "イベント一覧" : "キット一覧";
    DrawTitleText(14, 310, contentTitle, GetColor(235, 238, 245));
    // 2026-06-25: 素材アイコンだけでは判別しにくいため、現在選択中の名前を左一覧にも表示する。
    if (editorPrimaryTab == EDITOR_PRIMARY_KIT)
    {
        DrawSmallText(14, 330, "選択中：", GetColor(210, 230, 255));
        DrawSmallTextFit(70, 330, 138, GetEditorSelectedKitDisplayName(), GetColor(210, 230, 255));
    }
    else
    {
        DrawFormatStringToHandle(14, 330, GetColor(210, 230, 255), uiSmallFont,
            "選択中：%s", GetEditorSelectedItemDisplayName());
    }
    DrawNewEditorPalette();

    int helpBottom = 0;
    int objectTop = 0;
    int objectBottom = 0;
    GetEditorRightPanelLayout(helpBottom, objectTop, objectBottom);
    DrawNewEditorHelpPanel(EDITOR_TOP_H, helpBottom);
    DrawNewEditorObjectPanel(objectTop, objectBottom);

    DrawLine(EDITOR_LEFT_W, EDITOR_TOP_H, EDITOR_RIGHT_X, EDITOR_TOP_H, GetColor(110, 120, 138));
    if (pasteMode)
    {
        // 2026-06-25: コピー配置中の案内が左素材欄に被っていたため、現在階層表示の上へ移動。
        DrawFormatStringToHandle(EDITOR_LEFT_W + 16, SCREEN_H - 54, GetColor(80, 255, 255), uiFont,
            "コピー配置中 : 左クリックで配置 / 右クリックでキャンセル / Rで回転  サイズ %d x %d  回転 %d",
            copySizeX, copySizeZ, copyRotation & 3);
    }
    DrawFormatStringToHandle(EDITOR_LEFT_W + 16, SCREEN_H - 28, GetColor(255, 225, 90), uiFont,
        "現在の階層 : %d    選択中 : %s",
        currentLayer,
        editorPrimaryTab == EDITOR_PRIMARY_KIT ? GetEditorSelectedKitDisplayName() : GetEditorSelectedItemDisplayName());
}

// 2026-06-25: 旧レイアウトは移行確認用に残し、実行経路から外す。
static void DrawLegacyEditorUIRemainder()
{
    InitEditorUIFont();
    int uiX = GetUIX();

#pragma region タブ

    const char* tabName[TAB_MAX] =
    {
        "床",
        "壁",
        "角",
        "装飾",
        "敵",
        "イベント"
    };

    DrawPanelTitle(uiX + PAD, 6, "道具");

    for (int i = 0; i < TAB_MAX; i++)
    {
        int col = i % 3;
        int row = i / 3;

        int x1 = uiX + PAD + col * (TAB_W + 6);
        int y1 = 32 + row * (TAB_H + 6);
        int x2 = x1 + TAB_W;
        int y2 = y1 + TAB_H;

        DrawButton(x1, y1, x2, y2, tabName[i], i == currentTab);
    }

#pragma endregion


#pragma region 素材一覧

    DrawPanelTitle(uiX + PAD, 84, "素材一覧");
    // 2026-06-02: 素材一覧スクロール中でも操作方法が分かるよう、ホイール案内を表示するため追加。
    DrawSmallText(uiX + 150, 88, "ホイール", GetColor(180, 180, 180));

    int px = uiX + PAD;
    int py = 112;

    for (int i = 0; i < tabModelCount[currentTab]; i++)
    {
        int id = tabModelList[currentTab][i];
        int y = py + i * PALETTE_STEP - editorPaletteScrollY;
        if (y < py - PALETTE_ICON || y > 286)
            continue;

        DrawBox(
            px - 3, y - 3,
            px + PALETTE_ICON + 3, y + PALETTE_ICON + 3,
            (selectedModel == i) ? GetColor(255, 230, 40) : GetColor(55, 55, 60),
            TRUE
        );

        DrawBox(
            px, y,
            px + PALETTE_ICON, y + PALETTE_ICON,
            GetColor(35, 35, 38),
            TRUE
        );

        if (id >= 0 && id < MODEL_MAX && paletteTex[id] > 0)
        {
            DrawExtendGraph(
                px, y,
                px + PALETTE_ICON, y + PALETTE_ICON,
                paletteTex[id],
                TRUE
            );
        }
        else
        {
            DrawBox(
                px, y,
                px + PALETTE_ICON, y + PALETTE_ICON,
                GetColor(110, 110, 110),
                TRUE
            );
        }

        if (currentTab == EVENT)
        {
            const char* name = GetEditorEventDisplayName(id);

            DrawStringToHandle(
                px + 64,
                y + 16,
                name,
                GetColor(220, 220, 220),
                uiSmallFont
            );
        }
        /*
        else if (currentTab == ENEMY)
        {
            const char* name = GetEditorEnemyDisplayName(id);

            DrawStringToHandle(
                px + 64,
                y + 16,
                name,
                GetColor(220, 220, 220),
                uiSmallFont
            );
        }
        */
        else
        {
            DrawFormatStringToHandle(
                px + 64, y + 16,
                GetColor(220, 220, 220),
                uiSmallFont,
                "素材 %d",
                id
            );
        }

    }

#pragma endregion


#pragma region 状態

    DrawPanelTitle(uiX + PAD, 300, "状態");

    DrawFormatStringToHandle(
        uiX + PAD, 326,
        GetColor(255, 230, 40),
        uiFont,
        "選択素材 : %d",
        GetSelectedModel()
    );

    // 2026-05-26: 名前付き保存後に、どのCSVへ上書きされるか右側UIで確認できるよう表示する。
    DrawFormatStringToHandle(
        uiX + PAD, 352,
        GetColor(230, 230, 230),
        uiSmallFont,
        "マップ : %d\n"
        "保存名 : %s\n"
        "階層   : %d\n"
        "回転   : %d\n"
        "ブラシ : %s",
        currentMapIndex,
        gameCurrentMapName[0] != '\0' ? gameCurrentMapName : "未保存",
        currentLayer,
        currentRot,
        brushMode ? "ON" : "OFF"
    );

#pragma endregion


#pragma region 操作

    DrawPanelTitle(uiX + PAD, 430, "操作");

    DrawSmallText(uiX + PAD, 456, "Q/E    階層", GetColor(210, 210, 210));
    DrawSmallText(uiX + PAD, 476, "R      回転", GetColor(210, 210, 210));
    DrawSmallText(uiX + PAD, 496, "←/→  番号切替", GetColor(210, 210, 210));
    DrawSmallText(uiX + PAD, 516, "F5/F9 保存/読込", GetColor(210, 210, 210));

    DrawSmallText(uiX + 138, 456, "V   範囲選択", GetColor(210, 210, 210));
    DrawSmallText(uiX + 138, 476, "Ctrl+C コピー", GetColor(210, 210, 210));
    DrawSmallText(uiX + 138, 496, "Ctrl+V 貼付", GetColor(210, 210, 210));
    // 2026-05-20: 手動当たり判定はBOXを直接ドラッグして調整する操作表示へ変える。
    DrawSmallText(uiX + 138, 516, "右クリック削除", GetColor(210, 210, 210));
    DrawSmallText(uiX + PAD, 536, "BOX:ドラッグ調整", GetColor(210, 210, 210));

#pragma endregion


#pragma region ボタン

    DrawButton(
        uiX + PAD, 536,
        uiX + 116, 562,
        showGrid ? "グリッドON" : "グリッドOFF",
        showGrid
    );

    DrawButton(
        uiX + 128, 536,
        uiX + 248, 562,
        brushMode ? "ブラシON" : "ブラシOFF",
        brushMode
    );

    // 2026-05-20: セル編集ではなく、UnityのBoxColliderに近いBOX編集として見せる。
    DrawButton(
        uiX + PAD, 568,
        uiX + 116, 594,
        "BOX編集",
        collisionEditMode && !collisionEdgeEditMode
    );

    DrawButton(
        uiX + 128, 568,
        uiX + 248, 594,
        "辺編集",
        collisionEditMode && collisionEdgeEditMode
    );

    DrawButton(
        uiX + PAD, 600,
        uiX + 116, 626,
        "編集終了",
        false
    );

    // 2026-05-20: BOX編集が主操作になったため、辺の長さ/厚み切替は辺編集中だけ分かる名前にする。
    DrawButton(
        uiX + 128, 600,
        uiX + 248, 626,
        collisionDepthEditMode ? "辺 厚み" : "辺 長さ",
        collisionEdgeEditMode && collisionDepthEditMode
    );
#pragma endregion


#pragma region 3Dモデル操作

    // 2026-06-02: 3Dモデルを触る操作は右側パネルの一番下へまとめ、素材一覧とぶつからないよう追加。
    DrawModelOperationPanel(uiX);

#pragma endregion
}

#pragma endregion

