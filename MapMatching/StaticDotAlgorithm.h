#pragma once

//Qt
#include <QObject>
#include <QVector>

//Local
#include "Global.h"

class StaticDotAlgorithm : public QObject
{
    Q_OBJECT
public:
    explicit StaticDotAlgorithm(QObject* parent = nullptr);
    void setTrack(const QVector<QVector<double> > &roadTrack);
    QPair<int, latitudeLongitude>   getEdgeWithStation(const double latitude,
                                                       const double longitude);
private:
    /*!
     * \brief findEdge - рассчитывает проекцию точки на ребра
     * \param dotLatitude
     * \param dotLongitude
     * \return Возвращает номер ребра, проекция на которое была наименьшей
     */
    QPair<int, latitudeLongitude> findEdge(const double dotLatitude,
                                           const double dotLongitude);
    /*!
     * \brief isBelongsToDirectAndRight - проверяет лежит ли точка правее ребра
     * и принадлежит ли она ему
     * \param edgeStartPoint - координаты начала ребра
     * \param edgeEndPoint - координаты конца ребра
     * \param dot - координаты точки (остановки)
     * \return
     */
    bool isBelongsToDirectAndRight(const QPointF &edgeStartPoint,
                                   const QPointF &edgeEndPoint,
                                   const QPointF &dot);
private:
    QVector<QVector<double>> m_roadTrack;

};
