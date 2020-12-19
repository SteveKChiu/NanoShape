//*
//* https://github.com/SteveKChiu/nanoshape
//*
//* Copyright 2020, Steve K. Chiu <steve.k.chiu@gmail.com>
//*
//* This Source Code Form is subject to the terms of the Mozilla Public
//* License, v. 2.0. If a copy of the MPL was not distributed with this
//* file, You can obtain one at https://mozilla.org/MPL/2.0/.
//*

#include "NanoBrush.h"
#include "nanovg.h"

static constexpr int DASH_PATTERN_DIVS = 8;

//---------------------------------------------------------------------------

static inline NVGcolor toNVGcolor(const QColor& color)
{
    return nvgRGBAf(float(color.redF()), float(color.greenF()), float(color.blueF()), float(color.alphaF()));
}

NanoBrush::NanoBrush()
    : NanoBrush(QColor(Qt::transparent))
{
    // do nothing
}

NanoBrush::NanoBrush(Qt::GlobalColor color)
    : NanoBrush(QColor(color))
{
    // do nothing
}

NanoBrush::NanoBrush(const QColor& color)
    : m_paint(new NVGpaint)
{
    auto& p = *m_paint;
    memset(&p, 0, sizeof(p));
    nvgTransformIdentity(p.xform);
    p.radius = 0.0f;
    p.feather = 1.0f;
    p.innerColor = p.outerColor = toNVGcolor(color);
}

NanoBrush::NanoBrush(const NVGpaint& paint)
    : m_paint(new NVGpaint)
{
    *m_paint = paint;
}

NanoBrush::NanoBrush(const NanoBrush& that) noexcept
    : m_paint(new NVGpaint)
    , m_image(that.m_image)
{
    *m_paint = *that.m_paint;
}

NanoBrush::NanoBrush(NanoBrush&& that) noexcept
    : m_paint(that.m_paint)
    , m_image(that.m_image)
{
    that.m_paint = nullptr;
}

NanoBrush::~NanoBrush()
{
    delete m_paint;
}

NanoBrush NanoBrush::linearGradient(const QColor& startColor, const QColor& endColor, const QPointF& start, const QPointF& end)
{
    return NanoBrush(nvgLinearGradient(nullptr, float(start.x()), float(start.y()), float(end.x()), float(end.y()),
            toNVGcolor(startColor), toNVGcolor(endColor)));
}

NanoBrush NanoBrush::boxGradient(const QColor& innerColor, const QColor& outerColor, const QRectF& box, qreal cornerRadius, qreal feather)
{
    return NanoBrush(nvgBoxGradient(nullptr, float(box.x()), float(box.y()), float(box.width()), float(box.height()),
            float(cornerRadius), float(feather), toNVGcolor(innerColor), toNVGcolor(outerColor)));
}

NanoBrush NanoBrush::radialGradient(const QColor& innerColor, const QColor& outerColor, const QPointF& center, qreal innerRadius, qreal outerRadius)
{
    return NanoBrush(nvgRadialGradient(nullptr, float(center.x()), float(center.y()), float(innerRadius), float(outerRadius),
            toNVGcolor(innerColor), toNVGcolor(outerColor)));
}

NanoBrush NanoBrush::imagePattern(const QImage& image, const QRectF& rect, qreal rotation, qreal opacity)
{
    auto part = rect;
    if (part.isEmpty()) {
        part.setCoords(0, 0, image.width(), image.height());
    }

    NanoBrush brush(nvgImagePattern(nullptr, float(part.x()), float(part.y()), float(part.width()), float(part.height()),
            nvgDegToRad(float(rotation)), 1, float(opacity)));
    brush.m_image = image;
    return brush;
}

NanoBrush NanoBrush::dashPattern(const QColor& color, const QVector<qreal>& patternRef, qreal offset, qreal unitWidth)
{
    if (patternRef.isEmpty()) {
        return color;
    }

    auto pattern = patternRef;
    if (pattern.count() & 1) {
        pattern += patternRef.last();
    }

    int x = 0;
    for (auto v : qAsConst(pattern)) {
        x += qRound(v * DASH_PATTERN_DIVS);
    }

    QImage image(x, 1, QImage::Format_RGBA8888_Premultiplied);

    x = 0;
    QRgb on = qRgba(0xff, 0xff, 0xff, 0xff);
    QRgb off = 0;
    auto rgb = on;
    for (auto v : qAsConst(pattern)) {
        auto len = qRound(v * DASH_PATTERN_DIVS);
        for (int i = 0; i < len; ++i) {
            image.setPixel(x++, 0, rgb);
        }
        rgb = rgb == on ? off : on;
    }

    NVGpaint p;
    memset(&p, 0, sizeof(p));

    nvgTransformIdentity(p.xform);
    p.image = 2;
    p.innerColor = p.outerColor = toNVGcolor(color);
    p.dashRun = x;
    p.dashOffset = float(offset);
    p.dashUnit = float(unitWidth / DASH_PATTERN_DIVS);

    NanoBrush brush(p);
    brush.m_image = image;
    return brush;
}

bool NanoBrush::operator==(const NanoBrush& that) const
{
    if (this == &that) return true;
    return m_image == that.m_image && memcmp(m_paint, that.m_paint, sizeof(*m_paint)) == 0;
}

NanoBrush& NanoBrush::operator=(const NanoBrush& that) noexcept
{
    if (this == &that) return *this;
    m_image = that.m_image;
    *m_paint = *that.m_paint;
    return *this;
}

NanoBrush& NanoBrush::operator=(NanoBrush&& that) noexcept
{
    qSwap(m_image, that.m_image);
    qSwap(m_paint, that.m_paint);
    return *this;
}
