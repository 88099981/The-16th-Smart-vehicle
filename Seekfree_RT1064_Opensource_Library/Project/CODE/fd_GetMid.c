#include "fd_GetMid.h"

//data
uint8 img[IMG_Y][IMG_X];
EDGE edge[EDGE_MAX];    //边界结构体数组 边界数值为0的时候为丢边
uint8 EdgeNum;    //边沿数
uint8 EdgeLosePos[EDGE_MAX];  //丢边位置 (Y轴           【单帧初始化】
uint8 EdgeLoseNum;   //丢边数                           【单帧初始化】
int16 mid[EDGE_MAX];    //TODO 确定中线数组所需大小      【单帧初始化】
int8 CamError;

//flag
uint8 flag_LoseEdge_part_L; //左部分丢边     【单帧初始化】
uint8 flag_LoseEdge_part_R; //              【单帧初始化】
uint8 flag_LoseEdge_all_L;  //              【单帧初始化】
uint8 flag_LoseEdge_all_R;  //              【单帧初始化】
uint8 flag_Cross;    //十字                 【单帧初始化】
uint8 flag_Round_ARM_L;  //左环岛预位
uint8 flag_Round_ARM_R;  //右环岛预位
uint8 flag_Is_This_Round;   //环岛决策标志  询问电感(调用完记得复位)    （为什么不问问神奇海螺呢？
uint8 flag_Round_in_L;   //左环岛环中 从入环开始置位
uint8 flag_Round_in_R;   //右环岛环中
uint8 flag_MidStart_in_Black;   //Hor_Search 函数中给定起始点在黑区内    【单帧初始化】
uint8 flag_Ver_Search_Finish;   //垂直扫描完成                          【单帧初始化】
uint8 flag_Indct_Help_Me;   //求助电感循迹  调用完记得复位               【单帧初始化】

//TODO 一侧丢边大致采用通过一边按定长补另一边的方式 或者直接使用底中点和顶中点的连线 （具体情况具体分析


//******************************* Base Function *********************************/
void init(void)
{
    //****单帧标志位初始化****
    flag_LoseEdge_part_L=0;
    flag_LoseEdge_part_R=0;
    flag_LoseEdge_all_L=0;
    flag_LoseEdge_all_R=0;
    flag_Cross=0;
    flag_MidStart_in_Black=0;
    flag_Ver_Search_Finish=0;
    flag_Indct_Help_Me=0;
    //************************

    for(uint8 i=0;i<EDGE_MAX;i++)
    {
        EdgeLosePos[i]=0;
        edge[i].Lx=0;
        edge[i].Rx=0;
        mid[i]=0;
    }
}




void Y_Change(void)
{
    uint8 temp;
    for (uint8 i = 0; i < IMG_Y / 2; i++)
    {
        for (uint8 j = 0; j < IMG_X; j++)
        {
            temp = img[i][j];
            img[i][j] = img[IMG_Y - i - 1][j];
            img[IMG_Y - i - 1][j] = temp;
        }
    }
}




void Del_Edge_Data(uint8 p1_y,uint8 p2_y)
{
    for(uint8 i=p1_y;i<=p2_y;i++)
    {
        edge[i].Lx=0;
        edge[i].Rx=0;
    }
}
//使用前两个点连线的斜率设置目标点
void Set_Edge_BySlope(uint8 p1_y,uint8 p2_y,uint8 target_p_y,uint8 l_or_r) //通过因为高度和边沿点是对应的，所以输入纵坐标来表示对应的点
{
    float Slope=0;  //斜率 其实是cot
    
    if(l_or_r)
    {
        Slope=(edge[p1_y].Lx - edge[p2_y].Lx)/(p1_y - p2_y);
        edge[target_p_y].Lx = edge[p1_y].Lx + Slope*target_p_y;
    }
    else
    {
        Slope=(edge[p1_y].Rx - edge[p2_y].Rx)/(p1_y - p2_y);
        edge[target_p_y].Rx = edge[p1_y].Lx + Slope*target_p_y;
    }
}




