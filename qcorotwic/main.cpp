// SPDX-FileCopyrightText: 2021 Daniel Vrátil <dvratil@kde.org>
//
// SPDX-License-Identifier: MIT

#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "utils/literals.h"
#include "authmanager.h"

#include <QTimer>

using namespace QCoroTwic::Literals;

int main(int argc, char **argv) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;
    engine.load(u"qrc:/qml/main.qml"_qs);

    return app.exec();
}
