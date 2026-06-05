#include <QtTest>
#include "proxyserver.h"

class ProxyServerTest : public QObject
{
    Q_OBJECT

private slots:

    // =========================================================
    // TESTS FOR startListening()
    // =========================================================

    void test_startListening_valid()
    {
        ProxyServer server;

        bool result = server.startListening(QHostAddress::LocalHost, 5555);

        QVERIFY(result);
        QVERIFY(server.isListening());
    }

    void test_startListening_invalidPort()
    {
        ProxyServer server;

        // Port already occupied simulation
        QTcpServer occupied;
        occupied.listen(QHostAddress::LocalHost, 5556);

        bool result = server.startListening(QHostAddress::LocalHost, 5556);

        QVERIFY(!result);
    }

    void test_startListening_anyAddress()
    {
        ProxyServer server;

        bool result = server.startListening(QHostAddress::Any, 5557);

        QVERIFY(result);
        QCOMPARE(server.serverPort(), quint16(5557));
    }

    // =========================================================
    // TESTS FOR setIntercept()
    // =========================================================

    void test_setIntercept_enable()
    {
        ProxyServer server;

        server.setIntercept(true);

        // Проверяем что сервер не падает
        QVERIFY(true);
    }

    void test_setIntercept_disable()
    {
        ProxyServer server;

        server.setIntercept(false);

        QVERIFY(true);
    }

    void test_setIntercept_toggle()
    {
        ProxyServer server;

        server.setIntercept(true);
        server.setIntercept(false);
        server.setIntercept(true);

        QVERIFY(true);
    }

    // =========================================================
    // TESTS FOR clearQueue()
    // =========================================================

    void test_clearQueue_empty()
    {
        ProxyServer server;

        server.clearQueue();

        QVERIFY(true);
    }

    void test_clearQueue_afterIntercept()
    {
        ProxyServer server;

        server.setIntercept(true);

        server.clearQueue();

        QVERIFY(true);
    }

    void test_clearQueue_multipleCalls()
    {
        ProxyServer server;

        server.clearQueue();
        server.clearQueue();
        server.clearQueue();

        QVERIFY(true);
    }
};

QTEST_MAIN(ProxyServerTest)
#include "tst_proxyserver.moc"
