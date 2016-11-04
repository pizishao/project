using UnityEngine;
using System.Collections;

public class test : MonoBehaviour
{
    [SerializeField]
    UIWindow redWindowPrefab;

    [SerializeField]
    UIWindow blueWindowPrefab;

    UIWindow redWindow;
    UIWindow blueWindow;

    // Use this for initialization
    void Start () 
    {        
        /*AssetManager.Instance.DownLoadURL = "http://192.168.0.51/AssetStream/";
        AssetManager.Instance.DownLoadManifest(delegate(string error)
        {
            if (!string.IsNullOrEmpty(error))
            {
                Debug.Log(error);
                return;
            }            
        });*/        
	}
	
    void OnGUI()
    {
        if (GUILayout.Button("TEST"))
        {
            /*Caching.CleanCache();

            AssetManager.Instance.LoadAsset<GameObject>("Cube", "cube", delegate(string err, GameObject obj)
            {
                if (!string.IsNullOrEmpty(err))
                {
                    Debug.Log(err);
                    return;
                }

                Instantiate(obj);
            });*/

            redWindow = GameObject.Instantiate<GameObject>(redWindowPrefab.gameObject).GetComponent<UIWindow>();
            blueWindow = GameObject.Instantiate<GameObject>(blueWindowPrefab.gameObject).GetComponent<UIWindow>();

            redWindow.transform.localScale = Vector3.one;
            redWindow.transform.localRotation = Quaternion.identity;
            redWindow.transform.localPosition = Vector3.zero;

            blueWindow.transform.localScale = Vector3.one;
            blueWindow.transform.localRotation = Quaternion.identity;
            blueWindow.transform.localPosition = Vector3.zero;

            redWindow.OriginalLayer = WindowLayer.UIMain /*WindowLayer.ClickScreenEffect*/;
            blueWindow.OriginalLayer = WindowLayer.UIMainView /*WindowLayer.ChatMessage*/;
            UIWindowManager.Instance.PushWindow(redWindow);
            UIWindowManager.Instance.PushWindow(blueWindow);

            redWindow.transform.SetParent(GameObject.Find("UIWindow").transform);
            blueWindow.transform.SetParent(GameObject.Find("UIWindow").transform);
        }
    }

	// Update is called once per frame
	void Update () 
    {
	
	}
}
