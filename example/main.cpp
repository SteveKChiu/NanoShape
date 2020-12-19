//*
//* https://github.com/SteveKChiu/nanoshape
//*
//* Copyright 2020, Steve K. Chiu <steve.k.chiu@gmail.com>
//*
//* This Source Code Form is subject to the terms of the Mozilla Public
//* License, v. 2.0. If a copy of the MPL was not distributed with this
//* file, You can obtain one at https://mozilla.org/MPL/2.0/.
//*

#include "NanoShape.h"
#include "NanoShapeExample.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>

//---------------------------------------------------------------------------

static void registerQmlTypes()
{
    // register nano shape types
    qmlRegisterType<NanoShape>("NanoShape", 1, 0, "NanoShape");
    qmlRegisterUncreatableType<NanoShapePainter>("NanoShape", 1, 0, "NanoShapePainter", "inner class of NanoShape");

    // register c++ example
    qmlRegisterType<NanoShapeExample>("NanoShapeExample", 1, 0, "NanoShapeExample");
}

int Q_DECL_EXPORT main(int argc, char* argv[])
{
    qputenv("QSG_INFO", "1");
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;
    registerQmlTypes();
    engine.load(QUrl("qrc:///main.qml"));
    return app.exec();
}
