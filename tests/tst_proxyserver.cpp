// SPDX-License-Identifier: MIT
// Encoding: UTF-8
#include <QtTest>
#include <QTcpSocket>
#include <QSignalSpy>
#include "proxyserver.h"

/**
 * Unit + integration tests for ProxyServer.
 *
 * Structure:
 *   1. startListening  — return value, state, port accessor, error signal
 *   2. setIntercept    — state accessor, emitted signals, idempotency
 *   3. clearQueue      — signals when empty; multiple calls
 *   4. forward/drop    — no-op behaviour when no request is active
 *   5. queueDepth      — initial value
 *   6. Integration     — real TCP sockets; intercept, queueing, drop, clear
 *
 * Port strategy: pass port 0 to let the OS pick a free port, then read it
 * back with server.serverPort().  This avoids hard-coded port conflicts
 * between tests and CI environments.
 */
class ProxyServerTest : public QObject
{
    Q_OBJECT

    // ── Helpers ─────────────────────────────────────────────────────────────

    /** Minimal HTTP/1.1 GET that ProxyConnection's parser will accept. */
    static QByteArray makeGet(const QByteArray &host = "example.com")
    {
        return "GET http://" + host + "/ HTTP/1.1\r\n"
               "Host: " + host + "\r\n\r\n";
    }

    /**
     * Opens a TCP connection to LocalHost:port, writes data, and returns the
     * live socket.  The caller owns the socket and must keep it alive for the
     * duration of the test (dropping it closes the connection prematurely).
     * Returns nullptr if the connection cannot be established within 1 s.
     */
    QTcpSocket *connectAndSend(quint16 port, const QByteArray &data)
    {
        auto *sock = new QTcpSocket(this);
        sock->connectToHost(QHostAddress::LocalHost, port);
        if (!sock->waitForConnected(1000)) {
            delete sock;
            return nullptr;
        }
        sock->write(data);
        sock->flush();
        return sock;
    }

private slots:

    // =========================================================
    // 1. startListening
    // =========================================================

    void test_startListening_success_returnsTrue()
    {
        ProxyServer server;
        bool ok = server.startListening(QHostAddress::LocalHost, 0);
        QVERIFY(ok);
    }

    void test_startListening_success_serverIsListening()
    {
        ProxyServer server;
        server.startListening(QHostAddress::LocalHost, 0);
        QVERIFY(server.isListening());
    }

    void test_startListening_success_assignsRealPort()
    {
        ProxyServer server;
        server.startListening(QHostAddress::LocalHost, 0);
        // OS must have assigned a non-zero port
        QVERIFY(server.serverPort() > 0);
    }

    void test_startListening_portInUse_returnsFalse()
    {
        QTcpServer occupied;
        occupied.listen(QHostAddress::LocalHost, 0);
        quint16 takenPort = occupied.serverPort();

        ProxyServer server;
        bool ok = server.startListening(QHostAddress::LocalHost, takenPort);
        QVERIFY(!ok);
    }

    void test_startListening_portInUse_serverNotListening()
    {
        QTcpServer occupied;
        occupied.listen(QHostAddress::LocalHost, 0);

        ProxyServer server;
        server.startListening(QHostAddress::LocalHost, occupied.serverPort());
        QVERIFY(!server.isListening());
    }

    void test_startListening_portInUse_emitsServerError()
    {
        QTcpServer occupied;
        occupied.listen(QHostAddress::LocalHost, 0);

        ProxyServer server;
        QSignalSpy spy(&server, &ProxyServer::serverError);
        server.startListening(QHostAddress::LocalHost, occupied.serverPort());

        QCOMPARE(spy.count(), 1);
        // Error message must not be empty
        QVERIFY(!spy.at(0).at(0).toString().isEmpty());
    }

    void test_startListening_success_noServerErrorEmitted()
    {
        ProxyServer server;
        QSignalSpy spy(&server, &ProxyServer::serverError);
        server.startListening(QHostAddress::LocalHost, 0);
        QCOMPARE(spy.count(), 0);
    }

    // =========================================================
    // 2. setIntercept
    // =========================================================

    void test_setIntercept_enable_isInterceptingTrue()
    {
        ProxyServer server;
        server.setIntercept(true);
        QVERIFY(server.isIntercepting());
    }

    void test_setIntercept_disable_isInterceptingFalse()
    {
        ProxyServer server;
        server.setIntercept(true);
        server.setIntercept(false);
        QVERIFY(!server.isIntercepting());
    }

