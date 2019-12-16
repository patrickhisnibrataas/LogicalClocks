#include "logicalclocks.h"
#include <QSet>

#include <QDebug>

namespace {

// Check if vectorClock1 happened before, after og concurrently with vectorClock2. The compare function is heavily inspired by the Voldemort Project: https://github.com/voldemort/voldemort
VectorClock::LocalOccured compare(VectorClock* vectorClock1, VectorClock* vectorClock2)
{
    auto vc1VersionGreater = false;
    auto vc2VersionGreater = false;

    const auto vc1Keys = vectorClock1->ids();
    const auto vc2Keys = vectorClock2->ids();
    const auto commonKeys = vc1Keys.toSet().intersect(vc2Keys.toSet());

    if (vc1Keys.size() > commonKeys.size())
        vc1VersionGreater = true;

    if (vc2Keys.size() > commonKeys.size())
        vc2VersionGreater = true;

    for (const auto& key : commonKeys) {
        if (vc1VersionGreater && vc2VersionGreater)
            break;

        const auto vc1Version = vectorClock1->count().find(key).value();
        const auto vc2Version = vectorClock2->count().find(key).value();

        if (vc1Version > vc2Version)
            vc1VersionGreater = true;
        else if (vc1Version < vc2Version)
            vc2VersionGreater = true;
    }

    if (!vc1VersionGreater && !vc2VersionGreater)
        return VectorClock::LocalOccured::BeforeRemote;
    else if (vc1VersionGreater && !vc2VersionGreater)
        return VectorClock::LocalOccured::AfterRemote;
    else if (!vc1VersionGreater && vc2VersionGreater)
        return VectorClock::LocalOccured::BeforeRemote;
    else
        return VectorClock::LocalOccured::ConcurrentlyWithRemote;
}
}

Clock::Clock()
{
}

Clock::Clock(qint32 counter)
    : m_counter(counter)
{
}

qint32 Clock::event()
{
    m_counter += 1;
    return m_counter;
}

qint32 Clock::send()
{
    m_counter += 1;
    return m_counter;
}

qint32 Clock::receive(qint32 counter, bool isRemote)
{    
    if (isRemote)
        m_counter = std::max(m_counter, counter);
    else
        m_counter = std::max(++m_counter, counter);

    return m_counter;
}

qint32 Clock::count() const
{
    return m_counter;
}

VectorClock::VectorClock(qint32 localId)
    : m_localId(localId)
{
    addElement(m_localId, 0);
}

VectorClock::VectorClock(qint32 localId, QMap<qint32, qint32> vector)
    : m_localId(localId)
{
    for (auto it = vector.constBegin(); it != vector.constEnd(); ++it)
        addElement(it.key(), it.value());
}

void VectorClock::addElement(qint32 id, qint32 counter)
{
    Q_ASSERT(m_vector.constFind(id) == m_vector.constEnd());
    m_vector.insert(id, std::make_shared<Clock>(counter));
}

QMap<qint32, qint32> VectorClock::event()
{
    Q_ASSERT(m_vector.constFind(m_localId) != m_vector.constEnd());
    m_vector.value(m_localId)->event();
    return count();
}

QMap<qint32, qint32> VectorClock::send()
{
    Q_ASSERT(m_vector.constFind(m_localId) != m_vector.constEnd());
    m_vector.value(m_localId)->send();
    return count();
}

qint32 VectorClock::localId() const
{
    return m_localId;
}

QList<qint32> VectorClock::ids() const
{
    return m_vector.keys();
}

QMap<qint32, qint32> VectorClock::count() const
{
    QMap<qint32, qint32> vector;
    for (auto it = m_vector.constBegin(); it != m_vector.constEnd(); ++it)
        vector.insert(it.key(), it.value()->count());

    return vector;
}

VectorClock::LocalOccured VectorClock::receive(QMap<qint32, qint32> vector)
{
    VectorClock vc(0, vector);
    const auto occured = compare(this, &vc);

    // Update local vector's common clocks
    updateLocalVectorToGreatestClocksOfLocalAndRemote(vector);

    // Insert all new clocks to local vector
    for (auto it = vector.constBegin(); it != vector.constEnd(); ++it)
        if (m_vector.find(it.key()) == m_vector.constEnd())
            addElement(it.key(), it.value());

    return occured;
}

void VectorClock::updateLocalVectorToGreatestClocksOfLocalAndRemote(const QMap<qint32, qint32> &vector)
{
    for (auto it = vector.constBegin(); it != vector.constEnd(); ++it)
        if (m_vector.find(it.key()) != m_vector.constEnd()) {
            auto localClock = m_vector.find(it.key())->get();
            localClock->receive(it.value(), it.key() != localId());
        }
}

VersionedData::VersionedData(const QVariant &data, qint32 localClockId, const QMap<qint32, qint32> &vectorclocks, std::function<QVariant (const QVariant &, const QVariant &)> conflictResolution)
    : m_data(data),
      m_vectorClock(localClockId, vectorclocks),
      m_conflictResolution(conflictResolution)
{
}

QVariant VersionedData::data() const
{
    return m_data;
}

void VersionedData::sendData()
{
    m_vectorClock.send();
}

void VersionedData::onDataModified()
{
    m_vectorClock.event();
}

void VersionedData::onDataReceived(const QMap<qint32, qint32> &vector, const QVariant &data) {
    const auto occured = m_vectorClock.receive(vector);
    if (occured == VectorClock::LocalOccured::AfterRemote) {
        // Do nothing as local data is newer than remote data
    } else if (occured == VectorClock::LocalOccured::BeforeRemote) {
        // Update local data with never remote data
        m_data = data;
    } else if (occured == VectorClock::LocalOccured::ConcurrentlyWithRemote) {
        // Apply conflict resolution strategy and update data
        m_data = m_conflictResolution(m_data, data);
    }
}
