# CS-4350 Cityscape Assignment

### D'Anyil Landry

## Controls:

WASD: Movement\
R: Regenerate cityscape\
I: Toggle Infinite Mode

Escape: Pause + Toggle GUI\
End: End the program

Mouse Movement: Look around\
Mouse Scroll: Zoom

## Project Structure:

### data:

All data files for the project; shaders, textures, etc.

### src:

All of the project source, including the main.cpp entrypoint and all cityscape files.

### phi:

Phi is the micro-engine I've put together for this assignment. I wrote every line, save for the App class, which is basically just an adlib of the W_App class from wolf, but with support for other OpenGL context versions and some basic performanc monitoring. It has a few RAII wrapper classes for OpenGL resources (buffer objects, textures, shaders, etc.), and a small number of more complex resources like a renderable mesh class and a basic camera.

## Extras Chosen:

### Road Generation:

Roads are just painted onto each ground tile texture. Nothing crazy.

### Lighting:

Traditional deferred shading with light volume proxy geometry is used due to the large number of lights in the scene. I considered implementing tiled deferred or clustered deferred shading to remove all of the unnecessary extra gBuffer reads per fragment, but it proved too difficult in the timeframe available.

There are 2 main directional lights (sun + moon), and ~400 point lights (at night, when the streetlights are on).

### Building Shapes:

Each building is generated story by story, face by face. Texture offsets into the main building atlas are procedurally generated for each face based on constructor arguments and rng.

## Other Considerations:

### Infinite Mode:

Pressing the I key will toggle Infinite Mode. In this state, city blocks will be generated and deleted around the camera as you move around.

### Sky:

...

### Shadow Mapping:

...

### Automagical VAOs:

A number of internal vertex formats are included which can be used to automatically construct a VertexAttributes object (My VAO wrapper class). This is only applicable when you tightly pack your vertices into the buffer you supply to the constructor, but that's the majority of VBO use cases anyway, so I think the convenience is warranted :)