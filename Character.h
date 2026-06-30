//Character.h
#pragma once
#include"DxLib.h"
#include"Model.h"
#include<vector>
using namespace std;
class Character {
protected:
	float speed;
	VECTOR pos = VGet(0, 0, 0);
public:
	Character(float speed):speed(speed){}
	virtual void Init() = 0;
	virtual void Update() = 0;
	virtual void Draw() = 0;
	VECTOR getVECTOR(){ return pos; };
};

struct Angle {
	float x;
	float y;
};

enum class PlayerState {
	Jump,
	Idle,
	Walk,
	Dash
};


class Player : public Character {
	int m_model = 0;
	int m_animeIndex = 0;
	int m_Size = 0;
	int m_currentAnimNo = -1;
	
	int m_animIdle = 1;
	int m_animWalk = 2;
	int m_animJump = 0;

	bool m_isMoving = false;
	bool m_isDashing = false;

	float y = 0;
	float vy = 0;
	float m_gravity = -0.6f;
	bool m_isGround = true;

	int m_life = 0;

	///ヒロシが追加　レイヤー
	int m_layer = 0;

	PlayerState m_state = PlayerState::Idle;
	Angle angle = { 0.0f, 0.3f };
	Angle C_angle = { 0.0f, 0.3f };
public:
	Player();
	~Player();

	void Init()override;
	void Update()override;
	void Draw() override;

	void Move();

	void Jump();

	void Death();
	// 2026-06-02: 罠や投げ弾からPlayerへダメージを渡し、HUDで残りライフを見せるため追加。
	void Damage();
	bool IsAlive() const;
	int GetLife() const;
	int GetLayer()const;
	void UpdateState();
	void UpdateAnimation();

	void ChangeModel(const ModelData& data);

	Angle getAngle() { return angle; }

	void SetCameraAngle(Angle& a);
};

class Camela : public Character {
	Player& p;
	Angle camelaAngle = {0.0f, 0.3f };
	float distance;
	int frontLight;
public:
	Camela(Player& p);
	void Init()override;
	void Update()override;
	void Draw() override;

	void MoveAngle();
};
/*
//6/8追加
//敵関連
class Enemy : public Character {
    int m_layer = 0;
    float y = 0;
    bool isActive;
public:
    Enemy();
    void Init()override;
    void Update()override;
    void Draw()override;

    void Move();
    void SetOption(VECTOR p);
};

class EnemyManager {
    vector<Enemy> enemies;
    vector<int> pool;
public:
    void Init();
    void Update();
    void Draw();
};

class EnemyBullet {
public:
    void Init();
    void Update();
    void Draw();
};

class EnemyBulletManager {
    vector<EnemyBullet> bullets;
    vector<int> pool;
public:
    void Init();
    void Update();
    void Draw();
};
*/

//アイテム系
enum class ItemType {
	KEY,

};
class Item {
	VECTOR pos;
public:
	void Init(VECTOR pos, ItemType type);
	void Update();
	void Draw();
};

class ItemManager {
	vector<Item> items;
public:
	void Init();
	void Update();
	void Draw();
};