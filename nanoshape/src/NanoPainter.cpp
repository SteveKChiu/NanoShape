//*
//* https://github.com/SteveKChiu/nanoshape
//*
//* Copyright 2020, Steve K. Chiu <steve.k.chiu@gmail.com>
//*
//* This Source Code Form is subject to the terms of the Mozilla Public
//* License, v. 2.0. If a copy of the MPL was not distributed with this
//* file, You can obtain one at https://mozilla.org/MPL/2.0/.
//*

#include "NanoPainter.h"
#include "NanoMaterial.h"
#include "nanovg.h"

#include <QPolygonF>
#include <QQuickItem>
#include <QQuickWindow>
#include <QSGGeometryNode>
#include <QSGTexture>

#ifndef NANOSHAPE_TRACE
#define NANOSHAPE_TRACE 0
#endif

#if NANOSHAPE_TRACE
#include <QDebug>
#endif

//---------------------------------------------------------------------------

struct NanoPainterCall
{
    struct Geometry
    {
        unsigned mode;
        std::vector<NVGvertex> data;

        Geometry(unsigned m, const NVGvertex* p, int n)
            : mode(m), data(p, p + n) {}
    };

    QString name;
    NanoPainter::Composite composite = NanoPainter::Composite::SourceOver;
    NVGpaint paint;
    QImage image;
    float fringe = 0;
    float strokeWidth = 0;
    float strokeThreshold = 0;
    std::vector<Geometry> data;
};

//---------------------------------------------------------------------------

class NanoPainterPrivate
{
public:
    static int renderCreate(void* uptr)
    {
#if NANOSHAPE_TRACE
        qDebug() << "renderCreate" << uptr;
#else
        Q_UNUSED(uptr)
#endif
        return 1;
    }

    static int renderCreateTexture(void* uptr, int type, int w, int h, int imageFlags, const unsigned char*)
    {
#if NANOSHAPE_TRACE
        qDebug() << "renderCreateTexture" << uptr
                 << "type:" << type << "w:" << w << "h:" << h << "imageFlags:" << imageFlags;
#else
        Q_UNUSED(uptr)
        Q_UNUSED(type)
        Q_UNUSED(w)
        Q_UNUSED(h)
        Q_UNUSED(imageFlags)
#endif
        return 1;
    }

    static int renderDeleteTexture(void* uptr, int image)
    {
#if NANOSHAPE_TRACE
        qDebug() << "renderDeleteTexture" << uptr
                 << "image:" << image;
#else
        Q_UNUSED(uptr)
        Q_UNUSED(image)
#endif
        return 1;
    }

    static int renderUpdateTexture(void* uptr, int image, int x, int y, int w, int h, const unsigned char*)
    {
#if NANOSHAPE_TRACE
        qDebug() << "renderUpdateTexture" << uptr
                 << "image:" << image << "x:" << x << "y:" << y << "w:" << w << "h:" << h;
#else
        Q_UNUSED(uptr)
        Q_UNUSED(image)
        Q_UNUSED(x)
        Q_UNUSED(y)
        Q_UNUSED(w)
        Q_UNUSED(h)
#endif
        return 1;
    }

    static int renderGetTextureSize(void* uptr, int image, int* w, int* h)
    {
#if NANOSHAPE_TRACE
        qDebug() << "renderGetTextureSize" << uptr
                 << "image:" << image;
#else
        Q_UNUSED(uptr)
        Q_UNUSED(image)
#endif
        *w = 512;
        *h = 512;
        return 1;
    }

    static void renderViewport(void* uptr, float width, float height, float devicePixelRatio)
    {
#if NANOSHAPE_TRACE
        qDebug() << "renderViewport" << uptr
                 << "width:" << width << "height:" << height << "devicePixelRatio:" << devicePixelRatio;
#else
        Q_UNUSED(uptr)
        Q_UNUSED(width)
        Q_UNUSED(height)
        Q_UNUSED(devicePixelRatio)
#endif
    }

    static void renderCancel(void* uptr)
    {
#if NANOSHAPE_TRACE
        qDebug() << "renderCancel" << uptr;
#else
        Q_UNUSED(uptr)
#endif
    }

    static void renderFlush(void* uptr)
    {
#if NANOSHAPE_TRACE
        qDebug() << "renderFlush" << uptr;
#else
        Q_UNUSED(uptr)
#endif
        static_cast<NanoPainterPrivate*>(uptr)->onRenderFlush();
    }

