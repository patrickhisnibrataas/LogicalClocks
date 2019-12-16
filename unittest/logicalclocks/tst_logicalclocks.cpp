#include <QtTest>
#include "logicalclocks.h"

class LogicalClocksTest : public QObject
{
    Q_OBJECT
private slots:
    void Clock_requireThat_ClockCountIsZeroWhenDefaultConstructed();
    void Clock_requireThat_ClockCountIsSetInAlternativeConstructor();
    void Clock_requireThat_ClockIsIncrementedOnEvent();
    void Clock_requireThat_ClockIsIncrementedOnSend();
    void Clock_requireThat_ClockIsIncrementedOnReceive();
    void Clock_requireThat_RemoteClockIsUsedOnReceiveWhenLocalClockIsLess();
    void Clock_requireThat_RemoteClockIsNotUsedOnReceiveWhenLocalClockIsGreater();
    void Clock_requireThat_EitherClockIsUsedOnReceiveWhenLocalClockEqualsRemoteClock();

    void VectorClock_requireThat_VectorClockCountIsAllZeroWhenDefaultConstructed();
    void VectorClock_requireThat_VectorClockCountIsSetInAlternativeConstructor();
    void VectorClock_requireThat_LocalClockIncrementsOnEvent();
    void VectorClock_requireThat_LocalClockIncrementsOnSendAndTransmittMessageWithVectorClock();
    void VectorClock_requireThat_OnlyLocalClockIncrementsOnReceivedMessageIfRemoteMessageIsEqualToLocalMessage();
    void VectorClock_requireThat_NewRemoteClocksAreAddedOnReceive();
    void VectorClock_requireThat_OnReceiveGreatestValueOfLocalClockAndRemoteClockAreAddedToLocalClock();
    void VectorClock_requireThat_ReceiveReturnsOccurredBeforeWhenLocalVectorClocksAreAllLessThanRemoteVectorClocks();
    void VectorClock_requireThat_ReceiveReturnsOccurredAfterWhenLocalVectorClocksAreAllGreaterThanRemoteVectorClocks();
    void VectorClock_requireThat_ReceiveReturnsOccurredBeforeWhenCommonLocalVectorClocksAndRemoteVectorClocksAreAllEqual();
    void VectorClock_requireThat_ReceiveReturnsOccurredConcurrentlyWhenSomeLocalClocksAreGreaterThanSomeRemoteClocksAndViceVersa();

    void VersionedData_requireThat_CanBeConstructedProperly();
    void VersionedData_requireThat_LocalDataIsNotUpdatedWithRemoteDataWhenLocalVersionIsGreaterThanRemoteVersionOnReceive();
    void VersionedData_requireThat_LocalDataIsUpdatedWithRemoteDataWhenLocalVersionIsLessThanRemoteVersionOnReceive();
    void VersionedData_requireThat_LocalDataIsUpdatedWithRemoteDataWhenLocalVersionIsEqualToRemoteVersionOnReceive();
    void VersionedData_requireThat_LocalDataIsUpdatedAccordingToConflictResolutionStrategyWhenLocalAndRemoteChangesHappenedConcurrently();
};

void LogicalClocksTest::Clock_requireThat_ClockCountIsZeroWhenDefaultConstructed()
{
    Clock clock;
    QCOMPARE(clock.count(), 0);
}

void LogicalClocksTest::Clock_requireThat_ClockCountIsSetInAlternativeConstructor()
{
    const auto expected = qint32(99);
    Clock clock(expected);
    QCOMPARE(clock.count(), expected);
}

void LogicalClocksTest::Clock_requireThat_ClockIsIncrementedOnEvent()
{
    Clock clock;
    const auto clockBefore = clock.count();
    const auto clockAfter = clock.event();
    QCOMPARE(clockAfter, clockBefore + 1);
}

void LogicalClocksTest::Clock_requireThat_ClockIsIncrementedOnSend()
{
    Clock clock;
    const auto clockBefore = clock.count();
    const auto clockAfter = clock.send();
    QCOMPARE(clockAfter, clockBefore + 1);
}

void LogicalClocksTest::Clock_requireThat_ClockIsIncrementedOnReceive()
{
    Clock clock;
    const auto clockBefore = clock.count();
    const auto clockAfter = clock.receive(clockBefore);
    QCOMPARE(clockAfter, clockBefore + 1);
}

void LogicalClocksTest::Clock_requireThat_RemoteClockIsUsedOnReceiveWhenLocalClockIsLess()
{
    Clock clock;
    const auto clockBefore = clock.count();
    const auto clockAfter = clock.receive(clockBefore + 1);
    QCOMPARE(clockAfter, clockBefore + 1);
}

