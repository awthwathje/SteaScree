#ifndef INTERFACEADJUSTER_H
#define INTERFACEADJUSTER_H

#include <QObject>
#include <QPushButton>


class InterfaceAdjuster : public QObject
{
    Q_OBJECT


public:
    explicit InterfaceAdjuster(QObject *parent = 0);


public slots:
    void setButtonsPadding(QList<QPushButton*> buttonList, QString os);

};

#endif // INTERFACEADJUSTER_H
