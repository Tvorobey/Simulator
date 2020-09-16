//Local
#include "DriveEmulator.h"

//SDK
#include <CConsoleLogger/CConsoleLogger.h>

//Qt
#include <QDebug>

namespace
{
    const int MAIN_TIMER_DELAY      = 1000;
    const int STATION_TIMER_DELAY   = 4000;
    const int SECOND_TO_MS          = 1000;

}

DriveEmulator::DriveEmulator(QObject *parent)
    : AbstractDataEmulator(parent),
      m_mainTimer(new QTimer(this)),
      m_stationTimer(new QTimer(this)),
      m_mainTimerDelay(MAIN_TIMER_DELAY),
      m_stationTimerDelay(STATION_TIMER_DELAY),
      m_state(State::GnssStation),
      m_edgeCount(0),
      m_stationOnEdgeCount(0)
{}

void DriveEmulator::setRoadTrack(const QVector<QVector<double> > &roadTrack)
{
    reset();
    AbstractDataEmulator::setRoadTrack(roadTrack);
}

void DriveEmulator::setStationsOnEdge(const QMap<int, QVector<latitudeLongitude> > &stationsOnEdge)
{
    reset();
    AbstractDataEmulator::setStationsOnEdge(stationsOnEdge);
}

void DriveEmulator::start()
{
    connect(m_mainTimer, &QTimer::timeout, this, &DriveEmulator::onMainTimerTimeOut);
    connect(m_stationTimer, &QTimer::timeout, this, &DriveEmulator::onStationTimerTimeOut);

    if (m_roadTrack.empty())
    {
        PRINT_CONSOLE_MESSAGE_ERROR("Пустой трек маршрутов");
        return;
    }

    if (m_state == GnssStation)  m_mainTimer->start(m_mainTimerDelay);
    else m_stationTimer->start(m_stationTimerDelay);
}

void DriveEmulator::stop()
{
    reset();
}

void DriveEmulator::pause()
{
    disconnect(m_mainTimer, nullptr, nullptr, nullptr);
    disconnect(m_stationTimer, nullptr, nullptr, nullptr);

    m_mainTimer->stop();
    m_stationTimer->stop();
}

void DriveEmulator::setMotionDelay(int sec)
{
    m_mainTimerDelay = sec * SECOND_TO_MS;
}

void DriveEmulator::setOnStationDelay(int sec)
{
    m_stationTimerDelay = sec * SECOND_TO_MS;
}

void DriveEmulator::checkCount()
{
    if (m_stationsOnEdge.contains(m_edgeCount))
    {
        if (++m_stationOnEdgeCount != m_stationsOnEdge.value(m_edgeCount).size())
            return;
    }

    if (++m_edgeCount == m_roadTrack.size())
        stop();
    else
        m_mainTimer->start(MAIN_TIMER_DELAY);

    m_stationTimer->stop();
    m_stationOnEdgeCount = 0;
}

void DriveEmulator::reset()
{
    disconnect(m_mainTimer, nullptr, nullptr, nullptr);
    disconnect(m_stationTimer, nullptr, nullptr, nullptr);
    m_mainTimer->stop();
    m_stationTimer->stop();
    m_edgeCount = 0;
    m_stationOnEdgeCount = 0;
    m_state = GnssStation;
    m_mainTimerDelay = MAIN_TIMER_DELAY;
    m_stationTimerDelay = STATION_TIMER_DELAY;
}

void DriveEmulator::onMainTimerTimeOut()
{
    qDebug() << "count: " << m_edgeCount << "\n roadTrack size: " << m_roadTrack.size();

    PRINT_CONSOLE_MESSAGE_INFO(QString("Публикую координаты трека\n"
                                       "Номер ребра: %1\n"
                                       "Широта: %2\n"
                                       "Долгота: %3\n")
                               .arg(m_edgeCount)
                               .arg(m_roadTrack[m_edgeCount][0])
                               .arg(m_roadTrack[m_edgeCount][1]));
    emit postGnss(m_roadTrack[m_edgeCount][0], m_roadTrack[m_edgeCount][1]);

    if (m_stationsOnEdge.contains(m_edgeCount))
    {
        m_mainTimer->stop();
        m_stationTimer->start(STATION_TIMER_DELAY);
    }
    else
        checkCount();
}

void DriveEmulator::onStationTimerTimeOut()
{
    switch (m_state)
    {
        case GnssStation:
        {
            qDebug() << "stationsEdgeCOunt: " <<  m_stationOnEdgeCount;
            qDebug() << "m+count: " << m_edgeCount;
            qDebug() << "stations on edge size: " << m_stationsOnEdge.size();

            PRINT_CONSOLE_MESSAGE_INFO(QString("Публикую координаты остановки\n"
                                               "Номер ребра: %1\n"
                                               "Широта: %2\n"
                                               "Долгота: %3\n")
                                       .arg(m_edgeCount)
                                       .arg(m_stationsOnEdge.value(m_edgeCount)[m_stationOnEdgeCount].first)
                                       .arg(m_stationsOnEdge.value(m_edgeCount)[m_stationOnEdgeCount].second));

            emit postGnss(m_stationsOnEdge.value(m_edgeCount)[m_stationOnEdgeCount].first,
                          m_stationsOnEdge.value(m_edgeCount)[m_stationOnEdgeCount].second);

            m_state = OpenDoor;
        }
        break;

        case OpenDoor:
        {
            PRINT_CONSOLE_MESSAGE_INFO("Публикую состояние для дверей - ОТКРЫТЫ");

            emit postDoorState(true);

            m_state = CloseDoor;
        }
        break;

        case CloseDoor:
        {
            PRINT_CONSOLE_MESSAGE_INFO("Публикую состояние для дверей - ЗАКРЫТЫ");

            emit postDoorState(false);

            m_state = GnssStation;

            checkCount();
        }
        break;
    }
}
