#include <QProcess>
#include <QtTest>

class AppTests final : public QObject
{
    Q_OBJECT

private slots:
    void versionOutput();
    void persianShellLoads();

private:
    static QByteArray runApp(const QStringList &arguments, int *exitCode);
};

QByteArray AppTests::runApp(const QStringList &arguments, int *exitCode)
{
    const QString appPath = qEnvironmentVariable("PARSINAMA_TEST_APP");
    if (appPath.isEmpty()) {
        qFatal("PARSINAMA_TEST_APP was not set");
    }

    QProcess process;
    process.setProgram(appPath);
    process.setArguments(arguments);
    process.start();
    if (!process.waitForStarted(5000) || !process.waitForFinished(5000)) {
        qFatal("Application did not finish within the smoke-test timeout");
    }

    *exitCode = process.exitCode();
    return process.readAllStandardOutput() + process.readAllStandardError();
}

void AppTests::versionOutput()
{
    int exitCode = -1;
    const QByteArray output = runApp({QStringLiteral("--version")}, &exitCode);
    QCOMPARE(exitCode, 0);
    QCOMPARE(output.trimmed(), QByteArray("LeoMoon ParsiNama ") + QByteArray(PARSINAMA_VERSION));
}

void AppTests::persianShellLoads()
{
    int exitCode = -1;
    const QByteArray output = runApp({QStringLiteral("--smoke-test")}, &exitCode);
    QCOMPARE(exitCode, 0);
    QVERIFY2(!output.contains("QQmlApplicationEngine failed"), output.constData());
}

QTEST_GUILESS_MAIN(AppTests)

#include "app_tests.moc"
