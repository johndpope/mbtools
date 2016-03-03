# osm2mbtiles

The application will convert an OpenStreetMap file to mbtiles format (https://www.mapbox.com/help/an-open-platform/#mbtiles). 
The application splits data to layers and maps 
selected OSM key/value pairs to to each layer. For this purpose the application reads a config file the defines each layer, rules to filter raw data and actions to store data to designated layer attributes.

For example:

```
@tracks, lines, type, bridge
bridge == 'yes' { store bridge 'true' ; continue ; }
highway == 'tertiary' { store type 'primary' }
highway == 'track'  && ( tracktype== 'grade1' || tracktype == 'grade2') { store type 'primary' }
highway == 'track' && ( tracktype== 'grade3' ) { store type 'secondary' }
highway == 'service' { store type 'secondary' }
highway == 'unclassified' { store type 'secondary' }
highway == 'track' && tracktype== 'grade4'  && ( smoothness == 'very bad' || smoothness == 'horrible' ) { store type 'tertiary' }
highway == 'track' && tracktype== 'grade5' { store type 'tertiary' }
highway == 'track' { store type 'secondary' }
```

defines a layer (sqlite geometry table) named "tracks" of geometry type "lines" with columns "type" and "bridge" and populated according to the rules given below.


