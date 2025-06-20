#pragma once

class InputSystem
{
public:
    static void update();

    static void onKeyDown(unsigned int key);
    static void onKeyUp(unsigned int key);

    static bool isKeyDown(unsigned int key);

    static bool isKeyJustPressed(unsigned int key);

private:
    static bool m_currentKeys[256];
    static bool m_previousKeys[256];
};