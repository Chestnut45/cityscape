#include "cityscape.hpp"

Cityscape* cityscape = nullptr;

// Application entrypoint
int main(int, char**)
{
    // Create the cityscape app
    cityscape = new Cityscape();

    // Run the app
    cityscape->Run();

    // Delete and exit
    delete cityscape;
    return 0;
}