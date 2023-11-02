#pragma once

#include "../wolf/wolf.h"

class Grid2D
{
    public:
        Grid2D(int linesPerHalfSpace);
        ~Grid2D();

        void update(float dt);
        void render(const glm::mat4& mView, const glm::mat4& mProj);
        void showAxes() { m_showAxes = true; }
        void hideAxes() { m_showAxes = false; }

    private:

        void _createGrid(int linesPerHalfSpace);
        void _createAxes();

        wolf::VertexBuffer* m_pVB = 0;
        wolf::VertexDeclaration* m_pDecl = 0;
        wolf::Program* m_pProgram = 0;
        wolf::Color4 m_color;

        wolf::VertexBuffer *m_pAxesVB = 0;
        wolf::VertexDeclaration *m_pAxesDecl = 0;

        int m_numVerts = 0;
        bool m_showAxes = true;
};