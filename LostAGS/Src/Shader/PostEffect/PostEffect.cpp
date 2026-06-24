#include "PostEffect.h"

PostEffect::PostEffect()
{
    width_ = 0;
    height_ = 0;

    damagePower_ = 0.0f;
}

PostEffect::~PostEffect()
{
}

void PostEffect::Init(int width, int height, const std::string& shaderPath)
{
    width_ = width;
    height_ = height;

    materials_.resize(TYPE::MAX);
    renderers_.resize(TYPE::MAX);

    CreateEffect(TYPE::MONOTONE, shaderPath + "Monotone.cso", 1);
    CreateEffect(TYPE::HORROR, shaderPath + "Horror.cso", 1);
    CreateEffect(TYPE::GAMING, shaderPath + "Gaming.cso", 1);
    CreateEffect(TYPE::WAVE, shaderPath + "Wave.cso", 1);
    CreateEffect(TYPE::BLOOD, shaderPath + "Blood.cso", 1);
    CreateEffect(TYPE::SEPIA, shaderPath + "Sepia.cso", 1);

    Select({ TYPE::HORROR });
}

void PostEffect::CreateEffect(TYPE type, const std::string& shaderFileName, int constBufFloat4Size)
{
    materials_[type] = std::make_unique<PixelMaterial>();

    materials_[type]->Load(shaderFileName, constBufFloat4Size);

    renderers_[type] =
        std::make_unique<PixelRenderer>(
            *materials_[type],
            width_,
            height_);

    renderers_[type]->MakeSquareVertex(
        Vector2(0, 0),
        Vector2(width_, height_));
}

void PostEffect::Select(std::initializer_list<TYPE> effects)
{
    activeEffects_.clear();

    for (TYPE effect : effects)
    {
        if (effect < 0 || effect >= TYPE::MAX)
        {
            continue;
        }

        activeEffects_.push_back(effect);
    }
}

void PostEffect::Update(float time)
{
    for (TYPE effect : activeEffects_)
    {
        UpdateParam(effect, time);

        materials_[effect]->Update();
    }
}

void PostEffect::UpdateParam(TYPE type, float time)
{
    switch (type)
    {
    case TYPE::MONOTONE:
        materials_[type]->SetParam(
            0,
            FLOAT4{ 0.0f, 1.0f, 1.0f, 1.0f });
        break;

    case TYPE::HORROR:
        materials_[type]->SetParam(
            0,
            FLOAT4{ time, 1.0f, 1.0f, 1.0f });
        break;
    case TYPE::GAMING:
        materials_[type]->SetParam(
            0,
            FLOAT4{ time, 1.0f, 1.0f, 1.0f });
        break;
    case TYPE::WAVE:
        materials_[type]->SetParam(
            0,
            FLOAT4{ time, 0.02f, 20.0f, 2.0f });
        break;
    case TYPE::BLOOD:
        materials_[type]->SetParam(
            0,
            FLOAT4{ time,damagePower_,0.0f,0.0f });

        break;
    case TYPE::SEPIA:
        // g_color.x をセピア強度として使用（0.0～1.0）
        materials_[type]->SetParam(
            0,
            FLOAT4{ 1.0f, 0.0f, 0.0f, 0.0f });
        break;
    }
}

void PostEffect::Draw(int screenHandle)
{
    for (TYPE effect : activeEffects_)
    {
        renderers_[effect]->Draw(screenHandle);
    }
}