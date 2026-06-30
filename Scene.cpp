//Scene.cpp
#include"Scene.h"
#include"Constant.h"
#include"SceneManager.h"
#include"MapLoader.h"
#include"Character.h"
#include"Input.h"
#include"UI.h"
#include"UIManager.h"
#include"DxLib.h"
#include"MapEditor.h"
#include"Image.h"
#include"FontManager.h"
// 2026-06-02: 取るもの/避けるもの/鍵/投げ敵をゲームシーンで動かすため追加。
#include"GameObjects.h"
// 2026-06-12: �����G��EVENT���番�����A�G��p�����Ƃ��ē��������ߒǉ��B
#include"GameEnemies.h"
#include<fstream>
#include<vector>
#include<string>
#include<cstring>
#include<windows.h>
using namespace std;
Player p;
Camela c(p);
static bool IsMapCsvExists(int mapIndex)
{
    if (mapIndex <= 0)
        return false;

	char fileName[64];
	if (gameCurrentMapName[0] != '\0')
	{
		sprintf_s(
			fileName,
			"%s.csv",
			gameCurrentMapName
		);
	}
	else
	{
		sprintf_s(
			fileName,
			"map%d.csv",
			mapIndex
		);
	}

    ifstream ifs(fileName);
    return ifs.good();
}

static vector<string> titleMapNames;

static void RefreshTitleMapNames();

static int FindTitleMapNameIndex(const char* mapName);

static void MoveTitleMapSelection(
	int& selectedMapIndex,
	int direction
);

static const char* GetSelectedTitleMapName(
	int selectedMapIndex
);

static bool IsEditorMapStartGoalOnFloor();

static int FindTitleMapNameIndex(const char* mapName)
{
	for (int i = 0; i < (int)titleMapNames.size(); i++)
	{
		if (titleMapNames[i] == mapName)
			return i;
	}

	return 0;
}

static bool IsTitleMapConfigCsv(const char* name)
{
	return strcmp(name, "model_config") == 0 || strcmp(name, "collision_config") == 0;
}

static string GetCsvBaseName(const char* fileName)
{
	string name = fileName;
	size_t dot = name.find_last_of('.');
	if (dot != string::npos)
		name = name.substr(0, dot);
	return name;
}



static void RefreshTitleMapNames()
{
	// 2026-05-27: タイトルではエディターで付けたCSV名を一覧にして、遊ぶマップを名前で選べるようにする。
	titleMapNames.clear();

	WIN32_FIND_DATAA findData;
	HANDLE handle = FindFirstFileA("*.csv", &findData);
	if (handle != INVALID_HANDLE_VALUE)
	{
		do
		{
			if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
				continue;

			string baseName = GetCsvBaseName(findData.cFileName);
			if (IsTitleMapConfigCsv(baseName.c_str()))
				continue;

			titleMapNames.push_back(baseName);
		} while (FindNextFileA(handle, &findData));

		FindClose(handle);
	}

	if (titleMapNames.empty())
		titleMapNames.push_back("map1");
}



static int FindSelectableMapIndex(int currentIndex, int direction)
{
    // 2026-05-18: map1.csv, map2.csv のように増やしたCSVだけをタイトル画面で選べるようにする。
    const int maxMapIndex = 99;

    for (int i = 1; i <= maxMapIndex; i++)
    {
        int nextIndex = currentIndex + direction * i;

        if (nextIndex < 1)
            nextIndex = maxMapIndex + nextIndex;

        if (nextIndex > maxMapIndex)
            nextIndex = nextIndex - maxMapIndex;

        if (IsMapCsvExists(nextIndex))
            return nextIndex;
    }

    return currentIndex;
}

static const char* GetSelectedTitleMapName(int selectedMapIndex)
{
	if (titleMapNames.empty())
		RefreshTitleMapNames();

	if (selectedMapIndex < 0 || selectedMapIndex >= (int)titleMapNames.size())
		return "map1";

	return titleMapNames[selectedMapIndex].c_str();
}

Title::Title():first(false), selectedMapIndex(0) {
	Init();
}

