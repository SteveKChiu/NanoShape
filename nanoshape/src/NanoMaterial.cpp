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

#include "NanoMaterial.h"

#include <QSGMaterialShader>
#include <QSGTexture>
#include <QQuickWindow>

//---------------------------------------------------------------------------

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)

class NanoMaterialShader : public QSGMaterialShader
{
public:
    NanoMaterialShader()
    {
        setFlag(UpdatesGraphicsPipelineState);
        setShaderFileName(VertexStage, QLatin1String(":/NanoShape/NanoShader.vert.qsb"));
        setShaderFileName(FragmentStage, QLatin1String(":/NanoShape/NanoShader.frag.qsb"));
    }

    virtual void initResource()
    {
        // it is fine this function is never called, this is just to make sure it is linked
        Q_INIT_RESOURCE(NanoShaders);
    }

    virtual bool updateUniformData(RenderState& state, QSGMaterial* newMaterial, QSGMaterial* oldMaterial) override
    {
        auto m = static_cast<NanoMaterial*>(newMaterial);
        auto m0 = static_cast<NanoMaterial*>(oldMaterial);
        auto& buf = *state.uniformData();
        bool changed = false;

        if (state.isMatrixDirty()) {
            const QMatrix4x4 mat = state.combinedMatrix();
            memcpy(buf.data(), mat.data(), 64);
            changed = true;
        }

        if (state.isOpacityDirty()) {
            auto p = reinterpret_cast<float*>(buf.data() + 64);
            *p = state.opacity();
            changed = true;
        }

        if (!oldMaterial || m->info() != m0->info()) {
            auto p = reinterpret_cast<NanoMaterial::UniformBuffer*>(buf.data() + 64 + 16);
            *p = m->info();
            changed = true;
        }

        return changed;
    }

    virtual void updateSampledImage(RenderState& state, int binding, QSGTexture** texture, QSGMaterial* material, QSGMaterial*) override
    {
        if (binding != 1) return;
        auto m = static_cast<NanoMaterial*>(material);
        auto t = m->texture();
        if (t) t->commitTextureOperations(state.rhi(), state.resourceUpdateBatch());
        *texture = t;
    }

    virtual bool updateGraphicsPipelineState(RenderState&, GraphicsPipelineState* ps, QSGMaterial* material, QSGMaterial*) override
    {
        auto m = static_cast<NanoMaterial*>(material);
        auto src = GraphicsPipelineState::One;
        auto dst = GraphicsPipelineState::OneMinusSrcAlpha;

        switch (m->compositeOperation()) {
        case NanoPainter::Composite::SourceOver:
            break;
        case NanoPainter::Composite::SourceIn:
            src = GraphicsPipelineState::DstAlpha;
            dst = GraphicsPipelineState::Zero;
            break;
        case NanoPainter::Composite::SourceOut:
            src = GraphicsPipelineState::OneMinusDstAlpha;
            dst = GraphicsPipelineState::Zero;
            break;
        case NanoPainter::Composite::Atop:
            src = GraphicsPipelineState::DstAlpha;
            dst = GraphicsPipelineState::OneMinusSrcAlpha;
            break;
        case NanoPainter::Composite::DestinationOver:
            src = GraphicsPipelineState::OneMinusDstAlpha;
            dst = GraphicsPipelineState::One;
            break;
        case NanoPainter::Composite::DestinationIn:
            src = GraphicsPipelineState::Zero;
            dst = GraphicsPipelineState::SrcAlpha;
            break;
        case NanoPainter::Composite::DestinationOut:
            src = GraphicsPipelineState::Zero;
            dst = GraphicsPipelineState::OneMinusSrcAlpha;
            break;
        case NanoPainter::Composite::DestinationAtop:
            src = GraphicsPipelineState::OneMinusDstAlpha;
            dst = GraphicsPipelineState::SrcAlpha;
            break;
        case NanoPainter::Composite::Lighter:
            src = GraphicsPipelineState::One;
            dst = GraphicsPipelineState::One;
            break;
        case NanoPainter::Composite::Copy:
            src = GraphicsPipelineState::One;
            dst = GraphicsPipelineState::Zero;
            break;
        case NanoPainter::Composite::Xor:
            src = GraphicsPipelineState::OneMinusDstAlpha;
            dst = GraphicsPipelineState::OneMinusSrcAlpha;
            break;
        }

        if (ps->blendEnable && ps->srcColor == src && ps->dstColor == dst) return false;
        ps->blendEnable = true;
        ps->srcColor = src;
        ps->dstColor = dst;
        return true;
    }
};

