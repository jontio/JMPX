#include "volumemeter.h"

#include <iostream>
Volumemeter::Volumemeter(QWidget *parent) :
    QWidget(parent)
{
    myUsewarningcolor=true;
    myVol=0.0;
    myDir=Horizontal;
    myBarcolor=Qt::green;
}


void Volumemeter::paintEvent(QPaintEvent *)
{
    paintBorder();
    paintBar();



}

void Volumemeter::paintBorder()
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    if(myDir==Horizontal) painter.setWindow(0, 0, 310, 50);
     else painter.setWindow(0, 0, 50, 310);

    qreal bread=40;
    qreal len=300;

    qreal w;
    qreal h;

    if(myDir==Horizontal)
    {
        w=len;
        h=bread;
    }
     else
     {
         h=len;
         w=bread;
     }


    qreal xs=1;
    qreal ys=1;
    qreal ws=-1-xs;

    QBrush brush;


    QColor colBack(134,153,171);
    brush.setColor(colBack);
    brush.setStyle(Qt::SolidPattern);

    painter.setPen(QPen(colBack, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.setBrush(brush);
    QRectF border(5, 5, w, h);
    if(myDir==Horizontal)
    {
        painter.drawRoundRect(border, 5, 30);
    }
     else
     {
         painter.drawRoundRect(border, 30, 5);
     }


     //--
      //brush.setColor(colBack.lighter(150));
      //brush.setStyle(Qt::SolidPattern);

      painter.setPen(QPen(colBack.lighter(110), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
      //painter.setBrush(brush);
      QRectF border3(4, 4, w+2, h+2);
      if(myDir==Horizontal)
      {
          painter.drawRoundRect(border3, 5, 30);
      }
       else
       {
           painter.drawRoundRect(border3, 30, 5);
       }
    //--



    painter.setPen(QPen(colBack.lighter(120), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    QLinearGradient linGrad(0, 5+ys, 0, 5+ys+3);
    linGrad.setColorAt(0, colBack.lighter(120));
    linGrad.setColorAt(1, colBack.lighter(140));
    linGrad.setSpread(QGradient::PadSpread);
    painter.setBrush(linGrad);
    QRectF border2(5+xs, 5+ys, w+ws, h);
    if(myDir==Horizontal)
    {
        painter.drawRoundRect(border2, 5, 30);
    }
     else
     {
         painter.drawRoundRect(border2, 30, 5);
     }



}

void Volumemeter::paintBar()
{



    if(myVol==0)return;
    if(myVol<0)myVol=0;
    if(myVol>1)myVol=1;
    QPainter painter(this);
    if(myDir==Horizontal) painter.setWindow(0, 0, 310, 50);
     else painter.setWindow(0, 0, 50, 310);

    painter.setRenderHint(QPainter::Antialiasing);


    qreal bread=40;
    qreal len=300;

    qreal xs;
    qreal ys;

    qreal w;
    qreal h;



    if(myDir==Horizontal)
    {
        w=(len+1)*myVol;
        h=bread;
        xs=1;
        ys=1;
    }
     else
     {
         h=(len+1)*myVol;
         w=bread;
         xs=1;
         ys=1;
     }


     QPainterPath path;
     if(myDir==Horizontal) path.addRoundRect(5,5,len,bread+1,6,40);
      else path.addRoundRect(5,5,bread+1,len+1,30,5);
     painter.setClipPath(path);


    QColor colbar=myBarcolor;



    if(myUsewarningcolor)
    {
       // colbar=Qt::green;

        //hue changer
        int v=colbar.value();
        int s=colbar.saturation();
        int hue=-600.0*myVol+600;
        if(myVol>0.80)colbar.setHsv(hue,s,v);
    }


    //creates raised widget

    painter.setPen(QPen(colbar, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

    QLinearGradient linGrad;


    if(myDir==Horizontal)
    {
        linGrad.setStart(0, 5+ys);
        linGrad.setFinalStop(0, 5+ys+h);
        linGrad.setColorAt(0, colbar.lighter(120));
        linGrad.setColorAt(0.2, colbar);
        linGrad.setColorAt(0.8, colbar);
        linGrad.setColorAt(1, colbar.darker(120));
    }
     else
     {
         linGrad.setStart(5+xs, 0);
         linGrad.setFinalStop(5+xs+w, 0);
         linGrad.setColorAt(0, colbar.darker(120));
         linGrad.setColorAt(0.2, colbar);
         linGrad.setColorAt(0.8, colbar);
         linGrad.setColorAt(1, colbar.darker(120));
     }
    linGrad.setSpread(QGradient::PadSpread);
    painter.setBrush(linGrad);
    if(myDir==Horizontal)
    {
        QRectF border(5+xs, 5+ys, w, h-1.25);
        painter.drawRoundRect(border, 5, 30);
    }
     else
     {

         QRectF border(5+xs,len+4+ys ,w-1,-h);
         painter.drawRoundRect(border, 30, 5);
     }


}

void Volumemeter::setVolume(const qreal volume)
{
    if(((int)(volume*100.0))==((int)(myVol*100.0)))return;
    myVol=volume;
    update();
}
