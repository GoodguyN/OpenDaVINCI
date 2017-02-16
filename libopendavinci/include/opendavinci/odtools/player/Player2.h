/**
 * OpenDaVINCI - Portable middleware for distributed components.
 * Copyright (C) 2017 Christian Berger
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef OPENDAVINCI_TOOLS_PLAYER_PLAYER2_H_
#define OPENDAVINCI_TOOLS_PLAYER_PLAYER2_H_

#include <deque>
#include <fstream>
#include <map>
#include <thread>

#include <opendavinci/odcore/opendavinci.h>
#include <opendavinci/odcore/base/Mutex.h>
#include <opendavinci/odcore/data/Container.h>
#include <opendavinci/odcore/data/TimeStamp.h>
#include <opendavinci/odcore/io/URL.h>
#include <opendavinci/odcore/exceptions/Exceptions.h>

namespace odtools {
    namespace player {

        class PlayerDelegate;

        using namespace std;

        class IndexEntry {
            public:
                IndexEntry();
                IndexEntry(const int64_t &sampleTimeStamp, const uint32_t &filePosition);

            public:
                int64_t m_sampleTimeStamp;
                uint32_t m_filePosition;
                bool m_available;
        };

        /**
         * This class can be used to read previously recorded
         * data from a given URL.
         */
        class OPENDAVINCI_API Player2 {
            private:
                enum {
                    ONE_SECOND_IN_MICROSECONDS = 1000 *1000,
                    MAX_DELAY_IN_MICROSECONDS = 5 * ONE_SECOND_IN_MICROSECONDS,
                    LOOK_AHEAD_IN_S = 30,
                };

            private:
                /**
                 * "Forbidden" copy constructor. Goal: The compiler should warn
                 * already at compile time for unwanted bugs caused by any misuse
                 * of the copy constructor.
                 *
                 * @param obj Reference to an object of this class.
                 */
                Player2(const Player2 &/*obj*/) = delete;

                /**
                 * "Forbidden" assignment operator. Goal: The compiler should warn
                 * already at compile time for unwanted bugs caused by any misuse
                 * of the assignment operator.
                 *
                 * @param obj Reference to an object of this class.
                 * @return Reference to this instance.
                 */
                Player2& operator=(const Player2 &/*obj*/) = delete;

            public:
                /**
                 * Constructor.
                 *
                 * @param url Resource to play.
                 * @param autoRewind True if the file should be rewind at EOF.
                 */
                Player2(const odcore::io::URL &url, const bool &autoRewind);

                virtual ~Player2();

                /**
                 * This method returns the next container to be replayed.
                 *
                 * @return Next container to be replayed.
                 */
                odcore::data::Container getNextContainerToBeSent() throw (odcore::exceptions::ArrayIndexOutOfBoundsException);

                /**
                 * This method returns the delay to be waited before the next container should be delivered.
                 *
                 * @return delay to the next container in real time microseconds (us).
                 */
                uint32_t getDelay() const;

                /**
                 * This method returns true if there is more data to replay.
                 *
                 * @return true if there is more data to replay.
                 */
                bool hasMoreData() const;

                /**
                 * This method restarts the player.
                 */
                void rewind();

            private:
                /**
                 * This method initializes the global index where the sample
                 * time stamps are sorted chronocally and mapped to the 
                 * corresponding entries (like containers) in the corresponding
                 * recording files.
                 */
                void initializeIndex();

                /**
                 * This method computes the initially required amount of
                 * containers in the cache and fill the cache accordingly.
                 */
                void computeInitialCacheLevelAndFillCache();

                /**
                 * This method clears all caches.
                 */
                void resetCaches();

                /**
                 * This method resets the iterators.
                 */
                inline void resetIterators();

                /**
                 * This method actually fills the cache by trying to read up
                 * to maxNumberOfEntriesToReadFromFile from the rec file.
                 *
                 * @param maxNumberOfEntriesToReadFromFile Maximum number of entries to be read from file.
                 * @return Number of entries read from file.
                 */
                uint32_t fillContainerCache(const uint32_t &maxNumberOfEntriesToReadFromFile);

                /**
                 * This method checks the availability of the next container
                 * to be replayed from the cache.
                 */
                inline void checkAvailabilityOfNextContainerToBeReplayed();

            private:
                /**
                 * This method check if the Player has reached the end of the
                 * index (to throw an exception) or if the the Player starts
                 * over (using auto rewind).
                 *
                 * @throws odcore::exceptions::ArrayIndexOutOfBoundsException
                 */
                inline void checkForEndOfIndexAndThrowExceptionOrAutoRewind() throw (odcore::exceptions::ArrayIndexOutOfBoundsException);

            private: // Data for the Player.
                odcore::io::URL m_url;

                // Handle to .rec file.
                fstream m_recFile;
                bool m_recFileValid;

            private: // Player states.
                bool m_autoRewind;

            private: // Index and cache management.
                // Global index: Mapping SampleTimeStamp --> cache entry (holding the actual content from .rec, .rec.mem, or .h264 file)
                mutable odcore::base::Mutex m_indexMutex;
                multimap<int64_t, IndexEntry> m_index;

                // Pointers to the current container to be replayed and the
                // container that has be replayed from the global index.
                multimap<int64_t, IndexEntry>::iterator m_previousPreviousContainerAlreadyReplayed;
                multimap<int64_t, IndexEntry>::iterator m_previousContainerAlreadyReplayed;
                multimap<int64_t, IndexEntry>::iterator m_currentContainerToReplay;

                // Information about the index.
                multimap<int64_t, IndexEntry>::iterator m_nextEntryToReadFromRecFile;
                float m_containerReadFromFileThroughput;

                uint32_t m_desiredInitialLevel;

                // Fields to compute replay throughput for cache management.
                odcore::data::TimeStamp m_firstTimePointReturningAContainer;
                uint64_t m_numberOfReturnedContainersInTotal;
                float m_containerReplayThroughput;

                uint32_t m_delay;

            private:
                /**
                 * This method set the state of the containerCacheFilling thread.
                 *
                 * @param running False if the thread to fill the container cache shall be joined.
                 */
                void setContainerCacheFillingRunning(const bool &running);
                bool isContainerCacheFillingRunning() const;

                /**
                 * This method manages the cache.
                 */
                void manageCache();

            private:
                mutable odcore::base::Mutex m_containerCacheFillingThreadIsRunningMutex;
                bool m_containerCacheFillingThreadIsRunning;
                std::thread m_containerCacheFillingThread;

                // Mapping of pos_type (within .rec file) --> Container (read from .rec file).
                map<uint32_t, odcore::data::Container> m_containerCache;

            public:
                /**
                 * This method (un)registers a PlayerDelegate to process a
                 * specific Container differently.
                 *
                 * @param containerID Container ID to listen for.
                 * @param p PlayerDelegate or NULL to unregister.
                 */
                void registerPlayerDelegate(const uint32_t &containerID, PlayerDelegate* p);

            private:
                // Map to handle PlayerDelegates.
                odcore::base::Mutex m_mapOfPlayerDelegatesMutex;
                map<int32_t, PlayerDelegate*> m_mapOfPlayerDelegates;
        };

    } // player
} // tools

#endif /*OPENDAVINCI_TOOLS_PLAYER_PLAYER2_H_*/
