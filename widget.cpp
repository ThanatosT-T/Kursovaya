#include "widget.h"
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QPainter>
#include <QMessageBox>
#include <QGridLayout>
#include <QSpacerItem>
#include <QImage>
#include <QPainter>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("Нахождение точек экстремумов");
    resize(800, 600);

    loadButton = new QPushButton("Загрузить сигнал", this);
    editButton = new QPushButton("Изменение сигнала", this);
    saveGraphButton = new QPushButton("Сохранить сигнал", this);
    lineThicknessSlider = new QSlider(Qt::Horizontal, this);
    pointRadiusSlider = new QSlider(Qt::Horizontal, this);

    lineThicknessSlider->setMinimum(1);
    lineThicknessSlider->setMaximum(5);
    lineThicknessSlider->setValue(lineThickness);
    pointRadiusSlider->setMinimum(1);
    pointRadiusSlider->setMaximum(10);
    pointRadiusSlider->setValue(pointRadius);
    plotRegion = QRect(50, 50, width() - 100, height() - 100);

    QGridLayout *mainLayout = new QGridLayout;

    mainLayout->addWidget(loadButton, 0, 0, 1, 1);
    mainLayout->addWidget(editButton, 0, 1, 1, 1);
    mainLayout->addWidget(saveGraphButton, 0, 2, 1, 1);
    mainLayout->addWidget(lineThicknessSlider, 0, 3, 1, 1);
    mainLayout->addWidget(pointRadiusSlider, 0, 4, 1, 1);

    QSpacerItem *spacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    mainLayout->addItem(spacer,1,0,1,5);

    plotRegion = QRect(50, 100, width() - 100, height() - 150);
    setLayout(mainLayout);

    connect(loadButton, &QPushButton::clicked, this, &Widget::loadData);
    connect(editButton, &QPushButton::clicked, this, &Widget::openTextEditWidget);
    connect(saveGraphButton, &QPushButton::clicked, this, &Widget::saveGraph);
    connect(lineThicknessSlider, &QSlider::valueChanged, this, &Widget::updateLineThickness);
    connect(pointRadiusSlider, &QSlider::valueChanged, this, &Widget::updatePointRadius);

    textEditWidget = nullptr;
}

Widget::~Widget()
{
}

void Widget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);

    if (signalData.isEmpty())
    {
        painter.drawText(QRect(10,50,width() - 20, height() - 60), Qt::AlignCenter, "Сначала загрузи сигнал.");
        return;
    }

    drawAxes(painter);
    drawSignal(painter);
    drawExtrema(painter);
}

void Widget::loadData()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Загрузка сигнала", "", "Текстовые файлы (*.txt)");
    if (fileName.isEmpty())
        return;

    currentFileName = fileName;
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::critical(this, "Ошибка", "Невозможно открыть файл.");
        return;
    }

    signalData.clear();
    QTextStream in(&file);
    while (!in.atEnd())
    {
        QString line = in.readLine();
        bool ok;
        double value = line.toDouble(&ok);
        if (ok) {
            signalData.push_back(value);
        } else {
            QMessageBox::warning(this, "Предупреждение", "Неверные данные пропущенны.");
        }
    }
    file.close();
    findExtrema();
    update();
}

void Widget::openTextEditWidget()
{
    if(currentFileName.isEmpty())
    {
        QMessageBox::warning(this, "Предупреждение", "Сначала загрузи сигнал.");
        return;
    }

    if(textEditWidget)
    {
        textEditWidget->close();
        delete textEditWidget;
    }
    textEditWidget = new TextEditWidget(currentFileName, this);

    connect(textEditWidget, &TextEditWidget::fileSaved, this, [this](){
        updateSignalData();
        findExtrema();
        update();
    });

    textEditWidget->show();
}
void Widget::updateSignalData() {
    if(currentFileName.isEmpty())
        return;

    QFile file(currentFileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Ошибка", "Невозможно открыть файл.");
        return;
    }

    signalData.clear();
    QTextStream in(&file);
    while (!in.atEnd())
    {
        QString line = in.readLine();
        bool ok;
        double value = line.toDouble(&ok);
        if (ok) {
            signalData.push_back(value);
        } else {
            QMessageBox::warning(this, "Предупреждение", "Неверные данные пропущенны.");
        }
    }
    file.close();
}