void Connect(EDGE Target[],uint8 l_or_r,uint8 p1_y,uint8 p2_y) //通过因为高度和边沿点是对应的，所以输入纵坐标来表示对应的点    注意p1_y小于p2_y
{
    float Slope=0;   //斜率 其实是cot
    if(l_or_r)
    {
        if((Target[p1_y].Lx-Target[p2_y].Lx) != 0)  //垂直的时候就不用算了
        {
            Slope=(p1_y-p2_y)/(Target[p1_y].Lx-Target[p2_y].Lx);

            for(int i=1;i<=p2_y-p1_y;i++)
            {
                Target[p1_y+i].Lx=i*Slope+Target[p1_y].Lx;
            }
        }
    }
    else
    {
        if((Target[p1_y].Rx-Target[p2_y].Rx) != 0)  //垂直的时候就不用算了
        {
            Slope=(p1_y-p2_y)/(Target[p1_y].Rx-Target[p2_y].Rx);

            for(int i=1;i<=p2_y-p1_y;i++)
            {
                Target[p1_y+i].Rx=i*Slope+Target[p1_y].Rx;
            }
        }
    }
}





void Mid_Connect(int16 Target[],uint8 p1_y,uint8 p2_y) //通过因为高度和边沿点是对应的，所以输入纵坐标来表示对应的点    注意p1_y小于p2_y
{                                                      //重载 因为mid为int16类型
    float Slope=0;   //斜率 其实是cot
    if((Target[p1_y]-Target[p2_y]) != 0)  //垂直的时候就不用算了
    {
        Slope=(p1_y-p2_y)/(Target[p1_y]-Target[p2_y]);

        for(int i=1;i<=p2_y-p1_y;i++)
        {
            Target[p1_y+i]=i*Slope+Target[p1_y];
        }
    }
}



uint8 Hor_Search_Base(uint8 MidStart,uint8 y)    //水平扫线（最底层） *注意y为相对限制底框的高度 y为内部递归(可能)使用的参数，外部调用切勿赋值
{                                                  //c99 支持默认值
    flag_MidStart_in_Black=0;   //标志位置零

    if(MidStart>IMG_X-X_LIMIT || MidStart<X_LIMIT || y>IMG_Y-2*Y_LIMIT )  //范围验证
    {
        return 0;
    }

    if(img[y+Y_LIMIT][MidStart]==Black) //如果给定的中点在黑区内
    {
        uint8 Sum_of_white_zone=0;  //当前行白区个数 默认最多两个 修改个数请注意同时修改temp_edge数组的元素个数,以及其它的一堆都要改
        EDGE temp_edge[2];  //临时边界点

        for(uint8 i=MidStart;i>=X_LIMIT;i--)    //向左搜索
        {
            if(img[y+Y_LIMIT][i]==White)
            {
                temp_edge[0].Rx=i;
                Sum_of_white_zone=1;    //标志已经向左找到白区
            }

            if(Sum_of_white_zone==1 && img[y+Y_LIMIT][i]==Black)
            {
                temp_edge[1].Lx=i;
            }
        }

        for(uint8 i=MidStart;i<IMG_X-X_LIMIT;i++)    //向右搜索
        {
            if(img[y+Y_LIMIT][i]==White)
            {
                temp_edge[0].Lx=i;
                Sum_of_white_zone=2;    //标志已经向右找到白区
            }

            if(Sum_of_white_zone==2 && img[y+Y_LIMIT][i]==Black)
            {
                temp_edge[1].Rx=i;
            }
        }

        if(!Sum_of_white_zone)  //没白区
        {
            flag_Indct_Help_Me=1;   //请求切换到电感循迹
            return 0;
        }

        if((temp_edge[0].Rx-temp_edge[0].Lx) > (temp_edge[1].Rx-temp_edge[1].Lx))
        {
            MidStart=(temp_edge[0].Rx-temp_edge[0].Lx)/2;   //取最长白区为当前赛道 虽然我觉得取最近的赛道也行...
        }
        else
        {
            MidStart=(temp_edge[1].Rx-temp_edge[1].Lx)/2;
        }
    }

    for(uint8 i=MidStart;i>=X_LIMIT;i--)   //向左搜索
    {
        if(img[y+Y_LIMIT][i]==Black)  //TODO 改进判断噪点的方法
        {
            edge[y].Lx=i;                         //注意！丢线直接为默认值0 后面程序也是根据0判断边沿是否为特殊情况的
            break;
        }

        if(i==X_LIMIT)  //到边缘仍未找到黑区 判断丢线
        {
            flag_LoseEdge_part_L=1;
        }
    }

    for(uint8 i=MidStart;i<=IMG_X-X_LIMIT;i++)   //向右搜索
    {
        if(img[y+Y_LIMIT][i]==Black)
        {
            edge[y].Rx=i;
            break;
        }

        if(i==IMG_X-X_LIMIT)
        {
            flag_LoseEdge_part_R=1;
        }
    }

    if(edge[y].Lx==0)   //底层丢边 那就先拿图像边沿凑数吧
    {
        edge[y].Lx=X_LIMIT;
    }

    if(edge[y].Rx==0)
    {
        edge[y].Rx=IMG_X-X_LIMIT;
    }

    MidStart=(edge[0].Lx+edge[0].Rx)/2;

    return 0;
}




