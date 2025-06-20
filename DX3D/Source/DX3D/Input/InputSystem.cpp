#include <DX3D/Input/InputSystem.h>
#include <cstring>

bool InputSystem::m_currentKeys[256] = {};
bool InputSystem::m_previousKeys[256] = {};

void InputSystem::update()
{
    memcpy(m_previousKeys, m_currentKeys, sizeof(m_currentKeys));
}

void InputSystem::onKeyDown(unsigned int key)
{
    if (key < 256)
    {
        m_currentKeys[key] = true;
    }
}

void InputSystem::onKeyUp(unsigned int key)
{
    if (key < 256)
    {
        m_currentKeys[key] = false;
    }
}

bool InputSystem::isKeyDown(unsigned int key)
{
    if (key < 256)
    {
        return m_currentKeys[key];
    }
    return false;
}

bool InputSystem::isKeyJustPressed(unsigned int key)
{
    if (key < 256)
    {
        return m_currentKeys[key] && !m_previousKeys[key];
    }
    return false;
}