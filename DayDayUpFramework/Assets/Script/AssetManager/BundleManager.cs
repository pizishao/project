using UnityEngine;

using System;
using System.Collections;
using System.Collections.Generic;

enum BundleState
{
    Loading,
    Loaded, 
    LoadFailed,
}

class Bundle
{
    public string Name { get; set; }
    public AssetBundle assetBundle { get; set; }
    public BundleState State { get; set; }
    public WWW www;
}

class AssetLoadRequest
{
    public event Action<string, AssetBundle> onComplete;
    private List<Bundle> dependencyBundles = new List<Bundle>();
    public string AssetName { get; set; }

    public bool Completed
    {
        get
        {
            foreach (var dependBundle in dependencyBundles)
            {
                if (dependBundle.State == BundleState.LoadFailed)
                {
                    return true;
                }
                else if (dependBundle.State == BundleState.Loading)
                {
                    return false;
                }
            }

            return true;
        }
    }

    public void AddDependentBundle(Bundle bundle)
    {
        dependencyBundles.Add(bundle);
    }

    public void Notify()
    {
        foreach (var dependBundle in dependencyBundles)
        {
            if (dependBundle.State == BundleState.LoadFailed)
            {
                if (onComplete != null)
                {
                    onComplete(dependBundle.www.error, null);                    
                }
                return;
            }
            else if (dependBundle.State == BundleState.Loading)
            {
                return;
            }
        }

        if (onComplete != null)
        {
            onComplete(string.Empty, dependencyBundles[0].assetBundle);
        }
    }
}

public class BundleManager
{
    public string DownLoadURL { get; set; }

    private AssetBundleManifest manifest;
    private string manifestName="";

    Dictionary<string, Bundle> Bundles = new Dictionary<string,Bundle>();
    Dictionary<string, AssetLoadRequest> requests = new Dictionary<string, AssetLoadRequest>();

    void ParseManifestName()
    {
        DownLoadURL = DownLoadURL.TrimEnd('/');
        string [] splitStr = DownLoadURL.Split('/');
        if (splitStr.Length > 0)
        {
            manifestName = splitStr[splitStr.Length - 1];
        }

        DownLoadURL += "/";
    }

    public void DownLoadMainfest(Action<string> onComplete)
    {
        ParseManifestName();

        WWW www = new WWW(DownLoadURL + manifestName);
        Scheduler.Instance.WaitWWW(www, delegate(string error)
        {
            if (string.IsNullOrEmpty(error))
            {
                manifest = www.assetBundle.LoadAsset<AssetBundleManifest>("AssetBundleManifest");
            }

            onComplete(error);
            www.Dispose();
        });        
    }

    Bundle GetBundle(string name)
    {
        Bundle bundle = null;

        if (Bundles.TryGetValue(name, out bundle))
        {
            return bundle;
        }

        return null;
    }

    Bundle DownLoadBundle(string bundleName)
    {
        Hash128 bundleHash = manifest.GetAssetBundleHash(bundleName);
        string url = DownLoadURL+bundleName;

        WWW www = WWW.LoadFromCacheOrDownload(url, bundleHash);

        Bundle bundle = new Bundle();
        bundle.Name = bundleName;
        bundle.State = BundleState.Loading;
        bundle.www = www;
        bundle.assetBundle = null;

        Bundles.Add(bundleName, bundle);

        return bundle;
    }    

    public void LoadAsset(string assetName, string bundleName, Action<string, AssetBundle> onComplete)
    {
        AssetLoadRequest request = null;

        if (requests.TryGetValue(assetName, out request))
        {
            request.onComplete += onComplete;
            return;
        }

        AssetLoadRequest newRequest = new AssetLoadRequest();
        newRequest.onComplete += onComplete;
        newRequest.AssetName = assetName;

        Bundle bundle = GetBundle(bundleName);
        if (bundle == null)
        {
            bundle = DownLoadBundle(bundleName);            
        }
        newRequest.AddDependentBundle(bundle);

        string[] dependencyBundleNames = manifest.GetAllDependencies(bundleName);  
      
        foreach (var name in dependencyBundleNames)
        {
            Bundle dependBundle = GetBundle(name);

            if (dependBundle == null)
            {
                dependBundle = DownLoadBundle(name);
            }

            newRequest.AddDependentBundle(dependBundle);
        }

        requests.Add(assetName, newRequest);              
    }

    List<Bundle> GetCompleteBundles()
    {
        List<Bundle> ls = new List<Bundle>();

        foreach (var bundle in Bundles.Values)
        {
            if (bundle.State == BundleState.Loading)
            {
                if (bundle.www.isDone)
                {
                    if (string.IsNullOrEmpty(bundle.www.error))
                    {
                        bundle.assetBundle = bundle.www.assetBundle;
                        bundle.State = BundleState.Loaded;
                    }
                    else
                    {
                        bundle.State = BundleState.LoadFailed;
                    }

                    ls.Add(bundle);
                }
            }
        }

        return ls;
    }

    List<Bundle> GetFailedBundles()
    {
        List<Bundle> ls = new List<Bundle>();

        foreach (var bundle in Bundles.Values)
        {
            if (bundle.State == BundleState.LoadFailed)
            {
                ls.Add(bundle);
            }
        }

        return ls;
    }

    void ClearFailedBundles()
    {
        List<Bundle> failedBundles = GetFailedBundles();
        foreach (var bundle in failedBundles)
        {
            Bundles.Remove(bundle.Name);
        }
    }

    List<AssetLoadRequest> GetCompleteRequests()
    {
        List<AssetLoadRequest> completeRequests = new List<AssetLoadRequest>();

        foreach (var request in requests.Values)
        {
            if (request.Completed)
            {
                completeRequests.Add(request);
            }
        }

        return completeRequests;
    }

    void NotifyRequests(List<AssetLoadRequest> reqs)
    {
        foreach (var request in reqs)
        {
            requests.Remove(request.AssetName);
            request.Notify();
        }
    }

    void DisposeBundles(List<Bundle> bundles)
    {
        foreach (var bundle in bundles)
        {
            bundle.www.Dispose();
        }
    }

    public void Update()
    {        
        List<Bundle> completeBundles = GetCompleteBundles();
        List<AssetLoadRequest> completeRequests = GetCompleteRequests();

        ClearFailedBundles();
        NotifyRequests(completeRequests);
        DisposeBundles(completeBundles);
    }
}
