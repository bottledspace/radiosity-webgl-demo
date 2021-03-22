#if __EMSCRIPTEN__
# include <emscripten.h>
# include <emscripten/html5.h>
#include <GLES3/gl3.h>
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_PRECISION_HIGHP_FLOAT
#define GLM_FORCE_PURE
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

class Sol {
public:
	bool init(int argc, char *argv[]);
	void update();
	void draw() const;

private:	
	struct matgroup {
		Texture resid, accum;
		int ofs;
	};
	std::vector<matgroup> groups;
	
	void load();
	
	QuadMesh scene;
	QuadMesh::compiled compiled_scene;
	SDL_Window *window;
	Shader modelview_prog;
	
	Framebuffer splat;
	Shader splat_prog;
	
	Framebuffer findnext;
	Shader findnext_prog;
	
	Shader hcube_prog;
	Framebuffer hcube;
	
	float id;
	float energy;
};

bool Sol::init(int argc, char *argv[]) {
    window = SDL_CreateWindow("radiosity",
        100, 100, 512, 512, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (!window) {
        std::cerr << "Failed to create window."
                  << SDL_GetError() << std::endl;
        return false;
    }

    SDL_GLContext context = SDL_GL_CreateContext(window);
    if (!context) {
        std::cerr << "Error: Failed to create OpenGL context. "
                  << SDL_GetError() << std::endl;
        return false;
    }
    SDL_GL_MakeCurrent(window, context);

#ifndef __EMSCRIPTEN__
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Error: Failed to initialize GLEW." << std::endl;
        return false;
    }
#endif
	
    if (!load_obj(scene, "asset/sqube.obj")) {
        std::cerr << "Couldn't load scene." << std::endl;
		return false;
    }
    compiled_scene = scene.compile();
    modelview_prog.create("asset/modelview");
	
	hcube.create();
	hcube.color(0, Texture{4096,2048,Texture::r32f});
	hcube.depth(Texture{4096,2048,Texture::depth});
    hcube.complete();
    hcube_prog.create("asset/hcube");
	
	findnext.create();
	findnext.color(0, Texture{1,1,Texture::rgba32f});
	findnext.depth(Texture{1,1,Texture::depth});
    findnext.complete();
    findnext_prog.create("asset/findnext");
	
	splat.create();
	splat.color(0, {16*64,16*64,Texture::rgba32f});
	splat.color(1, {16*64,16*64,Texture::rgba32f});
	splat.depth({16*64,16*64,Texture::depth});
	splat.complete();
    splat_prog.create("asset/splat");
    
    groups.resize((compiled_scene.count+6*64*64-1)/(6*64*64));
    for (int i = 0; i < groups.size(); i++) {
		std::cout << "creating group " << i << std::endl;
        groups[i].accum.create(16*64,16*64,Texture::rgba32f);
		groups[i].resid.create(16*64,16*64,Texture::rgba32f);
		groups[i].ofs = i*6*64*64;
    }
    groups.front().resid.write((void *)std::array<float,4>{400000.0f,0.0f,0.0f,0.0f}.data(),512,0,1,1);
	return true;
}

void Sol::update() {
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
    for (const auto &group : groups) {
        findnext_prog.uniform("resid", 0, group.resid);
        compiled_scene.draw(group.ofs, 64*64*6);
    }
    float color[4];
    glReadPixels(0,0,1,1,GL_RGBA,GL_FLOAT, &color);
    id = color[0]-1;
    energy = color[1];
    if (id < 0)
		return; //  converged!

    // === Setup frame of reference ===

    const glm::vec3 a = scene.vert(id*4).loc,   b = scene.vert(id*4+1).loc;
    const glm::vec3 d = scene.vert(id*4+2).loc, c = scene.vert(id*4+3).loc;
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
        hcube_prog.uniform("side", i);
        hcube.bind((i%4)*1024,(i/4)*1024,1024,1024);
        compiled_scene.draw();
    }

    // === Transfer energy ===

    splat_prog.use();
    splat_prog.uniform("views", views);
    splat_prog.uniform("hcube", 0, hcube.color(0));
    splat_prog.uniform("frame", frame);
    splat_prog.uniform("emitter_id", id);
    splat_prog.uniform("energy", energy);
    splat_prog.uniform("darea", darea);
    glDisable(GL_DEPTH_TEST);
    for (auto &group : groups) {
        splat_prog.uniform("prev_accum", 1, group.accum);
        splat_prog.uniform("prev_resid", 2, group.resid);
        splat.bind();
        compiled_scene.draw(group.ofs, 64*64*6);
		splat.color(0).swap(group.accum);
		splat.color(1).swap(group.resid);
		splat.complete();
    }
}

void Sol::draw() const {
    static float angle = 0.0f;
    const glm::mat4 mvp =
        glm::perspective(float(M_PI)/4.0f, 1.0f, 0.1f, 500.0f)
      * glm::translate(glm::mat4{1.0f}, {0.0f,0.0f,-15.0f})
      * glm::rotate(glm::mat4{1.0f}, angle += 0.005f, {0.0f,1.0f,0.0f});

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0,0,512,512);
    glClearColor(.75,0.75,.75,1);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    modelview_prog.use();
	modelview_prog.uniform("mvp", mvp);
    for (const auto &group : groups) {
        modelview_prog.uniform("resid", 0, group.accum);
        compiled_scene.draw(group.ofs, 64*64*6);
    }
	
    SDL_GL_SwapWindow(window);
	glFinish();
}

EM_BOOL update(double time, void *userdata)
{
	Sol *app = (Sol *)userdata;
	app->update();
	app->draw();

    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
        if (ev.type == SDL_QUIT || (ev.type == SDL_KEYDOWN
                && ev.key.keysym.sym == SDLK_ESCAPE)) {
            SDL_Quit();
            return EM_FALSE;
        }
    }
	return EM_TRUE;
}

// The app needs to be global so it doesn't go out of scope when the animation
// callback uses it in the HTML 5 build.
Sol app;

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
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
#endif
	SDL_GL_SetSwapInterval(1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	if (!app.init(argc, argv)) {
		std::cerr << "Fatal error encountered. Exiting." << std::endl;
		return EXIT_FAILURE;
	}

#ifdef __EMSCRIPTEN__
	emscripten_request_animation_frame_loop(update, &app);
#else
	for (;;) {
		if (!update(0, &app))
			break;
		SDL_Delay(15);
	}
#endif
	return EXIT_SUCCESS;
}