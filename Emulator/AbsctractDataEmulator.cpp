//Local
#include "AbsctractDataEmulator.h"

AbstractDataEmulator::AbstractDataEmulator(QObject *parent)
    : QObject(parent)
{}

void AbstractDataEmulator::setRoadTrack(const QVector<QVector<double> > &roadTrack)
{
    reset();
    m_roadTrack = roadTrack;
}

void AbstractDataEmulator::setStationsOnEdge(const QMap<int, QVector<latitudeLongitude> > &stationsOnEdge)
{
    reset();
    m_stationsOnEdge = stationsOnEdge;
}


