#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QVector>
#include <QSlider>
#include <QPushButton>
#include <QRect>
#include <QPainter>
#include "texteditwidget.h"

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void loadData();
    void updateLineThickness(int value);
    void updatePointRadius(int value);
    void openTextEditWidget();
    void saveGraph();

private:
    QVector<double> signalData;
    QVector<int> extremumIndices;
    int lineThickness = 1;
    int pointRadius = 3;
    QPushButton *loadButton;
    QPushButton *editButton;
    QPushButton *saveGraphButton;
    QSlider *lineThicknessSlider;
    QSlider *pointRadiusSlider;
    QRect plotRegion;
    QString currentFileName;
    TextEditWidget *textEditWidget;

    void findExtrema();
    void drawSignal(QPainter& painter);
    void drawExtrema(QPainter& painter);
    void drawAxes(QPainter& painter);
    void updateSignalData();
    double findMinValue();
    double findMaxValue();
};
#endif // WIDGET_H
