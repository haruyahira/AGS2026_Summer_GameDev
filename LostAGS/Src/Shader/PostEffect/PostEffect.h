#pragma once
#include <vector>
#include <memory>
#include <initializer_list>
#include <DxLib.h>

#include "PixelMaterial.h"
#include "PixelRenderer.h"

class PostEffect
{
public:

    enum TYPE
    {
        MONOTONE,
        HORROR,
        GAMING,
        WAVE,
        BLOOD,
        SEPIA,

        MAX
    };

    PostEffect();
    ~PostEffect();

    void Init(int width, int height, const std::string& shaderPath);

    void Select(std::initializer_list<TYPE> effects);

    void Update(float time);

    void Draw(int screenHandle);

private:

    void CreateEffect(TYPE type, const std::string& shaderFileName, int constBufFloat4Size);

    void UpdateParam(TYPE type, float time);

private:

    std::vector<std::unique_ptr<PixelMaterial>> materials_;

    std::vector<std::unique_ptr<PixelRenderer>> renderers_;

    std::vector<TYPE> activeEffects_;

    int width_;
    int height_;

    float damagePower_;
};