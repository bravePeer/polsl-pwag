#pragma once
#include <chrono>
#include "component/component.hpp"

/// @brief Testing purpose
class ComponentSpamer : public Component
{
public:
    ComponentSpamer()
    : spamPeriod(1.f)
    { }

    ComponentSpamer(float spamPeriod, const std::string& text)
    : spamPeriod(spamPeriod), textToPrint(text)
    { }

protected:
    void update([[maybe_unused]] float deltaTime) override
    {
        // To też będzie dobrze ale chyba lepiej by było po prostu dodawać deltaTime
        auto timeNow = std::chrono::steady_clock::now();
        auto tmp = std::chrono::duration<float>(timeNow - lastSpamWrite).count();
        if(tmp > spamPeriod)
        {
            std::cout << textToPrint << std::endl;
            lastSpamWrite = timeNow;
        }
    }

private:
    float spamPeriod;
    std::string textToPrint;
    std::chrono::time_point<std::chrono::steady_clock> lastSpamWrite;
};