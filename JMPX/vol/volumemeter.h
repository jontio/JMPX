#ifndef VOLUMEMETER_H
#define VOLUMEMETER_H

#include <QWidget>
#include <QPainter>


class Volumemeter : public QWidget
{
    Q_OBJECT
    Q_ENUMS(Direction)
    Q_PROPERTY(qreal volume READ volume WRITE setVolume)
    Q_PROPERTY(Direction direction READ direction WRITE setDirection)
    Q_PROPERTY(bool usewarningcolor READ usewarningcolor WRITE setUsewarningcolor)

    Q_PROPERTY(QColor barcolor READ barcolor WRITE setBarcolor)

public:

    enum Direction {Vertical,Horizontal};
    Volumemeter(QWidget *parent = 0);
    void setVolume(const qreal volume);
    qreal volume() const {return myVol;}

    void setDirection(const Direction direction){myDir=direction;update();}
    Direction direction() const {return myDir;}

    void setUsewarningcolor(const bool useit){myUsewarningcolor=useit;update();}
    bool usewarningcolor() const {return myUsewarningcolor;}

    void setBarcolor(const QColor barcolor){myBarcolor=barcolor;update();}
    QColor barcolor() const {return myBarcolor;}
protected:
    void paintEvent(QPaintEvent *);
private:
    void paintBorder();
    void paintBar();

    Direction myDir;
    qreal myVol;
    bool myUsewarningcolor;
    QColor myBarcolor;
};

#endif  //VOLUMEMETER_H
