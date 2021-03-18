#pragma once

class QuadMesh {
public:
    // Represents a handle to the mesh sent to the GPU
    struct compiled {
        int count;
        GLuint vao;
        GLuint vbo;
        GLuint ibo;

        void draw(int ofs, int count) const {
            if (ofs+count > this->count)
                count = this->count-ofs;
            std::cout << "drawing compiled mesh " << vao << std::endl;
            glBindVertexArray(vao);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
            glEnableVertexAttribArray(0);
            glEnableVertexAttribArray(1);
            glEnableVertexAttribArray(2);
            glEnableVertexAttribArray(3);
            glEnableVertexAttribArray(4);
            glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, (void *)ofs);
        }
        void draw() const { draw(0, count); }
    };
    compiled compile(compiled &&prev = {0,0,0,0}) {
        compiled next = prev;
        if (!next.vao) {
            glGenBuffers(1, &next.vbo);
            glGenBuffers(1, &next.ibo);
            
            glGenVertexArrays(1, &next.vao);
            glBindVertexArray(next.vao);
            glBindBuffer(GL_ARRAY_BUFFER, next.vbo);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                sizeof(vertex), (void *)offsetof(vertex, loc));
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
                sizeof(vertex), (void *)offsetof(vertex, uv));
            glVertexAttribIPointer(2, 1, GL_INT,
                sizeof(vertex), (void *)offsetof(vertex, id));
            glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE,
                sizeof(vertex), (void *)offsetof(vertex, norm));
            glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE,
                sizeof(vertex), (void *)offsetof(vertex, area));
        }
        next.count = m_indices.size();
        glBindVertexArray(next.vao);
        glBindBuffer(GL_ARRAY_BUFFER, next.vbo);
        glBufferData(GL_ARRAY_BUFFER,
            m_verts.size()*sizeof(vertex),
            m_verts.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, next.ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
            m_indices.size()*sizeof(unsigned),
            m_indices.data(), GL_STATIC_DRAW);
        glBindVertexArray(0);
        std::cout << "compiled mesh with " << next.count
            << " indices to " << next.vao << std::endl;
        return next;
    }

    struct vertex {
        glm::vec3 loc;
        glm::vec2 uv;
        int id;
        glm::vec3 norm;
        float area;
    };
    const vertex &vert(unsigned i) const { return m_verts.at(i); }
    unsigned add_vert(vertex &&v)
        { m_verts.push_back(std::move(v)); return m_verts.size()-1; }
    
    unsigned add_face(unsigned a, unsigned b, unsigned c, unsigned d)
    // Convert to triangles implicitly, since OpenGL got rid of quads :(
        { m_indices.insert(m_indices.end(), {a,b,d, b,c,d}); return m_indices.size()-6; }
private:
    std::vector<vertex> m_verts;
    std::vector<unsigned> m_indices;
};

bool load_obj(QuadMesh &mesh, const char *fname)
{
    std::ifstream is{fname};
    if (!is)
        return false;

    std::string linebuf;
    std::vector<glm::vec3> verts;
    int id = 0;
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
            glm::vec3 norm = glm::cross(verts[b-1]-verts[a-1], verts[d-1]-verts[a-1]);
            float area = glm::length(norm); norm /= area;
            auto a2 = mesh.add_vert({verts[a-1], glm::vec2{0.0f, 0.0f}, id, norm, area});
            auto b2 = mesh.add_vert({verts[b-1], glm::vec2{1.0f, 0.0f}, id, norm, area});
            auto c2 = mesh.add_vert({verts[c-1], glm::vec2{1.0f, 1.0f}, id, norm, area});
            auto d2 = mesh.add_vert({verts[d-1], glm::vec2{0.0f, 1.0f}, id, norm, area});
            mesh.add_face(a2, b2, c2, d2);
            id++;
        }
        else
            continue;  // Silently ignore unrecognized commands
    }
    return true;
}