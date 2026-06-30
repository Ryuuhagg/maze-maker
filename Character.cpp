//Character.cpp
#include"Character.h"
#include"Input.h"
#include"Constant.h"
#include<math.h>
#include <algorithm>
#include"MapLoader.h"
#include"GameObjects.h"
#include"FontManager.h"
Player::Player() :Character(10)
{
    Init();
}


bool TryMoveLayerByStairs(
    int& layer,
    float worldX,
    float worldZ);


Player::~Player() {
    MV1DeleteModel(m_model);
}

void Player::Init() {
    ChangeModel(cat);

    MV1SetScale(m_model, VGet(0.5f, 0.5f, 0.5f));

    pos = GetStartPosition();
    y = 0;
    vy = 0;
    m_isGround = true;
    m_gravity = -0.6f;
    
    m_life = 3;

    m_Size = 10;
    angle = { 0.0f, 1.5f };

    m_layer = GetMapLayerFromWorldY(pos.y);
}

void Player::Update() {
    m_isMoving = false;
    if (m_life <= 0) {
        
    }
    else {

        Move();

        Jump();

        UpdateState();
        UpdateAnimation();

        MV1SetPosition(m_model, pos);
        MV1SetRotationXYZ(m_model, VGet(0, angle.x + DX_PI_F, 0));
        // m_layer はプレイヤーが実際にいる階層、gameLoadedLayer は描画・判定のために先読みする階層。
        // 階段の途中では上階を先にロードしたいので、高さではなく階段上の位置からロード階層を決める。
        bool onLayerStairs =
            IsStairsAtWorldLayer(m_layer, pos.x, pos.z) ||
            IsStairsAtWorldLayer(m_layer - 1, pos.x, pos.z);
        if (m_isGround && onLayerStairs)
        {
            SetGameLoadedLayer(GetMapLayerFromWorldY(pos.y));
        }
        else
        {
            SetGameLoadedLayer(m_layer);
        }
    }

}

void Player::Draw() {
    MV1DrawModel(m_model);
}

void Player::Move() {
    float x = Input::GetAxisLX();
    float y = Input::GetAxisLY();

    float moveX = 0;
    float moveZ = 0;

    moveX += sinf(C_angle.x) * y;
    moveZ += cosf(C_angle.x) * y;

    moveX += cosf(C_angle.x) * x;
    moveZ -= sinf(C_angle.x) * x;

    float length = sqrtf(moveX * moveX + moveZ * moveZ);
    if (length > 1.0f) {
        moveX /= length;
        moveZ /= length;
    }

    if (length > 0.1f) {
        angle.x = atan2f(moveX, moveZ);
    }

    float currentSpeed = speed;

    m_isMoving = length > 0.1f;

    if (Input::IsActionPressed(Action::Dash) && length > 0.1f) {
        currentSpeed *= 2.0f;
        m_isDashing = true;
    }
    else {
        m_isDashing = false;
    }

    const float playerRadius = 60.0f;
    VECTOR nextPos = pos;
    nextPos.x += moveX * currentSpeed;
    nextPos.z += moveZ * currentSpeed;
    pos = ResolvePlayerMapCollision(pos, nextPos, playerRadius, m_layer);
    // 階層変更は高さだけでは決めない。
    // 階段の上端/下端まで進んだ時だけ、TryMoveLayerByStairs が m_layer を切り替える。

}

void Player::Jump() {
    if (Input::IsActionTrigger(Action::Jump) && m_isGround) {
        vy = 20.0f;
        m_isGround = false;
    }

    vy += m_gravity;
    y += vy;

    if (y <= 0) {
        y = 0;
        vy = 0;
        m_isGround = true;
    }

    float groundY = GetMapGroundY(pos.x, pos.z, m_layer * BLOCK_SIZE);
    pos.y += (groundY + y - pos.y) * 0.2f;

    bool onLayerStairs =
        IsStairsAtWorldLayer(m_layer, pos.x, pos.z) ||
        IsStairsAtWorldLayer(m_layer - 1, pos.x, pos.z);
    if (m_isGround && onLayerStairs)
    {
        m_layer = GetMapLayerFromWorldY(pos.y + BLOCK_SIZE * 0.5f);
    }
}


void Player::UpdateState() {
    float x = Input::GetAxisLX();
    float y = Input::GetAxisLY();

    if (!m_isGround) {
        m_state = PlayerState::Jump;
    }
    else if (m_isDashing) {
        m_state = PlayerState::Dash;
    }
    else if (m_isMoving) {
        m_state = PlayerState::Walk;
    }
    else {
        m_state = PlayerState::Idle;
    }
}

void Player::Death() {
    // 2026-06-02: ギミックのダメージでライフが0未満へ減り続けないよう追加。
    if (m_life <= 0)
        return;

    m_life--;
    // 2026-06-15: ライフが残っている時は、STARTではなく有効チェックポイントへ復帰する。
    pos = GetGameRespawnPosition();
    SetGameLoadedLayer(m_layer);
    y = 0;
    vy = 0;
    m_isGround = true;
    m_gravity = -0.1f;
    m_layer = GetMapLayerFromWorldY(pos.y + BLOCK_SIZE * 0.5f);
}

