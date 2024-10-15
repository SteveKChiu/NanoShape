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

#include "NanoPainter.h"
#include "NanoMaterial.h"
#include "nanovg.h"

#include <QPainterPath>
#include <QPolygonF>
#include <QQuickItem>
#include <QQuickWindow>
#include <QSGGeometryNode>
#include <QSGTexture>
#include <QtMath>

#include <private/qtriangulator_p.h>

#ifndef NANOSHAPE_TRACE
#define NANOSHAPE_TRACE 0
#endif

#if NANOSHAPE_TRACE
#include <QDebug>
#endif

#define NVG_MOVETO 0
#define NVG_LINETO 1
#define NVG_BEZIERTO 2
#define NVG_CLOSE 3
#define NVG_WINDING 4

//---------------------------------------------------------------------------

// triangle fan not supported on some backend, e.g. direct3d
// convert it to triangle strip
static NVGvertex* copyTriangleStripFromFan(NVGvertex* strip, const NVGvertex* fan, int count)
{
    for (int i = 0; i < count; ++i) {
        auto pos = i / 2;
        if (i & 1) {
            pos = count - 1 - pos;
        }
        *strip++ = fan[pos];
    }
    return strip;
}

//---------------------------------------------------------------------------

struct NanoPainterCall
{
    struct Geometry
    {
        unsigned mode;
        std::vector<NVGvertex> vertexData;
        QTriangleSet triangleSet;

        Geometry(unsigned m)
            : mode(m) { }

        Geometry(const QTriangleSet& tri)
            : mode(QSGGeometry::DrawTriangles), triangleSet(tri) { }

        void addVertexData(const NVGvertex* p, int n, bool fan = false)
        {
            if (n == 0) return;

            if (!vertexData.empty()) {
                vertexData.emplace_back(vertexData.back());
                vertexData.emplace_back(*p);
            }

            if (fan) {
                auto pos = vertexData.size();
                vertexData.resize(pos + n);
                copyTriangleStripFromFan(&vertexData[pos], p, n);
            } else {
                vertexData.insert(vertexData.end(), p, p + n);
            }
        }
    };

    QString name;
    NanoPainter::Composite composite = NanoPainter::Composite::SourceOver;
    NVGpaint paint;
    QImage image;
    float fringe = 0;
    float strokeWidth = 0;
    float strokeThreshold = 0;
    std::vector<Geometry> data;

    void addFill(const NVGpath* paths, int npaths)
    {
        NanoPainterCall::Geometry* geo = nullptr;
        for (int i = 0; i < npaths; ++i) {
            auto& path = paths[i];
            if (path.nfill <= 0) continue;
            if (!geo) geo = &data.emplace_back(QSGGeometry::DrawTriangleStrip);
            geo->addVertexData(path.fill, path.nfill, true);
        }
    }

    void addStroke(const NVGpath* paths, int npaths)
    {
        NanoPainterCall::Geometry* geo = nullptr;
        for (int i = 0; i < npaths; ++i) {
            auto& path = paths[i];
            if (path.nstroke <= 0) continue;
            if (!geo) geo = &data.emplace_back(QSGGeometry::DrawTriangleStrip);
            geo->addVertexData(path.stroke, path.nstroke);
        }
    }
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
    float m_itemPixelRatio = 1;
    bool m_deferred;

    QString m_pathName;
    QTransform m_transform;
    qreal m_miterLimit = 10;
    Qt::PenCapStyle m_capStyle = Qt::FlatCap;
    Qt::PenJoinStyle m_joinStyle = Qt::MiterJoin;
    qreal m_strokeWidth = 1;
    NanoBrush m_strokeBrush = Qt::black;
    NanoBrush m_fillBrush = Qt::white;
    NanoPainter::Composite m_composite = NanoPainter::Composite::SourceOver;

    qreal m_dashOffset = 0;
    QVector<qreal> m_dashArray;
    QVector<float> m_dashArrayBuf;
    bool m_dashArrayDirty = false;

