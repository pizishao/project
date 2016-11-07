using UnityEngine;
using UnityEditor;

using System;
using System.Collections;
using System.Collections.Generic;

public class AssetManager : Singleton<AssetManager>
{
    public string DownLoadURL { get; set; }

    BundleManager bundleMgr = new BundleManager();        

    public void DownLoadManifest(Action<string> onComplete)
    {
        Scheduler.Instance.FrameEvent += bundleMgr.Update;
        bundleMgr.DownLoadURL = DownLoadURL;
        bundleMgr.DownLoadMainfest(onComplete);
    }

    void LoadLocalAsset<T>(string assetName, string bundleName, Action<string, T> onComplete)
        where T : UnityEngine.Object
    {
        string[] paths = AssetDatabase.GetAssetPathsFromAssetBundleAndAssetName(assetName, bundleName);
        if (paths == null || paths.Length == 0)
        {
            //Debug.LogError();
            return;
        }

        string assetPath = paths[0];
        T assetObj = AssetDatabase.LoadAssetAtPath(assetPath, typeof(T)) as T;

        if (onComplete != null)
        {
            onComplete(string.Empty, assetObj);
        }
    }

    public void LoadAsset<T>(string assetName, string bundleName, Action<string, T> onComplete)
        where T : UnityEngine.Object
    {
/*
#if UNITY_EDITOR
        LoadLocalAsset(assetName, bundleName, onComplete);
        return;
#endif*/

        bundleMgr.LoadAsset(assetName, bundleName, delegate(string error, AssetBundle assetBundle)
        {
            if (string.IsNullOrEmpty(error))
            {
                AssetBundleRequest request = assetBundle.LoadAssetAsync<T>(assetName);
                Scheduler.Instance.WaitAsync(request, delegate()
                {
                    T asset = (T)request.asset;
                    onComplete(string.Empty, asset);
                });

                return;
            }

            onComplete(error, null);
        });
    }
}
