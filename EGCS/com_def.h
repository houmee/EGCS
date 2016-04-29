/******************************************************************** 
修改时间:        2016/03/20 16:35
文件名称:        com_def.h
文件作者:        huming 
=====================================================================
功能说明:        公共文件定义 
--------------------------------------------------------------------- 
版本编号:        0.1
--------------------------------------------------------------------- 
其他说明:         
*********************************************************************/

/*********************************************************************
 * INCLUDES
 */
#ifndef		COM_DEF_H
#define		COM_DEF_H

#include "com_type.h"

/*********************************************************************
 * DEFINE
 */
#define ELVT_UP(x)    ( x==UP_ACC   || x==UP_CONST   || x==UP_DEC   || x== UP_PAUSE )
#define ELVT_DOWN(x)  ( x==DOWN_ACC || x==DOWN_CONST || x==DOWN_DEC || x== DOWN_PAUSE )
#define ELVT_STOP(x)  ( x==IDLE || x==UP_PAUSE || x==DOWN_PAUSE )

/*********************************************************************
 * GLOBAL VALUE
 */
extern double gSystemTime;
extern uint8  gBatch;

/*********************************************************************
 * CONSTANTS
 */
#define ELEVATOR_HEIGHT			28				//电梯箱体高度
#define MAX_OUT_REQUEST     10        //一个电梯接收最大的请求数目
#define MAX_PRIORITY_NUM	  100				//电梯运行列表最大优先级
#define MAX_PSG_FLOW_NUM    200       //乘客流最大人数
#define SYSTEM_TIME_STEP    1       //系统时间递增步长(s)
#define GRAVITY_ACCELERATE  9.7       //重力加速度
#define MAX_WAIT_TIME       1000
#define MAX_ENERGY          999999
#define PSG_ARRIVE_PLACE    0x7F

//匀速运行
#define ONE_FLOOR_TIME      2         //电梯匀速一层楼时间(s)
//加减速运行
#define CONST_SPEED_TIME    1.33      //匀速时间(s)
#define ACCELERATE_TIME     1         //加速时间(s)
#define DECELERATE_TIME     1         //加速时间(s)
#define REMAIN_GAP_TIME     1.67      //一层间距加减速剩余距离匀速移动距时间s)
#define CONST_SPEED_LENGTH  2         //匀速移动距离(m)
#define ACCELERATE_LENGTH   0.5       //减速移动距离(m)
#define DECELERATE_LENGTH   0.5       //加速移动距离(m)
#define REMAIN_GAP_LENGTH   2.5       //加减速剩余距离(m)

//运行参数
#define START_STOP_ENERGY   22500     //启停能耗(J)
#define CAR_WEIGHT          800       //轿厢重量(kg)
#define COUNT_WEIGHT        900       //配重质量(kg)
#define NET_CAR_WEIGHT      -100      //轿厢与配置净重质量(kg)
#define PSG_AVG_WEIGHT      65        //乘客平均质量(kg)
#define MAX_INNER_PSG_NUM   10        //电梯内最多人数
#define MAX_FLOOR_NUM				10				//楼层数
#define MAX_ELEVATOR_NUM		2				  //电梯（箱体）数
#define FLOOR_HEIGHT        3         //楼层间高度(m)
#define RUN_SPEED           1.5       //电梯运行速度(m/s)
#define ACCELERATE_SPEED    1         //电梯加速度(m/s2)
#define JERK_SPEED          1.8       //电梯加加速度(m/s3)
#define WEIGHT_CAPACITY     1000      //电梯最大载重(kg)
#define OPEN_CLOSE_TIME			2				  //开关门时间(s)
#define PSG_ENTER_TIME			1				  //乘客进入/离开时间时间(s)
    

//#define  TEST
/*********************************************************************
 * ENUMS
 */
enum FlowType			          //乘客流类型
{
  UP_FLOW,
  DOWN_FLOW,
  RAND_FLOW
};

enum ReqType			          //呼号请求类型
{
  OUT_REQ,
  IN_REQ
};

enum ElvtSchedState			//电梯箱体调度状态
{
  ACTIVE,
  SUSPEND
};

