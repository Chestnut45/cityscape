# Cityscape Assignment

### D'Anyil Landry

## Controls:

WASD: Movement\
R: Regenerate cityscape\
I: Toggle Infinite Mode

Escape: Pause (Uncapture mouse + freeze time)\
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

Deferred rendering is used due to the large number of lights in the scene.

There are 2 main directional lights (sun + moon), and ~400 point lights (1 point light per lamppost, 4 lampposts per block, 100 blocks loaded).

### Building Shapes:

Each building is generated story by story, face by face. Texture offsets are procedurally generated for each face based on constructor arguments and rng.

## Other Considerations:

### Infinite Mode:

...

### Sky:

...

### Shadow Mapping:

...

### Texture Atlasing:

...

### Automagical VAOs:

A number of internal vertex formats are included which can be used to automatically construct a VertexAttributes object (My VAO wrapper class). This is only applicable when you tightly pack your vertices into the buffer you supply to the constructor, but that's the majority of VBO use cases anyway, so I think the convenience is warranted.