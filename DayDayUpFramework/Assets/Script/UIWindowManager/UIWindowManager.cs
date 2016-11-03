using UnityEngine;
using UnityEngine.Assertions;

using System;
using System.Collections;
using System.Collections.Generic;

public class UIWindowManager : Singleton<UIWindowManager>
{
    Dictionary<WindowLayer, UIWindowStack> stacks = new Dictionary<WindowLayer, UIWindowStack>();
    Stack<UIWindow> openWindows = new Stack<UIWindow>();

    public UIWindowManager()
    {
        for (int i = (int)WindowLayer.None; i < Enum.GetNames(typeof(WindowLayer)).Length; i++)
        {
            UIWindowStack stack = new UIWindowStack();
            stack.Layer = (WindowLayer)i;
            stacks.Add((WindowLayer)i, stack);
        }
    }

    WindowLayer GetHighestLayer()
    {
        WindowLayer highestLayer = WindowLayer.None;

        foreach (var window in openWindows)
        {
            if (window.OriginalLayer > highestLayer)
            {
                highestLayer = window.OriginalLayer;
            }
        }

        return highestLayer;
    }

    void PushWindow(UIWindow window)
    {
        UIWindowStack stack = null;
        window.MutableLayer = window.OriginalLayer;

        if (window.OriginalLayer != WindowLayer.UIMain && window.OriginalLayer != WindowLayer.UIMainView)
        {
            WindowLayer highestLayer = GetHighestLayer();
            if (highestLayer > window.OriginalLayer)
            {
                window.MutableLayer = highestLayer;
                stacks.TryGetValue(highestLayer, out stack);
            }
        }
        else
        {
            stacks.TryGetValue(window.OriginalLayer, out stack);
        }

        Assert.IsTrue(stack != null);
        stack.PushWindow(window);
    }

    void PopWindow(UIWindow window)
    {
        UIWindowStack stack = null;

        stacks.TryGetValue(window.MutableLayer, out stack);
        Assert.IsTrue(stack != null);

        if (stack != null)
        {
            ;
        }
    }
}
