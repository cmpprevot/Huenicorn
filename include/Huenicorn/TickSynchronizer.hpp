#pragma once

#include <chrono>


namespace Huenicorn
{
  /**
   * @brief Timing tool allowing to sync on a time inverval and compute stats
   * 
   */
  class TickSynchronizer
  {
    static constexpr size_t LoadRateHistorySize = 10;

  public:
    //using TimeScale = std::ratio<1, 1>;
    using TimeScale = std::ratio<1, 1>;
    using TimeUnitType = double;
    using ClockType = std::chrono::steady_clock;
    using TimeUnit = std::chrono::duration<TimeUnitType, TimeScale>;
    using Duration = TimeUnit;
    using TimePoint = std::chrono::time_point<ClockType, TimeUnit>;


    /**
     * @brief Time statistics for exceeded time
     * 
     */
    struct Excess
    {
      Duration extra;
      float rate;
    };


  // Constructors / Destructor
    /**
     * @brief TickSynchronizer constructor
     * 
     * @param tickInterval The duration of the interval to sync on in defined \ref TimeScale
     */
    TickSynchronizer(TimeUnitType tickInterval);


  // Getters
    /**
     * @brief Returns the duration of the interval
     * 
     * @return TimeUnitType
     */
    TimeUnit tickInterval() const;


    /**
     * @brief Returns the load rate
     * 
     * @return float Proportion of average measured duration divided by the nominal time to wait
     */
    float loadRate(bool asPercents = false) const;


    /**
     * @brief Returns the last excess data
     * 
     * @return const Excess& the data structure of the last excess
     */
    const Excess& lastExcess() const;


  // Setters
    /**
     * @brief Sets the tick interval duration
     * 
     * @param tickInterval Tick interval duration to set
     */
    void setTickInterval(TimeUnitType tickInterval);


  // Methods
    /**
     * @brief Initializes the time point
     * 
     */
    void start();


    /**
     * @brief Waits until time point + duration is reached and updates time point
     * 
     */
    bool sync();


  private:
  // Private methods
    /**
     * @brief Internal code to compute proper waiting duration
     * 
     * @param startTime Time point to compare
     * @return int The time exces factor to compute for the next interval
     */
    TimeUnitType _syncWithTick(const TimePoint& startTime);


    /**
     * @brief Computes the ratio between measured loop time and tick interval and computes the average
     * 
     * @param startTime
     * @param now
     * @return float
     */
    inline float _computeLoad(const TimePoint& startTime, const TimePoint& now)
    {
      Duration duration = now - startTime;
      m_tickAverage = _approxRollingAverage<Duration>(m_tickAverage, duration);

      return m_tickAverage / m_tickInterval;
    }


    /**
     * @brief Computes a bufferless approximate rolling average
     * 
     * @tparam T Type of the numeric value to compute
     * @param average Previous average value
     * @param input New value to integrate
     * @return T Output average after integration of input
     */
    template<class T>
    inline T _approxRollingAverage(T average, T input)
    {
      average -= m_loadRateHistory[m_loadRateHistoryCursorId];
      m_loadRateHistory[m_loadRateHistoryCursorId] = input / LoadRateHistorySize;
      average += m_loadRateHistory[m_loadRateHistoryCursorId];
      m_loadRateHistoryCursorId = (m_loadRateHistoryCursorId + 1) % LoadRateHistorySize;

      return average;
    }

  // Attributes
    TimeUnit m_tickInterval;
    TimePoint m_timePoint;
    Duration m_tickAverage{0};

    Duration m_loadRateHistory[LoadRateHistorySize];
    unsigned m_loadRateHistoryCursorId{0};
    float m_loadRate;

    Excess m_lastExcess{Duration{0}, 0.f};
  };
}