    static void renderFill(void* uptr, NVGpaint* paint, NVGcompositeOperationState, NVGscissor*, float fringe, const float*, const NVGpath* paths, int npaths)
    {
#if NANOSHAPE_TRACE
        qDebug().noquote() << "renderFill" << uptr << "fringe:" << fringe;
        dump(paint);
        dump(paths, npaths);
#endif
        static_cast<NanoPainterPrivate*>(uptr)->onRenderFill(paint, fringe, paths, npaths);
    }

    static void renderStroke(void* uptr, NVGpaint* paint, NVGcompositeOperationState, NVGscissor*, float fringe, float strokeWidth, const NVGpath* paths, int npaths)
    {
#if NANOSHAPE_TRACE
        qDebug() << "renderStroke" << uptr << "fringe:" << fringe << "strokeWidth:" << strokeWidth;
        dump(paint);
        dump(paths, npaths);
#endif
        static_cast<NanoPainterPrivate*>(uptr)->onRenderStroke(paint, fringe, strokeWidth, paths, npaths);
    }

    static void renderTriangles(void* uptr, NVGpaint* paint, NVGcompositeOperationState, NVGscissor*, const NVGvertex* verts, int nverts, float fringe)
    {
#if NANOSHAPE_TRACE
        qDebug() << "renderTriangles" << uptr << "fringe:" << fringe;
        dump(paint);
        dump("verts:", verts, nverts);
#else
        Q_UNUSED(uptr)
        Q_UNUSED(paint)
        Q_UNUSED(verts)
        Q_UNUSED(nverts)
        Q_UNUSED(fringe)
#endif
    }

    static void renderDelete(void* uptr)
    {
#if NANOSHAPE_TRACE
        qDebug() << "renderDelete" << uptr;
#else
        Q_UNUSED(uptr)
#endif
    }

#if NANOSHAPE_TRACE
    static void dump(NVGpaint* paint)
    {
        qDebug() << "paint:";
        qDebug().noquote() << "  xform:" << dump(paint->xform, 6);
        qDebug().noquote() << "  extent:" << dump(paint->extent, 2);
        qDebug() << "  radius:" << paint->radius;
        qDebug() << "  feather:" << paint->feather;
        qDebug().noquote() << "  innerColor:" << dump(paint->innerColor);
        qDebug().noquote() << "  outerColor:" << dump(paint->outerColor);
        qDebug() << "  image:" << paint->image;
        qDebug() << "  dashRun:" << paint->dashRun;
        qDebug() << "  dashOffset:" << paint->dashOffset;
        qDebug() << "  dashUnit:" << paint->dashUnit;
    }

    static QString dump(const NVGcolor& color)
    {
        QString buf;
        QDebug(&buf) << QColor::fromRgbF(color.r, color.g, color.b, color.a);
        return buf;
    }

    static QString dump(const float* p, int count)
    {
        QString buf;
        QDebug(&buf) << QVector<float>(p, p + count);
        return buf;
    }

    static void dump(const NVGpath* paths, int npaths)
    {
        qDebug() << "paths:" << npaths;
        for (int i = 0; i < npaths; ++i) {
            auto& p = paths[i];
            qDebug() << "path #" << i;
            qDebug() << "  first:" << p.first;
            qDebug() << "  count:" << p.count;
            qDebug() << "  closed:" << bool(p.closed);
            qDebug() << "  nbevel:" << p.nbevel;
            qDebug() << "  winding:" << p.winding;
            qDebug() << "  convex:" << p.convex;
            dump("  fill:", p.fill, p.nfill);
            dump("  stroke:", p.stroke, p.nstroke);
        }
    }

    static void dump(const QString& name, const NVGvertex* verts, int nverts)
    {
        qDebug().noquote() << name << nverts;
        for (int i = 0; i < nverts; ++i) {
            auto& p = verts[i];
            qDebug() << "    x:" << p.x << "y:" << p.y << "u:" << p.u << "v:" << p.v;
        }
    }
#endif

    NVGcontext* m_nvg = nullptr;
    NVGparams m_params {};

    QQuickItem* m_item;
    QSGNode* m_node;
    bool m_deferred;

