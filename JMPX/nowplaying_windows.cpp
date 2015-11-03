#include "nowplaying_windows.h"
#include <QDebug>

NowPlaying::NowPlaying(QObject *parent) : QObject(parent)
{
    rt_title.clear();
    rt_title_ba.clear();

    //song title checker timer
    QTimer *songtitlecheckertimer= new QTimer(this);
    songtitlecheckertimer->setInterval(1000);
    connect(songtitlecheckertimer,SIGNAL(timeout()),this,SLOT(songtitlecheck()));
    songtitlecheckertimer->start();
}

void NowPlaying::songtitlecheck()
{

    //find player and check we are playing
    HWND hwndWinamp = FindWindow(L"Winamp v1.x",NULL);
    if((hwndWinamp==NULL)||(SendMessage(hwndWinamp,WM_USER, (WPARAM)NULL,(LPARAM)104)!=1))
    {
        if(!rt_title_ba.isEmpty())
        {
            rt_title.clear();
            rt_title_ba.clear();
            emit songtitlechanged(rt_title);
        }
        return;
    }

    //get title
    QByteArray title;
    title.resize(80);
    int len=GetWindowTextA(hwndWinamp,title.data(), title.size());
    if(len<1)
    {
        if(!rt_title_ba.isEmpty())
        {
            rt_title.clear();
            rt_title_ba.clear();
            emit songtitlechanged(rt_title);
        }
        return;
    }
    title.resize(len);
    if(title[0]>='0'&&title[0]<='9')//seems track number is prefaced?
    {
        int idx=title.indexOf(' ');
        title=title.right(title.size()-idx).trimmed();
    }
     else title=title.trimmed();

    //set RT with title
    if(rt_title_ba!=title)
    {
        rt_title_ba=title;
        rt_title=title;
        emit songtitlechanged(rt_title);
    }

}

