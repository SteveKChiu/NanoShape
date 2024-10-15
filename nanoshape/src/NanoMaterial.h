//
// https://github.com/SteveKChiu/nanoshape
//
// Copyright 2024, Steve K. Chiu <steve.k.chiu@gmail.com>
//
// The MIT License (http://www.opensource.org/licenses/mit-license.php)
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//

#pragma once

#include "NanoPainter.h"

#include <QSGMaterial>

class QQuickWindow;
class QSGTexture;
struct NVGpaint;

//---------------------------------------------------------------------------

class NanoMaterial : public QSGMaterial
{
public:
    enum Type
    {
        TypeColor,
        TypeGradient,
        TypeImagePattern,
    };

    struct UniformBuffer
    {
        float paintMatrix[3 * 4];
        float innerColor[4];
        float outerColor[4];
        float extent[2];
        float radius;
        float feather;
        float strokeMultiply;
        float strokeThreshold;
        qint32 type;
        qint32 edgeAA;

        UniformBuffer();
        bool operator==(const UniformBuffer& that) const;
        bool operator!=(const UniformBuffer& that) const { return !operator==(that); }
    };

public:
    NanoMaterial();
    virtual ~NanoMaterial();

    QString name() const { return m_name; }
    void setName(const QString& name);

    float strokeWidth() const { return m_strokeWidth; }
    void setStrokeWidth(float width);

    const UniformBuffer& info() const { return m_info; }
    void setInfo(const UniformBuffer& info);

    QSGTexture* texture() const { return m_texture; }
    void setTexture(QSGTexture* texture, bool owned = true);
    void setTextureImage(QQuickWindow* window, const QImage& image);

    NanoPainter::Composite compositeOperation() const { return m_composite; }
    void setCompositeOperation(NanoPainter::Composite op);

    virtual QSGMaterialType* type() const override;
    virtual int compare(const QSGMaterial* that) const override;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    virtual QSGMaterialShader* createShader(QSGRendererInterface::RenderMode renderMode) const override;
#else
    virtual QSGMaterialShader* createShader() const override;
#endif

private:
    NanoPainter::Composite m_composite = NanoPainter::Composite::SourceOver;
    QSGTexture* m_texture = nullptr;
    QImage m_textureImage;
    UniformBuffer m_info;
    QString m_name;
    float m_strokeWidth = 0;
    bool m_textureOwned = false;
};