    QString m_pathName;
    qreal m_miterLimit = 10;
    Qt::PenCapStyle m_capStyle = Qt::FlatCap;
    Qt::PenJoinStyle m_joinStyle = Qt::MiterJoin;
    qreal m_strokeWidth = 1;
    NanoBrush m_strokeBrush = Qt::black;
    NanoBrush m_fillBrush = Qt::white;
    NanoPainter::Composite m_composite = NanoPainter::Composite::SourceOver;

    std::vector<NanoPainterCall> m_pendingCalls;
    QSGGeometryNode* m_nextFreeNode = nullptr;
    QList<QSGMaterial*> m_freeMaterials;

    NanoPainterPrivate(QQuickItem* item, QSGNode* node, bool deferred);
    ~NanoPainterPrivate();

    void reset(QSGNode* node, bool deferred);
    void beginPath(const QString& name = {});
    void beginUpdate(QSGNode* node);
    QSGNode* endUpdate(QSGNode* node);

    void onRenderFill(NVGpaint* paint, float fringe, const NVGpath* paths, int npaths);
    void onRenderStroke(NVGpaint* paint, float fringe, float strokeWidth, const NVGpath* paths, int npaths);
    void onRenderFlush();

    NanoMaterial* addMaterial(const QString& name, NanoPainter::Composite composite, const NVGpaint& paint, const QImage& image, float width, float fringe, float strokeThreshold);
    QSGGeometryNode* addGeometry(QSGMaterial* material, bool& owned, unsigned mode, const NVGvertex* vertexData, int vertexCount);
};

//---------------------------------------------------------------------------

NanoPainterPrivate::NanoPainterPrivate(QQuickItem* item, QSGNode* node, bool deferred)
    : m_item(item)
    , m_node(node)
    , m_deferred(deferred)
{
    m_params.userPtr = this;
    m_params.edgeAntiAlias = item->antialiasing();
    m_params.renderCreate = &NanoPainterPrivate::renderCreate;
    m_params.renderCreateTexture = &NanoPainterPrivate::renderCreateTexture;
    m_params.renderDeleteTexture = &NanoPainterPrivate::renderDeleteTexture;
    m_params.renderUpdateTexture = &NanoPainterPrivate::renderUpdateTexture;
    m_params.renderGetTextureSize = &NanoPainterPrivate::renderGetTextureSize;
    m_params.renderViewport = &NanoPainterPrivate::renderViewport;
    m_params.renderCancel = &NanoPainterPrivate::renderCancel;
    m_params.renderFlush = &NanoPainterPrivate::renderFlush;
    m_params.renderFill = &NanoPainterPrivate::renderFill;
    m_params.renderStroke = &NanoPainterPrivate::renderStroke;
    m_params.renderTriangles = &NanoPainterPrivate::renderTriangles;
    m_params.renderDelete = &NanoPainterPrivate::renderDelete;

    m_nvg = nvgCreateInternal(&m_params);
    nvgBeginFrame(m_nvg, float(item->width()), float(item->height()), 1);
    beginUpdate(node);
}

NanoPainterPrivate::~NanoPainterPrivate()
{
    nvgDeleteInternal(m_nvg);
}

void NanoPainterPrivate::reset(QSGNode* node, bool deferred)
{
    m_params.edgeAntiAlias = m_item->antialiasing();
    nvgBeginFrame(m_nvg, float(m_item->width()), float(m_item->height()), 1);
    beginPath();

    m_miterLimit = 10;
    m_capStyle = Qt::FlatCap;
    m_joinStyle = Qt::MiterJoin;
    m_strokeWidth = 1;
    m_strokeBrush = Qt::black;
    m_fillBrush = Qt::white;
    m_composite = NanoPainter::Composite::SourceOver;

    m_pendingCalls.clear();
    m_deferred = deferred;

    beginUpdate(node);
}

void NanoPainterPrivate::beginPath(const QString& name)
{
    m_pathName = name;
    nvgBeginPath(m_nvg);
}

void NanoPainterPrivate::beginUpdate(QSGNode* node)
{
    m_node = node;
    m_freeMaterials.clear();
    m_nextFreeNode = nullptr;
    if (!node) return;

    auto geo = static_cast<QSGGeometryNode*>(node->firstChild());
    m_nextFreeNode = geo;

    while (geo) {
        if (geo->flags() & QSGNode::OwnsMaterial) {
            geo->setFlag(QSGNode::OwnsMaterial, false);
            m_freeMaterials += geo->material();
        }
        geo = static_cast<QSGGeometryNode*>(geo->nextSibling());
    }
}