    std::vector<NanoPainterCall> m_pendingCalls;
    QSGGeometryNode* m_nextFreeNode = nullptr;
    QList<QSGMaterial*> m_freeMaterials;

    NanoMaterial* m_updateMaterial = nullptr;
    bool m_updateMaterialTaken = false;

    NanoPainterPrivate(QQuickItem* item, QSGNode* node, float itemPixelRatio, bool deferred);
    ~NanoPainterPrivate();

    float itemPixelRatio();
    void applyTransform();
    void reset(QSGNode* node, bool deferred);
    void beginPath(const QString& name = {});
    void beginUpdate(QSGNode* node);
    QSGNode* endUpdate(QSGNode* node);

    void onRenderFill(NVGpaint* paint, float fringe, const NVGpath* paths, int npaths);
    void onRenderFillConvex(NVGpaint* paint, float fringe, const NVGpath* paths, int npaths);
    void onRenderStroke(NVGpaint* paint, float fringe, float strokeWidth, const NVGpath* paths, int npaths);
    void onRenderFlush();

    void beginUpdateVertexData(const QString& name, NanoPainter::Composite composite, const NVGpaint& paint, const QImage& image, float width, float fringe, float strokeThreshold);
    void updateVertexDataForStroke(const NVGpath* paths, int npaths);
    void updateVertexDataForFill(const NVGpath* paths, int npaths);
    void updateVertexData(const QTriangleSet& tri);
    void updateVertexData(unsigned mode, const std::vector<std::pair<const NVGvertex*, int>>& vertexData);
    void updateVertexData(unsigned mode, int vertexCount, int indexCount, std::function<void(NVGvertex*, uint*)> loader);
    void endUpdateVertexData();
};

//---------------------------------------------------------------------------

NanoPainterPrivate::NanoPainterPrivate(QQuickItem* item, QSGNode* node, float itemPixelRatio, bool deferred)
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
    m_itemPixelRatio = itemPixelRatio;
    nvgBeginFrame(m_nvg, float(item->width()), float(item->height()), this->itemPixelRatio());
    beginUpdate(node);
}

NanoPainterPrivate::~NanoPainterPrivate()
{
    nvgDeleteInternal(m_nvg);
}

float NanoPainterPrivate::itemPixelRatio()
{
    if (!qFuzzyIsNull(m_itemPixelRatio)) return m_itemPixelRatio;
    if (!m_item || !m_item->window()) return 1;
    m_itemPixelRatio = NanoPainter::itemPixelRatio(m_item);
    return m_itemPixelRatio;
}

void NanoPainterPrivate::applyTransform()
{
    auto& t = m_transform;
    nvgSetTransform(m_nvg, float(t.m11()), float(t.m12()), float(t.m21()), float(t.m22()), float(t.m31()), float(t.m32()));
}

