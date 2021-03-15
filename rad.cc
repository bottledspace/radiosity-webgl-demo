#if __EMSCRIPTEN__
# include <emscripten/emscripten.h>
#endif
#include <GL/glew.h>
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#define NO_SDL_GLEXT
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


#ifndef M_PI
const float M_PI = 3.14159f;
#endif

template <typename VertexT>
struct VertexDescriptor {};

template <>
struct VertexDescriptor<glm::vec3> {
    static void describe() {
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);
    }
    static void enable() {
        glEnableVertexAttribArray(0);
    }
};

enum class PrimitiveType : GLenum {
    Points    = GL_POINTS,
    Lines     = GL_LINES,
    Triangles = GL_TRIANGLES,
    LinesAdj  = GL_LINES_ADJACENCY,
};

template <typename VertexT>
struct VertexArray {
    VertexArray() {
        glGenVertexArrays(1, &m_vao);
        glBindVertexArray(m_vao);
        glGenBuffers(1, &m_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        VertexDescriptor<VertexT>::describe();
    }

    void draw_instanced(PrimitiveType type, int count, int offset, int size) const {
        VertexDescriptor<VertexT>::enable();
        glDrawArraysInstanced(static_cast<GLenum>(type), offset, size, count);
    } 
    void draw(PrimitiveType type, int offset, int size) const {
        VertexDescriptor<VertexT>::enable();
        glDrawArrays(static_cast<GLenum>(type), offset, size);
    }
    void draw(PrimitiveType type) const
        { draw(type, 0, size()); }

    void push_back(const VertexT &vert)
        { m_verts.push_back(vert); }
    
    auto size() const
        { return m_verts.size(); }
    auto &at(size_t i) const
        { return m_verts[i]; }

    void resync() {
        glBindVertexArray(m_vao);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER,
            m_verts.size()*sizeof(VertexT),
            m_verts.data(), GL_STATIC_DRAW);
    }

private:
    std::vector<VertexT> m_verts;
    GLuint m_vao;
    GLuint m_vbo;
};

bool load_obj(VertexArray<glm::vec3> &vb, const char *fname)
{
    std::ifstream is{fname};
    if (!is) {
    	std::cerr << "Cannot open " << fname << std::endl;
	    return false;
	}
	
    std::string linebuf;
    std::vector<glm::vec3> verts;
    while (std::getline(is, linebuf)) {
        std::stringstream ls{linebuf};
        std::string cmd;

        if (!(ls >> cmd))
            continue;  // Ignore empty lines
        if (cmd == "vt")
            continue;  // Ignore texture coordinates
        else if (cmd == "vn")
            continue;  // Ignore normal coordinates
        else if (cmd == "v") {
            float x, y, z;
            ls >> x >> y >> z;
            verts.push_back({x, y, z});
        }
        else if (cmd == "f") {
            int a, b, c, d;
            ls >> a >> b >> c >> d;
            vb.push_back(verts[a-1]);
            vb.push_back(verts[b-1]);
            vb.push_back(verts[c-1]);
            vb.push_back(verts[d-1]);
        }
        else
            continue;  // Silently ignore unrecognized commands
    }
    return true;
}


enum class ColorFormat : GLuint {
    None         = 0,
    Red          = GL_R32I,
    RedBlueGreen = GL_RGB8UI,
    DepthStencil = GL_DEPTH24_STENCIL8,
};

template <typename T> struct gl_type {};
template <> struct gl_type<float> { static GLenum value() { return GL_FLOAT; } };
template <> struct gl_type<int> { static GLenum value() { return GL_INT; } };