void Title::Init() {img = LoadGraph(title_image.path);
	SetMouseDispFlag(TRUE);
	changeFlg = 0;
	// 2026-06-15: タイトルから遊ぶ時、START/GOALが床の上にないマップを止める警告用。
	titleMapFloorWarning = false;
	// 2026-05-18: 前回選ばれていたマップ番号をタイトルの初期表示に使う。
	RefreshTitleMapNames();

	selectedMapIndex =gameCurrentMapName[0] != '\0'? FindTitleMapNameIndex(gameCurrentMapName): 0;
	uiManager.SetScrollEnabled(false);

	auto title_text = make_shared<Label>(Pos{ 100, 100 }, 0, 0, "TITLE", WHITE, 32);

	auto gameBtn = make_shared<Button>(Pos{ 1000, 300 }, 150, 50);
	gameBtn->SetOnClick([this]() {
		changeFlg = 1;
		});

	auto game_text = make_shared<Label>(Pos{ 1050, 320 }, 0, 0, "遊ぶ", BLACK);

	// 2026-05-18: CSVを切り替えてから遊べるよう、タイトルにマップ選択ボタンを追加。
	auto mapPrevBtn = make_shared<Button>(Pos{ 910, 230 }, 80, 40);
	mapPrevBtn->SetOnClick([this]() {
		MoveTitleMapSelection(selectedMapIndex, -1);
		});

	auto mapPrevText = make_shared<Label>(Pos{ 930, 242 }, 0, 0, "前へ", BLACK);

	auto mapNextBtn = make_shared<Button>(Pos{ 1160, 230 }, 80, 40);
	mapNextBtn->SetOnClick([this]() {
		MoveTitleMapSelection(selectedMapIndex, 1);
		});

	auto mapNextText = make_shared<Label>(Pos{ 1180, 242 }, 0, 0, "次へ", BLACK);

	auto createBtn = make_shared<Button>(Pos{ 1000, 400 }, 150, 50);
	createBtn->SetOnClick([this]() {
		changeFlg = 2;
		});

	auto create_text = make_shared<Label>(Pos{ 1050, 420 }, 0, 0, "作る", BLACK);

	auto optionBtn = make_shared<Button>(Pos{ 1000, 500 }, 150, 50);
	optionBtn->SetOnClick([this]() {
		changeFlg = 3;
		});

	auto option_text = make_shared<Label>(Pos{ 1050, 520 }, 0, 0, "設定", BLACK);

	auto endBtn = make_shared<Button>(Pos{ 1000, 600 }, 150, 50);
		endBtn->SetOnClick([this]() {
		DxLib_End();
		});

	auto end_text = make_shared<Label>(Pos{ 1050, 620 }, 0, 0, "終了", BLACK);

	//グループID設定
	mapPrevBtn->SetGroupId(1);
	mapNextBtn->SetGroupId(1);

	gameBtn->SetGroupId(2);

	createBtn->SetGroupId(3);

	optionBtn->SetGroupId(4);

	endBtn->SetGroupId(5);

	//UIMAnagerに追加
	uiManager.Add(title_text);

	uiManager.Add(mapPrevBtn);
	uiManager.Add(mapPrevText);
	uiManager.Add(mapNextBtn);
	uiManager.Add(mapNextText);

	uiManager.Add(gameBtn);
	uiManager.Add(game_text);

	uiManager.Add(createBtn);
	uiManager.Add(create_text);

	uiManager.Add(optionBtn);
	uiManager.Add(option_text);

	uiManager.Add(endBtn);
	uiManager.Add(end_text);
}
static void MoveTitleMapSelection(int& selectedMapIndex, int direction)
{
	if (titleMapNames.empty())
		RefreshTitleMapNames();

	if (titleMapNames.empty())
		return;

	selectedMapIndex += direction;
	if (selectedMapIndex < 0)
		selectedMapIndex = (int)titleMapNames.size() - 1;
	if (selectedMapIndex >= (int)titleMapNames.size())
		selectedMapIndex = 0;
}
void Title::Draw() {
	DrawGraph(0,0,img,true);
	//DrawString(WIDTH / 2, HEIGHT / 2, "タイトル画面",GetColor(255,255,255));
	uiManager.Draw();
	// 2026-05-18: 選択中のCSVが分かるよう、ボタンとは別に現在のマップ番号を表示する。
	DrawFormatString(1020, 240, WHITE, "MAP  %s.csv", GetSelectedTitleMapName(selectedMapIndex));

	if (titleMapFloorWarning) {
		// 2026-06-15: タイトルから遊ぶ時も、START/GOALが床上にない理由を表示するため追加。
		DrawBox(300, 250, 980, 420, GetColor(10, 10, 10), TRUE);
		DrawBox(300, 250, 980, 420, GetColor(255, 255, 255), FALSE);
		DrawString(330, 282, "スタートとゴールは床の上に置いてください。", GetColor(255, 255, 0));
		DrawString(330, 320, "このマップはまだ遊べません。", GetColor(220, 240, 255));
		DrawString(330, 360, "Enter/Space/C/F9: 閉じる", GetColor(220, 240, 255));
	}
}
void Title::Update(SceneManager& manager) {
	if (titleMapFloorWarning) {
		if (Input::IsKeyTrigger(KEY_INPUT_RETURN) || Input::IsKeyTrigger(KEY_INPUT_SPACE) ||
			Input::IsKeyTrigger(KEY_INPUT_C) || Input::IsKeyTrigger(KEY_INPUT_F9) ||
			Input::IsKeyTrigger(KEY_INPUT_Y) || Input::IsKeyTrigger(KEY_INPUT_N)) {
			titleMapFloorWarning = false;
		}
		return;
	}
	
	if (!first) {
		option.LoadOption(config);
		manager.GetSoundManager().SetVolume(config);
		Input::SetConfig(config);
		first = true;

		uiManager.SetSoundManager(&manager.GetSoundManager());
		manager.GetSoundManager().Play("title_bgm");
	}

	//Padのボタンを調べるやつ
	/*
	DINPUT_JOYSTATE js;

	if (GetJoypadDirectInputState(DX_INPUT_PAD1, &js) == 0)
	{
		for (int i = 0; i < 32; i++)
		{
			if (js.Buttons[i])
			{
				printfDx("Button[%d] が押された\n", i);
			}
		}
	}
	if (Input::IsPadTrigger(PAD_INPUT_7)) {
		printfDx("aaa");
	}
	*/
	//Padを振動させるやつ
	//StartJoypadVibration(DX_INPUT_PAD1, 1000, 2000, -1);

	switch (changeFlg) {
	case 1:
		// 2026-05-27: タイトルで選んだCSV名をゲーム側の読み込み名へ渡す。
		strcpy_s(gameCurrentMapName, sizeof(gameCurrentMapName), GetSelectedTitleMapName(selectedMapIndex));
		gameCurrentMapIndex = selectedMapIndex + 1;
		// 2026-06-15: タイトルから遊ぶ時も、START/GOALが床上にないマップは開始しない。
		LoadMapByName(gameCurrentMapName);
		if (!IsEditorMapStartGoalOnFloor()) {
			titleMapFloorWarning = true;
			changeFlg = 0;
			break;
		}
		manager.GetSoundManager().Play("confirm");
		manager.ChangeScene(
			make_unique<Game>(),
			make_unique<Fade>()
		);
		break;
	case 2:
		// 2026-05-27: タイトルで選んだCSV名をエディター側にも渡し、再編集できるようにする。
		strcpy_s(gameCurrentMapName, sizeof(gameCurrentMapName), GetSelectedTitleMapName(selectedMapIndex));
		manager.GetSoundManager().Play("confirm");
		manager.ChangeScene(
			make_unique<Create>(),
			make_unique<Slide>()
		);
		break;
	case 3:
		manager.GetSoundManager().Play("confirm");
		manager.ChangeScene(
			make_unique<OptionMenu>(),
			make_unique<Slide>()
		);
		break;
	}
	uiManager.Update();
}

