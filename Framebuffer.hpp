#pragma once

class Framebuffer {
public:
	void create() {
		glGenFramebuffers(1, &m_fbo);
	}
    bool complete() {
        const GLenum attachments[] = {
            GL_COLOR_ATTACHMENT0,
            GL_COLOR_ATTACHMENT1,
            GL_COLOR_ATTACHMENT2
        };
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
        for (int i = 0; i < m_colors.size(); i++)
            glFramebufferTexture2D(GL_FRAMEBUFFER, attachments[i],
                GL_TEXTURE_2D, m_colors[i].texid(), 0);
        glDrawBuffers(m_colors.size(), attachments);
        if (m_depth.valid()) {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                GL_TEXTURE_2D, m_depth.texid(), 0);
		}
        return (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
    }

    void bind(int x, int y, int width, int height) const {
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
        glViewport(x, y, width, height);
    }
    void bind() const
		{ bind(0, 0, m_colors.front().width(), m_colors.front().height()); }

	Texture &color(int n, const Texture &temp) {
		if (n >= m_colors.size())
			m_colors.resize(n+1);
		return m_colors[n] = temp;
	}
    Texture &color(int n = 0)
    	{ return m_colors[n]; }
    const Texture &color(int n = 0) const
        { return m_colors[n]; }
	Texture &depth(const Texture &temp) {
		return m_depth = temp;
	}
    Texture &depth()
        { return m_depth; }
    const Texture &depth() const
		{ return m_depth; }
private:
    GLuint m_fbo;
    std::vector<Texture> m_colors;
    Texture m_depth;
};