class Texture {
public:
    Texture(int width, int height, ColorFormat format)
    : m_width{width}, m_height{height}  {
        glGenTextures(1, &m_texid);
        glBindTexture(GL_TEXTURE_2D, m_texid);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

        switch (format) {
        case ColorFormat::Red:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_R32I, width, height,
                0, GL_RED_INTEGER, GL_INT, 0);
            m_format = GL_RED_INTEGER;
            break;
        case ColorFormat::RedBlueGreen:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height,
                0, GL_RGBA, GL_FLOAT, 0);
            m_format = GL_RGBA;
            break;
        case ColorFormat::DepthStencil:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F,
                width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
            m_format = GL_DEPTH_COMPONENT;
            break;
        }
    }

    template <typename T, int N>
    auto read() const {
		bind();
        std::vector<T> pixels(m_width*m_height*N);
        glGetTexImage(GL_TEXTURE_2D, 0, m_format, gl_type<T>::value(), pixels.data());
        return pixels;
    }

    template <typename T, int N>
    void write(std::array<T, N> &&pixel, int x, int y) {
		bind();
        glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, 1, 1, m_format,
            gl_type<T>::value(), pixel.data());
    }

    void gen_mipmaps() const {
		bind();
        glGenerateMipmap(GL_TEXTURE_2D);
    } 

    void bind(int n = 0) const {
        glActiveTexture(GL_TEXTURE0 + n);
        glBindTexture(GL_TEXTURE_2D, m_texid);
    }

    GLuint texid() const
        { return m_texid; }
private:
    GLuint m_texid;
    int m_width, m_height;
    GLenum m_format;
};

class Framebuffer {
public:
    Framebuffer(int width, int height, std::initializer_list<ColorFormat> &&formats, ColorFormat depth = ColorFormat::None)
    : m_width{width}, m_height{height} {
        glGenFramebuffers(1, &m_fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
        
        const GLenum attachments[] = {
            GL_COLOR_ATTACHMENT0,
            GL_COLOR_ATTACHMENT1,
            GL_COLOR_ATTACHMENT2
        };
        int i = 0;
        for (auto format : formats) {
            m_channels.emplace_back(width, height, format);
            glFramebufferTexture2D(GL_FRAMEBUFFER, attachments[i],
                GL_TEXTURE_2D, m_channels[i].texid(), 0);
            i++;
        }
        glDrawBuffers(formats.size(), attachments);
        if (depth != ColorFormat::None) {
            m_channels.emplace_back(width, height, depth);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                GL_TEXTURE_2D, m_channels.back().texid(), 0);
        }
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "Warn: Framebuffer failed." << std::endl;
            return;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void bind(int x, int y, int width, int height) const {
        glViewport(x, y, width, height);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
    }
    void bind() const { bind(0, 0, m_width, m_height); }

    const Texture &color(int n = 0) const
        { return m_channels[n]; }
    const Texture &depth() const
        { return m_channels.back(); }

private:
    int m_width, m_height;
    GLuint m_fbo;
    std::vector<Texture> m_channels;
};

enum class ShaderType : GLenum {
    Vertex   = GL_VERTEX_SHADER,
    Fragment = GL_FRAGMENT_SHADER,
    Geometry = GL_GEOMETRY_SHADER
};

class Shader {
public:
    Shader(std::initializer_list<std::pair<ShaderType,const char *>> &&sources) {
        m_prog = glCreateProgram();

        // Compile and link shaders
        std::vector<GLuint> ids;
        std::string temp;
        for (const auto &[type, filename] : sources) {
            ids.push_back(compile_shader(type, filename));
            temp = temp + filename + ", ";
        }
        glLinkProgram(m_prog);
        for (auto id : ids)
            glDeleteShader(id);

        // Handle errors and warnings
        GLint loglen, stat;
        glGetProgramiv(m_prog, GL_INFO_LOG_LENGTH, &loglen);
        if (loglen > 0) {
            std::vector<char> buffer(loglen+1);
            glGetProgramInfoLog(m_prog, loglen, NULL, buffer.data());
            std::cerr << buffer.data() << std::endl;
        }
        glGetProgramiv(m_prog, GL_LINK_STATUS, &stat);
        if (stat != GL_TRUE) {
            std::cerr << "Error: Failed to link " << temp
                << "Exiting." << std::endl;
            exit(1);
        }
    }
 
    void use()
        { glUseProgram(m_prog); m_color_slot = 0; }

