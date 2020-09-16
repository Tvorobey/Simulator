#ifndef APPCONTROLLER_H
#define APPCONTROLLER_H

//SDK
#include <CMainHandler/CCoreApplication.h>
#include "AbsctractDataEmulator.h"

class RoutesParser;
class TrackHandler;
class CBusDoorTopic;
class CNavigationGnssStateTopic;
class CInformatorRouteStateTopic;
class CMotionSimulatorTopic;

class AppController : public CCoreApplication
{
public:
    explicit AppController(QObject* parent = nullptr);

    int onAttach() override;
    int onDetach() override;
private slots:
    void onRouteStateDataReceive();
    void onSimulatorTopic();
private:
    bool reinitTrackHandler(const QByteArray& data);
    void reinitDataEmulator();
private:
    QScopedPointer<CBusDoorTopic>               m_door1;
    QScopedPointer<CBusDoorTopic>               m_door2;
    QScopedPointer<CBusDoorTopic>               m_door3;
    QScopedPointer<CBusDoorTopic>               m_door4;
    QScopedPointer<CNavigationGnssStateTopic>   m_gnssTopic;
    QScopedPointer<CInformatorRouteStateTopic>  m_routeStateTopic;
    QScopedPointer<CMotionSimulatorTopic>       m_simulatorTopic;

    QScopedPointer<TrackHandler>                m_trackHandler;
    QScopedPointer<AbstractDataEmulator>        m_dataEmulator;

    qint64                                      m_currentRouteId;
    qint64                                      m_currentSubrouteID;
};

#endif // APPCONTROLLER_H
