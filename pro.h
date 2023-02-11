#ifndef _PRO_H_
#define _PRO_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <linux/input.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define LCD_PATH       "/dev/fb0"
#define TOUCH_PATH     "/dev/input/event0"
#define PHOTO_PATH     "/pro/ps"
#define LCD_PIXEL_SIZE 4
#define COLOR_BAR_Y    230
#define COLOR_BAR_X    200
#define COLOR_BAR_W    400
#define COLOR_BAR_H    40
#define TOUCH_X_INIT   -1
#define TOUCH_Y_INIT   -1
#define PHOTO_PATH_LEN 256
#define PAGE_SIZE       9
//设计项目结构体
typedef struct pro_demo_inf
{
    int lcd;
    int ts;
    int * lcd_mmap_start;
    int touch_x,touch_y;
    int lcd_w;
    int lcd_h;
    int mmap_len;
    int bmp_w,bmp_h;
    int skip;
    int bmp_color_size;//计算要读取的图片的字节大小
    char * bmp_color_p;//存放图片像素点大小的堆空间首地址
    struct input_event touch;
}pro_demo_str, * pro_demo_poi;

//设计图片链表结构体
typedef struct photo_link_list
{
    char photo_path[PHOTO_PATH_LEN];
    int  photo_num;
    int  obj_x;
    int  obj_y;
    int page_num;

    struct photo_link_list * next;
    struct photo_link_list * prev;
}photo_node,*photo_link;

//设计电子相册结构体
typedef struct electronic_photo_album_inf
{
    DIR * dp;//目录流指针
    int photo_sum;//图片的个数
    int page_sum; //页的个数
    photo_node photo_list_head;
    
}photo_show_node,*photo_show_poin;

/*****************pro_set.c**************************/
pro_demo_poi Pro_Init();
int Pro_Loading(pro_demo_poi pro_demo_p);
int Show_Color_Bar(pro_demo_poi pro_demo_p,int bar_color);
int Pro_Free(pro_demo_poi pro_demo_p);
int Display_Photo(pro_demo_poi pro_demo_p,const char * bmp_file_path,int zoomout,int where_x,int where_y);
int Main_Ui_Touch_Ctrl(pro_demo_poi pro_demo_p);
int Get_Touch_Coordinates(pro_demo_poi pro_demo_p);


/********************photo_show.c*************************/

int Electronic_Photo_Album(pro_demo_poi pro_demo_p);
photo_show_poin Electronic_Photo_Album_Init();
photo_link Create_Photo_List_Node();
int Electronic_Photo_Album_Touch_Ctrl(pro_demo_poi pro_demo_p,photo_show_poin psp);
int Clean_Lcd(pro_demo_poi pro_demo_p,int lcd_color);
int Serach_Photo(photo_show_poin psp);
int Tail_Add_Photo_Node(photo_show_poin psp,photo_link new_node);
int Count_Page_Photo_Sum(photo_show_poin psp);
int Display_Photo_List(photo_show_poin psp);
int Thumbnail_Ui(int page_num,photo_show_poin psp,pro_demo_poi pro_demo_p);//显示缩略图界面

#endif