# Cityscape Assignment

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

### Components:

Components are anything that can be simulated. Typically they manage some resource and provide an interface to interact with its data.

### Resources:

Resources are OpenGL RAII Wrapper classes. I chose to write these to challenge myself to not use wolf and further my understanding of the OpenGL state machine.

## Extras Chosen:

### Road Generation:

Roads are just painted onto each ground tile texture. Nothing crazy.

### Lighting:

Traditional deferred shading with light volume proxy geometry is used due to the large number of lights in the scene. I considered implementing tiled deferred or clustered deferred shading to remove all of the unnecessary extra gBuffer reads per fragment, but this proved difficult without the use of compute shaders and adjustable workgroup sizes. Maybe a good candidate for something to try in next semester's class.

There are 2 main directional lights (sun + moon), and ~400 point lights (1 point light per lamppost, 4 lampposts per block, 100 blocks loaded).

### Building Shapes:

Each building is generated story by story, face by face. Texture offsets are procedurally generated for each face based on constructor arguments and rng.

## Other Considerations:

### Infinite Mode:

Pressing the I key will toggle Infinite Mode. In this state, city blocks will be generated and deleted around the camera as you move around.

### Sky:

...

### Shadow Mapping:

...

### Texture Atlasing:

...

### Automagical VAOs:

A number of internal vertex formats are included which can be used to automatically construct a VertexAttributes object (My VAO wrapper class). This is only applicable when you tightly pack your vertices into the buffer you supply to the constructor, but that's the majority of VBO use cases anyway, so I think the convenience is warranted :)