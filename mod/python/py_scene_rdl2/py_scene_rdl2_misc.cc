// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include "boost_python.h"
#include "py_scene_rdl2.h"

// scene_rdl2
#include <scene_rdl2/scene/rdl2/AsciiReader.h>
#include <scene_rdl2/scene/rdl2/BinaryReader.h>

#include <scene_rdl2/scene/rdl2/AsciiWriter.h>
#include <scene_rdl2/scene/rdl2/BinaryWriter.h>

#include <scene_rdl2/scene/rdl2/rdl2.h>
using namespace scene_rdl2;

namespace py_scene_rdl2
{
    //------------------------------------
    // rdl2::BinaryReader
    //------------------------------------

    class PyBinaryReader
    {
    private:
        std::shared_ptr<rdl2::SceneContext> mSceneContextPtr;
        rdl2::BinaryReader mBinaryReader;

    public:
        // no copy
        PyBinaryReader(const PyBinaryReader&) = delete;
        PyBinaryReader& operator=(const PyBinaryReader&) = delete;

        PyBinaryReader(std::shared_ptr<rdl2::SceneContext> sceneContextPtr)
            : mSceneContextPtr(sceneContextPtr)
            , mBinaryReader(*sceneContextPtr)
        {
        }

        void
        fromFile(const std::string& filename)
        {
            mBinaryReader.fromFile(filename);
        }
    };

    void
    registerBinaryReaderPyBinding()
    {
        using PyBinaryReaderClass_t = bp::class_<PyBinaryReader,
                                                std::shared_ptr<PyBinaryReader>,
                                                boost::noncopyable>;

        PyBinaryReaderClass_t("BinaryReader",
                             "A BinaryReader object can decode a binary stream of RDL data into a SceneContext. "
                             "It can be used to load a SceneContext from a serialized file, apply incremental "
                             "updates from a network socket, etc. \n"
                             "\n"
                             "Since BinaryReader needs to make modifications to the SceneContext, it cannot "
                             "operate on a const (read-only) context. It must be used at a point where "
                             "the SceneContext is mutable. \n"
                             "\n"
                             "The BinaryReader maintains no state other than the SceneContext it is supposed "
                             "to modify, so keeping it around to apply multiple incremental updates to the "
                             "SceneContext should work just fine. \n"
                             "\n"
                             "The BinaryReader can handle binary data from a number of sources. There are "
                             "convenience functions for reading RDL data from a file or a generic input "
                             "stream. These methods handle proper framing of the RDL binary data. The method "
                             "which reads binary data directly from byte strings assumes the framing has already "
                             "been removed and the appropriate manfiest and payload buffers have been extracted. \n"
                             "\n"
                             "RDL framing is very simple, so if you want to handle it at a higher level and "
                             "read directly into byte strings it's not very hard. The frame looks like this: \n"
                             "\n\n"
                             " +---------+---------+------------+------------+ \n"
                             " |  mlen   |  plen   |  manifest  |  payload   | \n"
                             " +---------+---------+------------+------------+ \n"
                             " | 8 bytes | 8 bytes | mlen bytes | plen bytes | \n"
                             " +---------+---------+------------+------------+ \n"
                             " ^-- first byte                    last byte --^ \n"
                             "\n\n"
                             "NOTE: Both mlen and plen are 64-bit unsigned integers, in network byte order (big endian). \n"
                             "\n"
                             "This encoding allows us to easily read the manifest and payload into separate buffers. "
                             "The manifest must be decoded serially, but once decoded, we have offsets into each message "
                             "in the payload, so we can decode it in parallel. \n"
                             "\n"
                             "Thread Safety: \n"
                             "  - The SceneContext guarantees that operations that the BinaryReader takes (such as "
                             "creating new SceneObjects) happens in a threadsafe way. \n"
                             "  - Manipulating the same SceneObject in multiple threads is not safe. As such, "
                             "a binary RDL file with multiple copies of the same SceneObject may cause thread unsafety "
                             "in the BinaryReader if those updates are decoded in parallel. The BinaryWriter will "
                             "never produce such files, but it's something to keep in mind. \n"
                             "  - Since the BinaryReader writes into SceneContext data (in particular, SceneObjects), "
                             "it is not safe to be mucking about with that data in another thread while the "
                             "BinaryReader is working.",
                             bp::init<std::shared_ptr<rdl2::SceneContext>>( bp::arg("SceneContext") ))

            .def("fromFile",
                 &PyBinaryReader::fromFile,
                 bp::arg("filename"),
                 "Opens the file with the given filename and attempts to read its contents as a stream of RDL "
                 "binary."
                 "\n"
                 "Input:    filename    The path to the RDL binary file on the filesystem.");
    }