Game::Game(GameLaunchMode launchMode) : launchMode(launchMode){
	Init();
}

void Game::Init() {
	isPause = false;
	goTitle = false;

	confirmSaveOnTitle = false;

	if (gameCurrentMapName[0] != '\0')
	{
		LoadMapByName(gameCurrentMapName);
	}

	Input::CenterMouse();

	restart = false;
	goOption = false;
	showingOption = false;

	timer.Reset();

	BuildPauseUI();

	InitGameMap();
	// 2026-06-02: マップ読込後のEVENT配置からゲーム中ギミックを生成するため追加。
	InitGameObjects();
	// 2026-06-12: [ENEMY]����ǂ񂾓G���AEVENT�M�~�b�N�Ƃ͕ʂɏ��������邽�ߒǉ��B
	//InitGameEnemies();
	p.Init();
}

void Game::Draw() {
	p.Draw();
	//DrawString(WIDTH / 2, HEIGHT-550, "ゲーム画面", GetColor(255, 255, 255));
	DrawGameMap();
	// 2026-05-18: 迷路探索中に階層と通った道が見えるよう、3D描画の後にミニマップを重ねる。
	// 2026-06-02: 取るもの/鍵/敵弾などを3D上に表示し、状態HUDも出すため追加。
	DrawGameObjects();
	// 2026-06-12: �G�{�̂ƓG�e��EVENT�`�悩�番���ĕ`�悷�邽�ߒǉ��B
	//DrawGameEnemies();
	DrawMiniMap(p.getVECTOR());
	DrawGameObjectHUD();
	DrawFormatString(20, 92, WHITE, "LIFE %d", p.GetLife());

	elapsed = (timer.Elapsed()/1000)%60;
	int minutes = timer.Elapsed() / 60000;
	
	DrawFormatStringToHandle(WIDTH / 2, 100, WHITE, FontManager::Get(32), "%02d:%02d", minutes, elapsed);

	if (isPause) {
		if (showingOption) {
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, 200);
			DrawBox(0, 0, WIDTH, HEIGHT, BLACK, TRUE);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

			optionUI.Draw();
		}
		else {
			DrawOption();
			uiManager.Draw();
		}
	}
}

