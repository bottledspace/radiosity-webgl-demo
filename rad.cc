#if __EMSCRIPTEN__
# include <emscripten.h>
# include <emscripten/html5.h>
#include <GLES3/gl3.h>
#else
#include <GL/glew.h>
#define NO_SDL_GLEXT
#define SDL_MAIN_HANDLED
#endif
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <array>
#include <type_traits>
#include <algorithm>
#include "Mesh.hpp"
#include "Texture.hpp"
#include "Framebuffer.hpp"
#include "Shader.hpp"

#ifndef M_PI
const float M_PI = 3.14159f;
#endif


const auto proj = glm::perspective(float(M_PI)/2.0f, 1.0f, 0.1f, 500.0f);
const std::array<glm::mat4,5> views {
    proj,                                                         // forward
    proj*glm::rotate(glm::mat4{1}, float(M_PI)/2.0f, {+1.0f,0.0f,0.0f}), // up
    proj*glm::rotate(glm::mat4{1}, float(M_PI)/2.0f, {-1.0f,0.0f,0.0f}), // down
    proj*glm::rotate(glm::mat4{1}, float(M_PI)/2.0f, {0.0f,-1.0f,0.0f}), // right
    proj*glm::rotate(glm::mat4{1}, float(M_PI)/2.0f, {0.0f,+1.0f,0.0f}), // left
};

QuadMesh mesh;
QuadMesh::compiled compiled_mesh;
SDL_Window *window;
Shader modelview_prog;
Shader findnext_prog;
Shader splat_prog;
Shader hcube_prog;
Framebuffer hcube;
Framebuffer findnext;
std::vector<Framebuffer> atlas, atlasb;
float id = 0;
float energy = 0.0;