    //------------------------------------
    // rdl2::AsciiReader
    //------------------------------------

    class PyAsciiReader
    {
    private:
        std::shared_ptr<rdl2::SceneContext> mSceneContextPtr;
        rdl2::AsciiReader mAsciiReader;

    public:
        // no copy
        PyAsciiReader(const PyAsciiReader&) = delete;
        PyAsciiReader& operator=(const PyAsciiReader&) = delete;

        PyAsciiReader(std::shared_ptr<rdl2::SceneContext> sceneContextPtr)
            : mSceneContextPtr(sceneContextPtr)
            , mAsciiReader(*sceneContextPtr)
        {
        }

        void
        fromFile(const std::string& filename)
        {
            mAsciiReader.fromFile(filename);
        }

        void
        fromString(const std::string& code, const std::string& chunkName = "@rdla")
        {
            mAsciiReader.fromString(code, chunkName);
        }
    };

    void
    registerAsciiReaderPyBinding()
    {
        using PyAsciiReaderClass_t = bp::class_<PyAsciiReader,
                                                std::shared_ptr<PyAsciiReader>,
                                                boost::noncopyable>;

        PyAsciiReaderClass_t("AsciiReader",
                             "An AsciiReader object can decode a text stream of RDL data into a "
                             "SceneContext. It can be used to load a SceneContext from a file, "
                             "apply incremental updates from a network socket, etc."
                             "\n"
                             "Since AsciiReader needs to make modifications to the SceneContext, "
                             "it cannot operate on a const (read-only) context. It must be used at "
                             "a point where the SceneContext is mutable."
                             "\n"
                             "The AsciiReader can handle text data from a number of sources. There "
                             "are convenience functions for reading RDL data from a file or a generic "
                             "input stream. In contrast to the binary format, the ASCII format is "
                             "NOT FRAMED. This means that fromFile() and fromStream() will continue "
                             "gobbling up text data until EOF. If you need to handle framing the text "
                             "data, do it at a higher level and pass the individual chunks of "
                             "text data to fromString()."
                             "\n"
                             "Thread Safety: \n"
                             "  - The SceneContext guarantees that operations that an AsciiReader "
                             "takes (such as creating new SceneObjects) happens in a threadsafe way. \n"
                             "  - Manipulating the same SceneObject in multiple threads is not safe. "
                             "Since the AsciiReader processes the file serially, this is only a problem if "
                             "you are mucking about with SceneObjects in another thread while the "
                             "AsciiReader is working.",
                             bp::init<std::shared_ptr<rdl2::SceneContext>>( bp::arg("SceneContext") ))

            .def("fromFile",
                 &PyAsciiReader::fromFile,
                 bp::arg("filename"),
                 "Opens the file with the given filename and attempts to read its contents as a stream of RDL text."
                 "\n"
                 "Input:    filename    The path to the RDL ASCII file on the filesystem.")

            .def("fromString",
                 &PyAsciiReader::fromString,
                 ( bp::arg("input"), bp::arg("chunkName") = "@rdla" ),
                 "Reads RDL text from the given string. The chunk name is an optional string which can be "
                 "used to identify the source of the RDL data in error messages (for example, the filename "
                 "when reading from a file)."
                 "\n"
                 "Inputs:    input        String of text containing RDL data (NOTE: labeled as 'code' in C++ API). \n"
                 "           chunkName    The name of the source of this RDL data. (optional)");
    }

    //------------------------------------
    // rdl2::AsciiWriter
    //------------------------------------

    class PyAsciiWriter
    {
    private:
        std::shared_ptr<rdl2::SceneContext> mSceneContextPtr;
        rdl2::AsciiWriter mAsciiWriter;

    public:
        // no copy
        PyAsciiWriter(const PyAsciiWriter&) = delete;
        PyAsciiWriter& operator=(const PyAsciiWriter&) = delete;

        PyAsciiWriter(std::shared_ptr<rdl2::SceneContext> sceneContextPtr)
            : mSceneContextPtr(sceneContextPtr)
            , mAsciiWriter(*sceneContextPtr)
        {
        }