void LogicalClocksTest::Clock_requireThat_RemoteClockIsNotUsedOnReceiveWhenLocalClockIsGreater()
{
    const auto clockBefore = qint32(100);
    Clock clock(clockBefore);
    const auto clockAfter = clock.receive(0);
    QCOMPARE(clockAfter, clockBefore + 1);
}

void LogicalClocksTest::Clock_requireThat_EitherClockIsUsedOnReceiveWhenLocalClockEqualsRemoteClock()
{
    const auto clockBefore = qint32(100);
    Clock clock(clockBefore);
    const auto clockAfter = clock.receive(clockBefore);
    QCOMPARE(clockAfter, clockBefore + 1);
}

void LogicalClocksTest::VectorClock_requireThat_VectorClockCountIsAllZeroWhenDefaultConstructed()
{
    VectorClock vectorClock(0);
    const auto vectorCount = vectorClock.count();
    QCOMPARE(vectorClock.localId(), 0);
    QCOMPARE(vectorCount.find(vectorClock.localId()).value(), 0);
}

void LogicalClocksTest::VectorClock_requireThat_VectorClockCountIsSetInAlternativeConstructor()
{
    QMap<qint32, qint32> init;
    init.insert(0, 10);
    init.insert(1, 99);
    init.insert(2, 13);

    VectorClock vectorClock(1, init);
    const auto vectorCount = vectorClock.count();
    QCOMPARE(vectorClock.localId(), 1);
    QCOMPARE(vectorCount.find(0).value(), 10);
    QCOMPARE(vectorCount.find(1).value(), 99);
    QCOMPARE(vectorCount.find(2).value(), 13);
}

void LogicalClocksTest::VectorClock_requireThat_LocalClockIncrementsOnEvent()
{
    VectorClock vectorClock(0);
    const auto vectorCountBefore = vectorClock.count();
    const auto vectorCountAfter = vectorClock.event();
    QCOMPARE(vectorCountAfter.find(vectorClock.localId()).value(), vectorCountBefore.find(vectorClock.localId()).value() + 1);
}

void LogicalClocksTest::VectorClock_requireThat_LocalClockIncrementsOnSendAndTransmittMessageWithVectorClock()
{
    qRegisterMetaType<QMap<qint32, qint32>>("QMap<qint32, qint32>");
    QMap<qint32, qint32> init;
    init.insert(0, 10);
    init.insert(1, 99);
    init.insert(2, 13);

    VectorClock vectorClock(1, init);
    vectorClock.count();
    const auto actual = vectorClock.send();

    QMap<qint32, qint32> expected;
    expected.insert(0, 10);
    expected.insert(1, 100);
    expected.insert(2, 13);

    QCOMPARE(actual, expected);
}

void LogicalClocksTest::VectorClock_requireThat_OnlyLocalClockIncrementsOnReceivedMessageIfRemoteMessageIsEqualToLocalMessage()
{
    QMap<qint32, qint32> localVectorClock;
    localVectorClock.insert(0, 10);
    localVectorClock.insert(1, 99);
    localVectorClock.insert(2, 13);

    QMap<qint32, qint32> remoteVectorClock;
    remoteVectorClock.insert(0, 10);
    remoteVectorClock.insert(1, 99);
    remoteVectorClock.insert(2, 13);

    VectorClock vectorClock(1, localVectorClock);
    const auto vectorCountBefore = vectorClock.count();
    vectorClock.receive(remoteVectorClock);
    const auto vectorCountAfter = vectorClock.count();

    QCOMPARE(vectorCountAfter.size(), vectorCountBefore.size());
    QCOMPARE(vectorCountAfter.find(0).value(), vectorCountBefore.find(0).value());
    QCOMPARE(vectorCountAfter.find(1).value(), vectorCountBefore.find(1).value() + 1);
    QCOMPARE(vectorCountAfter.find(2).value(), vectorCountBefore.find(2).value());
}

void LogicalClocksTest::VectorClock_requireThat_NewRemoteClocksAreAddedOnReceive()
{
    QMap<qint32, qint32> localVectorClock;
    localVectorClock.insert(0, 10);
    localVectorClock.insert(1, 99);
    localVectorClock.insert(2, 13);

    QMap<qint32, qint32> remoteVectorClock;
    remoteVectorClock.insert(0, 10);
    remoteVectorClock.insert(1, 99);
    remoteVectorClock.insert(2, 13);
    remoteVectorClock.insert(3, 36);

    VectorClock vectorClock(1, localVectorClock);
    const auto vectorCountBefore = vectorClock.count();
    vectorClock.receive(remoteVectorClock);
    const auto vectorCountAfter = vectorClock.count();

    QCOMPARE(vectorCountAfter.size(), remoteVectorClock.size());
    QCOMPARE(vectorCountAfter.find(0).value(), vectorCountBefore.find(0).value());
    QCOMPARE(vectorCountAfter.find(1).value(), vectorCountBefore.find(1).value() + 1);
    QCOMPARE(vectorCountAfter.find(2).value(), vectorCountBefore.find(2).value());
    QCOMPARE(vectorCountAfter.find(3).value(), remoteVectorClock.find(3).value());
}