void iterate() {
    // === Find next emitter ===
    // Render every patch as a single pixel. Its average energy is first
    // found by computing the mipmap for the energy texture atlas. Its
    // 'color' is then its id, and its depth is the inverse of its energy,
    // so that after we finish rendering the patch with the most energy can
    // be found by reading back the frame buffer.

    findnext_prog.use();
    findnext.bind();
    glEnable(GL_DEPTH_TEST);
    glClearColor(0,0,777.0f,0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    for (int i = 0; i < atlasb.size(); i++) {
        findnext_prog.uniform("resid", atlasb[i].color(1));
        compiled_mesh.draw(i*6*64*64, 6*64*64);
    }
    float color[4];
    glReadPixels(0,0,1,1,GL_RGBA,GL_FLOAT, &color);
    id = color[0]-1;
    energy = color[1];
    if (id < 0)
		return; //  converged!

    // === Setup frame of reference ===

    const glm::vec3 a = mesh.vert(id*4).loc,   b = mesh.vert(id*4+1).loc;
    const glm::vec3 d = mesh.vert(id*4+2).loc, c = mesh.vert(id*4+3).loc;
    const float u = 0.5f, v = 0.5f;
    const glm::vec3 up     = glm::normalize(b-a);
    const glm::vec3 right  = glm::normalize(c-a);
    const glm::vec3 forwd  = glm::cross(right, up);
    const glm::vec3 pos = (u*a+(1.0f-u)*b)*v + (u*c+(1.0f-u)*d)*(1.0f-v);
    const float darea = glm::length(glm::cross(b-a, c-a));
    const auto frame = glm::lookAt(pos, pos-forwd, up);


    // === Draw hemicube ===

    hcube_prog.use();
    hcube_prog.uniform("frame", frame);
    hcube_prog.uniform("views", views);
    hcube.bind();
    glEnable(GL_DEPTH_TEST);
    glClearColor(0,0,0,0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    for (int i = 0; i < 5; i++) {
        // TODO: switch to instanced draw
        hcube_prog.uniform("side", i);
        hcube.bind((i%4)*1024,(i/4)*1024,1024,1024);
        compiled_mesh.draw();
    }

    // === Transfer energy ===

    splat_prog.use();
    splat_prog.uniform("views", views);
    splat_prog.uniform("hcube", hcube.color(0));
    splat_prog.uniform("frame", frame);
    splat_prog.uniform("emitter_id", id);
    splat_prog.uniform("energy", energy);
    splat_prog.uniform("darea", darea);
    glDisable(GL_DEPTH_TEST);
    for (int i = 0; i < atlasb.size(); i++) {
        splat_prog.uniform("prev_accum", atlasb[i].color(0));
        splat_prog.uniform("prev_resid", atlasb[i].color(1));
        atlas[i].bind();
        compiled_mesh.draw(i*6*64*64, 6*64*64);
    }
	
    for (auto &fb : atlasb) { fb.color(0).gen_mipmaps(); fb.color(1).gen_mipmaps(); }
    for (auto &fb : atlas)  { fb.color(0).gen_mipmaps(); fb.color(1).gen_mipmaps(); }
}

EM_BOOL update(double time, void *_userdata)
{
	iterate();
	
    // === Draw the scene ===

    static float angle = 0.0f;
    const auto mvp =
        glm::perspective(float(M_PI)/2.0f, 1.0f, 0.5f, 500.0f)
      * glm::translate(glm::mat4{1.0f}, {0,0,-7})
      * glm::rotate(glm::mat4{1.0f}, angle += 0.005f, {0,1,0});
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0,0,512,512);
    glClearColor(.75,.75,.75,1);
    glEnable(GL_CULL_FACE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    modelview_prog.use();
    modelview_prog.uniform("mvp", mvp);
    for (int i = 0; i < atlas.size(); i++) {
        modelview_prog.uniform("resid", atlas[i].color(0));
        compiled_mesh.draw(i*6*64*64, 6*64*64);
    }
	
    SDL_GL_SwapWindow(window);
	glFinish();
    std::swap(atlas, atlasb);
	return EM_TRUE;
};

int main(int argc, char *argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Failed to initialize SDL: "
                  << SDL_GetError() << std::endl;
        return 1;
    }
#if __EMSCRIPTEN__
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#else
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
#endif

    window = SDL_CreateWindow("radiosity",
        100, 100, 512, 512, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Failed to create window."
                  << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_GLContext context = SDL_GL_CreateContext(window);
    if (!context) {
        std::cerr << "Error: Failed to create OpenGL context. "
                  << SDL_GetError() << std::endl;
        return 1;
    }
    SDL_GL_MakeCurrent(window, context);

#ifndef __EMSCRIPTEN__
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Error: Failed to initialize GLEW." << std::endl;
        return 1;
    }
#endif

	const char *meshname = (argc > 2)? argv[1] : "asset/sqube.obj";
    if (!load_obj(mesh, meshname)) {
        std::cerr << "Couldn't load mesh." << std::endl;
        return 1;
    }
    compiled_mesh = mesh.compile();

    modelview_prog = Shader::compile({
        {ShaderType::Vertex,   "asset/modelview.vert.glsl"},
        {ShaderType::Fragment, "asset/modelview.frag.glsl"}
    });
    hcube_prog = Shader::compile({
        {ShaderType::Vertex,   "asset/hcube.vert.glsl"},
        {ShaderType::Fragment, "asset/hcube.frag.glsl"}
    });
    splat_prog = Shader::compile({
        {ShaderType::Vertex,   "asset/splat.vert.glsl"},
        {ShaderType::Fragment, "asset/splat.frag.glsl"}
    });
    findnext_prog = Shader::compile({
        {ShaderType::Vertex,   "asset/findnext.vert.glsl"},
        {ShaderType::Fragment, "asset/findnext.frag.glsl"}
    });

    hcube = Framebuffer::create(4096,2048,{ColorFormat::Red},ColorFormat::DepthStencil);
    hcube.compile();
    findnext = Framebuffer::create(1,1,{ColorFormat::RedBlueGreen},ColorFormat::DepthStencil);
    findnext.compile();
    for (int i = 0; i < compiled_mesh.count/6; i += 64*64) {
        atlasb.push_back(Framebuffer::create(
			16*64,16*64,std::initializer_list<ColorFormat>{
            ColorFormat::RedBlueGreen,ColorFormat::RedBlueGreen},
            ColorFormat::DepthStencil));
        atlas.push_back(Framebuffer::create(
			16*64,16*64,std::initializer_list<ColorFormat>{
            ColorFormat::RedBlueGreen,ColorFormat::RedBlueGreen},
            ColorFormat::DepthStencil));
    }
    atlasb.front().color(1).write<float,4>({400000.0f,0,0,0},512,0);
    for (auto &fb : atlasb) { fb.compile(); fb.color(0).gen_mipmaps(); fb.color(1).gen_mipmaps(); }
    for (auto &fb : atlas)  { fb.compile(); fb.color(0).gen_mipmaps(); fb.color(1).gen_mipmaps(); }
	glFinish();


#ifdef __EMSCRIPTEN__
	emscripten_request_animation_frame_loop(update, 0);
#else
	for (;;) {
		update(0, nullptr);
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT || (ev.type == SDL_KEYDOWN
                    && ev.key.keysym.sym == SDLK_ESCAPE)) {
                SDL_Quit();
                return 0;
            }
        }
		SDL_Delay(15);
	}
#endif
}