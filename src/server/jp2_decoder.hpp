#ifndef __JP2_DECODER_H__
#define __JP2_DECODER_H__

// JPEG2000 decoder using openjpeg library
// currently only supporting single band 8 bit data (grayscale)

#include <stdio.h>
#include <cstdint>
#include <string>


class JP2Decoder {
public:

    JP2Decoder() ;

    // open file and read stream headers
    bool open(const std::string &file_name) ;
    bool read(uint64_t tile, char *buffer) ;

public:

    uint32_t image_width_ ;
    uint32_t image_height_ ;

    uint32_t tile_width_ ;
    uint32_t tile_height_ ;
    uint32_t n_tiles_x_ ;
    uint32_t n_tiles_y_ ;
    uint32_t resolutions_ ;
    float georef_[6] ;

    bool is_valid_ ;
    std::string file_name_ ;
} ;


#endif
