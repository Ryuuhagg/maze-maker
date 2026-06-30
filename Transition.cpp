//Transition.cpp
#include"DxLib.h"
#include"Transition.h"
#pragma region Fade
void Fade::Update() {
	switch (state)
	{
	case TransitionState::Enter:
		alpha += 20;

		if (alpha >= 255)
		{
			alpha = 255;
			state = TransitionState::Switching;
		}
		break;

	case TransitionState::Exit:
		alpha -= 20;

		if (alpha <= 0)
		{
			alpha = 0;
			state = TransitionState::Finished;
		}
		break;
	}
}

void Fade::Draw() {
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
	DrawBox(0, 0, WIDTH, HEIGHT, GetColor(0, 0, 0), TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

#pragma endregion
#pragma region Slide
void Slide::Update() {
	switch (state) {
	case TransitionState::Enter:
		current += 20;

		if (current >= 0)
		{
			current = 0;
			state = TransitionState::Switching;
		}
		break;

	case TransitionState::Exit:
		current += 20;

		if (current > WIDTH)
		{
			current = WIDTH;
			state = TransitionState::Finished;
		}
		break;
	}
}

void Slide::Draw() {
	DrawBox(current, 0, current+WIDTH, HEIGHT, GetColor(100, 100, 100), TRUE);
}

#pragma endregion
#pragma region GOAL
Goal::Goal() {
	img = LoadGraph(goal_image.path);
	wait.Reset();
}
void Goal::Update() {
	switch (state) {
	case TransitionState::Enter:
		if (wait.After(3000)) { // 3秒後にシーン切り替え
			state = TransitionState::Switching;
		}
		break;

	case TransitionState::Exit:
		state = TransitionState::Finished;
		break;
	}
}
void Goal::Draw() {
	DrawExtendGraph(0, 0, WIDTH, HEIGHT, img, TRUE);
}
#pragma endregion