#pragma once

class Shader
{
public:

    int vsHandle_ = -1;

    int psHandle_ = -1;

public:

    void Load(
        const char* vsPath,
        const char* psPath
    );

    void Begin();

    void End();
};