void LogicalClocksTest::VectorClock_requireThat_OnReceiveGreatestValueOfLocalClockAndRemoteClockAreAddedToLocalClock()
{
    QMap<qint32, qint32> localVectorClock;
    localVectorClock.insert(0, 10);
    localVectorClock.insert(1, 99);
    localVectorClock.insert(2, 13);

    QMap<qint32, qint32> remoteVectorClock;
    remoteVectorClock.insert(0, 5);
    remoteVectorClock.insert(1, 105);
    remoteVectorClock.insert(2, 13);

    VectorClock vectorClock(1, localVectorClock);
    const auto vectorCountBefore = vectorClock.count();
    vectorClock.receive(remoteVectorClock);
    const auto vectorCountAfter = vectorClock.count();

    QCOMPARE(vectorCountAfter.find(0).value(), 10);
    QCOMPARE(vectorCountAfter.find(1).value(), 105);
    QCOMPARE(vectorCountAfter.find(2).value(), 13);
}

void LogicalClocksTest::VectorClock_requireThat_ReceiveReturnsOccurredBeforeWhenLocalVectorClocksAreAllLessThanRemoteVectorClocks()
{
    QMap<qint32, qint32> localVectorClock;
    localVectorClock.insert(0, 10);
    localVectorClock.insert(1, 99);
    localVectorClock.insert(2, 13);

    QMap<qint32, qint32> remoteVectorClock;
    remoteVectorClock.insert(0, 13);
    remoteVectorClock.insert(1, 105);
    remoteVectorClock.insert(2, 17);

    VectorClock vectorClock(1, localVectorClock);
    QCOMPARE(vectorClock.receive(remoteVectorClock), VectorClock::LocalOccured::BeforeRemote);
}

void LogicalClocksTest::VectorClock_requireThat_ReceiveReturnsOccurredAfterWhenLocalVectorClocksAreAllGreaterThanRemoteVectorClocks()
{
    QMap<qint32, qint32> localVectorClock;
    localVectorClock.insert(0, 13);
    localVectorClock.insert(1, 105);
    localVectorClock.insert(2, 17);

    QMap<qint32, qint32> remoteVectorClock;
    remoteVectorClock.insert(0, 10);
    remoteVectorClock.insert(1, 99);
    remoteVectorClock.insert(2, 13);

    VectorClock vectorClock(1, localVectorClock);
    QCOMPARE(vectorClock.receive(remoteVectorClock), VectorClock::LocalOccured::AfterRemote);
}

void LogicalClocksTest::VectorClock_requireThat_ReceiveReturnsOccurredBeforeWhenCommonLocalVectorClocksAndRemoteVectorClocksAreAllEqual()
{
    QMap<qint32, qint32> localVectorClock;
    localVectorClock.insert(0, 10);
    localVectorClock.insert(1, 99);
    localVectorClock.insert(2, 13);

    QMap<qint32, qint32> remoteVectorClock;
    remoteVectorClock.insert(0, 10);
    remoteVectorClock.insert(1, 99);
    remoteVectorClock.insert(2, 13);
    remoteVectorClock.insert(3, 100);

    VectorClock vectorClock(1, localVectorClock);
    QCOMPARE(vectorClock.receive(remoteVectorClock), VectorClock::LocalOccured::BeforeRemote);
}

void LogicalClocksTest::VectorClock_requireThat_ReceiveReturnsOccurredConcurrentlyWhenSomeLocalClocksAreGreaterThanSomeRemoteClocksAndViceVersa()
{
    QMap<qint32, qint32> localVectorClock;
    localVectorClock.insert(0, 10);
    localVectorClock.insert(1, 99);
    localVectorClock.insert(2, 13);

    QMap<qint32, qint32> remoteVectorClock;
    remoteVectorClock.insert(0, 11);
    remoteVectorClock.insert(1, 97);
    remoteVectorClock.insert(2, 13);

    VectorClock vectorClock(1, localVectorClock);
    QCOMPARE(vectorClock.receive(remoteVectorClock), VectorClock::LocalOccured::ConcurrentlyWithRemote);
}

void LogicalClocksTest::VersionedData_requireThat_CanBeConstructedProperly()
{
    const auto localData = QVariant::fromValue(QString("LocalData"));

    QMap<qint32, qint32> localVectorClock;
    localVectorClock.insert(0, 10);
    localVectorClock.insert(1, 99);
    localVectorClock.insert(2, 13);

    VersionedData versionedData(localData, 1, localVectorClock, [=](const QVariant& localData, const QVariant& remoteData) -> QVariant {
        Q_UNUSED(localData)
        Q_UNUSED(remoteData)
        return QVariant();
    });

    QCOMPARE(versionedData.data(), localData);
}

