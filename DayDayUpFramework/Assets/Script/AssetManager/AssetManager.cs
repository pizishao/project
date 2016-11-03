using UnityEngine;

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

    public void LoadAsset<T>(string assetName, string bundleName, Action<string, T> onComplete)
        where T : UnityEngine.Object
    {
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
