#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <ostream>

class Shader {
private:
    bool compile(GLuint &sh, const std::string &filename, std::ostream &log) {
        std::ifstream fs{filename, std::ios_base::binary};
        if (!fs) {
            log << "Error: Cannot open " << filename << "\n";
            return 0;
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
        std::cout << buffer.data() << std::endl;
        
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
			log << "Compiling " << filename << std::endl;
			log << buffer.data();
        }
        glGetShaderiv(sh, GL_COMPILE_STATUS, &stat);
        return (stat == GL_TRUE);
    }
public:
    bool create(const std::string &filename, std::ostream &log) noexcept {
        m_prog = glCreateProgram();
		GLuint vert = glCreateShader(GL_VERTEX_SHADER);
		GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);

        // Compile and link shaders
		if (compile(vert, filename + ".vert.glsl", log)
         && compile(frag, filename + ".frag.glsl", log)) {
        	glAttachShader(m_prog, vert);
        	glAttachShader(m_prog, frag);
        	glLinkProgram(m_prog);
		}
        glDeleteShader(vert);
		glDeleteShader(frag);
		
        // Handle errors and warnings
        GLint loglen, stat;
        glGetProgramiv(m_prog, GL_INFO_LOG_LENGTH, &loglen);
        if (loglen > 0) {
            std::vector<char> buffer(loglen+1);
            glGetProgramInfoLog(m_prog, loglen, NULL, buffer.data());
			log << "Linking " << filename << std::endl;
			log << buffer.data();
        }
        glGetProgramiv(m_prog, GL_LINK_STATUS, &stat);
        return (stat == GL_TRUE);
    }
	void create(const std::string &filename) {
		if (!create(filename, std::cerr))
			throw std::runtime_error("Failed to create shader program.");
	}
 
    void use() const
        { glUseProgram(m_prog); }

    void uniform(const char *name, int i) const
        { glUniform1i(glGetUniformLocation(m_prog, name), i); }
    void uniform(const char *name, float f) const
        { glUniform1f(glGetUniformLocation(m_prog, name), f); }
    void uniform(const char *name, const glm::mat4 &mat) const {
		auto location = glGetUniformLocation(m_prog, name);
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(mat));
	}
    void uniform(const char *name, const glm::vec3 &vec) const
        { glUniform3fv(glGetUniformLocation(m_prog, name),
            1, glm::value_ptr(vec)); }
    template <size_t N>
    void uniform(const char *name, const std::array<glm::mat4,N> &mats) const
        { glUniformMatrix4fv(glGetUniformLocation(m_prog, name),
            N, GL_FALSE, glm::value_ptr(mats[0])); }
    void uniform(const char *name, int slot, const Texture &texture) const {
		texture.bind(slot);
        glUniform1i(glGetUniformLocation(m_prog, name), slot);
    }

    GLuint m_prog;
};