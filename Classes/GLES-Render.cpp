/*
 * Copyright (c) 2006-2007 Erin Catto http://www.gphysics.com
 *
 * iPhone port by Simon Oliver - http://www.simonoliver.com - http://www.handcircus.com
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software
 * in a product, an acknowledgment in the product documentation would be
 * appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

#include "GLES-Render.h"
#include "cocos2d.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

USING_NS_CC;

GLESDebugDraw::GLESDebugDraw()
    : mRatio(1.0f)
{
    this->initShader();
}

GLESDebugDraw::GLESDebugDraw(float32 ratio)
    : mRatio(ratio)
{
    this->initShader();
}

void GLESDebugDraw::initShader(void)
{
    //mShaderProgram = ShaderCache::getInstance()->getProgram(GLProgram::SHADER_NAME_POSITION_U_COLOR);

    //mColorLocation = glGetUniformLocation(mShaderProgram->getProgram(), "u_color");
}

void GLESDebugDraw::DrawPolygon(const b2Vec2* old_vertices, int vertexCount, const b2Color& color)
{
    // 頂点を順番に描画
    for (int32 i = 0; i < vertexCount; ++i) {
        int32 nextIndex = (i + 1) % vertexCount;
        b2Vec2 v1 = old_vertices[i];
        b2Vec2 v2 = old_vertices[nextIndex];

        // ここで描画ライブラリに依存した描画を行う。例えば、OpenGLなら
        glColor3f(color.r, color.g, color.b);
        glBegin(GL_LINES);
        glVertex2f(v1.x, v1.y);
        glVertex2f(v2.x, v2.y);
        glEnd();
    }
}

void GLESDebugDraw::DrawSolidPolygon(const b2Vec2* old_vertices, int vertexCount, const b2Color& color)
{
    // ポリゴンの塗りつぶし描画
    glColor4f(color.r, color.g, color.b, 0.5f);  // 透明度を設定
    glBegin(GL_TRIANGLE_FAN);
    for (int32 i = 0; i < vertexCount; ++i) {
        glVertex2f(old_vertices[i].x, old_vertices[i].y);
    }
    glEnd();

    // ポリゴンのエッジを描画（オプション）
    glColor4f(color.r, color.g, color.b, 1.0f);  // 不透明でエッジを描画
    glBegin(GL_LINE_LOOP);
    for (int32 i = 0; i < vertexCount; ++i) {
        glVertex2f(old_vertices[i].x, old_vertices[i].y);
    }
    glEnd();
}

void GLESDebugDraw::DrawCircle(const b2Vec2& center, float32 radius, const b2Color& color)
{
}

void GLESDebugDraw::DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color)
{
}

void GLESDebugDraw::DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color)
{
}

void GLESDebugDraw::DrawTransform(const b2Transform& xf)
{
    b2Vec2 p1 = xf.p, p2;
    const float32 k_axisScale = 0.4f;
    p2 = p1 + k_axisScale * xf.q.GetXAxis();
    DrawSegment(p1, p2, b2Color(1, 0, 0));

    p2 = p1 + k_axisScale * xf.q.GetYAxis();
    DrawSegment(p1, p2, b2Color(0, 1, 0));
}

void GLESDebugDraw::DrawPoint(const b2Vec2& p, float32 size, const b2Color& color)
{
}

void GLESDebugDraw::DrawString(int x, int y, const char* string, ...)
{
    //    NSLog(@"DrawString: unsupported: %s", string);
        //printf(string);
        /* Unsupported as yet. Could replace with bitmap font renderer at a later date */
}

void GLESDebugDraw::DrawAABB(b2AABB* aabb, const b2Color& color)
{
}