        void
        setDeltaEncoding(bool deltaEncoding)
        {
            mAsciiWriter.setDeltaEncoding(deltaEncoding);
        }

        void
        setSkipDefaults(bool skipDefaults)
        {
            mAsciiWriter.setSkipDefaults(skipDefaults);
        }

        void
        toFile(const std::string& filename)
        {
            mAsciiWriter.toFile(filename);
        }

        std::string
        toString()
        {
            return mAsciiWriter.toString();
        }
    };

    void
    registerAsciiWriterPyBinding()
    {
        bp::class_<PyAsciiWriter, std::shared_ptr<PyAsciiWriter>, boost::noncopyable>
            ("AsciiWriter", "WRITE DOCSTRING LATER", bp::init<std::shared_ptr<rdl2::SceneContext>>( bp::arg("SceneContext")))

            .def("setDeltaEncoding",
                 &PyAsciiWriter::setDeltaEncoding,
                 bp::arg( "deltaEncoding" ),
                 "Turns on optimizations for encoding deltas of changed data. This results in major data compression and "
                 "improvements in decoding speed. The final data is reliant on attribute default values defined in "
                 "the rendering DSOs and values that have not changed since the last commit. \n"
                 "\n"
                 "If you are encoding data to be sent over the wire and immediately consumed, turn on delta "
                 "encoding. If you're encoding data to be stored on disk and want newer DSOs to supply new "
                 "default values, turn on delta encoding. If you're encoding data to be stored on disk and "
                 "want absolutely all values (including defaults) written to the file, turn delta encoding off. \n"
                 "\n"
                 "Input:    deltaEncoding    True to enable delta encoding, false to disable it (Disabled by default.)")

            .def("setSkipDefaults",
                 &PyAsciiWriter::setSkipDefaults,
                 bp::arg("skipDefaults"),
                 "Attributes at their default value are not written")
                 
            .def("toFile", &PyAsciiWriter::toFile, bp::arg("filename"))

            .def("toString", &PyAsciiWriter::toString);
    }

    //------------------------------------
    // rdl2::BinaryWriter
    //------------------------------------

    class PyBinaryWriter
    {
    private:
        std::shared_ptr<rdl2::SceneContext> mSceneContextPtr;
        rdl2::BinaryWriter mBinaryWriter;

    public:
        // no copy
        PyBinaryWriter(const PyBinaryWriter&) = delete;
        PyBinaryWriter& operator=(const PyBinaryWriter&) = delete;

        PyBinaryWriter(std::shared_ptr<rdl2::SceneContext> sceneContextPtr)
            : mSceneContextPtr(sceneContextPtr)
            , mBinaryWriter(*sceneContextPtr)
        {
        }

        void
        setTransientEncoding(bool deltaEncoding)
        {
            mBinaryWriter.setTransientEncoding(deltaEncoding);
        }

        void
        setDeltaEncoding(bool deltaEncoding)
        {
            mBinaryWriter.setDeltaEncoding(deltaEncoding);
        }

        void
        setSkipDefaults(bool skipDefaults)
        {
            mBinaryWriter.setSkipDefaults(skipDefaults);
        }

        void
        toFile(const std::string& filename)
        {
            mBinaryWriter.toFile(filename);
        }

    };

