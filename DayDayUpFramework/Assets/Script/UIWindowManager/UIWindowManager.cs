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
            if (window.DynamicLayer > highestLayer)
            {
                highestLayer = window.DynamicLayer;
            }
        }

        return highestLayer;
    }

    public void PushWindow(UIWindow window)
    {
        UIWindowStack stack = null;
        window.DynamicLayer = window.OriginalLayer;

        stacks.TryGetValue(window.OriginalLayer, out stack);

        if (window.OriginalLayer != WindowLayer.UIMain && window.OriginalLayer != WindowLayer.UIMainView)
        {
            WindowLayer highestLayer = GetHighestLayer();
            if ((int)highestLayer > (int)window.OriginalLayer)
            {
                window.DynamicLayer = highestLayer;
                stacks.TryGetValue(highestLayer, out stack);
            }
        }

        Assert.IsTrue(stack != null);
        stack.PushWindow(window);
        openWindows.Push(window);
    }

    void PopWindow(UIWindow window)
    {
        UIWindowStack stack = null;

        stacks.TryGetValue(window.DynamicLayer, out stack);
        Assert.IsTrue(stack != null);

        if (stack != null)
        {
            ;
        }
    }
}
