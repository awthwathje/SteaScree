#ifndef SCREENSHOT_H
#define SCREENSHOT_H

#include <QStringList>
#include <QPixmap>

struct Screenshot
{
    quint32 id;                 // unique id of a screenshot for an internal record keeping
    QStringList screenshot; // list of (path to file, new name)
    QPixmap thumbnail;      // thumbnail for LargeFileDialog window
    QList<quint32> geometry;    // width and height
    bool isLarge;           // if screenshot is too large for the Steam Cloud
    quint32 decision;           // what to do with a large screenshot
};

#endif // SCREENSHOT_H