void Widget::findExtrema()
{
    extremumIndices.clear();
    if (signalData.size() < 3)
        return;

    for(int i = 1; i < signalData.size() - 1; ++i)
    {
        if((signalData[i] > signalData[i-1] && signalData[i] > signalData[i+1]) ||
            (signalData[i] < signalData[i-1] && signalData[i] < signalData[i+1])
            )
        {
            extremumIndices.push_back(i);
        }
    }
}

void Widget::drawSignal(QPainter& painter)
{
    if (signalData.isEmpty())
        return;

    double minValue = findMinValue();
    double maxValue = findMaxValue();
    double yRange = maxValue - minValue;

    QPen pen;
    pen.setWidth(lineThickness);
    pen.setColor(Qt::blue);
    painter.setPen(pen);

    int w = plotRegion.width();
    int h = plotRegion.height();
    double xStep = (double)w / (signalData.size() - 1);

    QPointF previousPoint(plotRegion.x(), plotRegion.y() + h - (signalData[0] - minValue) / yRange * h);

    for (int i = 1; i < signalData.size(); ++i)
    {
        QPointF currentPoint(plotRegion.x() + xStep * i, plotRegion.y() + h - (signalData[i] - minValue) / yRange * h);
        painter.drawLine(previousPoint, currentPoint);
        previousPoint = currentPoint;
    }
}

void Widget::drawExtrema(QPainter& painter)
{
    QPen DrawCircle;
    DrawCircle.setColor(Qt::red);
    DrawCircle.setWidth(3);

    if(extremumIndices.isEmpty() || signalData.isEmpty())
        return;

    double minValue = findMinValue();
    double maxValue = findMaxValue();
    double yRange = maxValue - minValue;

    int w = plotRegion.width();
    int h = plotRegion.height();

    double xStep = (double)w / (signalData.size() - 1);

    painter.setPen(DrawCircle);
    for (int index : extremumIndices)
    {
        double x = plotRegion.x() + xStep * index;
        double y = plotRegion.y() + h - (signalData[index] - minValue) / yRange * h;
        painter.drawEllipse(QPointF(x,y), pointRadius, pointRadius);
        painter.drawEllipse(QPointF(x,y), 1, 1);
        QString valueString = QString::number(signalData[index], 'g', 3);
        painter.setPen(Qt::black);
        painter.drawText(QPointF(x + pointRadius + 2, y + 4), valueString);
        painter.setPen(DrawCircle);
    }
}

void Widget::drawAxes(QPainter& painter) {
    QPen pen;
    pen.setColor(Qt::black);
    pen.setWidth(1);
    painter.setPen(pen);

    int x0 = plotRegion.x();
    int y0 = plotRegion.y() + plotRegion.height();
    int x1 = plotRegion.x() + plotRegion.width();
    int y1 = plotRegion.y();

    painter.drawLine(x0, y0, x1, y0);
    painter.drawLine(x0, y1, x0, y0);

    painter.drawText(x1 - 20, y0 - 5, "X");
    painter.drawText(x0 + 5, y1 + 10, "Y");

    painter.drawEllipse(QPoint(x0, y0), 2, 2);
    painter.drawText(x0 - 10, y0 + 15, "0");
}

void Widget::saveGraph()
{
    if (signalData.isEmpty())
    {
        QMessageBox::warning(this, "Предупреждение", "Нет данных для сохранения");
        return;
    }
    QString fileName = QFileDialog::getSaveFileName(this, "Сохранить график", "", "PNG файлы (*.png)");
    if (fileName.isEmpty())
        return;

    QImage image(size(), QImage::Format_ARGB32);
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.fillRect(rect(), Qt::white);

    drawAxes(painter);
    drawSignal(painter);
    drawExtrema(painter);

    if (!image.save(fileName))
        QMessageBox::critical(this, "Ошибка", "Невозможно сохранить файл.");
}

void Widget::updateLineThickness(int value)
{
    lineThickness = value;
    update();
}

void Widget::updatePointRadius(int value)
{
    pointRadius = value;
    update();
}

double Widget::findMinValue()
{
    double minValue = 0;
    if(signalData.isEmpty())
        return minValue;
    minValue = signalData[0];
    for(int i = 1; i<signalData.size(); ++i)
    {
        if(signalData[i] < minValue)
            minValue = signalData[i];
    }
    return minValue;
}
double Widget::findMaxValue()
{
    double maxValue = 0;
    if(signalData.isEmpty())
        return maxValue;
    maxValue = signalData[0];
    for(int i = 1; i<signalData.size(); ++i)
    {
        if(signalData[i] > maxValue)
            maxValue = signalData[i];
    }
    return maxValue;
}
