#pragma once
#include <string>
#include <ctime>

namespace wolf { class App; }

class Sample 
{
public:
    Sample(wolf::App* pApp, const std::string& name) : m_name(name), m_pApp(pApp) {}
    virtual ~Sample() {}

    const std::string& getName() const { return m_name; }

    virtual void init() = 0;
    virtual void update(float dt) = 0;
    virtual void render(int width, int height) = 0;

protected:
    std::string m_name;
    wolf::App* m_pApp;
};