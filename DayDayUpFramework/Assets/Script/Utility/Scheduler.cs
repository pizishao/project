using System;
using System.Collections.Generic;
using UnityEngine;

/// <summary>
/// The singleton scheduler used to schedule tasks, include:
/// * Frame update event.
/// * Work thread send notify task to main thread.
/// * Send task to next frame.
/// * Wait an AsyncOpt finish.
/// * Wait a specify of time to execute.
/// </summary>
[DisallowMultipleComponent]
public class Scheduler : UnitySingleton<Scheduler>
{
    private ThreadNotifier threadNotifier = new ThreadNotifier();
    private NextFrameNotifier nextFrameNotifier = new NextFrameNotifier();
    private AsyncOptNotifier asyncOptNotifier = new AsyncOptNotifier();
    private TimeNotifier timeNotifier = new TimeNotifier();

    /// <summary>
    /// The frame event invoked each frame.
    /// </summary>
    public Action FrameEvent;

    /// <summary>
    /// The level loaded event invoked after level loaded.
    /// </summary>
    public event Action LevelLoadedEvent;

    /// <summary>
    /// Work thread post a task to the main thread.
    /// </summary>
    /// <see cref="ThreadNotifier.Post"/>
    public void PostTask(Action task)
    {
        this.threadNotifier.Post(task);
    }

    /// <summary>
    /// Delay a task and execute it next frame.
    /// </summary>
    /// <see cref="NextFrameNotifier.WaitNextFrame"/>
    public void WaitNextFrame(Action task)
    {
        this.nextFrameNotifier.WaitNextFrame(task);
    }

    /// <summary>
    /// Wait and watch an asynchronous operation.
    /// </summary>
    /// <see cref="AsyncOptNotifier.WaitAsync"/>
    public void WaitAsync(
        AsyncOperation operation,
        Action<float> progressDelegate,
        Action completeDelegate)
    {
        this.asyncOptNotifier.WaitAsync(
            operation, progressDelegate, completeDelegate);
    }

    /// <summary>
    /// Wait and watch an asynchronous operation, without the need of 
    /// progress notify.
    /// </summary>
    /// <see cref="AsyncOptNotifier.WaitAsync"/>
    public void WaitAsync(
        AsyncOperation operation,
        Action completeDelegate)
    {
        this.asyncOptNotifier.WaitAsync(
            operation, completeDelegate);
    }

    public void WaitWWW(WWW www, Action<string> onComplete)
    {
        this.asyncOptNotifier.WaitWWW(www, onComplete);
    }

    /// <summary>
    /// Wait time and execute a task.
    /// </summary>
    /// <see cref="TimeNotifier.WaitTime"/>
    public TimeNotifier.Timer WaitTime(
        float seconds, Action task)
    {
        return this.timeNotifier.WaitTime(
            seconds, task);
    }

    /// <summary>
    /// Wait unscaled time and execute a task.
    /// </summary>
    /// <see cref="TimeNotifier.WaitTimeUnscale"/>
    public TimeNotifier.Timer WaitTimeUnscale(
        float seconds, Action task)
    {
        return this.timeNotifier.WaitTimeUnscale(
            seconds, task);
    }

    /// <summary>
    /// Invoke a task repeatedly with a specify interval.
    /// </summary>
    /// <see cref="TimeNotifier.RepeatTime"/>
    public TimeNotifier.Timer RepeatTime(
        float interval, Action task)
    {
        return this.timeNotifier.RepeatTime(
            interval, task);
    }

    /// <summary>
    /// Invoke a task repeatedly with a specify interval.
    /// </summary>
    /// <see cref="TimeNotifier.RepeatTime"/>
    public TimeNotifier.Timer RepeatTime(
        float delay, float interval, Action task)
    {
        return this.timeNotifier.RepeatTime(
            delay, interval, task);
    }

    /// <summary>
    /// Repeats to invoke a task with a specify interval, using unscaled time.
    /// </summary>
    /// <see cref="TimeNotifier.RepeatTimeUnscale"/>
    public TimeNotifier.Timer RepeatTimeUnscale(
        float interval, Action task)
    {
        return this.timeNotifier.RepeatTimeUnscale(
            interval, task);
    }

    /// <summary>
    /// Repeats to invoke a task with a specify interval, using unscaled time.
    /// </summary>
    /// <see cref="TimeNotifier.RepeatTimeUnscale"/>
    public TimeNotifier.Timer RepeatTimeUnscale(
        float delay, float interval, Action task)
    {
        return this.timeNotifier.RepeatTimeUnscale(
            delay, interval, task);
    }

    /// <summary>
    /// Stop a specify timer.
    /// </summary>
    /// <param name="timer">The timer to be stopped.</param>
    public void StopTimer(TimeNotifier.Timer timer)
    {
        if (timer != null)
        {
            this.timeNotifier.StopTimer(timer);
        }
        else
        {
            Debug.LogError("StopTimer timer can't be null");
        }
    }

    /// <summary>
    /// 重新启动某个timer
    /// </summary>
    /// <param name="timer"></param>
    public void ReStartTimer(TimeNotifier.Timer timer)
    {
        if (timer != null)
        {
            this.timeNotifier.ReStartTimer(timer);
        }
        else
        {
            Debug.LogError("ReStartTimer timer can't be null");
        }
    }

    private void Awake()
    {
        GameObject.DontDestroyOnLoad(this);
    }

    private void Update()
    {
        if (null != this.FrameEvent)
        {
            this.FrameEvent();
        }

        this.threadNotifier.Update();
        this.nextFrameNotifier.Update();
        this.asyncOptNotifier.Update();
        this.timeNotifier.Update();
    }

    private void OnLevelWasLoaded(int index)
    {
        if (null != this.LevelLoadedEvent)
        {
            this.LevelLoadedEvent();
        }
    }
}
