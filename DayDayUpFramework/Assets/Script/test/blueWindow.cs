using UnityEngine;
using System.Collections;

public class blueWindow : UIWindow {

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
        Debug.Log(string.Format("blueWindow OnPause"));
    }

    public override void OnResume()
    {
        Debug.Log(string.Format("blueWindow OnResume"));
    }

    public override void OnShow()
    {
        Debug.Log(string.Format("blueWindow OnShow"));
    }
}
