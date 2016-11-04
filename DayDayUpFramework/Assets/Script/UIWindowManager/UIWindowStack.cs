using UnityEngine;

using System;
using System.Collections;
using System.Collections.Generic;

public class UIWindowStack 
{
    public WindowLayer Layer { get; set; }
    Stack<UIWindow> stackWindow = new Stack<UIWindow>();

    public void PushWindow(UIWindow window)
    {
        if (window != null)
        {
            stackWindow.Push(window);
        }

        UpdateSortingOrder();
    }

    public void UpdateSortingOrder()
    {
        int i = stackWindow.Count;

        foreach (var window in stackWindow)
        {
            window.IndexInLayer = i;
            i--;
        }
    }
}