void Game::Update(SceneManager& manager) {

	if (Input::IsActionTrigger(Action::Pause)) {
		isPause = !isPause;
		Input::CenterMouse();
	}

	if (!isPause) 
	{
		SetMouseDispFlag(FALSE);
		Input::UpdateMouseDeltaFromCenter();

		p.Update();
		c.Update();
		UpdateGameMap();

		timer.Resume();

		// 2026-06-02: 取るもの/避けるもの/鍵扉/投げ敵をプレイヤー位置に合わせて更新するため追加。
		bool playerDamaged = false;
		int playerLayer = GetMapLayerFromWorldY(p.getVECTOR().y + BLOCK_SIZE * 0.5f);
		UpdateGameObjects(p.getVECTOR(), playerLayer, playerDamaged);
		// 2026-06-12: �GAI�ƓG�e������Ɨ����čX�V���邽�ߒǉ��B
	//	UpdateGameEnemies(p.getVECTOR(), playerLayer, playerDamaged);
		if (playerDamaged)
		{
			manager.Trans(make_unique<Fade>());
			p.Damage();
		}

		// 2026-06-02: ライフが尽きた時にゲームを終わらせるため追加。
		if (!p.IsAlive())
		{
			manager.ChangeScene(make_unique<Title>(), make_unique<Slide>());
			return;
		}

		// 2026-06-02: 鍵または収集条件を満たした時だけゴールできるよう追加。
		// 6/19 変更
		if (IsGoal())
		{
			timer.Pause();
			manager.ChangeScene(make_unique<Result>(timer.Elapsed(), GameLaunchMode::EditorTestPlay), make_unique<Goal>());
			return;
		}
	}
	else {
		SetMouseDispFlag(TRUE);
		uiManager.SetSoundManager(&manager.GetSoundManager());
		timer.Pause();
		if (showingOption) {
			optionUI.SetSoundManager(&manager.GetSoundManager());
			optionUI.Update();

			if (optionUI.IsBackRequested()) {
				optionUI.ResetBackRequest();
				showingOption = false;
				BuildPauseUI();
			}
		}
		else {
			uiManager.Update();

			if (restart) {
				manager.GetSoundManager().Play("confirm");
				manager.ChangeScene(
					make_unique<Game>(),
					make_unique<Slide>()
				);
				return;
			}

			if (goTitle) {
				manager.GetSoundManager().Play("confirm");
				if (launchMode == GameLaunchMode::EditorTestPlay)
					manager.ChangeScene(make_unique<Create>(), make_unique<Slide>());
				else
					manager.ChangeScene(make_unique<Title>(), make_unique<Slide>());
				return;
			}

			if (goOption) {
				manager.GetSoundManager().Play("confirm");
				goOption = false;
				showingOption = true;
				optionUI.Init();
				return;
			}
		}
		return;
	}
}

