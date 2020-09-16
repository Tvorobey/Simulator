//Local
#include "TrackHandler.h"

//Qt
#include <QScopedPointer>
#include <QDebug>

//SDK
#include <CConsoleLogger/CConsoleLogger.h>

TrackHandler::TrackHandler(QObject *parent)
    : QObject(parent),
      m_staticDotAlgorithm(new StaticDotAlgorithm(this))
{}

void TrackHandler::setRoadTrack(const QVector<QVector<double> > &roadTrack)
{
    m_roadTrack = roadTrack;
    m_staticDotAlgorithm->setTrack(m_roadTrack);

    qDebug() << "FIRST COORDINATE OF TRACK: " << m_roadTrack[0];
}

void TrackHandler::matchStation(const QList<CInformatorRouteStateTopic::CInformatorStationInfo>& stations)
{
    m_stationsOnEdge.clear();

    for (int i = 0; i < stations.size(); ++i)
    {
        double latitude     = stations[i].m_latitude;
        double longitude    = stations[i].m_longitude;

        QPair<int, latitudeLongitude> edgeWithStation = m_staticDotAlgorithm->getEdgeWithStation(latitude, longitude);

        if (m_stationsOnEdge.contains(edgeWithStation.first))
        {
            m_stationsOnEdge[edgeWithStation.first].append(edgeWithStation.second);
        }
        else
            m_stationsOnEdge.insert(edgeWithStation.first, {edgeWithStation.second});
    }

    PRINT_CONSOLE_MESSAGE_INFO(QString("Найдено %1 ребер со станциями")
                               .arg(m_stationsOnEdge.size()));
}

QMap<int, QVector<latitudeLongitude> > TrackHandler::getStationsOnEdge() const
{
    return m_stationsOnEdge;
}

QVector<QVector<double> > TrackHandler::getRoadTrack() const
{
    return m_roadTrack;
}
