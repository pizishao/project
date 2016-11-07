using UnityEngine;
using System.Collections;

public class redWindow : UIWindow 
{
	// Use this for initialization
	void Start () 
    {
	
	}
	
	// Update is called once per frame
	void Update () 
    {
	
	}

    public override void OnClose()
    {
        base.OnClose();
    }

    public override void OnPause()
    {
        Debug.Log(string.Format("redWindow OnPause"));
    }

    public override void OnResume()
    {
        Debug.Log(string.Format("redWindow OnResume"));
    }

    public override void OnShow()
    {
        Debug.Log(string.Format("redWindow OnShow"));
    }
}
