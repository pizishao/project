using System;
using System.Collections.Generic;

using UnityEngine;

/// <summary>
/// Unity singleton, it will create an GameObject if it not existed in the 
/// scene.
/// </summary>
/// <typeparam name="T">The singleton class.</typeparam>
public class UnitySingleton<T> : MonoBehaviour where T : MonoBehaviour
{
    private static T instance;
    private static bool applicationIsQuitting = false;

    /// <summary>
    /// Gets the singleton instance, if the singleton MonoBehaviour is not 
    /// existed, it will automatic create a GameObject attach a new 
    /// MonoBehaviour as a singleton instance.
    /// </summary>
    public static T Instance
    {
        get
        {
            if (null == instance)
            {
                // Do not create singleton after the application is 
                // quitting.
                if (applicationIsQuitting)
                {
                    Debug.LogWarning("[Singleton] Instance '" + typeof(T) +
                        "' already destroyed on application quit." +
                        " Won't create again - returning null.");
                    return null;
                }

                // Do not create singleton if the application is not 
                // playing.
                if (!Application.isPlaying)
                {
                    Debug.LogWarning("[Singleton] Instance '" + typeof(T) +
                        " can not create in edit mode.");
                    return null;
                }

                // Find the object or create a new one.
                instance = GameObject.FindObjectOfType<T>();
                if (null == instance)
                {
                    var go = new GameObject(typeof(T).Name, typeof(T));
                    MonoBehaviour.DontDestroyOnLoad(go);
                    instance = go.GetComponent<T>();
                }
            }

            return instance;
        }
    }

    public static T GetInstance()
    {
        return Instance;
    }

    /// <summary>
    /// Release this singleton reference when the singleton destroyed.
    /// </summary>
    protected void OnDestroy()
    {
        instance = default(T);
        applicationIsQuitting = true;
    }
}