uint8 Hor_Search(uint8 MidStart,uint8 y)    //水平扫线（单行） *注意y为相对限制底框的高度
{
    flag_MidStart_in_Black=0;   //标志位置零

    if(MidStart>IMG_X-X_LIMIT || MidStart<X_LIMIT || y>IMG_Y-2*Y_LIMIT )  //范围验证
    {
        return 0;
    }

    if(img[y+Y_LIMIT][MidStart]==Black) //如果给定的中点在黑区内
    {
        flag_MidStart_in_Black=1;   //FIXME 未处理 又好像处理了一部分...
        return 0;
    }

    for(uint8 i=MidStart;i>=X_LIMIT;i--)   //向左搜索
    {
        if(img[y+Y_LIMIT][i]==Black)  //TODO 改进判断噪点的方法
        {
            edge[y].Lx=i;                         //注意！丢线直接为默认值0 后面程序也是根据0判断边沿是否为特殊情况的
            break;
        }

        if(i==X_LIMIT)  //到边缘仍未找到黑区 判断丢线
        {
            flag_LoseEdge_part_L=1;
        }
    }

    for(uint8 i=MidStart;i<=IMG_X-X_LIMIT;i++)   //向右搜索
    {
        if(img[y+Y_LIMIT][i]==Black)
        {
            edge[y].Rx=i;
            break;
        }

        if(i==IMG_X-X_LIMIT)
        {
            flag_LoseEdge_part_R=1;
        }
    }

    if(y>3) // 待边沿稳定后
    {
        if(edge[y].Lx-edge[y-2].Lx>=10 || edge[y].Lx-edge[y-2].Lx<=-10) //TODO EdgeLosePos可能会储存相同的高度（双侧丢边时）
        {
            edge[y].Lx=0;   //突变视作丢边
            EdgeLosePos[EdgeLoseNum++]=y;    //储存丢边信息
        }

        if(edge[y].Rx-edge[y-2].Rx>=10 || edge[y].Rx-edge[y-2].Rx<=-10)
        {
            edge[y].Rx=0;   //突变视作丢边
            EdgeLosePos[EdgeLoseNum++]=y;    //储存丢边信息
        }
    }

//****************丢线补全标志位************************
    if(y)   //不处理底边
    {
        if(edge[y].Lx==0 && edge[y].Rx!=0)
        {
            flag_LoseEdge_part_L=1;
            //edge[y].Lx=edge[y].Rx-Width_Cali(y);
        }

        else if(edge[y].Rx==0 && edge[y].Lx!=0)  //
        {
            flag_LoseEdge_part_R=1;
            //edge[y].Rx=edge[y].Lx+Width_Cali(y);
        }
        else if(edge[y].Rx==0 && edge[y].Lx==0) //坑爹的两侧丢边    
        {                                   //TODO 初步思路 垂直扫描寻找能联通的垂直路径 取左右两极限位为边沿
            flag_LoseEdge_part_L=1;
            flag_LoseEdge_part_R=1;
            flag_Cross=1;
        }
    }

//***********************************************

    return ((edge[y].Rx-edge[y].Lx)/2);
}



