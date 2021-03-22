

class Texture {
public:
	static constexpr struct {
		GLenum sized;
		GLenum channels;
		GLenum datatype;
	} fmtab[] = {
		// Formats in this table are not meant to be exhaustive of all possible
		// format combinations, but rather a small subset which has been tested
		// on all target platforms (particularly webgl).
		{ GL_R32F, GL_RED, GL_FLOAT },
		{ GL_RGBA32F, GL_RGBA, GL_FLOAT },
		{ GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT },
	};
	enum format_type {
		r32f = 0,
		rgba32f,
		depth
	};
	
	Texture()
	: m_texid{0}
		{}
	Texture(int width, int height, format_type format)
	: m_texid{0}
		{ create(width, height, format); }
	
	void swap(Texture &other) {
		std::swap(other.m_texid, m_texid);
		std::swap(other.m_format, m_format);
		std::swap(other.m_width, m_width);
		std::swap(other.m_height, m_height);
	}
	
    void create(int width, int height, format_type format) {
		m_width = width;
		m_height = height;
		m_format = format;
        glGenTextures(1, &m_texid);
        glBindTexture(GL_TEXTURE_2D, m_texid);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glTexImage2D(GL_TEXTURE_2D, 0, fmtab[format].sized, width, height,
            0, fmtab[format].channels, fmtab[format].datatype, 0);
    }

    void write(void *pixel, int x, int y, int width, int height) {
		glBindTexture(GL_TEXTURE_2D, m_texid);
        glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, width, height, fmtab[m_format].channels,
            fmtab[m_format].datatype, pixel);
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
	format_type m_format;
    int m_width, m_height;
};