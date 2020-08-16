#include "clockwidget.h"

#include <QPainter>
#include <QtMath>
#include <QTime>
#include <QPropertyAnimation>
#include <QTimer>
#include <QDebug>

class ClockWidgetPrivate{
public:
    ClockWidgetPrivate(QWidget *parent)
        : owner(parent){
        QTime time = QTime::currentTime();
        hour = time.hour();
        minute = time.minute();
        second = time.minute();
        microSecond = time.msec();
    }
    QWidget *owner;
    QColor borderColor = QColor(254, 254, 254);
    QColor backgroundColor = QColor(80, 80, 80);
    QColor foregroundColor = QColor(254, 254, 254);
    QColor hourColor = QColor(160, 160, 160);
    QColor minuteColor = QColor(174, 174, 174);
    QColor secondColor = QColor(159, 159, 159);
    int hour = 0;
    int minute = 0;
    int second = 0;
    int microSecond = 0;
    QTimer timer;
};

ClockWidget::ClockWidget(QWidget *parent)
    : QWidget(parent)
    , d(new ClockWidgetPrivate(this))
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(&d->timer, &QTimer::timeout, this, &ClockWidget::setOffset);
    d->timer.start(1000);
}

ClockWidget::~ClockWidget()
{
}

void ClockWidget::setBorderColor(const QColor &color)
{
    d->borderColor = color;
    update();
}

QColor ClockWidget::borderColor() const
{
    return d->borderColor;
}

void ClockWidget::setBackgroundColor(const QColor &color)
{
    d->backgroundColor = color;
    update();
}

QColor ClockWidget::backgroundColor() const
{
    return d->backgroundColor;
}

void ClockWidget::setForegroundColor(const QColor &color)
{
    d->foregroundColor = color;
    update();
}

QColor ClockWidget::foregroundColor() const
{
    return d->foregroundColor;
}

void ClockWidget::setHourColor(const QColor &color)
{
    d->hourColor = color;
    update();
}

QColor ClockWidget::hourColor() const
{
    return d->hourColor;
}

void ClockWidget::setMinuteColor(const QColor &color)
{
    d->minuteColor = color;
    update();
}

QColor ClockWidget::minuteColor() const
{
    return d->minuteColor;
}

void ClockWidget::setSecondColor(const QColor &color)
{
    d->secondColor = color;
    update();
}

QColor ClockWidget::secondColor() const
{
    return d->secondColor;
}

QSize ClockWidget::sizeHint() const
{
    return QSize(500, 500);
}

QSize ClockWidget::minimumSizeHint() const
{
    return QSize(300, 300);
}

void ClockWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    painter.translate(width() / 2, height() / 2);
    double linew = 2 * painter.pen().widthF() / painter.transform().m11();

    // 外边框
    drawBorder(&painter);
    // 背景
    drawBackground(&painter);
    // 刻度线
    drawScale(&painter, linew);
    // 刻度值
    drawScaleNum(&painter);
    // 时钟指针
    drawHour(&painter);
    // 分钟指针
    drawMinute(&painter);
    // 秒钟指针
    drawSecond(&painter);
    // 中心盖板
    drawDot(&painter);
}

void ClockWidget::setOffset()
{
    QTime time = QTime::currentTime();
    d->hour = time.hour();
    d->minute = time.minute();
    d->second = time.second();
    d->microSecond = time.msec();
    update();
}

void ClockWidget::drawBorder(QPainter *painter)
{
    double min = qMin(width(), height());
    double radius = min / 2.0 - min / 10.0;
    painter->setPen(Qt::NoPen);
    painter->setBrush(d->borderColor);
    painter->drawEllipse(-radius, -radius, radius * 2, radius * 2);
}

void ClockWidget::drawBackground(QPainter *painter)
{
    double min = qMin(width(), height());
    double radius = min / 2.0 - min / 9.0;
    painter->save();
    painter->setPen(Qt::NoPen);
    painter->setBrush(d->backgroundColor);
    painter->drawEllipse(-radius, -radius, radius * 2, radius * 2);
    painter->restore();
}