    /** Enabling intercept must NOT emit queue/next signals — nothing changed. */
    void test_setIntercept_enable_noSignalsEmitted()
    {
        ProxyServer server;
        QSignalSpy spyQueue(&server, &ProxyServer::queueChanged);
        QSignalSpy spyNext (&server, &ProxyServer::nextIntercepted);

        server.setIntercept(true);

        QCOMPARE(spyQueue.count(), 0);
        QCOMPARE(spyNext.count(),  0);
    }

    /** Disabling intercept must emit queueChanged(0) exactly once. */
    void test_setIntercept_disable_emitsQueueChanged0()
    {
        ProxyServer server;
        server.setIntercept(true);
        QSignalSpy spy(&server, &ProxyServer::queueChanged);

        server.setIntercept(false);

        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.at(0).at(0).toInt(), 0);
    }

    /** Disabling intercept must emit nextIntercepted(-1, {}) to clear the UI. */
    void test_setIntercept_disable_emitsNextIntercepted_minus1_withEmptyData()
    {
        ProxyServer server;
        server.setIntercept(true);
        QSignalSpy spy(&server, &ProxyServer::nextIntercepted);

        server.setIntercept(false);

        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.at(0).at(0).toInt(), -1);
        QVERIFY(spy.at(0).at(1).toByteArray().isEmpty());
    }

    /** Every setIntercept(false) call must emit its signals, even consecutive ones. */
    void test_setIntercept_repeatedDisable_emitsEachTime()
    {
        ProxyServer server;
        QSignalSpy spy(&server, &ProxyServer::queueChanged);

        server.setIntercept(false); // call 1
        server.setIntercept(false); // call 2

        QCOMPARE(spy.count(), 2);
        QCOMPARE(spy.at(0).at(0).toInt(), 0);
        QCOMPARE(spy.at(1).at(0).toInt(), 0);
    }

    // =========================================================
    // 3. clearQueue (no active connections)
    // =========================================================

    void test_clearQueue_empty_emitsQueueChanged0()
    {
        ProxyServer server;
        QSignalSpy spy(&server, &ProxyServer::queueChanged);

        server.clearQueue();

        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.at(0).at(0).toInt(), 0);
    }

    void test_clearQueue_empty_emitsNextIntercepted_minus1_withEmptyData()
    {
        ProxyServer server;
        QSignalSpy spy(&server, &ProxyServer::nextIntercepted);

        server.clearQueue();

        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.at(0).at(0).toInt(), -1);
        QVERIFY(spy.at(0).at(1).toByteArray().isEmpty());
    }

    /** Each clearQueue() call must emit its two signals independently. */
    void test_clearQueue_multipleCalls_eachEmitsBothSignals()
    {
        ProxyServer server;
        QSignalSpy spyQueue(&server, &ProxyServer::queueChanged);
        QSignalSpy spyNext (&server, &ProxyServer::nextIntercepted);

        server.clearQueue();
        server.clearQueue();
        server.clearQueue();

        QCOMPARE(spyQueue.count(), 3);
        QCOMPARE(spyNext.count(),  3);
        for (int i = 0; i < 3; ++i)
            QCOMPARE(spyQueue.at(i).at(0).toInt(), 0);
    }

    // =========================================================
    // 4. forwardPendingRequest / dropPendingRequest — no active request
    // =========================================================

    /** forwardPendingRequest with m_activeId == -1 must be a silent no-op. */
    void test_forwardPending_noActive_noSignalsEmitted()
    {
        ProxyServer server;
        QSignalSpy spyQueue(&server, &ProxyServer::queueChanged);
        QSignalSpy spyNext (&server, &ProxyServer::nextIntercepted);

        server.forwardPendingRequest("GET / HTTP/1.1\r\nHost: x\r\n\r\n");

        QCOMPARE(spyQueue.count(), 0);
        QCOMPARE(spyNext.count(),  0);
    }

    /** dropPendingRequest with m_activeId == -1 must be a silent no-op. */
    void test_dropPending_noActive_noSignalsEmitted()
    {
        ProxyServer server;
        QSignalSpy spyQueue(&server, &ProxyServer::queueChanged);
        QSignalSpy spyNext (&server, &ProxyServer::nextIntercepted);

        server.dropPendingRequest();

        QCOMPARE(spyQueue.count(), 0);
        QCOMPARE(spyNext.count(),  0);
    }

    // =========================================================
    // 5. queueDepth accessor
    // =========================================================

    void test_queueDepth_initial_isZero()
    {
        ProxyServer server;
        QCOMPARE(server.queueDepth(), 0);
    }

    // =========================================================
    // 6. Integration — real TCP connections
    // =========================================================

    /**
     * A single intercepted request must trigger requestIntercepted with a
     * positive connId and non-empty raw bytes.
     */
    void test_intercept_singleRequest_emitsRequestIntercepted()
    {
        ProxyServer server;
        server.startListening(QHostAddress::LocalHost, 0);
        server.setIntercept(true);

        QSignalSpy spy(&server, &ProxyServer::requestIntercepted);

        QScopedPointer<QTcpSocket> client(
            connectAndSend(server.serverPort(), makeGet()));
        QVERIFY2(client, "Failed to connect to proxy");

        QVERIFY(spy.wait(2000));
        QCOMPARE(spy.count(), 1);
        QVERIFY(spy.at(0).at(0).toInt() > 0);             // valid connId
        QVERIFY(!spy.at(0).at(1).toByteArray().isEmpty()); // raw request present
    }

    /**
     * While one request is being reviewed, a second connection must be queued
     * and signal nextIntercepted with connId == -2.
     */
    void test_intercept_twoRequests_secondGoesInQueue()
    {
        ProxyServer server;
        server.startListening(QHostAddress::LocalHost, 0);
        server.setIntercept(true);

        QSignalSpy spyIntercepted(&server, &ProxyServer::requestIntercepted);
        QSignalSpy spyNext       (&server, &ProxyServer::nextIntercepted);

        // First request → becomes active
        QScopedPointer<QTcpSocket> client1(
            connectAndSend(server.serverPort(), makeGet("first.com")));
        QVERIFY2(client1, "Failed to connect client1");
        QVERIFY(spyIntercepted.wait(2000));
        QCOMPARE(spyIntercepted.count(), 1);

        // Second request → must be enqueued
        QScopedPointer<QTcpSocket> client2(
            connectAndSend(server.serverPort(), makeGet("second.com")));
        QVERIFY2(client2, "Failed to connect client2");
        QVERIFY(spyNext.wait(2000));

        QCOMPARE(spyNext.last().at(0).toInt(), -2); // -2 = queued, not shown
        QCOMPARE(server.queueDepth(), 1);
    }

    /**
     * After dropping the only intercepted request (empty queue) the server must
     * emit nextIntercepted(-1, {}) to clear the UI panel.
     */
    void test_dropPending_withActive_emptyQueue_emitsNextIntercepted_minus1()
    {
        ProxyServer server;
        server.startListening(QHostAddress::LocalHost, 0);
        server.setIntercept(true);

        QSignalSpy spyIntercepted(&server, &ProxyServer::requestIntercepted);
        QScopedPointer<QTcpSocket> client(
            connectAndSend(server.serverPort(), makeGet()));
        QVERIFY2(client, "Failed to connect");
        QVERIFY(spyIntercepted.wait(2000));

        QSignalSpy spyNext(&server, &ProxyServer::nextIntercepted);
        server.dropPendingRequest();

        // promoteNext() is called synchronously inside dropPendingRequest()
        QCOMPARE(spyNext.count(), 1);
        QCOMPARE(spyNext.at(0).at(0).toInt(), -1);
        QVERIFY(spyNext.at(0).at(1).toByteArray().isEmpty());
    }

    /**
     * clearQueue with an active request must emit queueChanged(0) so the UI
     * counter resets to zero.
     */
    void test_clearQueue_withActiveRequest_emitsQueueChanged0()
    {
        ProxyServer server;
        server.startListening(QHostAddress::LocalHost, 0);
        server.setIntercept(true);

        QSignalSpy spyIntercepted(&server, &ProxyServer::requestIntercepted);
        QScopedPointer<QTcpSocket> client(
            connectAndSend(server.serverPort(), makeGet()));
        QVERIFY2(client, "Failed to connect");
        QVERIFY(spyIntercepted.wait(2000));

        QSignalSpy spyQueue(&server, &ProxyServer::queueChanged);
        server.clearQueue();

        QCOMPARE(spyQueue.count(), 1);
        QCOMPARE(spyQueue.at(0).at(0).toInt(), 0);
    }
};

QTEST_MAIN(ProxyServerTest)
#include "tst_proxyserver.moc"