enum ElvtRunState			//电梯箱体运行状态
{
	IDLE,				        //待机状态0
  UP_ACC,             //向上加速1
  UP_CONST,           //向上匀速2
  UP_DEC,             //向上加速3
  UP_PAUSE,			      //向上暂停4
  DOWN_ACC,           //向下加速5
  DOWN_CONST,         //向下匀速6
  DOWN_DEC,           //向下减少7
	DOWN_PAUSE,			    //向下暂停8
	MALFUNCTION				  //故障9

  //IDLE,				        //待机状态
  //UP_RUN,             //向上运行
  //UP_PAUSE,			      //向上暂停
  //DOWN_RUN,           //向下运行
  //DOWN_PAUSE,			    //向下暂停
  //MALFUNCTION				  //故障
};

enum RunDir			//电梯运行方向（趋势）
{
	DIR_DOWN,				    //向上
	DIR_UP,					    //向下
	DIR_NONE				  //无方向性
};

enum EvtType			//事件类型
{
	EVT_PASSAGER_IN,		//乘客进入
	EVT_PASSAGER_OUT    //乘客离开
};

enum PsgState			//乘客状态
{
  PSG_NONE,
  PSG_WAIT,		    //等待
  PSG_TRAVEL,     //乘梯
  PSG_ARRIVE      //到达目的层
};

/*********************************************************************
 * TYPEDEFS
 */
typedef struct TargetVal
{ 
  double m_fWaitTime;
  double m_fEnergy;
} sTargetVal;

typedef struct OutRequest
{ 
  uint16		    m_iPassagerID;      //请求乘客编号
	RunDir     	  m_eReqDir;          //外部请求方向
	uint8		      m_iReqCurFlr;       //外部请求所在楼层号
  uint8		      m_iReqDestFlr;      //外部请求目的楼层号
	double		    m_iReqTime;         //外部请求时间
  uint16        m_iReqNum;          //相同请求同一时间请求次数

  bool operator <(const OutRequest& rhs) const // 升序排序时必须写的函数
  {
    return (m_iReqCurFlr < rhs.m_iReqCurFlr);
  }

  bool operator >(const OutRequest& rhs) const // 降序排序时必须写的函数
  {
    return (m_iReqCurFlr > rhs.m_iReqCurFlr);
  }

  bool operator == (const OutRequest& rhs) const
  {
    return (m_iReqCurFlr == rhs.m_iReqCurFlr);
  }
} sOutRequest;

typedef struct  Passenger
{ 
  uint16	  m_iPsgID;          //乘客序号
  uint8		  m_iReqCurFlr;      //请求所在楼层号
  uint8		  m_iDestFlr;        //目的楼层
  RunDir    m_eReqDir;         //外部请求方向
  double    m_dReqTime;        //请求时间
  double    m_dWaitTime;       //等待时间
  double	  m_dAllTime;        //乘梯时间
  PsgState  m_ePsgState;       //乘客状态
  uint8     m_iCurPlace;       //乘客所在位置

  bool operator <(const Passenger& rhs) const // 升序排序时必须写的函数
  {
    return (m_dReqTime < rhs.m_dReqTime);
  }

  bool operator >(const Passenger& rhs) const // 降序排序时必须写的函数
  {
    return (m_dReqTime > rhs.m_dReqTime);
  }

  bool operator == (const Passenger& rhs) const
  {
    return (m_iPsgID == rhs.m_iPsgID);
  }

} sPassengerInfo;

typedef struct
{
  uint8		m_iEvtid;          //事件序号
	EvtType m_eType;           //事件类型
	uint8		m_iPsgID;          //乘客序号
	uint16  m_iEvtTime;        //事件时间
}sEvent;

typedef struct RunItem
{ 
  ReqType     m_eReqType;        //序号请求来源(内部、外部)
  int8        m_iPriority;       //运行优先级
  RunDir		  m_eElvDir;         //运行方向
  uint8       m_iDestFlr;        //运行目的层
  sTargetVal  m_sTarVal;

  bool operator <(const RunItem& rhs) const // 升序排序时必须写的函数
  {
    return (m_iPriority < rhs.m_iPriority);
  }
  bool operator >(const RunItem& rhs) const // 降序排序时必须写的函数
  {
    return (m_iPriority > rhs.m_iPriority);
  }

  bool operator == (const RunItem& rhs) 
  {
    return ( m_iDestFlr == rhs.m_iDestFlr );
  }
}sRunItem;

/*********************************************************************
*********************************************************************/
#endif /* COM_DEF_H */