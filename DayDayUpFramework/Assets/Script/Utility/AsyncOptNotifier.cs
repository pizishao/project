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
    private LinkedList<AsyncOperationer> awaitAsyncOperationers = new LinkedList<AsyncOperationer>();

    private LinkedList<AsyncWWW> asyncWWWList = new LinkedList<AsyncWWW>();
    private LinkedList<AsyncWWW> awaitAsyncWWWList = new LinkedList<AsyncWWW>();


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
        this.awaitAsyncOperationers.AddLast(opt);
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
        this.awaitAsyncOperationers.AddLast(opt);
    }

    public void WaitWWW(WWW www, Action<string> onComplete)
    {
        AsyncWWW asyncWWW = new AsyncWWW();
        asyncWWW.www = www;
        asyncWWW.CompleteDelegate = onComplete;

        awaitAsyncWWWList.AddLast(asyncWWW);
    }

    /// <summary>
    /// Update and check each frame.
    /// </summary>
    public void Update()
    {
        foreach (var linkNode in awaitAsyncOperationers)
        {
            asyncOperationers.AddLast(linkNode);
        }

        awaitAsyncOperationers.Clear();

        foreach (var item in awaitAsyncWWWList)
        {
            asyncWWWList.AddLast(item);
        }

        awaitAsyncWWWList.Clear();

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

        var asyncWWWLinkNode = this.asyncWWWList.First;
        while (asyncWWWLinkNode != null)
        {
            var nextAsyncWWWLinkNode = asyncWWWLinkNode.Next;
            var asyncWWW = asyncWWWLinkNode.Value;

            if (asyncWWW.www.isDone)
            {
                asyncWWWList.Remove(asyncWWWLinkNode);

                try
                {
                    asyncWWW.CompleteDelegate(asyncWWW.www.error);
                }
                catch (Exception e)
                {
                    Debug.LogError(e.ToString());
                }                
            }

            asyncWWWLinkNode = nextAsyncWWWLinkNode;
        }
    }

    private class AsyncOperationer
    {
        public AsyncOperation Operation { get; set; }

        public Action<float> ProgressDelegate { get; set; }

        public Action CompleteDelegate { get; set; }

        public float LastProgress { get; set; }
    }

    private class AsyncWWW
    {
        public WWW www
        {
            get;
            set;
        }

        public Action<string> CompleteDelegate
        {
            get;
            set;
        }
    }
}
