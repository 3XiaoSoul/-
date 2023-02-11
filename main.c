#include "pro.h"



int  main()
{

    pro_demo_poi pro_demo_p = Pro_Init();
    if(pro_demo_p == (pro_demo_poi)-1)
    {
        printf("Pro Device Init Failed !\n");
        return -1;
    }

    if(Main_Ui_Touch_Ctrl(pro_demo_p) == -1)
    {
        printf("进入主界面失败！\n");
        return -1;
    }

    if(Pro_Free(pro_demo_p) == -1)
    {
        printf("Pro Decive Free Failed !\n");
        return -1;
    }

    return 0;
}