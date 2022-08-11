# VoxelTechDemo
This is a small project I'm working on to hone my OpenGL and C++ skills. It is a work-in-progress Minecraft-inspired voxel engine with procedural terrain generation. Currently, I only plan for the scene to be for viewing, but that may change as both the project and I grow.

---

## This is the nightly branch!
This is my backup branch for when I work on this project and want to save what I've done that day.
While this is the best place to see the most up-to-date code, there is no guarantee that the code will work. Because of this, I'll be adding "(BUGGY)" to any commits where the code either does not run or does not work as intended. Commits that are "working" will be committed as normal, so you can view the "working" code at those commits.
__PLEASE NOTE:__ These commits may not be optimized and are usually rough drafts of a finished implementation.

---

### My plans for this project are to:
1. Abstract Block/Chunk creation    __DONE__
2. Optimize number of draw calls (frustum culling, occlusion culling, greedy meshing (maybe))
3. Manage chunk generation
4. Add biome generation
5. Implement Perlin Worms for cave generation
6. Beautification (lighting, water shader, skybox, etc.)