#include <stdio.h>
#include <iostream>
#include <glm/glm.hpp>

#include "../wolf/wolf.h"
#include "../samplefw/SampleRunner.h"

#include "cityscape.hpp"

// Application entrypoint
int main(int, char**)
{
    // Create and run the cityscape app
    Cityscape cityscape;
    cityscape.run();

    // Exit
    return 0;
}