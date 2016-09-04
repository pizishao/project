using System;
using System.Collections.Generic;

using UnityEngine;

/// <summary>
/// Keep task from main thread and delay invoke them next frame.
/// </summary>
public class NextFrameNotifier
{
    private LinkedList<Action> nextFrameTasks =
        new LinkedList<Action>();

    // .当前执行任务列表
    private List<Action> currFrameTasks =
        new List<Action>();

    /// <summary>
    /// Delay a task and execute it next frame.
    /// </summary>
    public void WaitNextFrame(Action task)
    {
        this.nextFrameTasks.AddLast(task);
    }

    /// <summary>
    /// Update and check each frame.
    /// </summary>
    public void Update()
    {
        this.currFrameTasks.AddRange(nextFrameTasks);
        nextFrameTasks.Clear();

        foreach (var task in currFrameTasks)
        {
            try
            {
                task.Invoke();
            }
            catch (Exception e)
            {
                Debug.LogError(e.ToString());
            }
        }

        currFrameTasks.Clear();
    }
}
