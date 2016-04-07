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

#include <QtTest>
#include <QQmlEngine>

#include "application.h"
#include "yamlapplicationscanner.h"
#include "abstractruntime.h"
#include "runtimefactory.h"

class tst_Runtime : public QObject
{
    Q_OBJECT

public:
    tst_Runtime();

private slots:
    void factory();
};

class TestRuntimeManager;

class TestRuntime : public AbstractRuntime
{
    Q_OBJECT

public:
    explicit TestRuntime(AbstractContainer *container, const Application *app, AbstractRuntimeManager *manager)
        : AbstractRuntime(container, app, manager)
        , m_running(false)
    { }

    State state() const
    {
        return m_running ? Active : Inactive;
    }

    qint64 applicationProcessId() const
    {
        return m_running ? 1 : 0;
    }

public slots:
    bool start()
    {
        m_running = true;
        return true;
    }

    void stop(bool forceKill)
    {
        Q_UNUSED(forceKill);
        m_running = false;
    }

private:
    bool m_running;

};

class TestRuntimeManager : public AbstractRuntimeManager
{
    Q_OBJECT

public:
    TestRuntimeManager(const QString &id, QObject *parent)
        : AbstractRuntimeManager(id, parent)
    { }

    static QString defaultIdentifier() { return qSL("foo"); }

    bool inProcess() const
    {
        return !AbstractRuntimeManager::inProcess();
    }

    TestRuntime *create(AbstractContainer *container, const Application *app)
    {
        return new TestRuntime(container, app, this);
    }
};


tst_Runtime::tst_Runtime()
{ }

void tst_Runtime::factory()
{
    RuntimeFactory *rf = RuntimeFactory::instance();

    QVERIFY(rf);
    QVERIFY(rf == RuntimeFactory::instance());
    QVERIFY(rf->runtimeIds().isEmpty());

    QVERIFY(rf->registerRuntime<TestRuntimeManager>());
    QVERIFY(rf->runtimeIds() == QStringList() << qSL("foo"));

    QVERIFY(!rf->create(0, 0));

    QByteArray yaml =
            "formatVersion: 1\n"
            "formatType: am-application\n"
            "---\n"
            "id: com.pelagicore.test\n"
            "name: { en_US: 'Test' }\n"
            "icon: icon.png\n"
            "code: test.foo\n"
            "runtime: foo\n";

    QTemporaryFile temp;
    QVERIFY(temp.open());
    QCOMPARE(temp.write(yaml), yaml.size());
    temp.close();

    QString error;
    Application *a = nullptr;
    try {
        a = YamlApplicationScanner().scan(temp.fileName());
    } catch (const Exception &e) {
        QVERIFY2(false, qPrintable(e.errorString()));
    }
    QVERIFY(a);

    AbstractRuntime *r = rf->create(0, a);
    QVERIFY(r);
    QVERIFY(r->application() == a);
    QVERIFY(r->manager()->inProcess());
    QVERIFY(r->state() == AbstractRuntime::Inactive);
    QVERIFY(r->applicationProcessId() == 0);
    {
        QScopedPointer<QQmlEngine> engine(new QQmlEngine());
        QVERIFY(!r->inProcessQmlEngine());
        r->setInProcessQmlEngine(engine.data());
        QVERIFY(r->inProcessQmlEngine() == engine.data());
        r->setInProcessQmlEngine(0);
    }
    QVERIFY(r->start());
    QVERIFY(r->state() == AbstractRuntime::Active);
    QVERIFY(r->applicationProcessId() == 1);
    r->stop();
    QVERIFY(r->state() == AbstractRuntime::Inactive);
    QVERIFY(!r->securityToken().isEmpty());

    delete r;
    delete rf;
}

QTEST_MAIN(tst_Runtime)

#include "tst_runtime.moc"
