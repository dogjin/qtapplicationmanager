/****************************************************************************
**
** Copyright (C) 2016 Pelagicore AG
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Pelagicore Application Manager.
**
** $QT_BEGIN_LICENSE:GPL-QTAS$
** Commercial License Usage
** Licensees holding valid commercial Qt Automotive Suite licenses may use
** this file in accordance with the commercial license agreement provided
** with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and The Qt Company.  For
** licensing terms and conditions see https://www.qt.io/terms-conditions.
** For further information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
** SPDX-License-Identifier: GPL-3.0
**
****************************************************************************/

#pragma once

#include "applicationinterface.h"
#include "notification.h"

class QmlInProcessRuntime;

class QmlInProcessNotification : public Notification
{
public:
    QmlInProcessNotification(QObject *parent = 0, ConstructionMode mode = Declarative);

    void componentComplete() override;

protected:
    uint libnotifyShow() override;
    void libnotifyClose() override;

private:
    ConstructionMode m_mode;
    QString m_appId;

    friend class QmlInProcessApplicationInterface;
};


class QmlInProcessApplicationInterface : public ApplicationInterface
{
    Q_OBJECT

public:
    explicit QmlInProcessApplicationInterface(QmlInProcessRuntime *runtime = 0);

    QString applicationId() const override;
    QVariantMap additionalConfiguration() const override;

    Q_INVOKABLE Notification *createNotification();

private:
    QmlInProcessRuntime *m_runtime;
    //QList<QPointer<QmlInProcessNotification> > m_allNotifications;

    friend class QmlInProcessRuntime;
};


class QmlInProcessApplicationInterfaceExtension : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(bool ready READ isReady NOTIFY readyChanged)
    Q_PROPERTY(QObject *object READ object NOTIFY objectChanged)

public:
    explicit QmlInProcessApplicationInterfaceExtension(QObject *parent = nullptr);

    QString name() const;
    bool isReady() const;
    QObject *object() const;

protected:
    void classBegin() override;
    void componentComplete() override;

public slots:
    void setName(const QString &name);

signals:
    void readyChanged();
    void objectChanged();

private:
    QString m_name;
    QObject *m_object = nullptr;
    bool m_complete = false;
};
