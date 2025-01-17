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

#include <QGuiApplication>
#include <QQmlApplicationEngine>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include "NanoShape.h"
#include "NanoShapeExample.h"
#endif

//---------------------------------------------------------------------------

static void registerQmlTypes()
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    // register nano shape types
    qmlRegisterType<NanoShape>("NanoShapeExample", 1, 0, "NanoShape");
    qmlRegisterUncreatableType<NanoShapePainter>("NanoShapeExample", 1, 0, "NanoShapePainter", "inner class of NanoShape");

    // register c++ example
    qmlRegisterType<NanoShapeExample>("NanoShapeExample", 1, 0, "NanoShapeExample");
#endif
}

int Q_DECL_EXPORT main(int argc, char* argv[])
{
    qputenv("QSG_INFO", "1");
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;
    registerQmlTypes();

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    engine.load(QUrl("qrc:///main.qml"));
#else
    engine.addImportPath(QGuiApplication::applicationDirPath() + "/qml");
    engine.addImportPath("qrc:///qml");
    engine.load("qrc:///qml/NanoShapeExample/main.qml");
#endif

    return app.exec();
}