QSGNode* NanoPainterPrivate::endUpdate(QSGNode* node)
{
    if (!m_item->window()) return node;

    if (!m_deferred && node != m_node) {
        delete node;
        node = m_node;
    }

    if (m_deferred) {
        beginUpdate(node);
    }

    nvgEndFrame(m_nvg);
    node = m_node;

    while (m_nextFreeNode) {
        auto next = static_cast<QSGGeometryNode*>(m_nextFreeNode->nextSibling());
        node->removeChildNode(m_nextFreeNode);
        m_nextFreeNode = next;
    }

    qDeleteAll(m_freeMaterials);
    reset(nullptr, true);
    return node;
}

void NanoPainterPrivate::onRenderFill(NVGpaint* paint, float fringe, const NVGpath* paths, int npaths)
{
    if (npaths <= 0) return;
    auto name = m_pathName + QLatin1String("_fill");

    if (!m_deferred) {
        auto mat = addMaterial(name, m_composite, *paint, m_fillBrush.image(), fringe, fringe, -1);
        bool owned = false;

        for (int i = 0; i < npaths; ++i) {
            auto& path = paths[i];
            if (path.nfill <= 0) continue;
            addGeometry(mat, owned, QSGGeometry::DrawTriangleFan, path.fill, path.nfill);
        }

        if (m_params.edgeAntiAlias) {
            for (int i = 0; i < npaths; ++i) {
                auto& path = paths[i];
                if (path.nstroke <= 0) continue;
                addGeometry(mat, owned, QSGGeometry::DrawTriangleStrip, path.stroke, path.nstroke);
            }
        }

        if (!owned) {
            m_freeMaterials += mat;
        }
        return;
    }

    auto& call = m_pendingCalls.emplace_back();
    call.name = name;
    call.composite = m_composite;
    call.paint = *paint;
    call.image = m_fillBrush.image();
    call.fringe = fringe;
    call.strokeWidth = fringe;
    call.strokeThreshold = -1;

    for (int i = 0; i < npaths; ++i) {
        auto& path = paths[i];
        if (path.nfill <= 0) continue;
        call.data.emplace_back(QSGGeometry::DrawTriangleFan, path.fill, path.nfill);
    }

    if (m_params.edgeAntiAlias) {
        for (int i = 0; i < npaths; ++i) {
            auto& path = paths[i];
            if (path.nstroke <= 0) continue;
            call.data.emplace_back(QSGGeometry::DrawTriangleStrip, path.stroke, path.nstroke);
        }
    }

    if (call.data.empty()) {
        m_pendingCalls.pop_back();
    }
}

void NanoPainterPrivate::onRenderStroke(NVGpaint* paint, float fringe, float strokeWidth, const NVGpath* paths, int npaths)
{
    if (npaths <= 0) return;
    auto name = m_pathName + QLatin1String("_stroke");

    if (!m_deferred) {
        auto mat = addMaterial(name, m_composite, *paint, m_strokeBrush.image(), fringe, strokeWidth, -1);
        bool owned = false;

        for (int i = 0; i < npaths; ++i) {
            auto& path = paths[i];
            if (path.nstroke <= 0) continue;
            addGeometry(mat, owned, QSGGeometry::DrawTriangleStrip, path.stroke, path.nstroke);
        }

        if (!owned) {
            m_freeMaterials += mat;
        }
        return;
    }

    auto& call = m_pendingCalls.emplace_back();
    call.name = name;
    call.composite = m_composite;
    call.paint = *paint;
    call.image = m_strokeBrush.image();
    call.fringe = fringe;
    call.strokeWidth = strokeWidth;
    call.strokeThreshold = -1;

    for (int i = 0; i < npaths; ++i) {
        auto& path = paths[i];
        if (path.nstroke <= 0) continue;
        call.data.emplace_back(QSGGeometry::DrawTriangleStrip, path.stroke, path.nstroke);
    }

    if (call.data.empty()) {
        m_pendingCalls.pop_back();
    }
}

void NanoPainterPrivate::onRenderFlush()
{
    for (auto& call : m_pendingCalls) {
        auto mat = addMaterial(call.name, call.composite, call.paint, call.image, call.fringe, call.strokeWidth, call.strokeThreshold);
        bool owned = false;

        for (auto& geo : call.data) {
            addGeometry(mat, owned, geo.mode, geo.data.data(), int(geo.data.size()));
        }
    }

    m_pendingCalls.clear();
}

