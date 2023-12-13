# CS-4350 Cityscape Assignment

### D'Anyil Landry

![new_buildings.png](https://github.com/Chestnut45/cityscape/blob/main/new_buildings.png)

## Prerequisites

- CMake
- A C++20 compliant compiler
- C/C++ Tools VSCode extension

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
Left Shift: Speed Boost\
R: Regenerate cityscape\

Mouse Movement: Look around\
Mouse Scroll: Zoom (FOV adjust)

## Project Structure:

### data:

All data files for the project; shaders, textures, models, etc.

### src:

All of the cityscape source files, including the main.cpp entrypoint and all non-engine code.

### phi:

Phi is the micro-engine I put together for this assignment. I wrote it from scratch, except for the App class, which is basically just an adlibbed copy of the W_App class from wolf, but with support for other OpenGL context versions, Dear ImGUI, and some basic performance monitoring. It has a few RAII wrapper classes for OpenGL resources (buffer objects, textures, etc.), and a small number of more complex resources like a mesh/model class and a render batch class. It's by no means complete, but it's a great starting point for my personal projects and I can add more features to it as I encounter the need for them :)

## Extras Chosen:

### Road Generation:

Roads are just manually painted onto the ground tile and instanced around the scene. Nothing crazy.

### Lighting:

Traditional deferred rendering is used due to the large number of lights in the scene. I considered implementing tiled deferred or clustered deferred shading to remove all of the extra gBuffer reads per fragment, but it seemed not worth the cost in time to develop.

There are 2 main directional lights (sun + moon), and ~400 point lights (at the default render distance), when the streetlights are on.

Both directional lights and point lights are using the Blinn-Phong model (with the adjusted half way vector).

Global lighting is run on every fragment generated by a single fullscreen triangle during the global pass, and for point lights, light volume proxy geometry is used to only generate fragments for pixels that will actually be affected by each light.

### Building Generation:

