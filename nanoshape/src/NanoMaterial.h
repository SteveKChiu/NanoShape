//*
//* https://github.com/SteveKChiu/nanoshape
//*
//* Copyright 2020, Steve K. Chiu <steve.k.chiu@gmail.com>
//*
//* This Source Code Form is subject to the terms of the Mozilla Public
//* License, v. 2.0. If a copy of the MPL was not distributed with this
//* file, You can obtain one at https://mozilla.org/MPL/2.0/.
//*

#pragma once

#include "NanoPainter.h"

#include <QSGMaterial>

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
        TypeDashPattern,
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
        float dashOffset;
        float dashUnit;
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
    UniformBuffer m_info;
    QString m_name;
    float m_strokeWidth = 0;
    bool m_textureOwned = false;
};
