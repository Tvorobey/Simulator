#pragma once

//Local
#include "AbsctractDataEmulator.h"

//Qt
#include <QTimer>

enum State
{
    GnssStation = 0,
    OpenDoor,
    CloseDoor
};

class DriveEmulator : public AbstractDataEmulator
{
    Q_OBJECT
public:
    explicit DriveEmulator(QObject* parent = nullptr);
    void setRoadTrack(const QVector<QVector<double>>& roadTrack);
    void setStationsOnEdge(const QMap<int, QVector<latitudeLongitude>>& stationsOnEdge);
    void start() override;
    void stop() override;
    void pause() override;
    void setMotionDelay(int sec) override;
    void setOnStationDelay(int sec) override;
protected:
    void reset()override;
private:
    void checkCount();
private slots:
    void onMainTimerTimeOut();
    void onStationTimerTimeOut();
private:
    QTimer* m_mainTimer;
    QTimer* m_stationTimer;
    int     m_mainTimerDelay;
    int     m_stationTimerDelay;

    State   m_state;
    int     m_edgeCount;
    int     m_stationOnEdgeCount;
};