![wireframe_normals.png](https://github.com/Chestnut45/cityscape/blob/main/wireframe_normals.png)

Each building is generated story by story, face by face. Texture offsets into the building texture atlas are procedurally generated for each face based on variant and wall type arguments, and extra features like awnings are placed per-face based on what type (window, door, etc.) of face is being generated.

![buildingAtlasFullAlpha.png](https://github.com/Chestnut45/cityscape/blob/main/data/textures/buildingAtlasFullAlpha.png)

A specular map is baked into the alpha channel of the buildingAtlas.png file (and the cityBlockGround.png file), so only surfaces that should be shiny are.

The buildings are drawn using the Phi::RenderBatch class to group together meshes of matching vertex formats into a single draw call.

## Other Considerations:

### Infinite Generation:

City blocks will be loaded / unloaded around the camera as you move through the city. The default render distance of 5 ensures that at least 400 buildings are loaded, since a single block can have 4-12 buildings, and render distance 5 means a 10x10 grid of blocks will be generated.

### Sky:

The sky's skybox colors are blended with both main directional light colors by the amount of "ambient" in the scene, and interpolated over time of day.

### Weather Simulation:

On December 4th, there was a really cool looking snowstorm, and it inspired me to add snow to the cityscape.

- Snow will only accumulate if `Snow` is checked
- Snow will accumulate faster based on the `Intensity` value for the storm.
- Snow will melt 2x faster during the day.
- Snow accumulation/melting will only update if `Time Advance` is checked.
- The snow particle effect consists of 20,000 particles each rasterized as a GL_POINT with a random size.
- The particles are updated entirely on the GPU, all the CPU does is calculate initial positions once on program startup, and issue a single draw call once per frame.

![snowy_night.png](https://github.com/Chestnut45/cityscape/blob/main/snowy_night.png)

### Shadow Mapping

A single shadow map is generated from the currently active global light's position, facing the camera, every frame. Only the buildings and streetlights are rendered during this pass, and they're rendered with an empty fragment shader so only the shadow map's depth buffer is updated.

![shadows_1.png](https://github.com/Chestnut45/cityscape/blob/main/shadows_1.png)

## Super Cool Technical Details

### Persistent Mapped Buffer Streaming

Phi's GPUBuffer class uses many buffer streaming techniques, which are useful for streaming data to the GPU with minimal driver overhead. If you create a buffer of any of the dynamic types, it will be persistently mapped (until the resource is destroyed), with the `GL_MAP_COHERENT_BIT` flag set. This ensures that all writes through the pointer returned by `glMapBufferRange()` are seen by any subsequent OpenGL operations.

The main caveat with using persistently mapped buffers is that you have to perform synchronization yourself. It's your responsibility not to write to any section of the buffer that is currently being read from by OpenGL. To this end, the GPUBuffer class provides the `Lock()`, `Sync()`, and `SwapSections()` methods. `Lock()` inserts a fence sync object into the OpenGL command stream and associates it with the current section of the buffer, `Sync()` performs a client-blocking sync call until the current section's sync object has been signaled, and `SwapSections()` moves to the next buffer section, or back to the beginning if the current section is the last.

The main reason for the double/triple dynamic buffer types in Phi is to minimize the client sync points. The general method is to write to section A of the buffer, issue commands that read from section A, place a sync on section A, then start writing to section B (while the GPU is still reading from section A). In an ideal world, by the time we issue commands to read from the final section and swap back to section A for writing, section A's sync object will already be signaled, so the `Sync()` method will return immediately.

### Snow Effect Shader

Immediately after calculating the position offset due to wind and assigning a value to gl_Position and gl_PointSize, the vertex shader applies a constant downward velocity to each snowflake particle. Once a particle falls out of the effect box that surrounds the camera, its y position is wrapped back up to the top of the box so it can be reused. To update the particles' positions, after all vertex shader outputs have finished, we simply write back the updated particle position to the same location of the same buffer by binding the buffer object containing the vertex data to an indexed SSBO binding point that is accessible to the vertex shader. Writing to a buffer this way requires careful ordering and write placement, however. If any of those writes would overlap with another write or read that frame, it would be considered undefined behaviour.

Since each snowflake is rendered to the geometry buffer, they will also automatically have the entire scene's lighting applied to them (including shadows!). This is achieved by the vertex shader generating normals for each snowflake that are based on the same noise values used to generate the wind offsets. You should be able to see the effect of this by standing close to the street lights, where some snowflakes may reflect the light from the street light not closest to them. Rationale for this behaviour is that snowflakes would be rotating due to the wind, so the light reflections could be from *any* nearby light.

(Relevant resources: data/shaders/snow.[v|f]s)

### Automagical VAOs:

There are a few internal vertex formats included in phi/vertex.hpp that can be used with the `Mesh<>` and `RenderBatch<>` template classes. They can also be used to automatically construct a `VertexAttributes` object (Phi's VAO wrapper class). This is only applicable when you tightly pack your vertices / indices into the buffer(s) you supply to the constructor, but that happens often enough I think the convenience is warranted :)

## Challenges

There are definitely some things in this assignment that could/should have been done differently. Below is a small outline of some design / implementation challenges I faced along the way.

- EnTT is not being used in a way that lets it shine. I originally thought it would be a good fit for separating rendering logic and the different components that make up the city, but the components ended up being almost entirely self contained; not many (if any) entities actually use more than one component. In its current state, the entity registry is mostly being used as a big bucket for anything being simulated, with contiguous views into individual components used for quick and easy sequential access during each render pass.
- The shadow maps exhibit some significant peter-panning, despite front-face culling during the shadow map generation pass. This is most noticeable when a building is casting a shadow on its own awning (just before and just after "noon" in the simulation, when the sun is the highest in the sky). I think it may be fixable by tuning the min/max shadow bias values, but I only started implementing shadow mapping on December 11th, and my other exams are fast approaching.
- The actual cityscape source files are generally worse in terms of organization than Phi, because I spent more time experimenting with creative ideas in src/, and wanted more reusable single-purpose code for the engine classes. In particular, cityscape.cpp is monolithic, though I tried to keep it organized enough to be able to navigate.