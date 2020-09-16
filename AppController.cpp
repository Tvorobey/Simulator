//Local
#include "AppController.h"
#include "TrackHandler.h"
#include "DriveEmulator.h"

//SDK
#include "Mqtt/Topics/Informator/RoutesParser.h"
#include "Mqtt/Topics/Informator/InformatorDefines.h"

//SDK Topics
#include "Mqtt/Topics/Bus/CBusDoorTopic.h"
#include "Mqtt/Topics/Navigation/CNavigationGnssStateTopic.h"
#include "Mqtt/Topics/Informator/CInformatorRouteStateTopic.h"
#include "Mqtt/Topics/Simulator/CMotionSimulatorTopic.h"

//Qt
#include <QScopedPointer>
#include <QFile>

namespace
{
    const QString FILE_NAME = "autoinf_120.json";
}

AppController::AppController(QObject *parent)
    : CCoreApplication(parent),
      m_door1(new CBusDoorTopic(1)),
      m_door2(new CBusDoorTopic(2)),
      m_door3(new CBusDoorTopic(3)),
      m_door4(new CBusDoorTopic(4)),
      m_gnssTopic(new CNavigationGnssStateTopic()),
      m_routeStateTopic(new CInformatorRouteStateTopic()),
      m_simulatorTopic(new CMotionSimulatorTopic()),
      m_trackHandler(new TrackHandler(this)),
      m_dataEmulator(new DriveEmulator(this)),
      m_currentRouteId(-1),
      m_currentSubrouteID(-1)
{
    connect(m_dataEmulator.data(), &DriveEmulator::postGnss, [this](double latitude, double longitude)
    {
        CNavigationGnssStateTopic gnssData;
        gnssData.setAltitude(146);
        gnssData.setDir(0);
        gnssData.setFixType("3D");
        gnssData.setHDop(1);
        gnssData.setIsValid(true);
        gnssData.setLatitude(latitude);
        gnssData.setLongitude(longitude);
        gnssData.setPDop(1);
        gnssData.setSatUsed(15);
        gnssData.setSatVisible(26);
        gnssData.setSpeed(0.5);
        gnssData.setTime(1599059506);
        gnssData.setVDop(1);

        publishToMqttTopic(gnssData);
    });

    connect(m_dataEmulator.data(), &DriveEmulator::postDoorState, [this](bool isOpen)
    {
        m_door1->setValue(isOpen);
        m_door2->setValue(isOpen);
        m_door3->setValue(isOpen);
        m_door4->setValue(isOpen);

        publishToMqttTopic(m_door1.data());
        publishToMqttTopic(m_door2.data());
        publishToMqttTopic(m_door3.data());
        publishToMqttTopic(m_door4.data());
    });
}

int AppController::onAttach()
{
    subscribeToMqttTopic(m_routeStateTopic.data());
    connect(m_routeStateTopic.data(), &CMqttTopic::messageReceived,
            this, &AppController::onRouteStateDataReceive);

    subscribeToMqttTopic(m_simulatorTopic.data());
    connect(m_simulatorTopic.data(), &CMqttTopic::messageReceived,
            this, &AppController::onSimulatorTopic);

    return 0;
}

int AppController::onDetach()
{
    return 0;
}

void AppController::onRouteStateDataReceive()
{
    if (m_routeStateTopic->isValid())
    {
        if (m_currentRouteId == m_routeStateTopic->getRouteId() && m_currentSubrouteID == m_routeStateTopic->getTripId())
        {
            PRINT_CONSOLE_MESSAGE_INFO("Получили такой же маршрут, ничего не делаем");
            return;
        }

        m_currentRouteId    = m_routeStateTopic->getRouteId();
        m_currentSubrouteID = m_routeStateTopic->getTripId();

        QFile jsonFile(FILE_NAME);

        if (!jsonFile.exists())
        {
            PRINT_CONSOLE_MESSAGE_ERROR(QString("Нет файла с треками: %1")
                                        .arg(FILE_NAME));
            return;
        }

        if (!jsonFile.open(QIODevice::ReadOnly))
        {
            PRINT_CONSOLE_MESSAGE_ERROR(QString("Невозможно открыть файл %1 для чтения")
                                        .arg(FILE_NAME));
            return;
        }

        if (reinitTrackHandler(jsonFile.readAll()))
        {
            reinitDataEmulator();
        }
        else
            PRINT_CONSOLE_MESSAGE_ERROR("Не удалось проинициализировать трек маршрутов");
    }
    else
        PRINT_CONSOLE_MESSAGE_WARN("Пришел невалидный топик ROUTE_STATE");
}

void AppController::onSimulatorTopic()
{
    if (m_simulatorTopic->isValid())
    {
        switch (m_simulatorTopic->getAction())
        {
            case Action::Stop: m_dataEmulator->stop();  break;

            case Action::Start:
            {
                m_dataEmulator->setMotionDelay(m_simulatorTopic->getMotionDelay());
                m_dataEmulator->setOnStationDelay(m_simulatorTopic->getOnStationDelay());
                m_dataEmulator->start();
            }
            break;

            case Action::Pause: m_dataEmulator->pause(); break;
        }
    }
}

bool AppController::reinitTrackHandler(const QByteArray &data)
{
    bool result = false;

    RoutesParser jsonParser(data);

    PRINT_CONSOLE_MESSAGE_INFO(QString("Выбран route_id: %1\n"
                                       "Выбран подмаршрут: %2")
                               .arg(m_currentRouteId)
                               .arg(m_currentSubrouteID));

    if (m_currentRouteId == -1 || m_currentSubrouteID  == -1)
        PRINT_CONSOLE_MESSAGE_WARN("Маршрут еще не выбран");
    else
    {
        const QVector<SubrouteInfo_t>& subroutes = jsonParser.subroutes(m_currentRouteId);

        if (subroutes.empty())
            PRINT_CONSOLE_MESSAGE_ERROR("Пустой список подмаршрутов");
        else
        {
            // Ищем имеется ли в файле с трэками, трэк для маршрута с tripID
            // Если есть, то возьмем его, а нет, так нет
            const auto subroute = std::find_if(subroutes.begin(), subroutes.end(),
                                               [this](const SubrouteInfo_t& value)
            {
                bool result = false;

                if (value.idTrip == m_currentSubrouteID)
                    result = true;

                return result;
            });

            if (subroute != subroutes.end())
                m_trackHandler->setRoadTrack(subroute->roadTrack);
            else
                PRINT_CONSOLE_MESSAGE_ERROR(QString("Не нашли трека с tripId: %1")
                                            .arg(m_currentSubrouteID));

            // Устанавилваем остановки, но берем их из топика
            int currentSubrouteIndex = m_routeStateTopic->getCurrentSubroute();
            const auto& stations = m_routeStateTopic->getSubroutes()[currentSubrouteIndex].m_stations;
            m_trackHandler->matchStation(stations);


            PRINT_CONSOLE_MESSAGE_INFO(QString("Первое ребро со станцией: %1")
                                       .arg(m_trackHandler->getStationsOnEdge().firstKey()));

            result = true;
        }
    }

    return result;
}

void AppController::reinitDataEmulator()
{
    m_dataEmulator->setRoadTrack(m_trackHandler->getRoadTrack());
    m_dataEmulator->setStationsOnEdge(m_trackHandler->getStationsOnEdge());
}



