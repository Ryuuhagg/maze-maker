//constant.h
#pragma once
#include"DxLib.h"

const int WIDTH = 1280;
const int HEIGHT = 720;

const int WHITE = GetColor(255, 255, 255);
const int BLACK = GetColor(0, 0, 0);

template <typename T>
T Clamp(T v, T min, T max) {
    if (v < min) return min;
    if (v > max) return max;
    return v;
}