void Game::DrawOption() {
	SetUseZBuffer3D(FALSE);
	SetWriteZBuffer3D(FALSE);

	// 背景を少し暗くする
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);
	DrawBox(0, 0, WIDTH, HEIGHT, BLACK, TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	// 中央パネル
	int panelX = 440;
	int panelY = 140;
	int panelW = 400;
	int panelH = 460;

	DrawBox(panelX, panelY, panelX + panelW, panelY + panelH, GetColor(20, 20, 30), TRUE);
	DrawBox(panelX, panelY, panelX + panelW, panelY + panelH, WHITE, FALSE);

	DrawString(panelX + 145, panelY + 35, "PAUSE", WHITE);

	SetUseZBuffer3D(TRUE);
	SetWriteZBuffer3D(TRUE);
}

void Game::BuildPauseUI() {
	uiManager.Clear();
	uiManager.SetScrollEnabled(false);

	goTitle = false;
	restart = false;
	goOption = false;

	int panelX = 440;
	int panelY = 140;

	int btnX = panelX + 75;
	int btnY = panelY + 110;
	int btnW = 250;
	int btnH = 60;
	int gap = 80;

	auto backBtn = make_shared<Button>(Pos{ btnX, btnY }, btnW, btnH);
	backBtn->SetOnClick([this]() {
		isPause = false;
		});

	auto backText = make_shared<Label>(Pos{ btnX + 95, btnY + 20 }, 0, 0, "再開", BLACK, 20);

	auto optionBtn = make_shared<Button>(Pos{ btnX, btnY + gap }, btnW, btnH);
	optionBtn->SetOnClick([this]() {
		goOption = true;
		});

	auto optionText = make_shared<Label>(Pos{ btnX + 95, btnY + gap + 20 }, 0, 0, "設定", BLACK, 20);

	auto restartBtn = make_shared<Button>(Pos{ btnX, btnY + gap * 2 }, btnW, btnH);
	restartBtn->SetOnClick([this]() {
		restart = true;
		});

	auto restartText = make_shared<Label>(Pos{ btnX + 75, btnY + gap * 2 + 20 }, 0, 0, "リスタート", BLACK, 20);

	auto titleBtn = make_shared<Button>(Pos{ btnX, btnY + gap * 3 }, btnW, btnH);
	titleBtn->SetOnClick([this]() {
		goTitle = true;
		});

	const char* exitLabel = launchMode == GameLaunchMode::EditorTestPlay ? "編集に戻る" : "タイトルへ";
	auto titleText = make_shared<Label>(Pos{ 580, 510 }, 0, 0, exitLabel, BLACK, 20);


	backBtn->SetGroupId(0);
	optionBtn->SetGroupId(1);
	restartBtn->SetGroupId(2);
	titleBtn->SetGroupId(3);

	uiManager.Add(backBtn);
	uiManager.Add(backText);
	uiManager.Add(optionBtn);
	uiManager.Add(optionText);
	uiManager.Add(restartBtn);
	uiManager.Add(restartText);
	uiManager.Add(titleBtn);
	uiManager.Add(titleText);
}
// 6/19追加
// ゴールの判定長いからこれにまとめる
bool Game::IsGoal() {
	int playerLayer = GetMapLayerFromWorldY(p.getVECTOR().y + BLOCK_SIZE * 0.5f);
	return IsGoalWorldPosition(playerLayer, p.getVECTOR().x, p.getVECTOR().z) && IsGameObjectGoalUnlocked();
}

Result::Result(int time, GameLaunchMode launchMode) : clearTime(time) ,launchMode(launchMode){
	Init();
	SetMouseDispFlag(TRUE);
}

