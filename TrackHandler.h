#pragma once

//Qt
#include <QObject>
#include <QVector>
#include <QMap>

//SDK
#include "Mqtt/Topics/Informator/InformatorDefines.h"
#include <Mqtt/Topics/Informator/CInformatorRouteStateTopic.h>

//Local
#include "StaticDotAlgorithm.h"
#include "Global.h"

class StaticDotAlgorithm;


class TrackHandler : public QObject
{
    Q_OBJECT
public:
    explicit TrackHandler(QObject *parent = nullptr);
    void setRoadTrack(const QVector<QVector<double>>& roadTrack);
    QMap<int, QVector<latitudeLongitude>> getStationsOnEdge() const;
    QVector<QVector<double>> getRoadTrack() const;
    void matchStation(const QList<CInformatorRouteStateTopic::CInformatorStationInfo>& stations);
private:
    QVector<QVector<double>> m_roadTrack;
    /*!
     * \brief m_stationsOnEdge - спроецированные координаты остановки
     * на трек
     * key - номер ребра, на котором находится остановка
     * value - вектор координат остановок
     */
    QMap<int, QVector<latitudeLongitude>>   m_stationsOnEdge;

    QScopedPointer<StaticDotAlgorithm>      m_staticDotAlgorithm;
};


