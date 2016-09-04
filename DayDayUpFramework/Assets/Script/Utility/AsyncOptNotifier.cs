using System;
using System.Collections.Generic;

using UnityEngine;

/// <summary>
/// Track and watch async operation finished.
/// </summary>
public class AsyncOptNotifier
{
    // The async operation watcher list.
    private LinkedList<AsyncOperationer> asyncOperationers = new LinkedList<AsyncOperationer>();
    private LinkedList<AsyncOperationer> safeAsyncOperationers = new LinkedList<AsyncOperationer>();


    /// <summary>
    /// Wait and watch an async operation.
    /// </summary>
    /// <param name="asynopt">The operation to be watched.</param>
    /// <param name="progressDelegate">Noticy when the progress changed.</param>
    /// <param name="completeDelegate">Notice when the operation is complete.</param>
    public void WaitAsync(
        AsyncOperation operation,
        Action<float> progressDelegate,
        Action completeDelegate)
    {
        var opt = new AsyncOperationer();
        opt.Operation = operation;
        opt.ProgressDelegate = progressDelegate;
        opt.CompleteDelegate = completeDelegate;
        opt.LastProgress = 0.0f;
        this.safeAsyncOperationers.AddLast(opt);
    }

    public void WaitAsync(
        AsyncOperation operation,
        Action completeDelegate)
    {
        var opt = new AsyncOperationer();
        opt.Operation = operation;
        opt.ProgressDelegate = null;
        opt.CompleteDelegate = completeDelegate;
        opt.LastProgress = 0.0f;
        this.safeAsyncOperationers.AddLast(opt);
    }

    /// <summary>
    /// Update and check each frame.
    /// </summary>
    public void Update()
    {
        foreach (var linkNode in safeAsyncOperationers)
        {
            asyncOperationers.AddLast(linkNode);
        }

        safeAsyncOperationers.Clear();

        var operationerLinkNode = this.asyncOperationers.First;
        while (null != operationerLinkNode)
        {
            var nextOperationerLinkNode = operationerLinkNode.Next;
            var operationer = operationerLinkNode.Value;

            if (operationer.Operation.isDone)
            {
                this.asyncOperationers.Remove(operationerLinkNode);
                try
                {
                    operationer.CompleteDelegate();
                }
                catch (Exception e)
                {
                    Debug.LogError(e.ToString());
                }
            }
            else
            {
                if (operationer.LastProgress != operationer.Operation.progress)
                {
                    if (null != operationer.ProgressDelegate)
                    {
                        try
                        {
                            operationer.ProgressDelegate(operationer.Operation.progress);
                        }
                        catch (Exception e)
                        {
                            Debug.LogError(e.ToString());
                        }
                    }

                    operationer.LastProgress = operationer.Operation.progress;
                }
            } 

            operationerLinkNode = nextOperationerLinkNode;
        }
    }

    private class AsyncOperationer
    {
        public AsyncOperation Operation { get; set; }

        public Action<float> ProgressDelegate { get; set; }

        public Action CompleteDelegate { get; set; }

        public float LastProgress { get; set; }
    }
}
