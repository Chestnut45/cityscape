# Cityscape Assignment

### D'Anyil Landry

## Controls:

WASD: Movement\
R: Regenerate cityscape\
I: Toggle Infinite Mode

Escape: Pause\
End: End the program

Mouse Movement: Look around\
Mouse Scroll: Zoom

## Project Structure:

### Components:

Components are anything that can be simulated. Typically they will manage some resource over the lifetime of the program, and provide an interface to interact with its data.

### Resources:

Resources are OpenGL RAII Wrapper classes. I chose to write these to challenge myself to not use wolf and to better understand the OpenGL state machine.

## Extras Chosen:

### Road Generation:

Roads are essentially just painted onto each ground tile. Nothing else to it.

### Lighting:

Deferred lighting, point lights, sky class...

### Building Shapes:

Procedural generation + batching...

## Other Considerations:

### Sky:

...

### Shadow Mapping:

...

### Texture Atlasing:

...

### Automagical VAOs:

...