#pragma once

//Qt
#include <QObject>
#include <QVector>
#include <QMap>

//Local
#include "Global.h"

class AbstractDataEmulator : public QObject
{
    Q_OBJECT
public:
    explicit AbstractDataEmulator(QObject *parent = nullptr);
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void pause() = 0;
    virtual void setMotionDelay(int sec) = 0;
    virtual void setOnStationDelay(int sec) = 0;
    void setRoadTrack(const QVector<QVector<double>>& roadTrack);
    void setStationsOnEdge(const QMap<int, QVector<latitudeLongitude>>& stationsOnEdge);
signals:
    void postGnss(double latitude, double longitude);
    void postDoorState(bool isOpen);
protected:
    virtual void reset() = 0;
protected:
    QVector<QVector<double>> m_roadTrack;
    QMap<int, QVector<latitudeLongitude>> m_stationsOnEdge;
};