    void
    registerBinaryWriterPyBinding()
    {
        using PyBinaryWriterClass_t = bp::class_<PyBinaryWriter,
                                                std::shared_ptr<PyBinaryWriter>,
                                                boost::noncopyable>;

        PyBinaryWriterClass_t("BinaryWriter",
                             "A BinaryWriter object can decode a binary stream of RDL data into a SceneContext. "
                             "It can be used to save a SceneContext to a serialized file, create incremental "
                             "updates from a network socket, etc. \n"
                             "\n"
                             "Since BinaryWriter doesn't need to make any modifications to the SceneContext, it can "
                             "operate on a const (read-only) context.  It must have a consistent view of the context, "
                             "however, so you can't write to objects in another thread when the BinaryWriter is running. \n"
                             "\n"
                             "The BinaryWriter maintains no state other than the SceneContext it is supposed "
                             "to modify, so keeping it around to apply multiple incremental updates to the "
                             "SceneContext should work just fine. \n"
                             "\n"
                             "The BinaryWriter can handle binary data to a number of sinks. There are "
                             "convenience functions for reading RDL data from a file or a generic input "
                             "stream. These methods handle proper framing of the RDL binary data. The method "
                             "which writes binary data directly to byte strings assumes the framing will be added later by the caller.\n"
                             "\n"
                             "RDL framing is very simple, so if you want to handle it at a higher level and "
                             "write directly into byte strings it's not very hard. The frame looks like this: \n"
                             "\n\n"
                             " +---------+---------+------------+------------+ \n"
                             " |  mlen   |  plen   |  manifest  |  payload   | \n"
                             " +---------+---------+------------+------------+ \n"
                             " | 8 bytes | 8 bytes | mlen bytes | plen bytes | \n"
                             " +---------+---------+------------+------------+ \n"
                             " ^-- first byte                    last byte --^ \n"
                             "\n\n"
                             "NOTE: Both mlen and plen are 64-bit unsigned integers, in network byte order (big endian). \n"
                             "\n"
                             "This encoding allows us to easily read the manifest and payload into separate buffers. "
                             "The manifest must be decoded serially, but once decoded, we have offsets into each message "
                             "in the payload, so we can decode it in parallel. \n"
                             "\n"
                             "Thread Safety: \n"
                             "    - Since the BinaryWriter reads SceneContext data (in particular, SceneObjects), it is "
                             "not safe to be writing to SceneObjects in another thread while the BinaryWriter is working.",
                             bp::init<std::shared_ptr<rdl2::SceneContext>>( bp::arg("SceneContext") ))

            .def("toFile",
                 &PyBinaryWriter::toFile,
                 bp::arg("filename"),
                 "Opens the file with the given filename and attempts to write the RDL binary to it. "
                 "You can use the BinaryReader's fromFile() method to read these files. \n"
                 "\n"
                 "Input:    filename    The path to the RDL binary file on the filesystem.")

            .def("setDeltaEncoding",
                 &PyBinaryWriter::setDeltaEncoding,
                 bp::arg("deltaEncoding"),
                 "Turns on optimizations for encoding deltas of changed data. This results in major data compression and "
                 "improvements in decoding speed. The final data is reliant on attribute default values defined in "
                 "the rendering DSOs and values that have not changed since the last commit. \n"
                 "\n"
                 "If you are encoding data to be sent over the wire and immediately consumed, turn on delta "
                 "encoding. If you're encoding data to be stored on disk and want newer DSOs to supply new "
                 "default values, turn on delta encoding. If you're encoding data to be stored on disk and "
                 "want absolutely all values (including defaults) written to the file, turn delta encoding off. \n"
                 "\n"
                 "Input:    deltaEncoding    True to enable delta encoding, false to disable it (Disabled by default.)")

            .def("setTransientEncoding",
                 &PyBinaryWriter::setTransientEncoding,
                 bp::arg("transientEncoding"),
                 "Turns on optimizations for encoding transient data. This results in minor data compression and "
                 "improvements in decoding speed. However, the encoded data is NOT robust enough to support changes "
                 "in rendering DSOs. \n"
                 "\n"
                 "If you are encoding data to be sent over the wire and immediately consumed, turn on transient "
                 "encoding. If you're encoding data to be stored on disk, leave it off. \n"
                 "\n"
                 "Input:    transientEncoding    True to enable transient encoding, false to disable"
                 " it (Disabled by default.)")
            
            .def("setSkipDefaults",
                 &PyBinaryWriter::setSkipDefaults,
                 bp::arg("skipDefaults"),
                 "Attributes at their default value are not written")
                ;
                 
    
    }

    //------------------------------------
    // scene_rdl2 utility functions
    //------------------------------------

    static void
    writeSceneToFileHelper(const rdl2::SceneContext& context, const std::string& filePath)
    {
        rdl2::writeSceneToFile(context, filePath);
    }

    void
    registerSceneRdl2UtilsPyBinding()
    {
        bp::def("writeSceneToFile",
                &writeSceneToFileHelper,
                ( bp::arg("sceneContext"), bp::arg("filePath") ),
                "Convenience function for easily dumping a SceneContext to a file, with the type of writer inferred from the file extension."
                "\n"
                "Inputs:    context     The SceneContext to write out. \n"
                "           filePath    The path to the .rdla or .rdlb file.");
    }

} // namespace py_scene_rdl2