//******************************* Base Function *********************************/


//*******************************************************************************/

uint8 Ver_Search(uint8 MidStart)    //初始垂直扫线
{
    EdgeNum=0;
    flag_Ver_Search_Finish=0;

    for(uint8 i=Y_LIMIT;i<=IMG_Y-Y_LIMIT;i++)
    {
        EdgeNum=i;  //设定垂线高度 （扫线最大高度） EdgeNum 修正为数组长度 从Y_LIMIT开始
        if(img[i][MidStart]==Black)
        {
            flag_Ver_Search_Finish=1;
            break;
        }
    }

    EdgeNum-=Y_LIMIT;

    return(EdgeNum);
}





uint8 Uni_Ver_Search(uint8 MidStart)    //垂直扫线 通用垂直扫线 不会改变EdgeNum的值   *注意！ 这个函数是从最底边开始扫描的
{                                       //（别问为啥不写进一个函数里，就是懒
    uint8 IN_EdgeNum=0;

    for(uint8 i=0;i<=IMG_Y-Y_LIMIT;i++)
    {
        IN_EdgeNum=i;  //设定垂线高度 （扫线最大高度） EdgeNum 修正为数组长度 从Y_LIMIT开始
        if(img[i][MidStart]==Black)
        {
            flag_Ver_Search_Finish=1;
            break;
        }
    }

    IN_EdgeNum-=Y_LIMIT*2;  //从底边开始扫的 减去双倍的边缘高度

    return(IN_EdgeNum);
}




uint8 Judge(void)   //暂时无用
{
    uint8 Wider=0;  //变宽的行数

    for(uint8 i=0;i<EdgeNum;i++)
    {
        if(((edge[i].Rx-edge[i].Lx)-(edge[i+1].Rx-edge[i+1].Lx)) < -3)   //上一行比下方一行宽（正常为边线收束） -3为容差
            {
                Wider++;
            }
    }

    if(Wider>3 && Wider<EdgeNum/2) //变宽情况出现  （可能是十字、环岛入、三岔入、坡道） 设置 <EdgeNum/2 的条件是减少上坡道的干扰
    {

    }
    

    return 1;
}




inline uint8 Width_Cali(uint8 y)    //根据高度计算赛道宽度 有点逆透视的意思
{
    if(0)   //暂时改为标定值计算
        return((edge[0].Rx-edge[0].Lx)-(WIDTH_K*y)); //使用最底层宽度计算
    else
        return(WIDTH_BASE-(WIDTH_K*y)); //使用标定值计算
}




