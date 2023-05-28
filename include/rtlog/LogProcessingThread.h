#pragma once

#include <chrono>
#include <thread>

namespace rtlog
{
/**
 * @brief A class representing a log processing thread.
 *
 * This class represents a log processing thread that continuously dequeues log data from a LoggerType object and calls
 * a PrintLogFn object to print the log data. The wait time between each log processing iteration can be specified in
 * milliseconds.
 *
 * @tparam LoggerType The type of the logger object to be used for log processing.
 * @tparam PrintLogFn The type of the print log function object.
 */
template <typename LoggerType, typename PrintLogFn>
class LogProcessingThread
{
public:
    /**
     * @brief Constructs a new LogProcessingThread object.
     *
     * This constructor creates a new LogProcessingThread object. It takes a reference to a LoggerType object,
     * generally assumed to be some specialization of rtlog::Logger, a reference to a PrintLogFn object, and a wait time
     * in ms
     *
     * On construction, the LogProcessingThread will start a thread that will continually dequeue the messages from the
     * logger and call printFn on them.
     *
     * You must call Stop() to stop the thread and join it before your logger goes out of scope! Otherwise it's a
     * use-after-free
     *
     * See tests and examples for some ideas on how to use this class. Using ctad you often don't need to specify the
     * template parameters.
     *
     * @param logger The logger object to be used for log processing.
     * @param printFn The print log function object to be used to print the log data.
     * @param waitTime The time to wait between each log processing iteration.
     */
    LogProcessingThread( LoggerType& logger, PrintLogFn& printFn, std::chrono::milliseconds waitTime )
    : mPrintFn( printFn )
    , mLogger( logger )
    , mWaitTime( waitTime )
    {
        mThread = std::thread( &LogProcessingThread::ThreadMain, this );
    }

    ~LogProcessingThread()
    {
        if ( mThread.joinable() ) {
            Stop();
            mThread.join();
        }
    }

    void Stop()
    {
        mShouldRun.store( false );
    }


    LogProcessingThread( const LogProcessingThread& )            = delete;
    LogProcessingThread& operator=( const LogProcessingThread& ) = delete;
    LogProcessingThread( LogProcessingThread&& )                 = delete;
    LogProcessingThread& operator=( LogProcessingThread&& )      = delete;

private:
    void ThreadMain()
    {
        while ( mShouldRun.load() ) {

            if ( mLogger.PrintAndClearLogQueue( mPrintFn ) == 0 ) {
                std::this_thread::sleep_for( mWaitTime );
            }

            std::this_thread::sleep_for( mWaitTime );
        }

        mLogger.PrintAndClearLogQueue( mPrintFn );
    }

    PrintLogFn&               mPrintFn{};
    LoggerType&               mLogger{};
    std::thread               mThread{};
    std::atomic<bool>         mShouldRun{ true };
    std::chrono::milliseconds mWaitTime{};
};

} // namespace rtlog
