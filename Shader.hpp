
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

        std::ifstream fs{filename, std::ios_base::binary};
        if (!fs) {
            std::cerr << "Error: Cannot open " << filename << std::endl;
            exit(1);
        }

        // Read in file to buffer
        std::vector<char> buffer;
		for (;;) {
			int c = fs.get();
			if (c == EOF || fs.eof())
				break;
			buffer.push_back((char)c);
		}
		buffer.push_back(0);
        
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