#ifndef LOGICALCLOCKS_H
#define LOGICALCLOCKS_H

#include <QObject>
#include <QMap>
#include <QVariant>

// A Lamport timestamp logical clock: https://en.wikipedia.org/wiki/Lamport_timestamps
class Clock
{
public:
    Clock();
    Clock(qint32 counter);
    qint32 event();
    qint32 send();
    qint32 receive(qint32 counter, bool isRemote = false);
    qint32 count() const;

private:
    qint32 m_counter = 0;
};

// Vector clock: https://en.wikipedia.org/wiki/Vector_clock
class VectorClock {
public:
    enum class LocalOccured {
        BeforeRemote,
        AfterRemote,
        ConcurrentlyWithRemote
    };

    VectorClock(qint32 localId);
    VectorClock(qint32 localId, QMap<qint32, qint32> vector);
    QMap<qint32, qint32> event();
    QMap<qint32, qint32> send();
    LocalOccured receive(QMap<qint32, qint32> vector);
    QMap<qint32, qint32> count() const;
    qint32 localId() const;
    QList<qint32> ids() const;
    void addElement(qint32 localId, qint32 counter);

private:
    void updateLocalVectorToGreatestClocksOfLocalAndRemote(const QMap<qint32, qint32>& vector);

    qint32 m_localId;
    QMap<qint32, std::shared_ptr<Clock>> m_vector;
};

class VersionedData : public QObject {
    Q_OBJECT
public:
    VersionedData(const QVariant &data, qint32 localClockId, const QMap<qint32, qint32>& vectorclocks, std::function<QVariant(const QVariant&, const QVariant&)> conflictResolution);
    QVariant data() const;
    void sendData();

public slots:
    void onDataModified();
    void onDataReceived(const QMap<qint32, qint32>& vector, const QVariant& data);

private:
    QVariant m_data;
    VectorClock m_vectorClock;
    std::function<QVariant(const QVariant& localData, const QVariant& remoteData)> m_conflictResolution;
};

#endif // LOGICALCLOCKS_H