void LogicalClocksTest::VersionedData_requireThat_LocalDataIsNotUpdatedWithRemoteDataWhenLocalVersionIsGreaterThanRemoteVersionOnReceive()
{
    const auto localData = QVariant::fromValue(QString("LocalData"));

    QMap<qint32, qint32> localVectorClock;
    localVectorClock.insert(0, 10);
    localVectorClock.insert(1, 99);
    localVectorClock.insert(2, 13);

    VersionedData versionedData(localData, 1, localVectorClock, [=](const QVariant& localData, const QVariant& remoteData) -> QVariant {
        Q_UNUSED(localData)
        Q_UNUSED(remoteData)
        return QVariant();
    });

    QMap<qint32, qint32> remoteVectorClock;
    remoteVectorClock.insert(0, 9);
    remoteVectorClock.insert(1, 99);
    remoteVectorClock.insert(2, 13);

    const auto remoteData = QVariant::fromValue(QString("RemoteData"));
    versionedData.onDataReceived(remoteVectorClock, remoteData);
    QCOMPARE(versionedData.data(), localData);
}

void LogicalClocksTest::VersionedData_requireThat_LocalDataIsUpdatedWithRemoteDataWhenLocalVersionIsLessThanRemoteVersionOnReceive()
{
    const auto localData = QVariant::fromValue(QString("LocalData"));

    QMap<qint32, qint32> localVectorClock;
    localVectorClock.insert(0, 10);
    localVectorClock.insert(1, 54);
    localVectorClock.insert(2, 10);

    VersionedData versionedData(localData, 1, localVectorClock, [=](const QVariant& localData, const QVariant& remoteData) -> QVariant {
        Q_UNUSED(localData)
        Q_UNUSED(remoteData)
        return QVariant();
    });

    QMap<qint32, qint32> remoteVectorClock;
    remoteVectorClock.insert(0, 10);
    remoteVectorClock.insert(1, 99);
    remoteVectorClock.insert(2, 13);

    const auto remoteData = QVariant::fromValue(QString("RemoteData"));
    versionedData.onDataReceived(remoteVectorClock, remoteData);
    QCOMPARE(versionedData.data(), remoteData);
}

void LogicalClocksTest::VersionedData_requireThat_LocalDataIsUpdatedWithRemoteDataWhenLocalVersionIsEqualToRemoteVersionOnReceive()
{
    const auto localData = QVariant::fromValue(QString("LocalData"));

    QMap<qint32, qint32> localVectorClock;
    localVectorClock.insert(0, 10);
    localVectorClock.insert(1, 99);
    localVectorClock.insert(2, 13);

    VersionedData versionedData(localData, 1, localVectorClock, [=](const QVariant& localData, const QVariant& remoteData) -> QVariant {
        Q_UNUSED(localData)
        Q_UNUSED(remoteData)
        return QVariant();
    });

    QMap<qint32, qint32> remoteVectorClock;
    remoteVectorClock.insert(0, 10);
    remoteVectorClock.insert(1, 99);
    remoteVectorClock.insert(2, 13);

    const auto remoteData = QVariant::fromValue(QString("RemoteData"));
    versionedData.onDataReceived(remoteVectorClock, remoteData);
    QCOMPARE(versionedData.data(), remoteData);
}

void LogicalClocksTest::VersionedData_requireThat_LocalDataIsUpdatedAccordingToConflictResolutionStrategyWhenLocalAndRemoteChangesHappenedConcurrently()
{
    const auto localData = QVariant::fromValue(QString("LocalData"));

    QMap<qint32, qint32> localVectorClock;
    localVectorClock.insert(0, 15);
    localVectorClock.insert(1, 99);
    localVectorClock.insert(2, 13);

    const auto mergedData = QVariant::fromValue(QString("ConflictResolutedData"));
    VersionedData versionedData(localData, 1, localVectorClock, [=](const QVariant& localData, const QVariant& remoteData) -> QVariant {
        Q_UNUSED(localData)
        Q_UNUSED(remoteData)

        return mergedData;
    });

    QMap<qint32, qint32> remoteVectorClock;
    remoteVectorClock.insert(0, 10);
    remoteVectorClock.insert(1, 146);
    remoteVectorClock.insert(2, 13);

    const auto remoteData = QVariant::fromValue(QString("RemoteData"));
    versionedData.onDataReceived(remoteVectorClock, remoteData);
    QCOMPARE(versionedData.data(), mergedData);
}

QTEST_GUILESS_MAIN(LogicalClocksTest)

#include "tst_logicalclocks.moc"
