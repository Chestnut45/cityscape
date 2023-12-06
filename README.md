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

Escape: Pause + Toggle GUI\
End: End the program

WASD: Movement\
R: Regenerate cityscape\
I: Toggle Infinite Mode

Mouse Movement: Look around\
Mouse Scroll: Zoom

## Project Structure:

### data:

All data files for the project; shaders, textures, models, etc.

### src:

All of the cityscape source files, including the main.cpp entrypoint and all non-engine code.

### phi:

Phi is the micro-engine I've put together for this assignment. I wrote every line, except for the App class, which is basically just an adlib of the W_App class from wolf, but with support for other OpenGL context versions and some basic performance monitoring. It has a few RAII wrapper classes for OpenGL resources (buffer objects, textures, etc.), and a small number of more complex resources like a renderable mesh class and a basic camera. It's by no means complete, but it's a great starting point for my personal projects and I can add more features to it as I encounter the need for them.

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

The buildings are drawn using double buffering to build up a draw buffer section containing all currently loaded buildings' vertices and indices once per frame.

## Other Considerations:

### Infinite Mode:

Pressing the I key will toggle Infinite Mode. In this state, city blocks will be generated and deleted around the camera as you move around.

### Sky:

The sky's skybox colors are blended with both main directional light colors by the amount of "ambient" in the scene, and interpolated over time of day.

### Weather Simulation:

On December 4th, there was a really cool looking snowstorm, and it inspired me to add snow to the cityscape.

- Snow accumulation/melting will only update if `Time Advance` is checked.
- Snow will only accumulate if `Snowstorm` is also checked
- Snow will accumulate faster based on the `Intensity` value for the storm.
- Snow will melt 2x faster during the day.
- The snow particle effect consists of 20,000 particles each rasterized as a GL_POINT with a random size from 1 to 6.
- The particles are updated entirely on the GPU, all the CPU does is calculate initial positions once on program startup, and issue a draw call once per frame.

Immediately after calculating the position offset due to wind and assigning a value to gl_Position, the vertex shader applies a constant velocity downward (since snow has a relatively low terminal velocity), and wraps each particle's position back up to the top of the "effect box" that surrounds the camera. After all vertex shader outputs have finished, we simply write back the updated particle position to the same location of the same buffer by binding the buffer object to an indexed SSBO binding point.

Since each snowflake is rendered to the geometry buffer, they will also automatically have the entire scene's lighting applied to them. This is achieved by the vertex shader generating normals for each snowflake that are based on the same noise value used to generate the wind offsets. You should be able to see the effect of this by standing close to the street lights, where some snowflakes may reflect the light from the street light not closest to them. Rationale for this behaviour is that snowflakes would be rotating due to the wind, so the specular reflections could be from *any* nearby light.

![snow_loop.GIF](https://github.com/Chestnut45/cityscape/blob/main/snow_loop.GIF)

### Persistent Mapped Buffer Streaming

Phi's GPUBuffer class is made to use many buffer streaming techniques, which are useful for streaming data to the GPU with very minimal driver overhead. If you create a buffer of any of the dynamic types, it will be persistently mapped (until the resource is destroyed), using the `GL_MAP_COHERENT_BIT` flag set. This ensures that all writes through the pointer returned by `glMapBufferRange()` are seen by any subsequent OpenGL operations.

The main caveat with using persistently mapped buffers is that you must perform synchronization yourself. It's your responsibility not to write to the buffer while any OpenGL calls are reading from it. To this end, the GPUBuffer class provides the `Lock()`, `Sync()`, and `SwapSections()` methods. `Lock()` inserts a fence sync object and associates it with the current section of the buffer, `Sync()` performs a client-blocking sync call until the current section's sync object has been signaled, and `SwapSections()` moves to the next buffer section, or back to the beginning if the current section is the last.

The main reason for double/triple buffering is to minimize the client sync points. The general method is to write to section A of the buffer, issue commands that read from section A, place a sync on section A, then start writing to section B (while the GPU is still reading from section A). In an ideal world, by the time we send commands to read from the final section and swap back to section A for writing, section A's sync object will already be signaled, so the `Sync()` method will return immediately.

### Automagical VAOs:

A number of internal vertex formats are included which can be used to automatically construct a `VertexAttributes` object (Phi's VAO wrapper class). This is only applicable when you tightly pack your vertices into the buffer you supply to the constructor, but I think the convenience is warranted :)