static void premultiplyColor(float* rgba, const NVGcolor& c)
{
    rgba[0] = c.r * c.a;
    rgba[1] = c.g * c.a;
    rgba[2] = c.b * c.a;
    rgba[3] = c.a;
}

static void xformToMat3x4(float* m3, float* t)
{
    m3[0] = t[0];
    m3[1] = t[1];
    m3[2] = 0.0f;
    m3[3] = 0.0f;
    m3[4] = t[2];
    m3[5] = t[3];
    m3[6] = 0.0f;
    m3[7] = 0.0f;
    m3[8] = t[4];
    m3[9] = t[5];
    m3[10] = 1.0f;
    m3[11] = 0.0f;
}

static void updateMaterial(QQuickWindow* window, NanoMaterial* mat, NanoMaterial::UniformBuffer& info, const NVGpaint& paint, const QImage& image)
{
    QSGTexture* texture = nullptr;
    bool textureOwned = true;

    premultiplyColor(info.innerColor, paint.innerColor);
    premultiplyColor(info.outerColor, paint.outerColor);
    memcpy(info.extent, paint.extent, sizeof(info.extent));

    if (paint.image) {
        texture = window->createTextureFromImage(image);

        if (paint.dashRun) {
            info.type = NanoMaterial::TypeDashPattern;
            auto dashUnit = paint.dashUnit < 0 ? -paint.dashUnit * mat->strokeWidth() : paint.dashUnit;
            info.dashOffset = paint.dashOffset * dashUnit;
            info.dashUnit = float(paint.dashRun) * dashUnit;
            texture->setHorizontalWrapMode(QSGTexture::Repeat);
        } else {
            info.type = NanoMaterial::TypeImagePattern;
        }
    } else {
        info.radius = paint.radius;
        info.feather = paint.feather;

        if (memcmp(info.innerColor, info.outerColor, sizeof(info.innerColor)) == 0) {
            info.type = NanoMaterial::TypeColor;
        } else {
            info.type = NanoMaterial::TypeGradient;
        }
    }

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    // rhi keep complaining no texture, let's make it silent
    if (!texture && window->rendererInterface()->shaderType() == QSGRendererInterface::RhiShader) {
        static QSGTexture* dummyTexture;
        static QMetaObject::Connection dummyTextureConnection;

        if (!dummyTexture) {
            QImage dummy(4, 4, QImage::Format_RGBA8888_Premultiplied);
            dummy.fill(Qt::transparent);
            dummyTexture = window->createTextureFromImage(dummy);
            dummyTextureConnection = QQuickWindow::connect(window, &QQuickWindow::sceneGraphAboutToStop, [] {
                QQuickWindow::disconnect(dummyTextureConnection);
                dummyTextureConnection = {};
                delete dummyTexture;
                dummyTexture = nullptr;
            });
        }

        texture = dummyTexture;
        textureOwned = false;
    }
#endif

    float invxform[6];
    nvgTransformInverse(invxform, paint.xform);
    xformToMat3x4(info.paintMatrix, invxform);

    mat->setTexture(texture, textureOwned);
    mat->setInfo(info);
}

NanoMaterial* NanoPainterPrivate::addMaterial(const QString& name, NanoPainter::Composite composite, const NVGpaint& paint, const QImage& image, float fringe, float width, float threshold)
{
    NanoMaterial* mat;
    if (!m_freeMaterials.isEmpty()) {
        mat = static_cast<NanoMaterial*>(m_freeMaterials.takeLast());
    } else {
        mat = new NanoMaterial();
    }

    NanoMaterial::UniformBuffer info;
    info.strokeMultiply = (width * 0.5f + fringe * 0.5f) / fringe;
    info.strokeThreshold = threshold;
    info.edgeAA = m_params.edgeAntiAlias;

    mat->setName(name);
    mat->setStrokeWidth(width);
    mat->setCompositeOperation(composite);
    updateMaterial(m_item->window(), mat, info, paint, image);
    return mat;
}

