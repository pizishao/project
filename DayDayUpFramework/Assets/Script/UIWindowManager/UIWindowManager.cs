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

    void LoadWindow<T>(string assetName, string bundleName)
    {

    }

    public void PushWindow(string assetName, string bundleName, Action<UIWindow> onComplete)
    {
        AssetManager.Instance.LoadAsset<UIWindow>(assetName, bundleName, delegate(string error, UIWindow window)
        {
            if (!string.IsNullOrEmpty(error))
	        {
		        Debug.LogError(string.Format("load window at {0}:{1} failed!!!", assetName, bundleName));
                return;
	        }

            UIWindow newWindow = GameObject.Instantiate<GameObject>(window.gameObject).GetComponent<UIWindow>();

            newWindow.transform.localScale = Vector3.one;
            newWindow.transform.localRotation = Quaternion.identity;
            newWindow.transform.localPosition = Vector3.zero;

            UIWindowStack stack = null;
            newWindow.DynamicLayer = newWindow.OriginalLayer;

            stacks.TryGetValue(newWindow.OriginalLayer, out stack);

            if (newWindow.OriginalLayer != WindowLayer.UIMain && newWindow.OriginalLayer != WindowLayer.UIMainView)
            {
                WindowLayer highestLayer = GetHighestLayer();
                if ((int)highestLayer > (int)newWindow.OriginalLayer)
                {
                    newWindow.DynamicLayer = highestLayer;
                    stacks.TryGetValue(highestLayer, out stack);
                }
            }

            Assert.IsTrue(stack != null);
            stack.PushWindow(newWindow);

            UIWindow pendingWindow = null;

            if (openWindows.Count > 0)
            {
                pendingWindow = openWindows.Peek();
            }

            openWindows.Push(newWindow);

            if (pendingWindow != null)
            {
                pendingWindow.OnPause();
            }

            if (onComplete != null)
            {
                onComplete(newWindow);
            }

            newWindow.OnShow();             
        });        
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
