

class Texture {
public:
	Texture()
	: m_texid{0}
		{}
	Texture(int width, int height)
	: m_texid{0}
		{ create(width, height); }
	
	void swap(Texture &other) {
		std::swap(other.m_texid, m_texid);
		std::swap(other.m_width, m_width);
		std::swap(other.m_height, m_height);
	}
	
    Texture &create(int width, int height) {
		m_width = width;
		m_height = height;
        glGenTextures(1, &m_texid);
        glBindTexture(GL_TEXTURE_2D, m_texid);
		return *this;
    }
	// Formats provided here are not meant to be exhaustive of all possible
	// format combinations, but rather a small subset which has been tested
	// on all target platforms (particularly webgl).
	Texture &r32f() {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, m_width, m_height,
            0, GL_RED, GL_FLOAT, 0);
		return *this;
	}
	Texture &rgba32f() {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_width, m_height,
            0, GL_RGBA, GL_FLOAT, 0);
		return *this;
	}
	Texture &depth32f() {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, m_width, m_height,
            0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
		return *this;
	}
	Texture &nearest() {
    	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		return *this;
	}
	Texture &linear() {
    	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		return *this;
	}
	Texture &clamp() {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		return *this;
	}

    void write_rgba32f(const float *pixel, int x, int y, int width, int height) {
		glBindTexture(GL_TEXTURE_2D, m_texid);
        glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, width, height,
			GL_RGBA, GL_FLOAT, pixel);
    }

    void bind(int n = 0) const {
        glActiveTexture(GL_TEXTURE0 + n);
        glBindTexture(GL_TEXTURE_2D, m_texid);
    }


    GLuint texid() const
        { return m_texid; }
	bool valid() const
		{ return m_texid != 0; }
	int width() const
		{ return m_width; }
	int height() const
		{ return m_height; }
private:
    GLuint m_texid;
    int m_width, m_height;
};