void Result::Init() {
	uiManager.Clear();
	uiManager.SetScrollEnabled(false);

	goalImg = LoadGraph(resultGOAL_image.path);
	
	totalTime = 0;
	elapsed = 0;
	minutes = 0;

	changeflg = 0;

	goalScale = 0.0f;
	goalScaleBack = false;

	totalTime = 0;
	countTimer = 0;

	auto titleBtn = make_shared<Button>(Pos{ 400, 600 }, 150, 50);
	titleBtn->SetOnClick([this]() {
		changeflg = 1;
		});

	const char* exitLabel = launchMode == GameLaunchMode::EditorTestPlay ? "編集に戻る" : "タイトルへ";
	auto title_text = make_shared<Label>(Pos{ 425, 620 }, 0, 0, exitLabel, BLACK, 16);

	auto retryBtn = make_shared<Button>(Pos{ 700, 600 }, 150, 50);
	retryBtn->SetOnClick([this]() {
		changeflg = 2;
		});

	auto retry_text = make_shared<Label>(Pos{ 750, 620 }, 0, 0, "リトライ", BLACK);

	titleBtn->SetGroupId(1);
	retryBtn->SetGroupId(1);

	uiManager.Add(titleBtn);
	uiManager.Add(title_text);
	uiManager.Add(retryBtn);
	uiManager.Add(retry_text);
}

void Result::Draw() {
	DrawRotaGraph(WIDTH / 2, 50 + resultGOAL_image.h / 2, goalScale, 0.0, goalImg, true);
	DrawFormatString(300, 400, WHITE, "CLEAR TIME : %02d:%02d", minutes, elapsed);
	uiManager.Draw();
}

void Result::Update(SceneManager& manager) {
	uiManager.Update();

	ScaleUpAnimation();
	CountUp();
	

	switch (changeflg) {
	case 1:
		manager.GetSoundManager().Play("confirm");
		if (launchMode == GameLaunchMode::EditorTestPlay)
			manager.ChangeScene(make_unique<Create>(), make_unique<Slide>());
		else
			manager.ChangeScene(make_unique<Title>(), make_unique<Slide>());
		break;
	case 2:
		manager.GetSoundManager().Play("confirm");
		manager.ChangeScene(
			make_unique<Game>(),
			make_unique<Slide>()
		);
		break;
	}
}

void Result::ScaleUpAnimation() {
	if (!goalScaleBack)
	{
		goalScale += 0.05f;
		if (goalScale >= 1.15f) {
			goalScale = 1.15f;
			goalScaleBack = true;
		}
	}
	else
	{
		goalScale += (1.0f - goalScale) * 0.15f;
		if (fabsf(goalScale - 1.0f) < 0.01f) {
			goalScale = 1.0f;
		}
	}
}

void Result::CountUp() {
	if (!ScaleUpEnd())return;
	if (countTimer < countDuration)
	{
		countTimer++;

		float t = (float)countTimer / countDuration;

		t = 1.0f - (1.0f - t) * (1.0f - t);

		totalTime = (int)(clearTime * t);
	}
	else
	{
		totalTime = clearTime;
	}

	elapsed = (totalTime / 1000) % 60;
	minutes = totalTime / 60000;
}

static bool IsEditorMapPosInRange(int layer, int z, int x)
{
	return layer >= 0 && layer < BLOCK_NUM_Y && z >= 0 && z < BLOCK_NUM_Z && x >= 0 && x < BLOCK_NUM_X;
}
static bool IsEditorMapStartGoalOnFloor()
{
	// 2026- 06-19
	// : ゲーム開始時に落下/無効位置にならないよう、STARTとGOALの足元に床があるか確認するため追加。
	return IsEditorMapPosInRange(startY, startZ, startX) &&
		IsEditorMapPosInRange(goalY, goalZ, goalX) &&
		FloorMap[startY][startZ][startX] >= 0 &&
		FloorMap[goalY][goalZ][goalX] >= 0;
}

Create::Create() {
	Init();
}

void Create::Init() {
	SetMouseDispFlag(TRUE);
	InitEditor();
	goTitle = false;
	// 2026-05-19: エディターの戻るボタンは固定表示にしたいので、ホイールスクロールで動かないようにする。
	uiManager.SetScrollEnabled(false);

	auto titleBtn = make_shared<Button>(Pos{ 650, 10 }, 130, 36);
	titleBtn->SetOnClick([this]() {
		goTitle = true;
		});
	auto playCurrentMapBtn = make_shared<Button>(Pos{ 470, 10 }, 160, 36);
	playCurrentMapBtn->SetOnClick([this]() {
		playCurrentMap = true;
		});

	auto playCurrentMapText = make_shared<Label>(Pos{ 500, 20 }, 0, 0, "テストプレイ", BLACK);

	auto titleText = make_shared<Label>(Pos{ 682, 20 }, 0, 0, "タイトル", BLACK);

	uiManager.Add(titleBtn);
	uiManager.Add(titleText);
	uiManager.Add(playCurrentMapBtn);
	uiManager.Add(playCurrentMapText);
}