#endif

//---------------------------------------------------------------------------

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)

#include <QOpenGLContext>
#include <QOpenGLFunctions>

class NanoMaterialShader : public QSGMaterialShader
{
public:
    NanoMaterialShader()
    {
        setShaderSourceFile(QOpenGLShader::Vertex, QLatin1String(":/NanoShape/NanoShaderGLES.vert"));
        setShaderSourceFile(QOpenGLShader::Fragment, QLatin1String(":/NanoShape/NanoShaderGLES.frag"));
    }

    virtual void initResource()
    {
        // it is fine this function is never called, this is just to make sure it is linked
        Q_INIT_RESOURCE(NanoShadersGLES);
    }

    virtual void initialize() override
    {
        auto p = program();
        p->setUniformValue("tex", 0); // GL_TEXTURE0

        m_id_posMatrix = p->uniformLocation("qt_Matrix");
        m_id_opacity = p->uniformLocation("qt_Opacity");
        m_id_paintMatrix = p->uniformLocation("paintMatrix");
        m_id_innerColor = p->uniformLocation("innerColor");
        m_id_outerColor = p->uniformLocation("outerColor");
        m_id_extent = p->uniformLocation("extent");
        m_id_radius = p->uniformLocation("radius");
        m_id_feather = p->uniformLocation("feather");
        m_id_strokeMultiply = p->uniformLocation("strokeMultiply");
        m_id_strokeThreshold = p->uniformLocation("strokeThreshold");
        m_id_type = p->uniformLocation("type");
        m_id_edgeAA = p->uniformLocation("edgeAA");
    }

    virtual void updateState(const RenderState& state, QSGMaterial* newMaterial, QSGMaterial* oldMaterial) override
    {
        auto m = static_cast<NanoMaterial*>(newMaterial);
        auto m0 = static_cast<NanoMaterial*>(oldMaterial);
        auto p = program();
        auto f = state.context()->functions();

        if (state.isMatrixDirty()) {
            p->setUniformValue(m_id_posMatrix, state.combinedMatrix());
        }

        if (state.isOpacityDirty()) {
            p->setUniformValue(m_id_opacity, GLfloat(state.opacity()));
        }

        if (!m0 || m->info() != m0->info()) {
            auto& info = m->info();
            float paintMatrix[3 * 3];
            memcpy(&paintMatrix[0], &info.paintMatrix[0], 3 * sizeof(float));
            memcpy(&paintMatrix[3], &info.paintMatrix[4], 3 * sizeof(float));
            memcpy(&paintMatrix[6], &info.paintMatrix[8], 3 * sizeof(float));

            p->setUniformValueArray(m_id_paintMatrix, paintMatrix, 3, 3);
            p->setUniformValueArray(m_id_innerColor, info.innerColor, 1, 4);
            p->setUniformValueArray(m_id_outerColor, info.outerColor, 1, 4);
            p->setUniformValueArray(m_id_extent, info.extent, 1, 2);
            p->setUniformValue(m_id_radius, info.radius);
            p->setUniformValue(m_id_feather, info.feather);
            p->setUniformValue(m_id_strokeMultiply, info.strokeMultiply);
            p->setUniformValue(m_id_strokeThreshold, info.strokeThreshold);
            p->setUniformValue(m_id_type, info.type);
            p->setUniformValue(m_id_edgeAA, info.edgeAA);
        }

        if (!m0 || m->compositeOperation() != m0->compositeOperation()) {
            switch (m->compositeOperation()) {
            case NanoPainter::Composite::SourceOver:
                f->glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
                break;
            case NanoPainter::Composite::SourceIn:
                f->glBlendFunc(GL_DST_ALPHA, GL_ZERO);
                break;
            case NanoPainter::Composite::SourceOut:
                f->glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_ZERO);
                break;
            case NanoPainter::Composite::Atop:
                f->glBlendFunc(GL_DST_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                break;
            case NanoPainter::Composite::DestinationOver:
                f->glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_ONE);
                break;
            case NanoPainter::Composite::DestinationIn:
                f->glBlendFunc(GL_ZERO, GL_SRC_ALPHA);
                break;
            case NanoPainter::Composite::DestinationOut:
                f->glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
                break;
            case NanoPainter::Composite::DestinationAtop:
                f->glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_SRC_ALPHA);
                break;
            case NanoPainter::Composite::Lighter:
                f->glBlendFunc(GL_ONE, GL_ONE);
                break;
            case NanoPainter::Composite::Copy:
                f->glBlendFunc(GL_ONE, GL_ZERO);
                break;
            case NanoPainter::Composite::Xor:
                f->glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                break;
            }
        }

        if (m->texture()) {
            f->glActiveTexture(GL_TEXTURE0);
            m->texture()->bind();
        }
    }

    virtual void deactivate() override
    {
        auto* f = QOpenGLContext::currentContext()->functions();
        f->glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    }

    virtual const char* const* attributeNames() const override
    {
        static const char* _names[] = {
            "vertex",
            "tcoord",
            nullptr
        };
        return _names;
    }

