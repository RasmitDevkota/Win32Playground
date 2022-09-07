#include <iostream>
#include <vector>

class Tab
{
public:
    bool save = false;

    Tab(bool _save)
    {
        std::cout << "opened new tab";
    }

    void pin()
    {
        save ^= true;
    }
};

std::vector<Tab> tabs;

void closeTabs()
{
    for (auto& tab : tabs)
    {
        if (tab.save)
        {
            delete &tab;
        }
    }
}

int main()
{
    for (int i = 0; i < 4096; i++)
    {
        Tab newTab = new Tab((i < 2048));
    }

    Tab mainTab = new Tab(false);

    mainTab.pin();

    closeTabs();
}