// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <map>
#include <string>
#include <utility>

namespace rdl2_localize {

/**
 * The MinUniqueSuffixMap roughly behaves like a std::map. It consumes
 * canonicalized, absolute source paths (through insert()), and computes
 * the minimum unique suffix, which is returned by looking up the original
 * source path with at().
 *
 * The begin() and end() functions return a wrapper around an iterator which
 * can be advanced with preincrement operator++ and dereferenced with *. When
 * dereferenced, this iterator wrapper will produce a std::pair of
 * std::strings, where the first element is the source path and the second
 * element is the minimum unique suffix destination path.
 *
 * The minimum unique suffix is fancy way of saying "what's the shortest
 * trailing amount of this source path I can get away with and still have all
 * my paths be unique". We want to shorten the path as much as possible for
 * our destination file names, but we don't want any collisions for files
 * that may have the same base name but live in different directories.
 *
 * Example:
 *      MinUniqueSuffixMap copyPaths;
 *
 *      auto toothless = copyPaths.insert("/seq/shot/toothless/skin.mm");
 *      auto hiccup = copyPaths.insert("/seq/shot/hiccup/skin.mm");
 *      auto cove = copyPaths.insert("/seq/shot/envir/cove.mm");
 *
 *      auto toothyPaths = *toothless;
 *      std::cout << toothyPaths.first;  // prints "/seq/shot/toothess/skin.mm"
 *      std::cout << toothyPaths.second; // prints "toothless_skin.mm"
 *
 *      auto hiccupPaths = *hiccup;
 *      std::cout << hiccupPaths.first;  // prints "/seq/shot/hiccup/skin.mm"
 *      std::cout << hiccupPaths.second; // prints "hiccup_skin.mm"
 *
 *      auto covePaths = *cove;
 *      std::cout << covePaths.first;  // prints "/seq/shot/envir/cove.mm"
 *      std::cout << covePaths.second; // prints "cove.mm"
 */
class MinUniqueSuffixMap
{
private:
    // Tracks data related to the path in the dest -> source map.
    struct PathData
    {
        PathData(std::string pathPrefix, std::string sourcePath) :
            mPathPrefix(std::move(pathPrefix)),
            mSourcePath(std::move(sourcePath)),
            mDead(false)
        {
        }

        // Default copy.
        PathData(const PathData&) = default;
        PathData& operator=(const PathData&) = default;

        // Default move.
        PathData(PathData&&) = default;
        PathData& operator=(PathData&&) = default;

        // Takes the old destination path, pops off the last path component of
        // its path prefix, and prepends that component to the new destination
        // path with an underscore. Ex:
        //      Dest Path -> Path Prefix
        //      ------------------------
        //      file.mm -> /some/path/to
        //
        //      1) destPath = rotatePathComponent(destPath);
        //
        //      to_file.mm -> /some/path
        //
        //      2) destPath = rotatePathComponent(destPath);
        //
        //      path_to_file.mm -> /some
        std::string rotatePathComponent(const std::string& oldDestPath);

        // Rotates the path component, but also steals the guts of this
        // PathData object and returns a new one, marking this one dead. The
        // return value is ready to be inserted into the dest -> source map.
        std::pair<std::string, PathData> relocate(const std::string& oldDestPath);

        // The path prefix of the given destination file. At the start, that's
        // that's the dirname(), though it may get shortened if the path is
        // rotated.
        std::string mPathPrefix;

        // The original source path for this destination file.
        std::string mSourcePath;

        // Marks an element in the map as dead. Dead elements do not appear in
        // the output, but still exist in dest -> source map so that they
        // continue to collide with new destination paths as they are inserted.
        bool mDead;
    };

    typedef std::map<std::string, PathData> DestToSourceMap;
    typedef std::map<std::string, DestToSourceMap::const_iterator> SourceToDestMap;

public:
    /**
     * Wraps an internal iterator for traversing all the valid paths stored in
     * the map. This wrapper can be advanced with preincrement operator++,
     * compared for equality and inequality with other iterator wrappers, and
     * be dereferenced with operator*.
     *
     * Dereferencing the iterator wrapper returns a std::pair or std::strings,
     * where the first element is the source path and the second element is the
     * destination path. Note that operator-> is not defined (due to some
     * complexities in how it's handled), so this will not work:
     *
     *      iter->first    // compile error
     *
     * but this will:
     *
     *      (*iter).first  // ok
     *
     * Better yet, just copy the pair out entirely and reference it from there,
     * because it is built on the fly by traversing the internal data
     * structures, so it may be expensive to compute over and over.
     *
     *      auto paths = *iter;
     *      // use paths.first, paths.second...
     */
    class IteratorWrapper
    {
    public:
        bool operator==(const IteratorWrapper& b) const { return mIter == b.mIter; }
        bool operator!=(const IteratorWrapper& b) const { return mIter != b.mIter; }
        void operator++() { ++mIter; }

        std::pair<std::string, std::string> operator*() const;

    private:
        IteratorWrapper(const MinUniqueSuffixMap& me,
                        SourceToDestMap::const_iterator iter) :
            mMap(me),
            mIter(iter)
        {
        }

        const MinUniqueSuffixMap& mMap;
        SourceToDestMap::const_iterator mIter;

        friend class MinUniqueSuffixMap;
    };

    MinUniqueSuffixMap();

    // Clears the map of all its entries.
    void clear();

    /**
     * Inserts a new source path into the map and returns an iterator wrapper
     * to its location. The iterator wrapper can be dereferenced to get the
     * source and destination path names, or the destination path itself can
     * be fetched with at(). The destination path may change over time as more
     * source paths are inserted into the map.
     */
    IteratorWrapper insert(const std::string& sourcePath);

    /**
     * Returns the destination path associated with the given source path. This
     * destination path may change over time as more source paths are inserted
     * into the map. This may throw an exception (std::out_of_range) if the
     * given source path is not in the map.
     */
    const std::string& at(const std::string& sourcePath) const;

    /**
     * Returns an iterator wrapper to the beginning of the all the valid paths
     * in the map.
     */
    IteratorWrapper begin() const { return IteratorWrapper(*this, mSourceToDest.begin()); }

    /**
     * Returns an iterator wrapper to the end of the all the valid paths in the
     * map.
     */
    IteratorWrapper end() const { return IteratorWrapper(*this, mSourceToDest.end()); }

private:
    SourceToDestMap::const_iterator insert(std::pair<std::string, PathData> newEntry);

    // Map from desired destination paths to PathData, which holds (among other
    // things) the original source path. This allows us to find collisions
    // quickly.
    DestToSourceMap mDestToSource;

    // Map from original source paths to destination paths. This map is updated
    // whenever a collision occurs and destination paths are updated.
    SourceToDestMap mSourceToDest;

    friend class IteratorWrapper;
};

} // namespace rdl2_localize