QSGGeometryNode* NanoPainterPrivate::addGeometry(QSGMaterial* material, bool& owned, unsigned mode, const NVGvertex* vertexData, int vertexCount)
{
    if (!m_node) {
        m_node = new QSGNode();
    }

    QSGGeometryNode* node;
    QSGGeometry* geo;
    if (m_nextFreeNode) {
        node = m_nextFreeNode;
        node->setMaterial(material);
        m_nextFreeNode = static_cast<QSGGeometryNode*>(node->nextSibling());

        geo = node->geometry();
        geo->allocate(vertexCount);
    } else {
        geo = new QSGGeometry(QSGGeometry::defaultAttributes_TexturedPoint2D(), vertexCount);
        geo->setVertexDataPattern(QSGGeometry::StaticPattern);

        node = new QSGGeometryNode();
        node->setFlags(QSGNode::OwnsGeometry | QSGNode::OwnedByParent);
        node->setGeometry(geo);
        node->setMaterial(material);
        m_node->appendChildNode(node);
    }

    Q_ASSERT(geo->sizeOfVertex() == sizeof(NVGvertex));
    geo->markVertexDataDirty();

    if (mode == QSGGeometry::DrawTriangleFan) {
        // triangle fan not supported on some backend, e.g. direct3d
        // convert it to triangle strip
        geo->setDrawingMode(QSGGeometry::DrawTriangleStrip);
        auto dest = static_cast<NVGvertex*>(geo->vertexData());
        for (int i = 0; i < vertexCount; ++i) {
            auto pos = i / 2;
            if (i & 1) {
                pos = vertexCount - 1 - pos;
            }
            *dest++ = vertexData[pos];
        }
    } else {
        geo->setDrawingMode(mode);
        memcpy(geo->vertexData(), vertexData, size_t(vertexCount * geo->sizeOfVertex()));
    }

    node->setFlag(QSGNode::OwnsMaterial, !owned);
    node->markDirty(QSGNode::DirtyGeometry | QSGNode::DirtyMaterial);
    owned = true;
    return node;
}

//---------------------------------------------------------------------------

NanoPainter::NanoPainter(QQuickItem* item)
    : d(new NanoPainterPrivate(item, nullptr, true))
{
    // do nothing
}

NanoPainter::NanoPainter(QQuickItem* item, QSGNode* oldNode)
    : d(new NanoPainterPrivate(item, oldNode, false))
{
    // do nothing
}

NanoPainter::~NanoPainter()
{
    delete d;
}

void NanoPainter::reset()
{
    d->reset(nullptr, true);
}

void NanoPainter::reset(QSGNode* oldNode)
{
    d->reset(oldNode, false);
}

QTransform NanoPainter::transform() const
{
    float m[6];
    nvgCurrentTransform(d->m_nvg, m);
    return { qreal(m[0]), qreal(m[1]), qreal(m[2]), qreal(m[3]), qreal(m[4]), qreal(m[5]) };
}

void NanoPainter::setTransform(const QTransform& t)
{
    nvgTransform(d->m_nvg, float(t.m11()), float(t.m12()), float(t.m21()), float(t.m22()), float(t.m31()), float(t.m32()));
}

void NanoPainter::postTranslate(qreal x, qreal y)
{
    nvgTranslate(d->m_nvg, float(x), float(y));
}

void NanoPainter::postRotate(qreal degree)
{
    nvgRotate(d->m_nvg, nvgDegToRad(float(degree)));
}

void NanoPainter::postShear(qreal sh, qreal sv)
{
    auto trans = transform();
    trans.shear(sh, sv);
    setTransform(trans);
}

void NanoPainter::postScale(qreal sx, qreal sy)
{
    nvgScale(d->m_nvg, float(sx), float(sy));
}

void NanoPainter::resetTransform()
{
    nvgResetTransform(d->m_nvg);
}

NanoPainter::Composite NanoPainter::compositeOperation() const
{
    return d->m_composite;
}

void NanoPainter::setCompositeOperation(NanoPainter::Composite op)
{
    d->m_composite = op;
    nvgGlobalCompositeOperation(d->m_nvg, int(op));
}

qreal NanoPainter::miterLimit() const
{
    return d->m_miterLimit;
}

void NanoPainter::setMiterLimit(qreal limit)
{
    d->m_miterLimit = limit;
    nvgMiterLimit(d->m_nvg, float(limit));
}

Qt::PenCapStyle NanoPainter::capStyle() const
{
    return d->m_capStyle;
}

void NanoPainter::setCapStyle(Qt::PenCapStyle style)
{
    if (d->m_capStyle == style) return;
    d->m_capStyle = style;

    switch (style) {
    case Qt::RoundCap: nvgLineCap(d->m_nvg, NVG_ROUND); break;
    case Qt::SquareCap: nvgLineCap(d->m_nvg, NVG_SQUARE); break;
    default: nvgLineCap(d->m_nvg, NVG_BUTT); break;
    }
}

