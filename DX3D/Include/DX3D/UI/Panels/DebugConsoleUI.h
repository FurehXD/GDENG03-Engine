#pragma once

namespace dx3d
{
    class Logger;

    class DebugConsoleUI
    {
    public:
        explicit DebugConsoleUI(Logger& logger);

        void render();

    private:
        Logger& m_logger;
    };
}