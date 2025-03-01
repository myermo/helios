#pragma once

#include <string>
#include <filems/write/comps/SimpleSyncFileWriter.h>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/archive/binary_oarchive.hpp>

namespace helios { namespace filems {

using std::string;
using std::unique_ptr;
using std::ios_base;

/**
 * @author Alberto M. Esmoris Pena
 * @version 1.0
 * @brief Abstract child of SimpleSyncFileWriter which provides support for
 *  zipped output
 * @see filems::SimpleSyncFileWriter
 */
template <typename ... WriteArgs>
class ZipSyncFileWriter : public SimpleSyncFileWriter<WriteArgs...> {
protected:
using SimpleSyncFileWriter<WriteArgs...>::ofs;
    // ***  ATTRIBUTES  *** //
    // ******************** //
    /**
     * @brief Compressed output stream
     */
    boost::iostreams::filtering_ostream compressedOut;
    /**
     * @brief ZLib compression parameters
     */
    boost::iostreams::zlib_params zp;
    /**
     * @brief Binary output archive
     */
    unique_ptr<boost::archive::binary_oarchive> oa;

public:
    // ***  CONSTRUCTION / DESTRUCTION  *** //
    // ************************************ //
    /**
     * @brief Build a ZipSyncFileWriter
     * @param compressionMode Compression mode.
     * Use boost::iostreams::zlib::best_speed to reduce execution time at most.
     * Use boost::iostreams::zlib::best_compression to reduce size at most.
     */
    explicit ZipSyncFileWriter(
        const string &path,
        int compressionMode = boost::iostreams::zlib::best_compression
    ) :
        SimpleSyncFileWriter<WriteArgs ...>(
            path, ios_base::out | ios_base::binary
        )
    {
        zp = boost::iostreams::zlib_params(compressionMode);
        compressedOut.push(boost::iostreams::zlib_compressor(zp));
        compressedOut.push(ofs);
        oa = std::unique_ptr<boost::archive::binary_oarchive>(
            new boost::archive::binary_oarchive(compressedOut)
        );
    }
    virtual ~ZipSyncFileWriter() = default;

    // ***  F I N I S H  *** //
    // ********************* //
    /**
     * @brief ZipSyncFileWriter finishes the binary output archive instead of
     *  calling its parent finish method. This is necessary to prevent
     *  malfunctions coming from interaction between output archive closing
     *  and output stream closing
     * @see SimpleSyncFileWriter::finish
     */
    void finish() override {
        oa = nullptr;
        //SimpleSyncFileWriter<WriteArgs...>::finish();  // Must not be called
    }

};

}}
