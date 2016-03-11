#ifndef __MAP_VIEW_H__
#define __MAP_VIEW_H__

#include <QNetworkReply>
#include <QMainWindow>

class QAction;
class MapWidget ;

class MapView : public QMainWindow
{
    Q_OBJECT

public:
    MapView();

private:

    MapWidget *central_widget_;
    QString mapbox_js_ ;

private slots:

    void onLoadFinished(bool) ;
    void replyFinished(QNetworkReply*) ;
};

#endif