void Damn_Lose_Edge_all(void)   //双侧丢边
{

uint8 flag_Edge_Start_Exist=0;
uint8 flag_Edge_End_Exist=0;
uint8 EdgeStartPos=0;
uint8 EdgeEndPos=0;

for(uint8 i=0;i<EdgeNum;i++)
{
    if(edge[i].Lx!=0 && edge[i].Rx!=0)  //判断是否存在边沿
    {
        flag_Edge_Start_Exist=1;
        EdgeStartPos=i;
        break;
    }
}

for(uint8 i=EdgeNum;i>=0;i--)
{
    if(edge[i].Lx!=0 && edge[i].Rx!=0)  //判断是否存在边沿
    {
        flag_Edge_End_Exist=0;
        EdgeEndPos=i;
        break;
    }
}

if(flag_Edge_Start_Exist && flag_Edge_End_Exist)
{
    Connect(edge,1,EdgeStartPos,EdgeEndPos);
    Connect(edge,2,EdgeStartPos,EdgeEndPos);
}
    /*
    uint8 flag_Ver_Raise=0; //垂直高度激增
    uint8 flag_Ver_Drop=0;  //垂直高度跌落

    uint8 Ver_Raise_X=0;
    uint8 Ver_Drop_X=0;
    uint8 ver[camera_raw_data_X-X_LIMIT+1]={0}; //垂直高度数组
    uint8 Lose_Edge_Sum=0;  //丢边数
    uint8 Max_Ver=0;
    uint8 Max_Ver_X=0;  //最长垂线的横坐标
    uint8 temp=0;

    for(uint8 i=X_LIMIT;i<=camera_raw_data_X-X_LIMIT;i++)
    {   
        ver[i-X_LIMIT]=Uni_Ver_Search(i);   //存入数据
    }

    for(uint8 i=X_LIMIT;i<=camera_raw_data_X-X_LIMIT;i++)
    {
        if((i-X_LIMIT-5) >= 0)
        {
            if((ver[i-X_LIMIT]-ver[i-X_LIMIT-5]) >= 10)
            {
                Ver_Raise_X=i-X_LIMIT-5;
            }

            if((ver[i-X_LIMIT-5]-ver[i-X_LIMIT]) >= 10)
            {
                Ver_Drop_X=i-X_LIMIT;
            }
        }
    }

    for(uint8 i=0;i<=EdgeNum;i++)
    {
        if(Ver_Drop_X!=0 && Ver_Raise_X!=0) //两边都有高度突变
        {
            mid[i]=(Ver_Drop_X+Ver_Raise_X)/2;   //暂时用该值补中线
        }
        else if(Ver_Drop_X==0 && Ver_Raise_X!=0)
        {
            mid[i]=(Ver_Raise_X+0)/2;
        }
        else if(Ver_Drop_X!=0 || Ver_Raise_X==0)
        {
            mid[i]=(Ver_Drop_X+(camera_raw_data_X-X_LIMIT))/2;
        }
    }

    for(uint8 i=0;i<=EdgeNum;i++)
    {
        edge[i].Lx=mid[i]-Width_Cali(i)/2;  //FIXME 未考虑负值情况
        edge[i].Rx=mid[i]+Width_Cali(i)/2;
    }

    qDebug()<<"Drop:"<<Ver_Drop_X<<" Raise:"<<Ver_Raise_X<<"\n";
    

   for(int i=0;i<=EdgeNum;i++)
   {
       for(int j=0;j<EdgeLoseNum;j++)  //觉得比较麻烦 就直接嵌套了
       {
           if(i==EdgeLosePos[j])    //丢边
           {
               if(edge[i-1].Lx!=0)  //避免最底下就是白区 写的稀烂
               {
                   edge[i].Lx=edge[i-1].Lx;
               }
               
               if(edge[i-1].Rx!=0)
               {
                   edge[i].Rx=edge[i-1].Rx;
               }

           }
       }
   }
    */

}




void If_Lose_Edge(void) //丢边补全

{
    if(flag_LoseEdge_part_L || flag_LoseEdge_part_R)    //只要有部分丢边就会有全丢边 故不加入全丢边的条件
    {
        if(flag_LoseEdge_part_L*flag_LoseEdge_part_R != 0)  //两侧丢边
        {
            //FIXME 无法进入

            //Damn_Lose_Edge_all();   //双侧丢边函数    //FIXME 临时修改
            Del_Edge_Data(0,EdgeNum-1);
        }
        else if(flag_LoseEdge_part_L==1 && flag_LoseEdge_part_R==0)  //单左丢边
        {
            for(uint8 i=0;i<=EdgeNum;i++)
            {
                edge[i].Lx=edge[i].Rx-Width_Cali(i);    //删边补全  （直接覆写左边界数据
            }
            bb_time=20; //鸣笛
        }
        else if(flag_LoseEdge_part_L==0 && flag_LoseEdge_part_R==1)  //单右丢边
        {
            for(uint8 i=0;i<=EdgeNum;i++)
            {
                edge[i].Rx=edge[i].Lx+Width_Cali(i);
            }
            bb_time=20; //鸣笛
        }
    }

    flag_LoseEdge_part_L=0; //重置标志位
    flag_LoseEdge_part_R=0;
}