private:
    int m_id_posMatrix;
    int m_id_opacity;
    int m_id_paintMatrix;
    int m_id_innerColor;
    int m_id_outerColor;
    int m_id_extent;
    int m_id_radius;
    int m_id_feather;
    int m_id_strokeMultiply;
    int m_id_strokeThreshold;
    int m_id_type;
    int m_id_edgeAA;
};

#endif

//---------------------------------------------------------------------------

NanoMaterial::UniformBuffer::UniformBuffer()
{
    memset(this, 0, sizeof(*this));
}

bool NanoMaterial::UniformBuffer::operator==(const NanoMaterial::UniformBuffer& that) const
{
    return memcmp(this, &that, sizeof(*this)) == 0;
}

NanoMaterial::NanoMaterial()
{
    setFlag(Blending);
}

NanoMaterial::~NanoMaterial()
{
    if (m_textureOwned) delete m_texture;
}

void NanoMaterial::setName(const QString& name)
{
    m_name = name;
}

void NanoMaterial::setStrokeWidth(float width)
{
    m_strokeWidth = width;
}

void NanoMaterial::setInfo(const NanoMaterial::UniformBuffer& info)
{
    m_info = info;
}

void NanoMaterial::setTexture(QSGTexture* texture, bool owned)
{
    if (m_texture == texture) {
        m_textureOwned = owned;
        return;
    }

    if (m_textureOwned) delete m_texture;
    m_texture = texture;
    m_textureOwned = owned;
    m_textureImage = {};
}

void NanoMaterial::setTextureImage(QQuickWindow* window, const QImage& image)
{
    if (m_texture && m_textureImage == image) return;

    QSGTexture* texture = nullptr;
    bool textureOwned = true;

    if (!image.isNull()) {
        texture = window->createTextureFromImage(image);
    }

    if (!texture) {
        static QHash<QQuickWindow*, QPair<QSGTexture*, QMetaObject::Connection>> dummyTextures;
        auto& [dummyTexture, dummyTextureConnection] = dummyTextures[window];

        if (!dummyTexture) {
            QImage dummyImage(4, 4, QImage::Format_RGBA8888_Premultiplied);
            dummyImage.fill(Qt::transparent);

            dummyTexture = window->createTextureFromImage(dummyImage);
            dummyTextureConnection = QQuickWindow::connect(window, &QQuickWindow::sceneGraphInvalidated, [window] {
                auto [dummyTexture, dummyTextureConnection] = dummyTextures.take(window);
                QQuickWindow::disconnect(dummyTextureConnection);
                delete dummyTexture;
            });
        }

        texture = dummyTexture;
        textureOwned = false;
    }

    setTexture(texture, textureOwned);
    m_textureImage = image;
}

void NanoMaterial::setCompositeOperation(NanoPainter::Composite op)
{
    m_composite = op;
}

QSGMaterialType* NanoMaterial::type() const
{
    static QSGMaterialType type;
    return &type;
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)

QSGMaterialShader* NanoMaterial::createShader(QSGRendererInterface::RenderMode) const
{
    return new NanoMaterialShader;
}

#else

QSGMaterialShader* NanoMaterial::createShader() const
{
    return new NanoMaterialShader;
}

#endif

int NanoMaterial::compare(const QSGMaterial* other) const
{
    if (!other) return 1;
    auto that = static_cast<const NanoMaterial*>(other);
    if (this == that) return 0;
    if (m_composite != that->m_composite) return m_composite < that->m_composite ? -1 : 1;
    if (m_texture != that->m_texture) return m_texture < that->m_texture ? -1 : 1;
    return this < that ? -1 : 1;
}
