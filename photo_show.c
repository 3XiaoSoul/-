#include "pro.h"

int Electronic_Photo_Album_Touch_Ctrl(pro_demo_poi pro_demo_p,photo_show_poin psp)
{
    int page_num = 1;
    while (1)
    {
        Get_Touch_Coordinates(pro_demo_p);
        if(pro_demo_p->touch.type == EV_KEY && pro_demo_p->touch.code == BTN_TOUCH && pro_demo_p->touch.value == 0)//手松开才获取最新的坐标
        {

            if(pro_demo_p->touch_x>650 && pro_demo_p->touch_x < 800 && pro_demo_p->touch_y>80 && pro_demo_p->touch_y<280)//上一页按钮
            {   
                 printf("上一页！\n");
                if(page_num > 1)
                {
                    if(Thumbnail_Ui(--page_num,psp,pro_demo_p) == -1)
                    {
                        printf("Thumbnail_Ui fadile !\n");
                        return -1;
                    }
                }
               
            }

            if(pro_demo_p->touch_x>650 && pro_demo_p->touch_x < 800 && pro_demo_p->touch_y>280 && pro_demo_p->touch_y<480)//下一页
            {
                printf("下一页！\n");
                 if(page_num < psp->page_sum)
                {
                    if(Thumbnail_Ui(++page_num,psp,pro_demo_p) == -1)
                    {
                        printf("Thumbnail_Ui fadile !\n");
                        return -1;
                    }
                }
            }   

            if(pro_demo_p->touch_x>650 && pro_demo_p->touch_x < 800 && pro_demo_p->touch_y>0 && pro_demo_p->touch_y<80)//退出按钮
            {
                //显示主界面
                Display_Photo(pro_demo_p,"/pro/ui_main.bmp",1,0,0);
                pro_demo_p->touch_x = TOUCH_X_INIT;
                pro_demo_p->touch_y = TOUCH_Y_INIT;
                break;
            }

        }

    }
    return 0;
}


int Electronic_Photo_Album(pro_demo_poi pro_demo_p)
{
    printf("进入电子相册！\n");
    Clean_Lcd(pro_demo_p,0x00ffffff);

    photo_show_poin psp = Electronic_Photo_Album_Init();
    if(psp == (photo_show_poin) -1)
    {
        printf("Electronic_Photo_Album Init Failed!\n");
        return -1;
    }

    Display_Photo_List(psp);

    Thumbnail_Ui(1,psp,pro_demo_p);

    Electronic_Photo_Album_Touch_Ctrl(pro_demo_p,psp);
    return 0;
}

int Clean_Lcd(pro_demo_poi pro_demo_p,int lcd_color)
{
    for(int y=0; y<pro_demo_p->lcd_h; y++)
    {
        for(int x=0; x<pro_demo_p->lcd_w; x++)
        {
            *(pro_demo_p->lcd_mmap_start + pro_demo_p->lcd_w*y+x) = lcd_color;
        }
    }

    return 0;
}

photo_show_poin Electronic_Photo_Album_Init()
{
    photo_show_poin psp =(photo_show_poin)malloc(sizeof(photo_show_node));
    if(psp == NULL)
    {
        perror("malloc psp node failed");
        return (photo_show_poin)-1;
    }
    memset(psp,0,sizeof(photo_show_node));

    psp->page_sum  = 1;
    psp->photo_sum = 0;
    psp->photo_list_head.next = &(psp->photo_list_head);
    psp->photo_list_head.prev = &(psp->photo_list_head);

    
    if(Serach_Photo(psp) == -1)
    {
        printf("picture  search failed !\n");
        return (photo_show_poin)-1;
    }
    return psp;
}

photo_link Create_Photo_List_Node()
{
    photo_link node = (photo_link)malloc(sizeof(photo_node));
    if (node == NULL)
    {
        perror("malloc photo node failed");
        return (photo_link)-1;
    }

    memset(node,0,sizeof(photo_node));

    node->next = node->prev = node;

    return node;
}

