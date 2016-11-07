using UnityEngine;
using UnityEngine.Assertions;

using System.Collections;

public enum WindowLayer
{
    /// <summary>
    /// 无效层
    /// </summary>
    None = 0,

    /// <summary>
    /// 场景中3D物体所使用的HUD所在的层.
    /// </summary>
    SceneHUD,

    /// <summary>
    /// 场景的屏幕特效所在的层.
    /// </summary>
    SceneScreenEffect,

    /// <summary>
    /// HUD 显示的菜单
    /// </summary>
    UIHUD_MENU,
    /// <summary>
    /// UI的提示框所在的层.
    /// </summary>
    UIPop,

    /// <summary>
    /// 场景其他
    /// </summary>
    SceneOther,

    /// <summary>
    /// 场景字
    /// </summary>
    SceneLable,

    /// <summary>
    /// 战斗血量、蓝量     
    /// </summary>
    FightState,

    /// <summary>
    /// 战斗UI层
    /// </summary>
    FightUI,

    /// <summary>
    /// 使用提示
    /// </summary>
    EmployMsg,

    /// <summary>
    /// 消息区域层
    /// </summary>
    NewMsg,

    /// <summary>
    /// 副功能层
    /// </summary>
    UIHUDFunction,

    /// <summary>
    /// 用户主界面使用的HUD所在的层.
    /// 副功能层
    /// </summary>
    UIHUD,

    /// <summary>
    /// NPC对话层
    /// </summary>
    NpcDialogue,

    /// <summary>
    /// 被动消息层
    /// </summary>
    PassiveMsg,

    /// <summary>
    /// 主界面层
    /// </summary>
    UIMainView,


    /// <summary>
    /// 主功能层
    /// </summary>
    UIMainFunction,


    OthersUpUIMain,

    /// <summary>
    /// 聊天
    /// </summary>
    ChatMessage,

    /// <summary>
    /// 赌坊
    /// </summary>
    WagerHourse,

    /// <summary>
    /// 排行榜
    /// </summary>
    RankingList,

    /// <summary>
    /// 传闻
    /// </summary>
    Rumour,

    /// <summary>
    /// 我
    /// </summary>
    Myself,

    /// <summary>
    /// 主UI界面所在的层.
    /// </summary>
    UIMain,

    /// <summary>
    /// 新功能开启层
    /// </summary>
    NewSystem,

    /// <summary>
    /// 剧情，对话层
    /// </summary>
    StoryLine,

    /// <summary>
    /// 加载Loading层
    /// </summary>
    Loading,

    /// <summary>
    /// 上浮消息提示
    /// </summary>
    NoticeMsg,

    /// <summary>
    /// 剧情UI所在的层.
    /// </summary>
    UICinema,

    /// <summary>
    /// UI的屏幕特效所在的层.
    /// </summary>
    UIScreenEffect,

    /// <summary>
    /// 网络层
    /// </summary>
    NetWork,

    /// <summary>
    /// 屏幕点击特效
    /// </summary>
    ClickScreenEffect,
}

public class UIWindow : UIBase 
{
    [SerializeField]
    WindowLayer originalLayer = WindowLayer.None;

    WindowLayer dynamicLayer = WindowLayer.None;

    public WindowLayer OriginalLayer 
    {
        set
        {
            originalLayer = value;
        }

        get
        {
            return originalLayer;
        }
    }

    public WindowLayer DynamicLayer 
    {
        set
        {
            dynamicLayer = value;
        }

        get
        {
            return dynamicLayer;
        }
    }

    Canvas attachCanvas;

    public int IndexInLayer 
    {
        set
        {
            if (attachCanvas == null)
            {
                attachCanvas = GetComponent<Canvas>();
                Assert.IsTrue(attachCanvas != null);               
            }

            if (attachCanvas != null)
            {
                attachCanvas.sortingOrder = (int)DynamicLayer * 100 + value;
            }
        }
    }

    /// <summary>
    /// 界面被显示出来
    /// </summary>
    public virtual void OnShow()
    {

    }

    /// <summary>
    /// 界面暂停
    /// </summary>
    public virtual void OnPause()
    {

    }

    /// <summary>
    /// 界面恢复
    /// </summary>
    public virtual void OnResume()
    {

    }

    /// <summary>
    /// 界面被关闭
    /// </summary>
    public virtual void OnClose()
    {

    }
}
