using UnityEngine;
using System.Collections;

public class test : MonoBehaviour {

	// Use this for initialization
	void Start () 
    {        
        AssetManager.Instance.DownLoadURL = "http://192.168.0.51/AssetStream/";
        AssetManager.Instance.DownLoadManifest(delegate(string error)
        {
            if (!string.IsNullOrEmpty(error))
            {
                Debug.Log(error);
                return;
            }            
        });        
	}
	
    void OnGUI()
    {
        if (GUILayout.Button("TEST"))
        {
            Caching.CleanCache();

            AssetManager.Instance.LoadAsset<GameObject>("Cube", "cube", delegate(string err, GameObject obj)
            {
                if (!string.IsNullOrEmpty(err))
                {
                    Debug.Log(err);
                    return;
                }

                Instantiate(obj);
            });   
        }        
    }

	// Update is called once per frame
	void Update () 
    {
	
	}
}