void ClockWidget::drawScale(QPainter *painter, const double linew)
{
    painter->save();
    double min = qMin(width(), height());
    double radius = min / 2 - min / 8;
    QPen pen(d->foregroundColor);
    pen.setCapStyle(Qt::RoundCap);

    for (int i = 0; i <= 60; i++) {
        if (i % 5 == 0) {
            pen.setWidthF(linew * 1.5);
            painter->setPen(pen);
            painter->drawLine(0, radius - 10, 0, radius);
        } else {
            pen.setWidthF(linew * 0.5);
            painter->setPen(pen);
            painter->drawLine(0, radius - 5, 0, radius);
        }
        painter->rotate(6);
    }
    painter->restore();
}

void ClockWidget::drawScaleNum(QPainter *painter)
{
    painter->save();
    double min = qMin(width(), height());
    double radius = min / 2.0 - min / 4.8;
    painter->setPen(d->foregroundColor);
    QFont font("Microsoft YaHei", min / 20);
    painter->setFont(font);

    double startRad = 60 * (M_PI / 180);
    double deltaRad = 30 * (M_PI / 180);

    QFontMetrics fontMetrics(font);
    for (int i = 0; i < 12; i++) {
        double sina = qSin(startRad - i * deltaRad);
        double cosa = qCos(startRad - i * deltaRad);
        QString strValue = QString("%1").arg(i + 1);

        double textWidth = fontMetrics.width(strValue);
        double textHeight = fontMetrics.height();
        int x = radius * cosa - textWidth / 2;
        int y = -radius * sina + textHeight / 4;
        painter->drawText(x, y, strValue);
    }
    painter->restore();
}

void ClockWidget::drawHour(QPainter *painter)
{
    painter->save();
    QPen pen;
    pen.setCapStyle(Qt::RoundCap);
    painter->setPen(d->hourColor);
    painter->setBrush(d->hourColor);

    double min = qMin(width(), height()) / 6.0;
    QPolygonF pts;
    pts << QPointF(-3, 8) << QPointF(3, 8)
        << QPointF(2, -min) << QPointF(-2, -min);
    painter->rotate(30.0 * ((d->hour + d->minute / 60.0)));
    painter->drawConvexPolygon(pts);
    painter->restore();
}

void ClockWidget::drawMinute(QPainter *painter)
{
    painter->save();
    QPen pen;
    pen.setCapStyle(Qt::RoundCap);
    painter->setPen(d->minuteColor);
    painter->setBrush(d->minuteColor);

    double min = qMin(width(), height()) / 5.0;
    QPolygonF pts;
    pts << QPointF(-2, 8) << QPointF(2, 8)
        << QPointF(1, -min) << QPointF(-1, -min);
    painter->rotate(6.0 * (d->minute + d->second / 60.0));
    painter->drawConvexPolygon(pts);
    painter->restore();
}

void ClockWidget::drawSecond(QPainter *painter)
{
    painter->save();
    QPen pen;
    pen.setCapStyle(Qt::RoundCap);
    painter->setPen(d->secondColor);
    painter->setBrush(d->secondColor);

    double min = qMin(width(), height()) / 4.0;
    QPolygonF pts;
    pts << QPointF(-1, 10) << QPointF(1, 10) << QPointF(0, -min);
    painter->rotate(6.0 * (d->second + d->microSecond / 1000.0));
    painter->drawConvexPolygon(pts);
    painter->restore();
}

void ClockWidget::drawDot(QPainter *painter)
{
    painter->save();
    QConicalGradient coneGradient(0, 0, -90.0);
    coneGradient.setColorAt(0.0, d->backgroundColor);
    coneGradient.setColorAt(0.5, d->foregroundColor);
    coneGradient.setColorAt(1.0, d->backgroundColor);
    painter->setOpacity(0.9);
    painter->setPen(Qt::NoPen);
    painter->setBrush(coneGradient);
    painter->drawEllipse(-5, -5, 10, 10);
    painter->restore();
}