    void uniform(const char *name, int i)
        { glUniform1i(glGetUniformLocation(m_prog, name), i); }
    void uniform(const char *name, float f)
        { glUniform1f(glGetUniformLocation(m_prog, name), f); }
    void uniform(const char *name, const glm::mat4 &mat)
        { glUniformMatrix4fv(glGetUniformLocation(m_prog, name),
            1, GL_FALSE, glm::value_ptr(mat)); }
    void uniform(const char *name, const glm::vec3 &vec)
        { glUniform3fv(glGetUniformLocation(m_prog, name),
            1, glm::value_ptr(vec)); }
    template <size_t N>
    void uniform(const char *name, const std::array<glm::mat4,N> &mats)
        { glUniformMatrix4fv(glGetUniformLocation(m_prog, name),
            N, GL_FALSE, glm::value_ptr(mats[0])); }
    void uniform(const char *name, const Texture &texture) {
        texture.bind(m_color_slot);
        glUniform1i(glGetUniformLocation(m_prog, name), m_color_slot);
        m_color_slot++;
    }

private:
    GLuint compile_shader(ShaderType type, const char *filename) {
        GLuint sh = glCreateShader(static_cast<GLenum>(type));

        std::ifstream fs{filename};
        if (!fs) {
            std::cerr << "Error: Cannot open " << filename << std::endl;
            exit(1);
        }
        fs.seekg(0, std::ios_base::end);
        long length = fs.tellg();
        fs.seekg(0, std::ios_base::beg);

        // Read in file to buffer
        std::vector<char> buffer(length);
        fs.read(buffer.data(), buffer.size());
        
        // Compile shader
        const char *p = buffer.data();
        glShaderSource(sh, 1, &p, NULL);
        glCompileShader(sh);

        // Handle errors and warnings
        GLint loglen, stat;
        glGetShaderiv(sh, GL_INFO_LOG_LENGTH, &loglen);
        if (loglen > 0) {
            std::vector<char> buffer(loglen+1);
            glGetShaderInfoLog(sh, loglen, NULL, buffer.data());
            std::cerr << filename << ":\n" << buffer.data() << std::endl;
        }
        glGetShaderiv(sh, GL_COMPILE_STATUS, &stat);
        if (stat != GL_TRUE) {
            std::cerr << "Error: Failed to compile " << filename
                << ". Exiting." << std::endl;
            exit(1);
        }

        glAttachShader(m_prog, sh);
        return sh;
    }

    GLuint m_prog;
    int m_color_slot;
};


const auto proj = glm::perspective(float(M_PI)/2.0f, 1.0f, 0.1f, 500.0f);
const std::array<glm::mat4,5> views {
    proj,                                                         // forward
    proj*glm::rotate(glm::mat4{1}, float(M_PI)/2.0f, {+1.0f,0.0f,0.0f}), // up
    proj*glm::rotate(glm::mat4{1}, float(M_PI)/2.0f, {-1.0f,0.0f,0.0f}), // down
    proj*glm::rotate(glm::mat4{1}, float(M_PI)/2.0f, {0.0f,-1.0f,0.0f}), // right
    proj*glm::rotate(glm::mat4{1}, float(M_PI)/2.0f, {0.0f,+1.0f,0.0f}), // left
};

int main(int argc, char *argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Failed to initialize SDL: "
                  << SDL_GetError() << std::endl;
        return 1;
    } 
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    SDL_Window *window = SDL_CreateWindow("radiosity",
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

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Error: Failed to initialize GLEW." << std::endl;
        return 1;
    }
	
	const char *filename = (argc >= 2)? argv[1] : "asset/sqube.obj";

    VertexArray<glm::vec3> vb;
    if (!load_obj(vb, filename))
        return 1;
    vb.resync();

    Framebuffer hcube{4096,2048,{ColorFormat::Red},ColorFormat::DepthStencil};
    Framebuffer findnext{1,1,{ColorFormat::Red},ColorFormat::DepthStencil};
    std::vector<Framebuffer> atlas, atlasb;
    Shader init_prog {
        {ShaderType::Geometry, "asset/init_geom.glsl"},
        {ShaderType::Vertex,   "asset/init_vert.glsl"},
        {ShaderType::Fragment, "asset/init_frag.glsl"}
    };
    Shader modelview_prog {
        {ShaderType::Geometry, "asset/modelview_geom.glsl"},
        {ShaderType::Vertex,   "asset/modelview_vert.glsl"},
        {ShaderType::Fragment, "asset/modelview_frag.glsl"}
    };
    Shader hcube_prog {
        {ShaderType::Geometry, "asset/hcube_geom.glsl"},
        {ShaderType::Vertex,   "asset/hcube_vert.glsl"},
        {ShaderType::Fragment, "asset/hcube_frag.glsl"}
    };
    Shader splat_prog {
        {ShaderType::Geometry, "asset/splat_geom.glsl"},
        {ShaderType::Vertex,   "asset/splat_vert.glsl"},
        {ShaderType::Fragment, "asset/splat_frag.glsl"}
    };
    Shader findnext_prog {
        {ShaderType::Geometry, "asset/findnext_geom.glsl"},
        {ShaderType::Vertex,   "asset/findnext_vert.glsl"},
        {ShaderType::Fragment, "asset/findnext_frag.glsl"}
    };