void NanoPainterPrivate::reset(QSGNode* node, bool deferred)
{
    m_params.edgeAntiAlias = m_item->antialiasing();
    nvgBeginFrame(m_nvg, float(m_item->width()), float(m_item->height()), itemPixelRatio());
    beginPath();

    m_transform.reset();
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

    bool convex = true;
    for (int i = 0; i < npaths; ++i) {
        auto path = paths[i];
        if (path.nfill <= 0) continue;
        if (!path.convex || path.winding != NVG_CCW) convex = false;
    }

    if (convex) {
        onRenderFillConvex(paint, fringe, paths, npaths);
        return;
    }

    auto name = m_pathName + QLatin1String("_fill");
    QPainterPath path;
    QPainterPath p;
    bool hole = false;

    float* buf;
    auto n = nvgInternalCommands(m_nvg, &buf);

    for (int i = 0; i < n; ++i) {
        auto cmd = int(buf[i]);
        switch (cmd) {
        case NVG_MOVETO:
            if (!p.isEmpty()) {
                if (hole) {
                    path = path.subtracted(p);
                } else {
                    path.addPath(p);
                }
                p.clear();
            }
            hole = false;
            p.moveTo(buf[i + 1], buf[i + 2]);
            i += 2;
            break;
        case NVG_LINETO:
            p.lineTo(buf[i + 1], buf[i + 2]);
            i += 2;
            break;
        case NVG_BEZIERTO:
            p.cubicTo(buf[i + 1], buf[i + 2], buf[i + 3], buf[i + 4], buf[i + 5], buf[i + 6]);
            i += 6;
            break;
        case NVG_CLOSE:
            p.closeSubpath();
            break;
        case NVG_WINDING:
            hole = int(buf[i + 1]) == NVG_HOLE;
            i++;
            break;
        }
    }

    if (!p.isEmpty()) {
        if (hole) {
            path = path.subtracted(p);
        } else {
            path.addPath(p);
        }
    }
    auto tri = qTriangulate(path);

    if (!m_deferred) {
        beginUpdateVertexData(name, m_composite, *paint, m_fillBrush.image(), fringe, fringe, -1);
        updateVertexData(tri);
        if (m_params.edgeAntiAlias) updateVertexDataForStroke(paths, npaths);
        endUpdateVertexData();
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
    call.data.emplace_back(tri);
    if (m_params.edgeAntiAlias) call.addStroke(paths, npaths);
}

void NanoPainterPrivate::onRenderFillConvex(NVGpaint* paint, float fringe, const NVGpath* paths, int npaths)
{
    if (npaths <= 0) return;
    auto name = m_pathName + QLatin1String("_fill");

    if (!m_deferred) {
        beginUpdateVertexData(name, m_composite, *paint, m_fillBrush.image(), fringe, fringe, -1);
        updateVertexDataForFill(paths, npaths);
        if (m_params.edgeAntiAlias) updateVertexDataForStroke(paths, npaths);
        endUpdateVertexData();
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
    call.addFill(paths, npaths);
    if (m_params.edgeAntiAlias) call.addStroke(paths, npaths);
}

void NanoPainterPrivate::onRenderStroke(NVGpaint* paint, float fringe, float strokeWidth, const NVGpath* paths, int npaths)
{
    if (npaths <= 0) return;
    auto name = m_pathName + QLatin1String("_stroke");

    if (!m_deferred) {
        beginUpdateVertexData(name, m_composite, *paint, m_strokeBrush.image(), fringe, strokeWidth, -1);
        updateVertexDataForStroke(paths, npaths);
        endUpdateVertexData();
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
    call.addStroke(paths, npaths);
}

void NanoPainterPrivate::onRenderFlush()
{
    for (auto& call : m_pendingCalls) {
        if (call.data.empty()) continue;
        beginUpdateVertexData(call.name, call.composite, call.paint, call.image, call.fringe, call.strokeWidth, call.strokeThreshold);

        for (auto& geo : call.data) {
            if (geo.mode == QSGGeometry::DrawTriangles) {
                updateVertexData(geo.triangleSet);
            } else {
                updateVertexData(geo.mode, { { geo.vertexData.data(), int(geo.vertexData.size()) } });
            }
        }

        endUpdateVertexData();
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
    premultiplyColor(info.innerColor, paint.innerColor);
    premultiplyColor(info.outerColor, paint.outerColor);
    memcpy(info.extent, paint.extent, sizeof(info.extent));

    if (paint.image) {
        info.type = NanoMaterial::TypeImagePattern;
    } else {
        info.radius = paint.radius;
        info.feather = paint.feather;

        if (memcmp(info.innerColor, info.outerColor, sizeof(info.innerColor)) == 0) {
            info.type = NanoMaterial::TypeColor;
        } else {
            info.type = NanoMaterial::TypeGradient;
        }
    }

    float invxform[6];
    nvgTransformInverse(invxform, paint.xform);
    xformToMat3x4(info.paintMatrix, invxform);

    mat->setTextureImage(window, image);
    mat->setInfo(info);
}

void NanoPainterPrivate::beginUpdateVertexData(const QString& name, NanoPainter::Composite composite, const NVGpaint& paint, const QImage& image, float fringe, float width, float threshold)
{
    auto& mat = m_updateMaterial;
    m_updateMaterialTaken = false;

    if (!m_freeMaterials.isEmpty()) {
        mat = static_cast<NanoMaterial*>(m_freeMaterials.takeFirst());
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
}

void NanoPainterPrivate::updateVertexDataForStroke(const NVGpath* paths, int npaths)
{
    std::vector<std::pair<const NVGvertex*, int>> vertexData;
    for (int i = 0; i < npaths; ++i) {
        auto& path = paths[i];
        if (path.nstroke <= 0) continue;
        vertexData.emplace_back(path.stroke, path.nstroke);
    }
    updateVertexData(QSGGeometry::DrawTriangleStrip, vertexData);
}

void NanoPainterPrivate::updateVertexDataForFill(const NVGpath* paths, int npaths)
{
    std::vector<std::pair<const NVGvertex*, int>> vertexData;
    for (int i = 0; i < npaths; ++i) {
        auto& path = paths[i];
        if (path.nfill <= 0) continue;
        vertexData.emplace_back(path.fill, path.nfill);
    }
    updateVertexData(QSGGeometry::DrawTriangleFan, vertexData);
}

void NanoPainterPrivate::updateVertexData(const QTriangleSet& tri)
{
    updateVertexData(QSGGeometry::DrawTriangles, tri.vertices.size() / 2, tri.indices.size(), [&](NVGvertex* vertex, uint* index) {
        for (int i = 0, n = tri.vertices.size() - 1; i < n; i += 2, ++vertex) {
            vertex->x = float(tri.vertices.at(i));
            vertex->y = float(tri.vertices.at(i + 1));
            vertex->u = 0.5f;
            vertex->v = 1.0f;
        }

        auto indexData = static_cast<const quint32*>(tri.indices.data());
        for (int i = 0, n = tri.indices.size(); i < n; ++i) {
            *index++ = *indexData++;
        }
    });
}

void NanoPainterPrivate::updateVertexData(unsigned mode, const std::vector<std::pair<const NVGvertex*, int>>& vertexData)
{
    if (vertexData.empty()) return;

    int vertexCount = 0;
    for (auto [buf, n] : vertexData) {
        if (vertexCount > 0) vertexCount += 2;
        vertexCount += n;
    }

    auto fan = mode == QSGGeometry::DrawTriangleFan;
    if (fan) mode = QSGGeometry::DrawTriangleStrip;

    updateVertexData(mode, vertexCount, 0, [&](NVGvertex* vertexBuf, uint*) {
        bool first = true;
        for (auto [buf, n] : vertexData) {
            if (first) {
                first = false;
            } else {
                *vertexBuf++ = *(vertexBuf - 1);
                *vertexBuf++ = *buf;
            }

            if (fan) {
                vertexBuf = copyTriangleStripFromFan(vertexBuf, buf, n);
                continue;
            }

            for (int i = 0; i < n; ++i) {
                *vertexBuf++ = *buf++;
            }
        }
    });
}

void NanoPainterPrivate::updateVertexData(unsigned mode, int vertexCount, int indexCount, std::function<void(NVGvertex*, uint*)> loader)
{
    if (!m_node) {
        m_node = new QSGNode();
    }

    QSGGeometryNode* node;
    QSGGeometry* geo;
    if (m_nextFreeNode) {
        node = m_nextFreeNode;
        node->setMaterial(m_updateMaterial);
        m_nextFreeNode = static_cast<QSGGeometryNode*>(node->nextSibling());

        geo = node->geometry();
        geo->allocate(vertexCount, indexCount);
    } else {
        geo = new QSGGeometry(QSGGeometry::defaultAttributes_TexturedPoint2D(), vertexCount, indexCount, QSGGeometry::UnsignedIntType);
        geo->setVertexDataPattern(QSGGeometry::StaticPattern);
        geo->setIndexDataPattern(QSGGeometry::StaticPattern);

        node = new QSGGeometryNode();
        node->setFlags(QSGNode::OwnsGeometry | QSGNode::OwnedByParent);
        node->setGeometry(geo);
        node->setMaterial(m_updateMaterial);
        m_node->appendChildNode(node);
    }

    Q_ASSERT(geo->sizeOfVertex() == sizeof(NVGvertex));
    geo->setDrawingMode(mode);
    geo->markVertexDataDirty();

    auto vertexBuf = static_cast<NVGvertex*>(geo->vertexData());
    uint* indexBuf = nullptr;

    if (indexCount > 0) {
        geo->markIndexDataDirty();
        indexBuf = geo->indexDataAsUInt();
    }

    loader(vertexBuf, indexBuf);

    node->setFlag(QSGNode::OwnsMaterial, !m_updateMaterialTaken);
    node->markDirty(QSGNode::DirtyGeometry | QSGNode::DirtyMaterial);
    m_updateMaterialTaken = true;
}

void NanoPainterPrivate::endUpdateVertexData()
{
    if (!m_updateMaterialTaken) {
        m_freeMaterials += m_updateMaterial;
    }
    m_updateMaterial = nullptr;
    m_updateMaterialTaken = false;
}

//---------------------------------------------------------------------------

NanoPainter::NanoPainter(QQuickItem* item, float pixelScale)
    : d(new NanoPainterPrivate(item, nullptr, pixelScale, true))
{
    // do nothing
}

NanoPainter::NanoPainter(QQuickItem* item, QSGNode* oldNode, float pixelScale)
    : d(new NanoPainterPrivate(item, oldNode, pixelScale, false))
{
    // do nothing
}

NanoPainter::~NanoPainter()
{
    delete d;
}

float NanoPainter::itemPixelRatio() const
{
    return d->itemPixelRatio();
}

void NanoPainter::reset()
{
    d->reset(nullptr, true);
}

void NanoPainter::reset(QSGNode* oldNode)
{
    d->reset(oldNode, false);
}

const QTransform& NanoPainter::transform() const
{
    return d->m_transform;
}

void NanoPainter::setTransform(const QTransform& t)
{
    d->m_transform = t;
    d->applyTransform();
}

void NanoPainter::resetTransform()
{
    d->m_transform.reset();
    nvgResetTransform(d->m_nvg);
}

void NanoPainter::preTranslate(qreal x, qreal y)
{
    d->m_transform.translate(x, y);
    d->applyTransform();
}

void NanoPainter::preScale(qreal sx, qreal sy)
{
    d->m_transform.scale(sx, sy);
    d->applyTransform();
}

void NanoPainter::preRotate(qreal degree)
{
    d->m_transform.rotate(degree);
    d->applyTransform();
}

void NanoPainter::preSkewX(qreal degree)
{
    d->m_transform.shear(std::tan(qDegreesToRadians(degree)), 0);
    d->applyTransform();
}

void NanoPainter::preSKewY(qreal degree)
{
    d->m_transform.shear(0, std::tan(qDegreesToRadians(degree)));
    d->applyTransform();
}

void NanoPainter::preShear(qreal sh, qreal sv)
{
    d->m_transform.shear(sh, sv);
    d->applyTransform();
}

void NanoPainter::postTranslate(qreal x, qreal y)
{
    d->m_transform *= QTransform::fromTranslate(x, y);
    d->applyTransform();
}

void NanoPainter::postRotate(qreal degree)
{
    d->m_transform *= QTransform().rotate(degree);
    d->applyTransform();
}

void NanoPainter::postScale(qreal sx, qreal sy)
{
    d->m_transform *= QTransform::fromScale(sx, sy);
    d->applyTransform();
}

void NanoPainter::postSkewX(qreal degree)
{
    d->m_transform *= QTransform().shear(std::tan(qDegreesToRadians(degree)), 0);
    d->applyTransform();
}

void NanoPainter::postSKewY(qreal degree)
{
    d->m_transform *= QTransform().shear(0, std::tan(qDegreesToRadians(degree)));
    d->applyTransform();
}

void NanoPainter::postShear(qreal sh, qreal sv)
{
    d->m_transform *= QTransform().shear(sh, sv);
    d->applyTransform();
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

qreal NanoPainter::dashOffset() const
{
    return d->m_dashOffset;
}

void NanoPainter::setDashOffset(qreal offset)
{
    d->m_dashOffset = offset;
}

QVector<qreal> NanoPainter::dashPattern() const
{
    return d->m_dashArray;
}

void NanoPainter::setDashPattern(const QVector<qreal>& pattern)
{
    d->m_dashArray = pattern;
    d->m_dashArrayDirty = true;
}

qreal NanoPainter::strokeWidth() const
{
    return d->m_strokeWidth;
}

void NanoPainter::setStrokeWidth(qreal width)
{
    d->m_strokeWidth = width;
    d->m_dashArrayDirty = true;
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

void NanoPainter::closeSubpath()
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

void NanoPainter::addPolygon(const QPolygonF& path)
{
    if (path.count() < 2) return;

    auto n = path.size() - 1;
    moveTo(path.first());

    for (int i = 1; i < n; ++i) {
        lineTo(path[i]);
    }

    if (path.isClosed()) {
        closeSubpath();
    } else {
        lineTo(path[n]);
    }
}

void NanoPainter::addPath(const QPainterPath& path)
{
    for (int i = 0, n = path.elementCount(); i < n; ++i) {
        auto p = path.elementAt(i);
        if (p.isMoveTo()) {
            moveTo(p);
        } else if (p.isLineTo()) {
            lineTo(p);
        } else {
            QPointF c2 = path.elementAt(++i);
            QPointF ep = path.elementAt(++i);
            bezierTo(p, c2, ep);
        }
    }
}

void NanoPainter::asInverted()
{
    nvgPathWinding(d->m_nvg, NVG_HOLE);
}

void NanoPainter::stroke()
{
    if (d->m_dashArrayDirty) {
        d->m_dashArrayDirty = false;
        d->m_dashArrayBuf.clear();
        for (auto p : std::as_const(d->m_dashArray)) {
            d->m_dashArrayBuf += float(p * d->m_strokeWidth);
        }
        nvgDashArray(d->m_nvg, d->m_dashArrayBuf.data(), d->m_dashArrayBuf.size());
    }

    nvgDashOffset(d->m_nvg, float(d->m_dashOffset * d->m_strokeWidth));
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

float NanoPainter::itemPixelRatio(QQuickItem* item)
{
    if (!item || !item->window()) return 1;
    auto pr = item->window()->effectiveDevicePixelRatio();
    while (item) {
        pr *= item->scale();
        item = item->parentItem();
    }
    return qMax(0.5, pr);
}

static bool updatePaintNodeBrush(QQuickItem* item, QSGNode* root, const QString& name, const NanoBrush& brush)
{
    if (!root || name.isEmpty() || !item || !item->window()) return false;

    auto node = root->firstChild();
    bool updated = false;

    while (node) {
        auto mat = static_cast<NanoMaterial*>(static_cast<QSGGeometryNode*>(node)->material());
        if (mat->name() == name) {
            auto info = mat->info();
            updateMaterial(item->window(), mat, info, brush.paint(), brush.image());
            updated = true;

            auto rest = node;
            while (rest) {
                auto m = static_cast<QSGGeometryNode*>(rest)->material();
                if (m == mat) {
                    rest->markDirty(QSGNode::DirtyMaterial);
                }
                rest = rest->nextSibling();
            }
            break;
        }
        node = node->nextSibling();
    }

    return updated;
}

bool NanoPainter::updatePaintNodeStrokeBrush(QQuickItem* item, QSGNode* node, const QString& name, const NanoBrush& brush)
{
    return updatePaintNodeBrush(item, node, name + QLatin1String("_stroke"), brush);
}

bool NanoPainter::updatePaintNodeFillBrush(QQuickItem* item, QSGNode* node, const QString& name, const NanoBrush& brush)
{
    return updatePaintNodeBrush(item, node, name + QLatin1String("_fill"), brush);
}