void Set_Mid(void)  //计算中点&更新为折线(减少意外出错)
{

    for(uint8 i=0;i<=EdgeNum;i++)   //计算中点
    {
        if((edge[i].Lx * edge[i].Rx) != 0)  //对于未丢边的情况
        {
            mid[i]=(edge[i].Lx+edge[i].Rx)/2;
        }

        else if(edge[i].Lx==0 && edge[i].Rx!=0)  //
        {
            mid[i]=(edge[i].Lx+edge[i].Rx)/2;
            //mid[i]=edge[i].Rx-Width_Cali(i)/2;    
            //mid[i]=edge[i-1].Rx-mid[i-1];
        }

        else if(edge[i].Rx==0 && edge[i].Lx!=0)  //
        {
            mid[i]=(edge[i].Lx+edge[i].Rx)/2;
            //mid[i]=edge[i].Lx-Width_Cali(i)/2;
            //mid[i]=mid[i-1]-edge[i].Lx;
        }

        else
        {
            mid[i]=94;  //直接置于中点
        }
        
        if(i>=5)    //待中线稳定后，对超过容差的相邻中线进行判断
        {
            if(mid[i]-mid[i-1]>=20 || mid[i]-mid[i-1]<=-20)
            {
                EdgeNum=i-1;  //结束中点计算
            }
        }
        
    }

    
    for(uint8 i=1;i<(EdgeNum/MID_RESOLUTION)-1;i++)   //中线拟合成直线
    {
        Mid_Connect(mid,(i+1)*MID_RESOLUTION,i*MID_RESOLUTION);
    }
    

}




void Simple_Err_Check(void) //简单检错程序
{

}





void Print_Mid(void) //绘画中线
{
    for(uint8 i=Y_LIMIT; i<=EdgeNum+Y_LIMIT; i++)
    {
        img[i][edge[i-Y_LIMIT].Lx]=Gray;
        img[i][edge[i-Y_LIMIT].Rx]=Gray;

        if(mid[i]<IMG_X && mid[i]>0)    //我也不知道为什么当时要写这个奇怪的限制
            img[i][mid[i-Y_LIMIT]]=Gray;
    }

}




void Search(void)   //坑爹的扫线主函数
{

    uint8 MidStart=IMG_X/2;   //底边搜索起始点横坐标    
    //讲道理，我觉得我不应该把search里和hor_search里搞一个同名的局部变量，但是懒得改了
    flag_LoseEdge_all_L=0;
    flag_LoseEdge_all_R=0;

    init();
    Y_Change();

    Hor_Search_Base(MidStart,0);   //第一遍先确定扫描基准中线
    MidStart=(edge[0].Lx+edge[0].Rx)/2;    //TODO 需确保底层边缘有值 需加辅助函数

    Ver_Search(MidStart);   //垂直扫线 更新扫线最大高度 EdgeNum

    for(uint8 i=0;i<=EdgeNum;i++)   //水平扫线循环
    {
        Hor_Search(MidStart,i);
                                //TODO 缺一个对丢边格数的判断来对 flag_LoseEdge_all_X置位
    }

    If_Lose_Edge(); 

    Set_Mid();
    Print_Mid();
}




void CAM_Error(void)
{
    uint8 unit=EdgeNum/5;

    if(flag_Indct_Help_Me)  //出错的话偏差值设为0就好了
    {
        CamError=0;
    }
    else
    {
        CamError=(IMG_X/2)-(mid[unit]+mid[unit*2]+mid[unit*3]+mid[unit*4]+mid[unit*5-3])/5; //最顶上的边沿可信度较低
    }
    
}
