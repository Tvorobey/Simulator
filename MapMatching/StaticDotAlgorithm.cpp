//Local
#include "StaticDotAlgorithm.h"

//SDK
#include <CConsoleLogger/CConsoleLogger.h>

//Qt
#include <QLineF>
#include <QtMath>

namespace
{
    const int       LATITUDE            = 0;
    const int       LONGITUDE           = 1;
    const double    RADIAN_TO_DEGREE    = 57.2958;

    double wedgeProduct(const QLineF& firstLine, const QLineF& secondLine)
    {
        return (firstLine.dx() * secondLine.dy()) - (secondLine.dx() * firstLine.dy());
    }

    bool isSharpAngleBetweenVectors(const QLineF& firstLine, const QLineF& secondLine)
    {
        bool result = false;

        double scalarProduct    = firstLine.dx() * secondLine.dx() + firstLine.dy() * secondLine.dy();
        double absFirstLine     = qSqrt(qPow(firstLine.dx(), 2) + qPow(firstLine.dy(), 2));
        double absSecondLine  = qSqrt(qPow(secondLine.dx(), 2) + qPow(secondLine.dy(), 2));

        double cosAngle = scalarProduct / (absFirstLine * absSecondLine);

        double angle = qAcos(qAbs(cosAngle)) * RADIAN_TO_DEGREE;

        if (cosAngle < 0) angle = 180 - angle;

        if (angle >= 0 && angle <= 90) result = true;

        return result;
    }

    double lenghtToStraight(const QLineF& firstLine, const QLineF& secondLine)
    {
        double firstLineLenght = qSqrt(qPow(firstLine.dx(), 2) + qPow(firstLine.dy(), 2));

        double lenght = qAbs(wedgeProduct(firstLine, secondLine) / firstLineLenght);

        return lenght;
    }
}

StaticDotAlgorithm::StaticDotAlgorithm(QObject* parent) : QObject(parent)
{}

void StaticDotAlgorithm::setTrack(const QVector<QVector<double>>& roadTrack)
{
    m_roadTrack = roadTrack;
}

QPair<int, latitudeLongitude> StaticDotAlgorithm::getEdgeWithStation(const double latitude,
                                                                     const double longitude)
{
    return findEdge(latitude, longitude);
}

QPair<int, latitudeLongitude> StaticDotAlgorithm::findEdge(const double dotLatitude,
                                                           const double dotLongitude)
{
    QPair<int, latitudeLongitude> result;

    if (m_roadTrack.isEmpty())  PRINT_CONSOLE_MESSAGE_ERROR("Пустой трек маршрута");
    else
    {
        int edgeNumber = 0;

        // Массив хранящий в себе пару: длинна перпендикуляра к ребру и номер ребра
        QVector<QPair<double, int>>    lenghtAndEdgeNumber;
        for (auto it = m_roadTrack.begin(); it != m_roadTrack.end(); ++it)
        {
            // Если следующего элемента нет - мы на последнем ребре
            if (std::next(it) != m_roadTrack.end())
            {
                QPointF edgeStart((*it)[LONGITUDE], (*it)[LATITUDE]);
                QPointF edgeEnd((*std::next(it))[LONGITUDE], (*std::next(it))[LATITUDE]);
                QPointF dot(dotLongitude, dotLatitude);

                if (isBelongsToDirectAndRight(edgeStart, edgeEnd, dot))
                {
                    lenghtAndEdgeNumber.append({lenghtToStraight({edgeStart, edgeEnd}, {edgeStart, dot}),
                                                edgeNumber});
                }
                ++edgeNumber;
            }
        }
        // После того, как собрали все длинны перпендикуляров точки к ребрам
        // найдем минимальное значение - это искомое ребро, к котором принадлежит
        // остановка
        QPair<double, int> minLenghtToEdge = *std::min_element(lenghtAndEdgeNumber.begin(),
                                                               lenghtAndEdgeNumber.end());

        result = {minLenghtToEdge.second, {dotLatitude, dotLongitude}};
    }

    return result;
}

bool StaticDotAlgorithm::isBelongsToDirectAndRight(const QPointF& edgeStartPoint,
                                                   const QPointF& edgeEndPoint,
                                                   const QPointF& dot)
{
    bool result = false;

    // Остановки всегда находятся справа по направлению движения
    // По этому, если наша точка остановки лежит левее ребра (косое произведение векторов положительное)
    // То, она к нему точно не относится
    if (wedgeProduct({edgeStartPoint, edgeEndPoint}, {edgeStartPoint, dot}) < 0)
    {
        // Проверим падает ли перпендикуляр из точки на ребро
        if (isSharpAngleBetweenVectors({edgeStartPoint, edgeEndPoint}, {edgeStartPoint, dot})
            && isSharpAngleBetweenVectors({edgeEndPoint, edgeStartPoint}, {edgeEndPoint, dot}))
        {
            result = true;
        }
    }

    return result;
}

