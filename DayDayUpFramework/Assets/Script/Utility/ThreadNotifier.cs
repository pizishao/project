using System;
using System.Collections;
using System.Threading;
using System.Collections.Generic;

using UnityEngine;

public class ThreadNotifier
{
    Queue<Action> actionQueue = new Queue<Action>();

    void Post(Action task)
    {
        lock (actionQueue)
        {
            actionQueue.Enqueue(task);
        }
    }

    void Dispatch()
    {
        lock (actionQueue)
        {
            while (actionQueue.Count > 0)
            {
                var task = actionQueue.Dequeue();
                if (task != null)
                {
                    try
                    {
                        task();                           
                    }
                    catch (Exception ex)
                    {
                        Debug.LogError(ex.ToString());
                    }
                }
            }            
        }
    }
}
