#include "pro.h"


pro_demo_poi Pro_Init()
{
    pro_demo_poi pro_demo_p = (pro_demo_poi)malloc(sizeof(pro_demo_str));
    if(pro_demo_p == NULL)
    {
        perror("malloc");
        return (pro_demo_poi)-1;
    }
    memset(pro_demo_p,0,sizeof(pro_demo_str));

    //初始化触摸屏坐标为-1，避免触摸屏混乱
    pro_demo_p->touch_x = TOUCH_X_INIT;
    pro_demo_p->touch_y = TOUCH_Y_INIT;

    if((pro_demo_p->lcd = open(LCD_PATH,O_RDWR)) == -1)
    {
        perror("open lcd failed");
        return (pro_demo_poi)-1;
    }

    if((pro_demo_p->ts = open(TOUCH_PATH,O_RDONLY)) == -1)
    {
        perror("open touch lcd failed");
        return (pro_demo_poi)-1;
    }

    //获取可见区的w和h
    struct fb_var_screeninfo var_inf;
    if(ioctl(pro_demo_p->lcd,FBIOGET_VSCREENINFO,&var_inf) == -1)
    {
        perror("ioctl lcd failed");
        return (pro_demo_poi)-1;
    }
    pro_demo_p->lcd_h = var_inf.yres;
    pro_demo_p->lcd_w = var_inf.xres;
    
    printf("Get Lcd Device Inf Success,\n lcd w:%d\tlcd h:%d\n\n",\
                                        pro_demo_p->lcd_w,
                                        pro_demo_p->lcd_h);

    pro_demo_p->mmap_len = pro_demo_p->lcd_h * pro_demo_p->lcd_w * LCD_PIXEL_SIZE;

    if((pro_demo_p->lcd_mmap_start = (int *)mmap(NULL,
                                                    pro_demo_p->mmap_len,
                                                    PROT_READ | PROT_WRITE,
                                                    MAP_SHARED,
                                                    pro_demo_p->lcd,
                                                    0)) == MAP_FAILED)
    {
        perror("mmap lcd failed");
        return (pro_demo_poi)-1;
    }


    //显示楼顶界面
    if(Display_Photo(pro_demo_p,"/pro/zyns.bmp",1,0,0)== -1)
    {
        printf("display loading ui failed！\n");
        return (pro_demo_p)-1;
    }
    
    if(Pro_Loading(pro_demo_p) == -1)
    {
        printf("loading failed !\n");
        return (pro_demo_p)-1;
    }

    return pro_demo_p;
}


int Pro_Free(pro_demo_poi pro_demo_p)
{
    if(close(pro_demo_p->lcd) == -1)
    {
        perror("close lcd failed");
        return -1;
    }

    if(close(pro_demo_p->ts) == -1)
    {
        perror("close touch lcd failed");
        return -1;
    }
    
    if(munmap(pro_demo_p->lcd_mmap_start,pro_demo_p->mmap_len) == -1)
    {
        perror("munmap lcd failed");
        return -1;
    }
    
    free(pro_demo_p);
    return 0;
}


int Display_Photo(pro_demo_poi pro_demo_p,const char * bmp_file_path,int zoomout,int where_x,int where_y)
{
    int bmp_fd = open(bmp_file_path,O_RDONLY);
    if(bmp_fd == -1)
    {
        perror("open bmp");
        return -1;
    }

    /*读取图片 w 和 h*/
    lseek(bmp_fd,18,SEEK_SET);
    read(bmp_fd,&pro_demo_p->bmp_w,sizeof(pro_demo_p->bmp_w));
    read(bmp_fd,&pro_demo_p->bmp_h,sizeof(pro_demo_p->bmp_h));
    printf("obj photo w:%d----h:%d\n",pro_demo_p->bmp_w,pro_demo_p->bmp_h);

    if(pro_demo_p->bmp_w*3%4 !=0)
    {
        pro_demo_p->skip = 4-(pro_demo_p->bmp_w*3%4);
    }
    else
    {
        pro_demo_p->skip = 0;
    }
    printf("-----------skip:%d\n",pro_demo_p->skip);
    pro_demo_p->bmp_color_size = (pro_demo_p->bmp_w * pro_demo_p->bmp_h * 3) + pro_demo_p->skip*pro_demo_p->lcd_h;
    pro_demo_p->bmp_color_p = (char *)malloc(pro_demo_p->bmp_color_size * sizeof(char));
    if(pro_demo_p->bmp_color_p == NULL)
    {
        perror("mamloc bmp ");
        return -1;
    }

    /*读取图片像素点*/
    lseek(bmp_fd,54,SEEK_SET);
    read(bmp_fd,pro_demo_p->bmp_color_p,pro_demo_p->bmp_color_size);

    int * tmp_lcd_mmap_start = pro_demo_p->lcd_mmap_start + pro_demo_p->lcd_w * where_y + where_x;
    for(int y=0,n=0; y<pro_demo_p->bmp_h/zoomout; y++)
    {
        for(int x=0; x<pro_demo_p->bmp_w/zoomout; x++,n=n+(3*zoomout))
        {
            *(tmp_lcd_mmap_start + pro_demo_p->lcd_w * (pro_demo_p->bmp_h/zoomout-1-y)+x) = 
            *(pro_demo_p->bmp_color_p+n)   << 0 |
            *(pro_demo_p->bmp_color_p+n+1) << 8 | 
            *(pro_demo_p->bmp_color_p+n+2) <<16;
        }

    n += pro_demo_p->skip;
    if(zoomout!=1)
    {
        n += (pro_demo_p->bmp_w%zoomout)*3;
        
        n += ((pro_demo_p->bmp_w*(zoomout-1)*3)+(pro_demo_p->skip)*(zoomout-1));
    }

    }

    close(bmp_fd);

    return 0;
}

