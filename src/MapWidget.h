#ifndef __MAP_WIDGET_H__
#define __MAP_WIDGET_H__

#include <QWebView>

class MapWidget : public QWebView
{
    Q_OBJECT

public:
    MapWidget(QWidget *parent = 0);

};

#endif
