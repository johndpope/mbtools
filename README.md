# osm2sqlite

The application will convert an OpenStreetMap file to an SQLite database. 
The database is suitable for importing into applications such as QGIS by splitting data to layers and mapping 
selected OSM keys to specific columns for each layer in the database. For this purpose the application reads a config file the defines each layer, sqlite table columns,  rules to filter raw data and actions to store data to designated layer columns.

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