Qt::PenJoinStyle NanoPainter::joinStyle() const
{
    return d->m_joinStyle;
}

void NanoPainter::setJoinStyle(Qt::PenJoinStyle style)
{
    if (d->m_joinStyle == style) return;
    d->m_joinStyle = style;

    switch (style) {
    case Qt::RoundJoin: nvgLineJoin(d->m_nvg, NVG_ROUND); break;
    case Qt::BevelJoin: nvgLineJoin(d->m_nvg, NVG_BEVEL); break;
    default: nvgLineJoin(d->m_nvg, NVG_MITER); break;
    }
}

qreal NanoPainter::strokeWidth() const
{
    return d->m_strokeWidth;
}

void NanoPainter::setStrokeWidth(qreal width)
{
    d->m_strokeWidth = width;
    nvgStrokeWidth(d->m_nvg, float(width));
}

const NanoBrush& NanoPainter::strokeBrush() const
{
    return d->m_strokeBrush;
}

void NanoPainter::setStrokeBrush(const NanoBrush& brush)
{
    d->m_strokeBrush = brush;
    nvgStrokePaint(d->m_nvg, brush.paint());
}

const NanoBrush& NanoPainter::fillBrush() const
{
    return d->m_fillBrush;
}

void NanoPainter::setFillBrush(const NanoBrush& brush)
{
    d->m_fillBrush = brush;
    nvgFillPaint(d->m_nvg, brush.paint());
}

void NanoPainter::beginPath(const QString& name)
{
    d->beginPath(name);
}

void NanoPainter::moveTo(float x, float y)
{
    nvgMoveTo(d->m_nvg, x, y);
}

void NanoPainter::moveTo(const QPointF& point)
{
    nvgMoveTo(d->m_nvg, float(point.x()), float(point.y()));
}

void NanoPainter::lineTo(float x, float y)
{
    nvgLineTo(d->m_nvg, x, y);
}

void NanoPainter::lineTo(const QPointF& point)
{
    nvgLineTo(d->m_nvg, float(point.x()), float(point.y()));
}

void NanoPainter::bezierTo(float c1x, float c1y, float c2x, float c2y, float x, float y)
{
    nvgBezierTo(d->m_nvg, c1x, c1y, c2x, c2y, x, y);
}

void NanoPainter::bezierTo(const QPointF& c1, const QPointF& c2, const QPointF& e)
{
    nvgBezierTo(d->m_nvg, float(c1.x()), float(c1.y()), float(c2.x()), float(c2.y()), float(e.x()), float(e.y()));
}

void NanoPainter::quadTo(float cx, float cy, float x, float y)
{
    nvgQuadTo(d->m_nvg, cx, cy, x, y);
}

void NanoPainter::quadTo(const QPointF& c, const QPointF& e)
{
    nvgQuadTo(d->m_nvg, float(c.x()), float(c.y()), float(e.x()), float(e.y()));
}

void NanoPainter::arcTo(float c1x, float c1y, float c2x, float c2y, float radius)
{
    nvgArcTo(d->m_nvg, c1x, c1y, c2x, c2y, radius);
}

void NanoPainter::arcTo(const QPointF& c1, const QPointF& c2, qreal radius)
{
    nvgArcTo(d->m_nvg, float(c1.x()), float(c1.y()), float(c2.x()), float(c2.y()), float(radius));
}

void NanoPainter::closePath()
{
    nvgClosePath(d->m_nvg);
}

void NanoPainter::addArc(float cx, float cy, float radius, float a0, float a1, bool clockwise)
{
    nvgArc(d->m_nvg, cx, cy, radius, nvgDegToRad(a0), nvgDegToRad(a1), clockwise ? NVG_CW : NVG_CCW);
}

void NanoPainter::addArc(const QPointF& c, qreal radius, qreal a0, qreal a1, bool clockwise)
{
    nvgArc(d->m_nvg, float(c.x()), float(c.y()), float(radius), nvgDegToRad(float(a0)), nvgDegToRad(float(a1)), clockwise ? NVG_CW : NVG_CCW);
}

void NanoPainter::addRect(float x, float y, float width, float height)
{
    nvgRect(d->m_nvg, x, y, width, height);
}