void Player::Damage() {
    // 2026-06-02: GameObjectsから受けたダメージを既存の死亡/リスポーン処理へ集約するため追加。
    Death();
}

bool Player::IsAlive() const {
    // 2026-06-02: ライフが尽きた時にScene側でリザルトへ遷移するため追加。
    return m_life > 0;
}

int Player::GetLife() const {
    // 2026-06-02: ゲーム画面のHUDに残りライフを表示するため追加。
    return m_life;
}

int Player::GetLayer() const {
    return m_layer;
}

void Player::UpdateAnimation() {
    int nextAnim = 0;

    switch (m_state) {
    case PlayerState::Jump: nextAnim = 0; break;
    case PlayerState::Idle: nextAnim = 1; break;
    case PlayerState::Walk: nextAnim = 2; break;
    case PlayerState::Dash: nextAnim = 3; break;
    }

    if (nextAnim != m_currentAnimNo) {

        MV1DetachAnim(m_model, m_animeIndex);

        m_animeIndex = MV1AttachAnim(m_model, nextAnim);
        MV1SetAttachAnimBlendRate(m_model, m_animeIndex, 1.0f);

        m_currentAnimNo = nextAnim;
    }

    float now = MV1GetAttachAnimTime(m_model, m_animeIndex);
    float total = MV1GetAttachAnimTotalTime(m_model, m_animeIndex);

    now += 0.5f;
    now = fmod(now, total);

    MV1SetAttachAnimTime(m_model, m_animeIndex, now);
}

void Player::ChangeModel(const ModelData& data) {

    if (m_model != 0) {
        MV1DeleteModel(m_model);
    }

    m_model = MV1LoadModel(data.path);

    m_animIdle = data.idle;
    m_animWalk = data.walk;
    m_animJump = data.jump;

    m_animeIndex = MV1AttachAnim(m_model, m_animIdle);
    m_currentAnimNo = m_animIdle;
}

void Player::SetCameraAngle(Angle& a) { C_angle = a; }

Camela::Camela(Player& p) :Character(0), p(p), distance(360),frontLight(0)
{
    Init();
}

void Camela::Init() {
    camelaAngle = { 0.0f, 0.3f };

    frontLight = CreateSpotLightHandle(
        VGet(0, 0, 0),        // ライト位置
        VGet(0, -1, 0),       // 向き
        DX_PI_F / 4.0f,       // 外側の角度
        DX_PI_F / 8.0f,       // 内側の角度
        900.0f,               // 距離
        0.8f,                 // 強さ
        0.002f,
        0.0f
    );
}

void Camela::Update() {
    p.SetCameraAngle(camelaAngle);
    VECTOR target = p.getVECTOR();
    target.y += 80.0f; // 少し上を見るなら足元ではなく体の中心あたり

    VECTOR idealCamPos = VGet(
        target.x - sinf(camelaAngle.x) * distance,
        target.y + sinf(camelaAngle.y) * distance,
        target.z - cosf(camelaAngle.x) * distance
    );

    const float cameraRadius = 20.0f;
    int layer = p.GetLayer();

    VECTOR camPos = ResolveCameraCollision(
        target,
        idealCamPos,
        cameraRadius,
        layer
    );

    VECTOR dir = VSub(target, camPos);
    dir = VNorm(dir);

    SetLightPositionHandle(frontLight, camPos);
    SetLightDirectionHandle(frontLight, dir);

    VECTOR lightPos = VGet(
        target.x,
        target.y + 500.0f,
        target.z
    );

    SetLightPosition(lightPos);

    
    SetLightDirection(VGet(0,-1.0f,0));
    

    SetCameraPositionAndTarget_UpVecY(camPos, target);

    MoveAngle();
}

void Camela::Draw() {

}

void Camela::MoveAngle() {
    float padRX = Input::GetAxisRX();
    float padRY = Input::GetAxisRY();

    float mouseDX = (float)Input::GetMouseDeltaX();
    float mouseDY = (float)Input::GetMouseDeltaY();

    if (Input::IsCamelaRightLeftFlip()) {
        padRX *= -1.0f;
        mouseDX *= -1.0f;
    }

    if (Input::IsCamelaUpDownFlip()) {
        padRY *= -1.0f;
        mouseDY *= -1.0f;
    }

    camelaAngle.x += padRX * Input::GetPadSensitivity();
    camelaAngle.y += padRY * Input::GetPadSensitivity();

    camelaAngle.x += mouseDX * Input::GetMouseSensitivity();
    camelaAngle.y += mouseDY * Input::GetMouseSensitivity();

    camelaAngle.y = Clamp(camelaAngle.y, 0.0f, 1.5f);
}