int Serach_Photo(photo_show_poin psp)
{
    
    psp->dp = opendir(PHOTO_PATH);
    if(psp->dp == NULL)
    {
        perror("opendir photo failed");
        return -1;
    }

    while(1)
    {
        struct dirent * eq = readdir(psp->dp);
        if(eq == NULL)
        {
            printf("dir search finish");
            break;
        }

        if(eq->d_name[0] == '.') continue;

        if((eq->d_type == DT_REG) && (strcmp(&(eq->d_name[strlen(eq->d_name)-4]),".bmp") == 0))
        {
            photo_link new_node = Create_Photo_List_Node();
            if(new_node == (photo_link)-1)
            {
                printf("create photo new node failed !\n");
                return -1;
            }

            sprintf(new_node->photo_path,"%s/%s",PHOTO_PATH,eq->d_name);
            printf("hit obj , photo path was :%s\n",new_node->photo_path);

            Count_Page_Photo_Sum(psp);

            if(Tail_Add_Photo_Node(psp,new_node) == -1)
            {
                printf("add photo node failed!\n");
                return -1;
            }

        }
    }

    if(psp->photo_sum == 0) psp->page_sum = 0;

    return 0;
}


int Tail_Add_Photo_Node(photo_show_poin psp,photo_link new_node)
{
    if(new_node == NULL)
    {
        printf("new node abort ,add failed\n");
        return -1;
    }

    //如果是第n次
    new_node->next = &(psp->photo_list_head);
    (psp->photo_list_head).prev->next = new_node;
    new_node->prev = (psp->photo_list_head).prev;
    (psp->photo_list_head).prev = new_node;

    new_node->photo_num = psp->photo_sum;

    if(new_node->photo_num % PAGE_SIZE == 0)
    {
        new_node->page_num = new_node->photo_num/PAGE_SIZE;
    }
    else
    {
        new_node->page_num = new_node->photo_num/PAGE_SIZE+1;
    }
    
    
    
    return 0;
}


int Count_Page_Photo_Sum(photo_show_poin psp)
{
    psp->photo_sum++;
    if((psp->photo_sum-1) % PAGE_SIZE == 0 && psp->photo_sum>PAGE_SIZE)
    {
        psp->page_sum++;
    }


    return 0;
}


int Display_Photo_List(photo_show_poin psp)
{
    printf("photo sum %d:\n page sum %d\n",psp->photo_sum,psp->page_sum);

    if(psp->photo_list_head.next == &(psp->photo_list_head)) 
    {
        printf("Empty photo list,cant display!\n"); 
        return 0;
    }

    for(photo_link tmp_node = psp->photo_list_head.next; tmp_node != &(psp->photo_list_head); tmp_node = tmp_node->next)
    {
        printf("photo path %s \n photo num %d photo page %d\n",tmp_node->photo_path,tmp_node->photo_num,tmp_node->page_num);
    }

    return 0;
}
//2

int Thumbnail_Ui(int page_num,photo_show_poin psp,pro_demo_poi pro_demo_p)//显示缩略图界面
{
    Clean_Lcd(pro_demo_p,0xffff66);
    photo_link start_node = psp->photo_list_head.next;

    printf("即将显示第%d页的图片\n",page_num);
    for(int lp=0; lp< (page_num-1)*PAGE_SIZE; lp++)
    {
        start_node = start_node->next;
    }

    int x=0,y=0;
    for(int lp=0; lp<PAGE_SIZE; lp++)
    {
        if(start_node == &(psp->photo_list_head)) break;
        Display_Photo(pro_demo_p,start_node->photo_path,4,x,y);

        x+=220;
        if(x>600)
        {
            y+=140;
            x=0;
        }

        start_node = start_node->next;
        usleep(3000);
    }

    return 0;
}