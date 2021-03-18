

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
        std::vector<T> pixels(m_width*m_height*N);
        glGetTextureImage(m_texid, 0, m_format, gl_type<T>::value(),
            sizeof(pixels[0])*pixels.size(), pixels.data());
        return pixels;
    }

    template <typename T, int N>
    void write(std::array<T, N> &&pixel, int x, int y) {
        glTextureSubImage2D(m_texid, 0, x, y, 1, 1, m_format,
            gl_type<T>::value(), pixel.data());
    }

    void gen_mipmaps() const {
        glGenerateTextureMipmap(m_texid);
    } 

    void bind(int n = 0) const {
        glActiveTexture(GL_TEXTURE0 + n);
        glBindTexture(GL_TEXTURE_2D, m_texid);
        std::cout << "binding " << m_texid << " to " << n << std::endl;
    }

    GLuint texid() const
        { return m_texid; }
private:
    GLuint m_texid;
    int m_width, m_height;
    GLenum m_format;
};