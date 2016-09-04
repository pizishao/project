using System;
using System.Collections.Generic;

using UnityEngine;

/// <summary>
/// Notify between specify time.
/// </summary>
public class TimeNotifier
{
    private LinkedList<Timer> timers =
        new LinkedList<Timer>();

    private LinkedList<Timer> toBeAddedtimers =
       new LinkedList<Timer>();

    private LinkedList<Timer> toBeDeletedtimers =
       new LinkedList<Timer>();

    /// <summary>
    /// Wait time and execute a task.
    /// </summary>
    /// <param name="second">The delay time in second.</param>
    /// <param name="task">The task to execute.</param>
    public Timer WaitTime(float seconds, Action task)
    {
        var timer = new Timer(seconds, -1.0f, false, task);
        toBeAddedtimers.AddLast(timer);
        return timer;
    }

    /// <summary>
    /// Wait unscaled time and execute a task.
    /// </summary>
    /// <param name="second">The delay time in unscaled second.</param>
    /// <param name="task">The task to execute.</param>
    public Timer WaitTimeUnscale(float seconds, Action task)
    {
        var timer = new Timer(seconds, -1.0f, true, task);
        toBeAddedtimers.AddLast(timer);
        return timer;
    }

    /// <summary>
    /// Invoke a task repeatly with a specify interval.
    /// </summary>
    /// <param name="interval">Each repeat interval in second.</param>
    /// <param name="task">The task to execute.</param>
    public Timer RepeatTime(float interval, Action task)
    {
        var timer = new Timer(interval, interval, false, task);
        toBeAddedtimers.AddLast(timer);
        return timer;
    }

    /// <summary>
    /// Invoke a task repeatly with a specify interval.
    /// </summary>
    /// <param name="delay">The first invoke time in second.</param>
    /// <param name="interval">Each repeat but the first interval in second.</param>
    /// <param name="task">The task to execute.</param>
    public Timer RepeatTime(float delay, float interval, Action task)
    {
        var timer = new Timer(delay, interval, false, task);
        toBeAddedtimers.AddLast(timer);
        return timer;
    }

    /// <summary>
    /// Repeats to invoke a task.
    /// </summary>
    /// <param name="interval">Each repeat interval in unscaled second.</param>
    /// <param name="task">The task to execute.</param>
    public Timer RepeatTimeUnscale(float interval, Action task)
    {
        var timer = new Timer(interval, interval, true, task);
        toBeAddedtimers.AddLast(timer);
        return timer;
    }

    /// <summary>
    /// Invoke a task repeatly with a specify interval, using unscaled time.
    /// </summary>
    /// <param name="delay">The first invoke time in unscaled second.</param>
    /// <param name="interval">Each repeat but the first interval in unscaled second.</param>
    /// <param name="task">The task to execute.</param>
    public Timer RepeatTimeUnscale(float delay, float interval, Action task)
    {
        var timer = new Timer(delay, interval, false, task);
        toBeAddedtimers.AddLast(timer);
        return timer;
    }

    /// <summary>
    /// Stop a specify timer.
    /// </summary>
    /// <param name="timer">The timer to be stoped.</param>
    public void StopTimer(Timer timer)
    {
        toBeDeletedtimers.AddLast(timer);
    }


    /// <summary>
    /// 重新启动某个timer
    /// </summary>
    /// <param name="timer"></param>
    public void ReStartTimer(Timer timer)
    {
        timer.ResetTime();
        toBeAddedtimers.AddLast(timer);
    }

    /// <summary>
    /// Update and check each frame.
    /// </summary>
    public void Update()
    {
        foreach (var timer in toBeAddedtimers)
        {
            timers.AddLast(timer);
        }

        toBeAddedtimers.Clear();

        var timerLinkNode = this.timers.First;
        while (null != timerLinkNode)
        {
            var nextTimerLinkNode = timerLinkNode.Next;
            var timer = timerLinkNode.Value;

            if (toBeDeletedtimers.Find(timer) != null)
            {
                continue;
            }

            if (timer.Update())
            {
                this.timers.Remove(timer);
            }

            timerLinkNode = nextTimerLinkNode;
        }

        foreach (var timer in toBeDeletedtimers)
        {
            timers.Remove(timer);
        }

        toBeDeletedtimers.Clear();
    }

    public class Timer
    {
        /// <summary>
        /// 用于记录原始的数据
        /// </summary>
        private float mLeftTime;
        private float mRepeatTime;

        /// <summary>
        /// 真实进行计算的数据
        /// </summary>
        private float leftTime;
        private float repeatTime;
        private bool unscale;
        private Action task;

        public Timer(
            float leftTime, float repeatTime, bool unscale, Action task)
        {
            this.mLeftTime = leftTime;
            this.mRepeatTime = repeatTime;
            this.leftTime = leftTime;
            this.repeatTime = repeatTime;
            this.unscale = unscale;
            this.task = task;
        }

        public void ResetTime()
        {
            this.leftTime = this.mLeftTime;
            this.repeatTime = this.mRepeatTime;
        }

        public bool Update()
        {
            if (this.unscale)
            {
                this.leftTime -= Time.unscaledDeltaTime;
            }
            else
            {
                this.leftTime -= Time.deltaTime;
            }

            if (this.leftTime <= 0.0f)
            {
                this.task();

                if (repeatTime <= 0.0f)
                {
                    return true;
                }
                else
                {
                    this.leftTime = repeatTime;
                }
            }

            return false;
        }
    }
}
