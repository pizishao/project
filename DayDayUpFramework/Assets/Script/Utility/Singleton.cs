using System.Collections;
using UnityEngine;

/// <summary>
/// 普通的单件类，如果这个单件是只在主线程使用，这个比起SingletonSafe来说
/// 效率更好。
/// </summary>
/// <typeparam name="T">具体单件的类型</typeparam>
public class Singleton<T> where T : class, new()
{
    private static T instancce = null;

    public static T Instance
    {
        get
        {
            if (null == instancce)
            {
                instancce = new T();
            }

            return instancce;
        }
    }

    public static T GetInstance()
    {
        return Instance;
    }
}
