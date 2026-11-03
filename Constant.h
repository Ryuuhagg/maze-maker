//constant.h
#pragma once
#include"DxLib.h"

const int WIDTH = 1280;
const int HEIGHT = 720;

const int WHITE = GetColor(255, 255, 255);
const int BLACK = GetColor(0, 0, 0);

// ゲームプレイ中の挙動・判定に使う調整値。
namespace GameTuning {
    constexpr float Gravity = -0.6f;
    constexpr float EnemySearchRange = 900.0f;
    constexpr float PlayerCollisionRadius = 60.0f;
    constexpr float EnemyCollisionRadius = 10.0f;
    constexpr float MovementInputThreshold = 0.1f;
    constexpr float DashSpeedMultiplier = 2.0f;
    constexpr int EnemyPathRecalculationFrames = 30;
    constexpr float EnemyStepHeightTolerance = 5.0f;
    constexpr float EnemyGroundSnapRate = 0.2f;
    constexpr float EntityWorldPositionHeightOffset = 80.0f;
    constexpr float EventModelDrawHeightOffset = 8.0f;

    constexpr float DamageInvulnerabilityFrames = 90.0f;
    constexpr float GameObjectInteractionRadius = 115.0f;
    constexpr float CheckpointInteractionRadius = 130.0f;
    constexpr float LockedDoorHitRadius = 40.0f;
    constexpr float SegmentLengthEpsilonSquared = 0.0001f;
    constexpr float SegmentLengthEpsilon = 0.0001f;
    constexpr float DoorEndMarginRadiusFactor = 0.85f;
    constexpr float DoorFrontThicknessFactor = 0.19f;
    constexpr float DoorBackThicknessFactor = 0.03f;
    constexpr float DoorBackRadiusFactor = 0.75f;

    constexpr float EnemyProjectileSpawnHeight = 60.0f;
    constexpr float EnemyProjectileDirectionEpsilon = 0.001f;
    constexpr float EnemyProjectileSpeed = 14.0f;
    constexpr float EnemyInitialCooldownFrames = 30.0f;
    constexpr float EnemyContactRangeInBlocks = 4.0f;
    constexpr float EnemyContactCooldownFrames = 95.0f;
    constexpr float EnemyProjectileHitRadius = 90.0f;
    constexpr float EnemyProjectileWallCheckRadius = 30.0f;
    constexpr float EnemyAvoidanceJumpHeight = 80.0f;
    constexpr float EnemyVisualRadius = 68.0f;
    constexpr float EnemyProjectileVisualRadius = 32.0f;
    constexpr int EnemyVisualSegments = 16;
    constexpr int EnemyProjectileVisualSegments = 12;
}

// マップ形状・段差判定に使う比率。
namespace MapTuning {
    constexpr float HalfStepLowerHeightRatio = 0.25f;
    constexpr float HalfStepUpperHeightRatio = 0.75f;
    constexpr float LowPlatformTopOffsetRatio = 0.5f;
    constexpr float HighPlatformPostOffsetRatio = 0.12f;
    constexpr float HighPlatformPostRadiusRatio = 0.08f;
    constexpr float CircularCollisionRadiusRatio = 0.3f;
    constexpr float BoxCollisionWidthRatio = 0.35f;
    constexpr float BoxCollisionDepthRatio = 0.35f;
}

// プレイヤー追従カメラの調整値。
namespace CameraTuning {
    constexpr float DefaultPitch = 0.3f;
    constexpr float FollowDistance = 360.0f;
    constexpr float TargetHeightOffset = 80.0f;
    constexpr float CollisionRadius = 20.0f;
    constexpr float LightHeightOffset = 500.0f;
    constexpr float MinimumPitch = 0.0f;
    constexpr float MaximumPitch = 1.5f;
    constexpr float SpotOuterAngle = DX_PI_F / 4.0f;
    constexpr float SpotInnerAngle = DX_PI_F / 8.0f;
    constexpr float SpotRange = 900.0f;
    constexpr float SpotIntensity = 0.8f;
    constexpr float SpotAttenuation = 0.002f;
}

template <typename T>
T Clamp(T v, T min, T max) {
    if (v < min) return min;
    if (v > max) return max;
    return v;
}
// 7/17 BFSようの座標
struct MapNode
{
    int layer;
    int x;
    int z;

    bool operator==(const MapNode& other) const
    {
        return
            layer == other.layer &&
            x == other.x &&
            z == other.z;
    }
};
