//SceneManager.cpp
#include"SceneManager.h"

void SceneManager::Update() {
	if (!transition && next) {
		current = std::move(next);
	}
	if (transition) {
		transition->Update();
		if (transition->GetState() == TransitionState::Switching && next) {
			current = std::move(next);
			transition->StartExit();
		}

		if (transition->IsFinished()) {
			transition.reset();
			current->Init();
		}
		return;
	}
	if (current) current->Update(*this);
}

void SceneManager::Draw() {
	if (current) current->Draw();
	if (transition)transition->Draw();
}
void SceneManager::ChangeScene(unique_ptr<Scene> newScene,
							   unique_ptr<Transition> trans) {
	next = move(newScene);
	transition = move(trans);
}

void SceneManager::Trans(unique_ptr<Transition> trans) {
	transition = move(trans);
}

SoundManager& SceneManager::GetSoundManager() {
	return soundManager;
}