void NanoPainter::addRect(const QRectF& rect)
{
    nvgRect(d->m_nvg, float(rect.x()), float(rect.y()), float(rect.width()), float(rect.height()));
}

void NanoPainter::addRoundedRect(float x, float y, float width, float height, float radius)
{
    nvgRoundedRect(d->m_nvg, x, y, width, height, radius);
}

void NanoPainter::addRoundedRect(const QRectF& rect, qreal radius)
{
    nvgRoundedRect(d->m_nvg, float(rect.x()), float(rect.y()), float(rect.width()), float(rect.height()), float(radius));
}

void NanoPainter::addRoundedRect(float x, float y, float width, float height, float radiusTopLeft, float radiusTopRight, float radiusBottomLeft, float radiusBottomRight)
{
    nvgRoundedRectVarying(d->m_nvg, x, y, width, height,
            radiusTopLeft, radiusTopRight, radiusBottomRight, radiusBottomLeft);
}

void NanoPainter::addRoundedRect(const QRectF& rect, qreal radiusTopLeft, qreal radiusTopRight, qreal radiusBottomRight, qreal radiusBottomLeft)
{
    nvgRoundedRectVarying(d->m_nvg, float(rect.x()), float(rect.y()), float(rect.width()), float(rect.height()),
            float(radiusTopLeft), float(radiusTopRight), float(radiusBottomRight), float(radiusBottomLeft));
}

void NanoPainter::addEllipse(float centerX, float centerY, float radiusX, float radiusY)
{
    nvgEllipse(d->m_nvg, centerX, centerY, radiusX, radiusY);
}

void NanoPainter::addEllipse(const QRectF& rect)
{
    auto rx = rect.width() / 2;
    auto ry = rect.height() / 2;
    nvgEllipse(d->m_nvg, float(rect.x() + rx), float(rect.y() + ry), float(rx), float(ry));
}

void NanoPainter::addCircle(float centerX, float centerY, float radius)
{
    nvgCircle(d->m_nvg, centerX, centerY, radius);
}

void NanoPainter::addCircle(const QPointF& center, qreal radius)
{
    nvgCircle(d->m_nvg, float(center.x()), float(center.y()), float(radius));
}

void NanoPainter::addPath(const QPolygonF& path)
{
    if (path.count() < 2) return;

    auto n = path.size() - 1;
    moveTo(path.first());

    for (int i = 1; i < n; ++i) {
        lineTo(path[i]);
    }

    if (path.isClosed()) {
        closePath();
    } else {
        lineTo(path[n]);
    }
}

void NanoPainter::addPathList(const QList<QPolygonF>& paths)
{
    for (auto& path : paths) {
        addPath(path);
    }
}

void NanoPainter::stroke()
{
    nvgStroke(d->m_nvg);
}

void NanoPainter::fill()
{
    nvgFill(d->m_nvg);
}

QSGNode* NanoPainter::updatePaintNode(QSGNode* node)
{
    return d->endUpdate(node);
}

static bool updatePaintNodeBrush(QQuickItem* item, QSGNode* root, const QString& name, const NanoBrush& brush)
{
    if (!root || name.isEmpty() || !item || !item->window()) return false;

    NanoMaterial* mat = nullptr;
    auto node = root->firstChild();

    while (node) {
        auto m = static_cast<NanoMaterial*>(static_cast<QSGGeometryNode*>(node)->material());
        if (m->name() == name) {
            mat = m;
            break;
        }
        node = node->nextSibling();
    }

    if (!mat) return false;
    auto info = mat->info();
    updateMaterial(item->window(), mat, info, brush.paint(), brush.image());

    while (node) {
        auto m = static_cast<QSGGeometryNode*>(node)->material();
        if (m == mat) {
            node->markDirty(QSGNode::DirtyMaterial);
        }
        node = node->nextSibling();
    }
    return true;
}

bool NanoPainter::updatePaintNodeStrokeBrush(QQuickItem* item, QSGNode* node, const QString& name, const NanoBrush& brush)
{
    return updatePaintNodeBrush(item, node, name + QLatin1String("_stroke"), brush);
}

bool NanoPainter::updatePaintNodeFillBrush(QQuickItem* item, QSGNode* node, const QString& name, const NanoBrush& brush)
{
    return updatePaintNodeBrush(item, node, name + QLatin1String("_fill"), brush);
}
