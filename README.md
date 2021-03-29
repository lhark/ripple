# Ripple #

An OpenGL port of the SIGGRAPH 2017 [Water Wave Packets][1] paper's [implementation][2].

# Context and quirks #

This port was written as the final for an Advanced Infographics class. Most of
the work was focused on translating the HLSL shaders to GLSL, adapting existing
OpenGL code to replace the DirectX 11 setup code.

Given the limited timeframe, some bugs and performance issues still linger. On
the performance side of things, a bit of profiling seems to indicate the
program is GPU-bound, the biggest culprit being the fragment shader.

[1]: http://visualcomputing.ist.ac.at/publications/2017/WWP/
[2]: https://github.com/jeschke/water-wave-packets