int Show_Color_Bar(pro_demo_poi pro_demo_p,int bar_color)
{   
    int * lcd_mmap_start = pro_demo_p->lcd_mmap_start + (pro_demo_p->lcd_w * COLOR_BAR_Y + COLOR_BAR_X);
    int mask_color;

    if(bar_color == 0x00000000) 
        mask_color = 0;
    else
        mask_color = 1;

    //先显示空的矩形
    for(int x=-1; x<COLOR_BAR_W+1; x++)
    {
        for(int y=-1; y<COLOR_BAR_H+1; y++)
        {   

            if(mask_color == 0)
                bar_color+=0xf;
            else
                bar_color-=0xf;


            if( x>=-1 && x<=1) 
                *(lcd_mmap_start + pro_demo_p->lcd_w *y+x) = bar_color;//左边一束


            else if(x>=COLOR_BAR_W-1 && x<=COLOR_BAR_W+1)
                *(lcd_mmap_start + pro_demo_p->lcd_w *y+x) = bar_color;//右边一束


            else if(y>=-1 && y<= 1)
                *(lcd_mmap_start + pro_demo_p->lcd_w *y+x) = bar_color;//上面一束
 

            else if(y>=COLOR_BAR_H-1 && y<=COLOR_BAR_H+1)
                *(lcd_mmap_start + pro_demo_p->lcd_w *y+x) = bar_color;//下面一束
            else
                *(lcd_mmap_start + pro_demo_p->lcd_w *y+x) = 0x0066ffff;//下面一束

        }
        usleep(1000);

    }

    return 0;
}

int Pro_Loading(pro_demo_poi pro_demo_p)
{

    for(int lp=0; lp<2; lp++)
    {
        Show_Color_Bar(pro_demo_p,0x00ffffff);
        Display_Photo(pro_demo_p,"/pro/lod1.bmp",1,600,430);
        sleep(1);
        Display_Photo(pro_demo_p,"/pro/lod2.bmp",1,600,430);
        Show_Color_Bar(pro_demo_p,0x00000000);
        sleep(1);
        Display_Photo(pro_demo_p,"/pro/lod3.bmp",1,600,430);
        Show_Color_Bar(pro_demo_p,0x00ffffff);
        sleep(1);
    }   

    return 0;
}

int Get_Touch_Coordinates(pro_demo_poi pro_demo_p)
{
    
    read(pro_demo_p->ts,&pro_demo_p->touch,sizeof(pro_demo_p->touch));

    if(pro_demo_p->touch.type == EV_ABS && pro_demo_p->touch.code == ABS_X) pro_demo_p->touch_x = pro_demo_p->touch.value*800/1024;
    if(pro_demo_p->touch.type == EV_ABS && pro_demo_p->touch.code == ABS_Y) pro_demo_p->touch_y = pro_demo_p->touch.value*480/600;

    return 0;
}

int Main_Ui_Touch_Ctrl(pro_demo_poi pro_demo_p)
{
    //加载完成显示主界面
    if(Display_Photo(pro_demo_p,"/pro/ui_main.bmp",1,0,0)== -1)
    {
        printf("display loading ui failed！\n");
        return -1;
    }


    while (1)
    {
        Get_Touch_Coordinates(pro_demo_p);
        if(pro_demo_p->touch.type == EV_KEY && pro_demo_p->touch.code == BTN_TOUCH && pro_demo_p->touch.value == 0)//手松开才获取最新的坐标
        {

            if(pro_demo_p->touch_x>138 && pro_demo_p->touch_x < 265 && pro_demo_p->touch_y>300 && pro_demo_p->touch_y<433)//相册按钮
            {
                Electronic_Photo_Album(pro_demo_p);
            }

            if(pro_demo_p->touch_x>516 && pro_demo_p->touch_x<670 && pro_demo_p->touch_y>306 && pro_demo_p->touch_y<420)//游戏按钮
            {
                printf("启动游戏！\n");
            }

            if(pro_demo_p->touch_x >727 && pro_demo_p->touch_x<800 && pro_demo_p->touch_y>0 && pro_demo_p->touch_y<72)//退出按钮
            {
                //显示退出界面
                Clean_Lcd(pro_demo_p,0x00ff0000);
                break;
            }

        }

    }
    


    return 0;
}