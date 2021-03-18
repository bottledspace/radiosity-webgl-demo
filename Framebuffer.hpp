
class Framebuffer {
public:
    Framebuffer(int width, int height, std::initializer_list<ColorFormat> &&formats, ColorFormat depth = ColorFormat::None)
    : m_width{width}, m_height{height}, m_depth{false} {
        glGenFramebuffers(1, &m_fbo);
        for (auto format : formats)
            m_channels.emplace_back(width, height, format);
        if (depth != ColorFormat::None) {
            m_depth = true;
            m_channels.emplace_back(width, height, depth);
        }
    }

    void compile() {
        const GLenum attachments[] = {
            GL_COLOR_ATTACHMENT0,
            GL_COLOR_ATTACHMENT1,
            GL_COLOR_ATTACHMENT2
        };

        bind();
        for (int i = 0; i < (m_depth? (m_channels.size()-1) : m_channels.size()); i++)
            glFramebufferTexture2D(GL_FRAMEBUFFER, attachments[i],
                GL_TEXTURE_2D, m_channels[i].texid(), 0);
        glDrawBuffers(m_channels.size(), attachments);
        if (m_depth)
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                GL_TEXTURE_2D, m_channels.back().texid(), 0);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "Warn: Framebuffer failed." << std::endl;
            return;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void bind(int x, int y, int width, int height) const {
        glViewport(x, y, width, height);
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
        std::cout << "binding framebuffer " << m_fbo << std::endl;
    }
    void bind() const { bind(0, 0, m_width, m_height); }

    Texture &color(int n = 0)
        { return m_channels[n]; }
    Texture &depth()
        { return m_channels.back(); }

private:
    int m_width, m_height;
    bool m_depth;
    GLuint m_fbo;
    std::vector<Texture> m_channels;
};