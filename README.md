# CS-4350 Cityscape Assignment

### D'Anyil Landry

## Prerequisites

- C/C++ Tools VSCode extension
- CMake
- A C++20 compliant compiler
- NOTE: MSYS2 or ninja may be helpful for configuring cmake to use gcc on windows

## Build Instructions

### Linux/Windows

CMake: Configure\
CMake: Build (Target Cityscape)

### Tested Platforms

- GCC 12+ / CMake 3.27.4 / Ubuntu 23.10 / 6.5.0-10-lowlatency / NVIDIA RTX 3070ti on 535.129.03
- GCC 12+ / CMake 3.27.4 / Ubuntu 23.10 / 6.5.0-10-lowlatency / Intel Iris XE on Mesa 23.2.1-1ubuntu3
- GCC 13+ / CMake 3.27.8 / Windows 11 / NVIDIA RTX 3070ti on 532.09
- GCC 13+ / CMake 3.27.8 / Windows 11 / Intel Iris XE on 31.0.101.4255

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

All data files for the project; shaders, textures, models, etc.

### src:

All of the project source, including the main.cpp entrypoint and all cityscape files.

### phi:

Phi is the micro-engine I've put together for this assignment. I wrote every line, except the App class, which is basically just an adlib of the W_App class from wolf, but with support for other OpenGL context versions and some basic performance monitoring. It has a few RAII wrapper classes for OpenGL resources (buffer objects, textures, etc.), and a small number of more complex resources like a renderable mesh class and a basic camera. It's by no means complete, but it's a great starting point for my personal projects and I can add more features to it as I encounter the need for them.

## Extras Chosen:

### Road Generation:

Roads are just painted onto each ground tile texture. Nothing crazy.

### Lighting:

Traditional deferred rendering is used due to the large number of lights in the scene.

Light volume proxy geometry is used to only generate fragments for pixels that will actually be affected by each light.

I considered implementing tiled deferred or clustered deferred shading to remove all of the unnecessary extra gBuffer reads per fragment, but it proved too difficult in the timeframe available.

There are 2 main directional lights (sun + moon), and ~400 point lights at night, when the streetlights are on.

Both directional lights and point lights are using the Blinn-Phong model (with the adjusted half way vector).

### Building Shapes:

Each building is generated story by story, face by face. Texture offsets into the building texture atlas are procedurally generated for each face based on constructor arguments and rng. (If you are creating a "Door" face, it generates texture coordinates that correspond to the "Door" section of the texture, for the given building variant)

## Other Considerations:

### Infinite Mode:

Pressing the I key will toggle Infinite Mode. In this state, city blocks will be generated and deleted around the camera as you move around.

### Sky:

The sky's skybox colors will be blended with both main directional light colors by the amount of "ambient" in the scene, and interpolated over time of day.

### Persistent Mapped Buffer Streaming

Phi's GPUBuffer class allows us to use many buffer streaming techniques, which are useful for streaming data to the GPU with very minimal driver overhead. If you create a buffer of any of the dynamic types, it will be persistently mapped (until the resource is destroyed), using the `GL_MAP_COHERENT_BIT` flag set. This ensures that all writes through the pointer returned by `glMapBufferRange()` are seen by any subsequent OpenGL operations.

The main caveat with using persistently mapped buffers is that you must perform synchronization yourself. It's your responsibility not to write to the buffer while any OpenGL calls are reading from it. To this end, the GPUBuffer class provides the `Lock()`, `Sync()`, and `SwapSections()` methods. `Lock()` inserts a fence sync object and associates it with the current section of the buffer, `Sync()` performs a client-blocking sync call until the current section's sync object has been signaled, and `SwapSections()` moves to the next buffer section, or back to the beginning if the current section is the last.

The main reason for double/triple buffering is to minimize the client sync points. The general method is to write to section A of the buffer, issue commands that read from section A, place a sync on section A, then start writing to section B (while the GPU is still reading from section A). In an ideal world, by the time we send commands to read from the final section and swap back to section A for writing, section A's sync object will already be signaled, so the `Sync()` method will return immediately.

The buildings are drawn using double buffering to build up a draw buffer section containing all currently loaded buildings' vertices and indices once per frame.

### Shadow Mapping:

...

### Automagical VAOs:

A number of internal vertex formats are included which can be used to automatically construct a `VertexAttributes` object (My VAO wrapper class). This is only applicable when you tightly pack your vertices into the buffer you supply to the constructor, but that's the majority of VBO use cases anyway, so I think the convenience is warranted :)