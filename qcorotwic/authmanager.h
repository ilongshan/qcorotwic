// SPDX-FileCopyrightText: 2021 Daniel Vrátil <dvratil@kde.org>
//
// SPDX-License-Identifier: MIT

#include <QObject>
#include <QString>

#include <qcoro/task.h>


namespace QCoroTwic {

using AccessToken = QString;

class AuthManager : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;

public Q_SLOTS:
    QCoro::Task<AccessToken> authenticate();

};

} // namespace QCoroTwic