    for (int i = 0; i < vb.size()/4; i += 64*64) {
        atlasb.emplace_back(16*64,16*64,std::initializer_list<ColorFormat>{
            ColorFormat::RedBlueGreen,ColorFormat::RedBlueGreen},
            ColorFormat::DepthStencil);
        atlas.emplace_back(16*64,16*64,std::initializer_list<ColorFormat>{
            ColorFormat::RedBlueGreen,ColorFormat::RedBlueGreen},
            ColorFormat::DepthStencil);
    }
    
    Texture transfer{16*64,16*64,ColorFormat::RedBlueGreen};
    transfer.write<float,3>({400000.0f,0,0},512,0);
    init_prog.use();
    init_prog.uniform("transfer", transfer);
    atlasb.front().bind();
    glClearColor(0,0,0,0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    vb.draw(PrimitiveType::LinesAdj, 0, 4);

    int id = 0;
    float energy = 0.0;
    for (;;) {
        // === Find next emitter ===

        findnext_prog.use();
        findnext.bind();
        glEnable(GL_DEPTH_TEST);
        glClearColor(0,0,0,0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        for (int i = 0; i < vb.size()/4; i += 64*64) {
            atlasb[i/(64*64)].color(1).gen_mipmaps();
            findnext_prog.uniform("resid", atlasb[i/(64*64)].color(1));
            findnext_prog.uniform("patchofs", i);
            vb.draw(PrimitiveType::LinesAdj, i*4,
                std::min<int>(64*64*4, vb.size()-4*i));
        }
        id = findnext.color().read<int,1>()[0]-1;
        if (id < 0)
            goto Converged;
        energy = 1.0f/findnext.depth().read<float,1>()[0]-1.0f;


        // === Setup frame of reference ===
{
        const glm::vec3 a = vb.at(id*4),   b = vb.at(id*4+1);
        const glm::vec3 d = vb.at(id*4+2), c = vb.at(id*4+3);
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
            vb.draw(PrimitiveType::LinesAdj);
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
        for (int i = 0; i < vb.size()/4; i+=64*64) {
            splat_prog.uniform("patchofs", i);
            splat_prog.uniform("prev_accum", atlasb[i/(64*64)].color(0));
            splat_prog.uniform("prev_resid", atlasb[i/(64*64)].color(1));
            atlas[i/(64*64)].bind();
            vb.draw(PrimitiveType::LinesAdj, i*4,
                std::min<int>(4*64*64, vb.size()-4*i));
        }
}
        // === Draw the scene ===
Converged:
        static float angle = 0.0f;
        const auto mvp =
            glm::perspective(float(M_PI)/2.0f, 1.0f, 0.5f, 500.0f)
          * glm::translate(glm::mat4{1.0f}, {0,0,-7})
          * glm::rotate(glm::mat4{1.0f}, angle += 0.005f, {0,1,0});
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0,0,512,512);
        glClearColor(0,0,0,0);
        glEnable(GL_CULL_FACE);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        modelview_prog.use();
        modelview_prog.uniform("mvp", mvp);
        modelview_prog.uniform("emitter_id", -1);
        for (int i = 0; i < vb.size()/4; i += 64*64) {
            modelview_prog.uniform("resid", atlas[i/(64*64)].color(0));
            modelview_prog.uniform("patchofs", i);
            vb.draw(PrimitiveType::LinesAdj, i*4,
                std::min<int>(64*64*4, vb.size()-4*i));
        }
        SDL_GL_SwapWindow(window);


        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT || (ev.type == SDL_KEYDOWN
                    && ev.key.keysym.sym == SDLK_ESCAPE)) {
                SDL_Quit();
                return 0;
            }
        }
        SDL_Delay(15);
        std::swap(atlas, atlasb);
    }
}