void Create::Update(SceneManager& manager) {
	if (confirmSaveOnTitle) {
		// 2026-06-12: 未保存のままタイトルへ戻る時、編集内容を破棄してよいか確認するため追加。
		if (Input::IsKeyTrigger(KEY_INPUT_Y)) {
			ClearEditorMapDirty();
			confirmSaveOnTitle = false;
			manager.ChangeScene(make_unique<Title>(), make_unique<Slide>());
			return;
		}
		if (Input::IsKeyTrigger(KEY_INPUT_N) || Input::IsKeyTrigger(KEY_INPUT_C) || Input::IsKeyTrigger(KEY_INPUT_F9)) {
			confirmSaveOnTitle = false;
			goTitle = false;
			return;
		}

		return;
	}
	uiManager.Update();

	if (goTitle) {
		// 2026-05-27: 変更がある時は、タイトルへ戻る前に保存確認を出す。
		// 2026-06-12: 保存するかではなく、保存せず戻るかの確認に変更。
		if (IsEditorMapDirty()) {
			confirmSaveOnTitle = true;
			goTitle = false;
			return;
		}
		manager.ChangeScene(make_unique<Title>(), make_unique<Slide>());
		return;
	}
	if (playCurrentMap) {
		// 2026-06-15: エディターとゲーム処理は分けたまま、現在のマップを保存してGameシーンへ渡すため追加。
		playCurrentMap = false;
		if (!IsEditorMapStartGoalOnFloor()) {
			playMapFloorWarning = true;
			return;
		}
		if (IsEditorMapDirty()) {
			SaveMap();
		}
		if (gameCurrentMapName[0] == '\0') {
			gameCurrentMapIndex = currentMapIndex;
		}
		manager.ChangeScene(make_unique<Game>(GameLaunchMode::EditorTestPlay), make_unique<Slide>());
		return;
	}



	UpdateEditor();
}

void Create::Draw() {
	DrawEditor();
	uiManager.Draw();


	if (playMapFloorWarning) {
		// 2026-06-15: START/GOALが床の上にない時、プレイできない理由を画面に出すため追加。
		DrawBox(300, 250, 980, 420, GetColor(10, 10, 10), TRUE);
		DrawBox(300, 250, 980, 420, GetColor(255, 255, 255), FALSE);
		DrawString(330, 282, "スタートとゴールは床の上に置いてください。", GetColor(255, 255, 0));
		DrawString(330, 320, "床がない場所からは、このマップで遊べません。", GetColor(220, 240, 255));
		DrawString(330, 360, "Enter/Space/C/F9: 閉じる", GetColor(220, 240, 255));
	}
	else if (confirmSaveOnTitle) {
		// 2026-06-15: タイトルへ戻る確認中も、警告が見えるよう描画を戻す。
		DrawBox(300, 260, 980, 400, GetColor(10, 10, 10), TRUE);
		DrawBox(300, 260, 980, 400, GetColor(255, 255, 255), FALSE);
		DrawString(330, 292, "保存されてません。保存せず戻りますか？", GetColor(255, 255, 0));
		DrawString(330, 330, "Y: 保存せず戻る   N/C/F9: キャンセル", GetColor(220, 240, 255));
	}
}

OptionMenu::OptionMenu() {
	Init();
}

void OptionMenu::Init() {
	optionUI.Init();
	SetMouseDispFlag(TRUE);
}

void OptionMenu::Update(SceneManager& manager) {
	optionUI.SetSoundManager(&manager.GetSoundManager());
	uiManager.SetSoundManager(&manager.GetSoundManager());
	optionUI.Update();

	if (optionUI.IsBackRequested()) {
		manager.ChangeScene(
			make_unique<Title>(),
			make_unique<Slide>()
		);
	}
}

void OptionMenu::Draw() {
	optionUI.Draw();
}