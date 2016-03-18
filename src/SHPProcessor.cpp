#include "MapFile.h"

#include <boost/filesystem.hpp>
#include <spatialite.h>
#include <spatialite/gaiageo.h>
#include <shapefil.h>
#include <iomanip>

#include <iconv.h>

using namespace std ;
namespace fs = boost::filesystem ;

struct DBField {
    DBFFieldType type_ ;
    int width_, precision_ ;
    char name_[12] ;
};

string to_utf8( const string &src, const string &enc )
{
    std::vector<char> in_buf(src.begin(), src.end());
    char* src_ptr = &in_buf[0];
    size_t src_size = src.size();

    const size_t buf_size_ = 1024 ;
    const bool ignore_error_ = true ;

    std::vector<char> buf(buf_size_);
    std::string dst;

    iconv_t cd = iconv_open( "UTF-8", enc.c_str() );

    if ( cd != (iconv_t)-1 ) {

        while ( 0 < src_size ) {

            char* dst_ptr = &buf[0];
            size_t dst_size = buf.size();
            size_t res = ::iconv(cd, &src_ptr, &src_size, &dst_ptr, &dst_size);

            if (res == (size_t)-1) {
                if (errno == E2BIG)  ;
                else if ( ignore_error_ ) {
                    // skip character
                    ++src_ptr;
                    --src_size;
                } else {
                    break ;
                }
            }
            dst.append(&buf[0], buf.size() - dst_size);
        }

        iconv_close( cd );
    }

    return  dst ;
}

static void parse_record(DBFHandle db_handle, int index, const vector<DBField> &fields, Dictionary &dict, const string &enc) {
    for( uint i=0 ;i<fields.size() ; i++ ) {
        const DBField &f = fields[i] ;

        stringstream fstr ;
        if ( f.type_ == FTDouble ) {
            double val = DBFReadDoubleAttribute(db_handle, index, i) ;
            fstr << std::setiosflags(ios::fixed) << val ;
        }
        else if ( f.type_ == FTInteger ) {
            int val = DBFReadIntegerAttribute(db_handle, index, i) ;
            fstr << val ;
        }
        else if ( f.type_ == FTString ) {
            const char *p = DBFReadStringAttribute(db_handle, index, i) ;
            fstr << to_utf8(p, enc) ;
        }

        string sval = fstr.str() ;

        if ( !sval.empty() )
            dict.add(f.name_, sval) ;
    }
}

bool MapFile::processShpFile(const string &file_name, const string &table_name, int srid, const string &char_enc )
{
    SQLite::Database &db = handle() ;

    SQLite::Session session(&db) ;
    SQLite::Connection &con = session.handle() ;

    if ( !fs::exists(file_name) ) {
        cerr << "cannot read: " << file_name << endl ;
        return false ;
    }

    fs::path dir = fs::path(file_name).parent_path() ;
    string file_name_prefix = fs::path(file_name).stem().native() ;

    // open database file

    DBFHandle db_handle = DBFOpen( ( dir / (file_name_prefix + ".dbf") ).native().c_str(), "rb" );

    if ( !db_handle ) return false ;

    uint n_fields = DBFGetFieldCount(db_handle) ;

    vector<DBField> field_info ;

    for( uint i = 0 ; i<n_fields ; i++ ) {
        DBField fr ;

        fr.type_ = DBFGetFieldInfo( db_handle, i, fr.name_, &fr.width_, &fr.precision_);
        field_info.push_back(std::move(fr)) ;
    }

    SHPHandle shp_handle = SHPOpen( ( dir / file_name_prefix ).native().c_str(), "rb");

    int shp_entities, shp_geomtype ;
    SHPGetInfo( shp_handle, &shp_entities, &shp_geomtype, 0, 0) ;

    string out_geomtype ;

    switch ( shp_geomtype )
    {
    case SHPT_POINT:
    case SHPT_MULTIPOINT:
        out_geomtype = "points" ;
        break ;
    case SHPT_ARC:
        out_geomtype = "lines" ;
        break ;
    case SHPT_POLYGON:
        out_geomtype = "polygons" ;
        break ;
    default:    {
        cerr << file_name << ": unsupported geometry type (" << shp_geomtype << ")" << endl ;
        return false ;
    }
    }

    if ( !createLayerTable(table_name, out_geomtype, "3857") ) {
        cerr << "error creating layer: " << table_name << endl ;
        return false ;
    }

    SQLite::Transaction trans(con) ;

    string geoCmd = "Transform(?,3857)" ;
    SQLite::Command cmd(con, insertFeatureSQL(table_name, geoCmd)) ;

    for( uint i=0 ; i<shp_entities ; i++ ) {

        SHPObject *obj = SHPReadObject( shp_handle, i );

        gaiaGeomCollPtr geom = gaiaAllocGeomColl() ;
        geom->Srid = srid;

        if ( shp_geomtype == SHPT_POINT ) {
            geom->DeclaredType = GAIA_POINT;
            gaiaAddPointToGeomColl (geom, obj->padfX[0], obj->padfY[0]);
        }
        else if ( shp_geomtype == SHPT_ARC ) {
            gaiaLinestringPtr line = gaiaAllocLinestring (obj->nVertices);

            for (uint j=0 ; j<obj->nVertices ; j++) {
                double x = obj->padfX[j] ;
                double y = obj->padfY[j] ;
                gaiaSetPoint (line->Coords, j, x, y);
            }

            geom->DeclaredType = GAIA_LINESTRING;
            gaiaInsertLinestringInGeomColl (geom, line);
        }
        else if ( shp_geomtype == SHPT_POLYGON ) {

            geom->DeclaredType = GAIA_POLYGON;
            double *vx = obj->padfX, *vy = obj->padfY ;
            gaiaPolygonPtr gpoly = 0 ;

            for( int r = 0 ; r<obj->nParts ; r++ ) {
                uint count, last ;
                if ( r+1 < obj->nParts ) last = obj->panPartStart[r+1] ;
                else last = obj->nVertices ;

                count = last - obj->panPartStart[r] ;

                gaiaRingPtr ring = gaiaAllocRing (count);

                for (uint j=0 ; j<count ; j++) {
                    gaiaSetPoint (ring->Coords, j, *vx++, *vy++);
                }

                //      gaiaClockwise(ring) ;
                //     gaiaRingPtr iring = gaiaCloneRingSpecial(ring, GAIA_REVERSE_ORDER) ;

                if ( gpoly ) {
                    gaiaInsertInteriorRing(gpoly, ring);
                }
                else {

                    gpoly = gaiaInsertPolygonInGeomColl (geom, ring);
                }
            }
        }


        SHPDestroyObject(obj) ;

        Dictionary dict ;
        parse_record(db_handle, i, field_info, dict, char_enc) ;
        string tags = serializeTags(dict) ;

        unsigned char *blob ;
        int blob_sz ;

        gaiaToSpatiaLiteBlobWkb (geom, &blob, &blob_sz) ;

        cmd.bind(1, blob, blob_sz) ;
        cmd.bind(2, tags) ;

        cmd.exec() ;
        cmd.clear() ;

        free(blob);

    }

    trans.commit() ;


    return true ;

}



