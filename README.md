# Table of Contents
#-introduction
#-my-plans-for-this-project:
#-which-branch?
#-how-to-view-this-project

---

# Introduction
This is a small project I'm working on to hone my OpenGL and C++ skills. It is a work-in-progress Minecraft-inspired voxel engine with procedural terrain generation. Currently, I only plan for the scene to be for viewing, but that may change as both the project and I grow.

---

# My Plans for this Project:
1. Abstract Block/Chunk creation    __DONE__
2. Optimize number of draw calls (frustum culling, occlusion culling, greedy meshing (maybe))
3. Manage chunk generation    __IN PROGRESS__
4. Add biome generation
5. Implement Perlin Worms for cave generation
6. Beautification (lighting, water shader, skybox, etc.)

---

# Which Branch?
The main branch is going to be for major developments in my project. It will likely be pushed to very infrequently, though.
The nightly branch is being used more as a backup for when I work on the project than anything else, but it contains the most up-to-date code. There is no guarantee that the most recent commit's code will actually run, though, so I'll be adding "(BUGGY)" to any commits where the code either does not run or is not working anywhere close to intended. Commits that are "working" will be committed as normal, so you can view the "working" code at those commits.
__PLEASE KEEP IN MIND:__ "Working" commits, and especially "Buggy" commits, in the nightly branch may not be optimized and are usually rough drafts of a finished implementation.

---

# How to view this project
Firstly, clone this repository.
If you want to view the code, the solution file for Visual Studio is in the base directory of this repo, or you can simply view it from GitHub by navigating to it there. __If you want to adjust the render distance, in `ChunkManager.h`, there is a variable `m_ChunkRenderRadius` that you can modify.__
If you just want to see the project in action, go to the project directory (VoxelTechDemo/VoxelTechDemo/) and run the exectuable.

__NOTE:__ I have not run my application on any OS other than Windows, so I don't know if it will port to Mac OS or Linux well, but it should, given the portability of GLFW. Just ensure your GPU supports OpenGL 3.3.