#include "MapView.h"
#include "MapWidget.h"

#include <QtGui>
#include <QWebView>

MapView::MapView()
{
    central_widget_ = new MapWidget(this);
    setCentralWidget(central_widget_);
    central_widget_->resize(1024, 768) ;

    connect(central_widget_, SIGNAL(loadFinished(bool)),
        this, SLOT(onLoadFinished(bool)));

    QNetworkAccessManager *nam = central_widget_->page()->networkAccessManager();

    connect(nam, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));


}

void MapView::onLoadFinished(bool ok)
{
    qDebug() << ok ;

 //   central_widget_->page()->mainFrame()->evaluateJavaScript(mapbox_js_);
 /*   central_widget_->page()->mainFrame()->evaluateJavaScript(R"(
        mapboxgl.accessToken = 'pk.eyJ1IjoicGV0ZXJuaWs5OSIsImEiOiJjaWxraHZ2aGwwMDBhdWdsenV5NHBvaTJ1In0.gsHJ7w8ndB-gMb4I_k0_Ig';
        var map = new mapboxgl.Map({
            container: 'map', // container id
            style: 'data/mapbox.json', //stylesheet location
            center: [22.23, 41], // starting position
            zoom: 12 // starting zoom
        });
    )

    map.debug = true ;
    )") ;
*/
}

void MapView::replyFinished(QNetworkReply *r)
{
    qDebug() << r->url() << r->error();
}

