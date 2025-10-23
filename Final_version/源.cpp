#define _CRT_SECURE_NO_WARNINGS 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <conio.h>  // Windows下用于无回显输入的头文件,用于密码输入隐藏
#include <time.h>
#include <windows.h>
#include <direct.h>
#include <easyx.h>

#define MAX_NAME_LENGTH 2000                                                   //最大名称长度

// 用户管理系统
#define MAX_USERS 100
#define MAX_ADMS 10

typedef struct {  // 用户结构体
    char username[MAX_NAME_LENGTH];  // 用户名
    char password[MAX_NAME_LENGTH];  // 密码
    int is_admin;                    // 是否为管理员（1=是，0=否）
} User;

User users[MAX_USERS];
int user_count = 0;
int adm_count = 0;
int comfirm = 0;

// 函数声明（用户管理）
void regist_user();
void regist_admin();
void load_users();
void get_password(char* password, int max_len);  // 修复：补充参数声明
void save_users();
void initialize_users();

//下面的函数用于路线收藏
int file_exists(const char* file_path);                                          //文件存在判断函数，服务于收藏函数     
int display_star(const char* search_path, char filenames[][MAX_NAME_LENGTH]);    //收藏夹文件展示函数
void open_startxt(char* filepath);                                               //收藏夹文件文本展示函数
void search_star(int is_ask);                                                    //收藏夹查找函数
void arr_star(char* filepath, int line_count);                                   //收藏夹管理函数
void use_star(char* filepath, int linecount);                                    //收藏夹使用函数
void star_routes(char* start, char* end);                                        //收藏功能函数


// 线路管理系统
#define MAX_ROUTES 100         // 最大路线数量
#define MAX_STATIONS 50        // 最大站点数量
#define MAX_RUNS 10            // 一小时内最多班次

#define BACKUP_FILE "routes_backup.txt"    // 上一次操作备份文件
#define DEFAULT_FILE "routes_default.txt"  // 默认路线文件

typedef struct {  // 公交路线结构体
    char route_name[MAX_NAME_LENGTH];                  // 线路名称
    char stations[MAX_STATIONS][MAX_NAME_LENGTH];      // 站点列表
    int station_count;                                 // 站点数量
    int start_time;                                    // 首班车时间（小时）
    int end_time;                                      // 末班车时间（小时）
    int time_between_stations[MAX_STATIONS - 1];       // 站点间耗时（分钟）
    int departure_time[MAX_RUNS];                      // 每小时发车时间（分钟）
    int run_count;                                     // 每小时班次数量
} BusRoute;

BusRoute routes[MAX_ROUTES];
int route_count = 0;

// 函数声明（线路管理）
void load_routes();
void save_routes();
void add_route();
void modify_route();
void delete_route();
void display_all_routes();
void inquire_route(char* start, char* end, int is_ask);  // 路线查询主函数

// 恢复功能相关函数声明
void backup_routes();
void load_routes_from_file(const char* file_path);
void restore_last_routes();
void restore_default_routes();

// 时间计算系统
typedef struct {  // 路线时间信息
    int can_arrive;   // 是否可达（1=是，0=否）
    int wait_time;    // 等待时间（分钟）
    int sum_time;     // 总耗时（分钟）
    int arrive_time;  // 到达时间（分钟，如998=16:38）
} RouteTime;

// 函数声明（时间计算）
int get_current_time();
RouteTime inquire_time(int start_index, int end_index, int i, int current_time);

// 交互系统函数声明
void login();
void user_menu();
void admin_menu();
void star_menu(char* filepath, int line_count);

void view_map(char* route_name);
void show_All();

int station_exists(char* station);
void check_input(char* start, char* end, int* start_valid, int* end_valid);

// 用户管理函数实现
void load_users() {
    /*
    **************************加载用户信息函数**************************
    输入：无
    输出：无
    功能：从user.txt中把用户信息加载至缓存区，确保可以使用
    ******************************************************************
    */

    FILE* fp = fopen("users.txt", "r");
    if (fp == NULL) return;

    while (user_count < MAX_USERS) {
        int ret = fscanf(fp, "%s %s %d",
            users[user_count].username,
            users[user_count].password,
            &users[user_count].is_admin);
        if (ret == 3) user_count++;
        else if (ret == EOF) break;
        else {
            printf("用户数据格式错误，读取失败。\n");
            break;
        }
    }
    fclose(fp);
}

void get_password(char* password, int max_len) {
    /*
    **************************密码输入函数****************************
    输入：存储用户输入的密码的字符数组指针、最大长度
    输出：无返回值，但会通过 password 指针将输入的密码保存到外部变量。
    功能：实现密码的 “隐藏显示” 输入
    *****************************************************************
    */
    int i = 0;
    char c;
    while (i < max_len - 1) {
        c = _getch();
        if (c == '\r') break;  // 回车结束
        else if (c == '\b') {  // 退格处理
            if (i > 0) {
                i--;
                printf("\b \b");
                fflush(stdout);
            }
        }
        else {
            password[i++] = c;
            printf("*");
            fflush(stdout);
        }
    }
    password[i] = '\0';
    printf("\n");
}

void save_users() {
    /*
    **************************用户信息保存函数*************************
    输入：无
    输出：无
    功能：将用户信息从内存保存到磁盘
    *****************************************************************
    */
    FILE* fp = fopen("users.txt", "w");
    if (fp == NULL) {
        printf("保存用户失败：文件无法打开。\n");
        return;
    }

    /*循环写入用户结构体中的信息*/
    for (int i = 0; i < user_count; i++) {
        fprintf(fp, "%s %s %d\n",
            users[i].username, users[i].password, users[i].is_admin);
    }
    fclose(fp);
}

void regist_user() {  // 注册普通用户
    /*
    *************************普通用户注册函数***************************
    输入：无
    输出：无
    功能：注册账号
    ******************************************************************
    */
    /*确保用户信息不超过数组上限*/
    if (user_count >= MAX_USERS) {
        printf("用户数量已达上限。\n");
        return;
    }

    char temp_username[MAX_NAME_LENGTH];
    int is_duplicate = 0;
    do {
        /*重复性检测*/
        printf("请输入用户名: ");
        scanf("%s", temp_username);
        for (int i = 0; i < user_count; i++) {
            if (strcmp(users[i].username, temp_username) == 0) {
                is_duplicate = 1;
                printf("用户名已存在，请重新输入。\n");
                break;
            }
        }
    } while (is_duplicate);

    /*写入数据*/
    strcpy(users[user_count].username, temp_username);
    printf("请输入密码: ");
    get_password(users[user_count].password, MAX_NAME_LENGTH);
    users[user_count].is_admin = 0;
    user_count++;
    save_users();
    printf("注册成功！\n");
}

void regist_admin() {  // 注册管理员
    if (adm_count >= MAX_ADMS || user_count >= MAX_USERS) {
        printf("管理员数量已达上限。\n");
        return;
    }
    char temp_username[MAX_NAME_LENGTH];
    int is_duplicate;
    do {
        is_duplicate = 0;
        printf("请输入管理员用户名: ");
        scanf("%s", temp_username);
        for (int i = 0; i < user_count; i++) {
            if (strcmp(users[i].username, temp_username) == 0) {
                is_duplicate = 1;
                printf("用户名已存在，请重新输入。\n");
                break;
            }
        }
    } while (is_duplicate);
    strcpy(users[user_count].username, temp_username);
    printf("请输入管理员密码: ");
    get_password(users[user_count].password, MAX_NAME_LENGTH);
    users[user_count].is_admin = 1;
    user_count++;
    adm_count++;
    save_users();
    printf("管理员注册成功！\n");
}

// 备份当前路线数据到备份文件（用于“恢复上一次”）
void backup_routes() {
    /*
    *************************路线数据改动撤销函数************************
    输入：无
    输出：无
    功能：撤销上一步对路线数据的操作
    ******************************************************************
    */
    FILE* fp = fopen(BACKUP_FILE, "w");
    if (fp == NULL) {
        printf("备份路线失败：文件无法创建。\n");
        return;
    }

    // 与 save_routes() 逻辑一致，只是输出到备份文件
    for (int i = 0; i < route_count; i++) {
        fprintf(fp, "%s %d %d %d %d ",
            routes[i].route_name,
            routes[i].station_count,
            routes[i].start_time,
            routes[i].end_time,
            routes[i].run_count);
        for (int j = 0; j < routes[i].station_count; j++) {
            fprintf(fp, "%s ", routes[i].stations[j]);
        }
        for (int j = 0; j < routes[i].station_count - 1; j++) {
            fprintf(fp, "%d ", routes[i].time_between_stations[j]);
        }
        for (int j = 0; j < routes[i].run_count; j++) {
            fprintf(fp, "%d ", routes[i].departure_time[j]);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
    // printf("路线已备份到 %s\n", BACKUP_FILE); // 调试用，可注释
}

// 从指定文件加载路线（支持主文件/备份文件/默认文件）
void load_routes_from_file(const char* file_path) {
    /*
    *************************路线数据加载函数-pro版**************************
    输入：无
    输出：无
    功能：在load_routes函数加载不了时，可以从磁盘中调用backup/defult至内存结构体中
    **********************************************************************
    */
    FILE* fp = fopen(file_path, "r");
    if (fp == NULL) {
        printf("文件 %s 不存在，请手动创建后重试。\n", file_path);
        return;
    }

    // 清空当前路线缓存，避免数据叠加
    route_count = 0;
    while (route_count < MAX_ROUTES) {
        int ret = fscanf(fp, "%s %d %d %d %d",
            routes[route_count].route_name,
            &routes[route_count].station_count,
            &routes[route_count].start_time,
            &routes[route_count].end_time,
            &routes[route_count].run_count);
        if (ret != 5) break;

        for (int i = 0; i < routes[route_count].station_count; i++) {
            fscanf(fp, "%s", routes[route_count].stations[i]);
        }
        for (int i = 0; i < routes[route_count].station_count - 1; i++) {
            fscanf(fp, "%d", &routes[route_count].time_between_stations[i]);
        }
        for (int i = 0; i < routes[route_count].run_count; i++) {
            fscanf(fp, "%d", &routes[route_count].departure_time[i]);
        }
        route_count++;
    }
    fclose(fp);
    // 加载后同步保存到主文件，确保数据一致
    save_routes();
    printf("已从 %s 加载路线，共 %d 条线路。\n", file_path, route_count);
}

// 线路管理函数实现
void load_routes() {
    /*
    ***************************加载路线信息***************************
    输入：无
    输出：无
    功能：从routes.txt中把路线信息加载至缓存区，确保可以使用
    ****************************************************************
    */
    FILE* fp = fopen("routes.txt", "r");
    if (fp == NULL) return;

    while (route_count < MAX_ROUTES) {
        int ret = fscanf(fp, "%s %d %d %d %d",
            routes[route_count].route_name,
            &routes[route_count].station_count,
            &routes[route_count].start_time,
            &routes[route_count].end_time,
            &routes[route_count].run_count);
        if (ret != 5) break;

        // 读取站点名称
        for (int i = 0; i < routes[route_count].station_count; i++) {
            fscanf(fp, "%s", routes[route_count].stations[i]);
        }
        // 读取站点间耗时
        for (int i = 0; i < routes[route_count].station_count - 1; i++) {
            fscanf(fp, "%d", &routes[route_count].time_between_stations[i]);
        }
        // 读取每小时发车时间
        for (int i = 0; i < routes[route_count].run_count; i++) {
            fscanf(fp, "%d", &routes[route_count].departure_time[i]);
        }
        route_count++;
    }
    fclose(fp);
}

void save_routes() {
    /*
    ***************************路线保存函数*****************************
    输入：无
    输出：无
    功能：保存路线结构体在内存中的数据至磁盘中
    ******************************************************************
    */

    FILE* fp = fopen("routes.txt", "w");
    if (fp == NULL) {
        printf("保存路线失败：文件无法打开。\n");
        return;
    }

    /*不同线路循环写入*/
    for (int i = 0; i < route_count; i++) {
        fprintf(fp, "%s %d %d %d %d ",
            routes[i].route_name,
            routes[i].station_count,
            routes[i].start_time,
            routes[i].end_time,
            routes[i].run_count);
        // 写入站点
        for (int j = 0; j < routes[i].station_count; j++) {
            fprintf(fp, "%s ", routes[i].stations[j]);
        }
        // 写入站点间耗时
        for (int j = 0; j < routes[i].station_count - 1; j++) {
            fprintf(fp, "%d ", routes[i].time_between_stations[j]);
        }
        // 写入发车时间
        for (int j = 0; j < routes[i].run_count; j++) {
            fprintf(fp, "%d ", routes[i].departure_time[j]);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
}

void add_route() {
    /*
    ***************************路线添加函数*****************************
    输入：无
    输出：无
    功能：增加路线文件中指定线路的数据，即针对结构体的操作
    ******************************************************************
    */
    /*检测是否达到上限*/
    if (route_count >= MAX_ROUTES) {
        printf("路线数量已达上限。\n");
        return;
    }
    display_all_routes();                                               // 显示现有路线

    char temp_route_name[MAX_NAME_LENGTH];                              // 临时存储数组
    int is_duplicate = 0;                                               // 判断是否重复的逻辑参数

    /*重复检测*/
    do {
        printf("请输入新线路名称: ");
        scanf("%s", temp_route_name);
        for (int i = 0; i < route_count; i++) {
            if (strcmp(routes[i].route_name, temp_route_name) == 0) {
                is_duplicate = 1;
                printf("线路名称已存在，请重新输入。\n");
                break;
            }
        }
    } while (is_duplicate);
    strcpy(routes[route_count].route_name, temp_route_name);            // 将临时数据转移到结构体中


    /*输入其他数据*/
    printf("请输入站点数量: ");
    scanf("%d", &routes[route_count].station_count);
    printf("请输入首班车时间(小时): ");
    scanf("%d", &routes[route_count].start_time);
    printf("请输入末班车时间(小时): ");
    scanf("%d", &routes[route_count].end_time);
    printf("请输入每小时发车次数: ");
    scanf("%d", &routes[route_count].run_count);

    // 输入站点名称
    for (int i = 0; i < routes[route_count].station_count; i++) {
        printf("请输入第%d个站点名称: ", i + 1);
        scanf("%s", routes[route_count].stations[i]);
    }
    // 输入站点间耗时
    for (int i = 0; i < routes[route_count].station_count - 1; i++) {
        printf("请输入%s到%s的时间(分钟): ",
            routes[route_count].stations[i],
            routes[route_count].stations[i + 1]);
        scanf("%d", &routes[route_count].time_between_stations[i]);
    }
    // 输入每小时发车时间
    for (int i = 0; i < routes[route_count].run_count; i++) {
        printf("请输入第%d次发车时间(分钟): ", i + 1);
        scanf("%d", &routes[route_count].departure_time[i]);
    }
    backup_routes();                                                  // 备份上一步的数据
    route_count++;                                                    // 总路线+1
    save_routes();                                                    //保存到磁盘
    printf("线路添加成功！\n");
}

void modify_route() {
    /*
    ***************************路线修改函数****************************
    输入：无
    输出：无
    功能：修改路线文件中指定线路的数据，即针对结构体的操作
    ******************************************************************
    */
    char route_name[MAX_NAME_LENGTH];
    printf("请输入要修改的线路名称: ");
    scanf("%s", route_name);

    for (int i = 0; i < route_count; i++) {
        if (strcmp(routes[i].route_name, route_name) == 0) {
            printf("请输入新的站点数量: ");
            scanf("%d", &routes[i].station_count);
            printf("请输入新的首班车时间(小时): ");
            scanf("%d", &routes[i].start_time);
            printf("请输入新的末班车时间(小时): ");
            scanf("%d", &routes[i].end_time);
            printf("请输入新的每小时发车次数: ");
            scanf("%d", &routes[i].run_count);

            /*重新输入站点*/
            for (int j = 0; j < routes[i].station_count; j++) {
                printf("请输入第%d个站点名称: ", j + 1);
                scanf("%s", routes[i].stations[j]);
            }

            /*重新输入站点间耗时*/
            for (int j = 0; j < routes[i].station_count - 1; j++) {
                printf("请输入%s到%s的时间(分钟): ",
                    routes[i].stations[j],
                    routes[i].stations[j + 1]);
                scanf("%d", &routes[i].time_between_stations[j]);
            }

            /*重新输入发车时间*/
            for (int j = 0; j < routes[i].run_count; j++) {
                printf("请输入第%d次发车时间(分钟): ", j + 1);
                scanf("%d", &routes[i].departure_time[j]);
            }
            backup_routes();                                      // 备份这一次的更改
            save_routes();                                        // 保存更改到磁盘
            printf("线路修改成功！\n");
            return;
        }
    }
    printf("未找到该线路。\n");
}

void delete_route() {
    /*
    ***************************路线删除函数***************************
    输入：无
    输出：无
    功能：删除路线文件中的指定线路
    *****************************************************************
    */
    char route_name[MAX_NAME_LENGTH];
    printf("请输入要删除的线路名称: ");
    scanf("%s", route_name);

    /*遍历routes文件，寻找与routes名相同的文件行数*/
    for (int i = 0; i < route_count; i++) {
        if (strcmp(routes[i].route_name, route_name) == 0) {          // 寻找到目标行数
            for (int j = i; j < route_count - 1; j++) {
                routes[j] = routes[j + 1];                            // 移动后续路线覆盖当前路线
            }
            backup_routes();                                          // 备份这一步的数据
            route_count--;                                            // 总路数-1
            printf("线路删除成功！\n");
            return;
        }
    }
    printf("未找到该线路。\n");
}

// 恢复上一次操作的路线（从备份文件加载）
void restore_last_routes() {
    /*
    ***************************路线改动撤销函数*************************
    输入：无
    输出：无
    功能：将目前的routes重置为一步操作前的routes设置
    ******************************************************************
    */
    int confirm;
    printf("确定要恢复到上一次操作的路线吗？(1=确定,0=取消): ");
    /*合法性检测*/
    if (scanf("%d", &confirm) != 1 || confirm != 1) {
        printf("已取消恢复操作。\n");
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
        return;
    }
    // 从备份文件加载并覆盖
    load_routes_from_file(BACKUP_FILE);
}

// 恢复默认路线（从手动创建的default.txt加载）
void restore_default_routes() {
    /*
    ***************************路线重置部分****************************
    输入：无
    输出：无
    功能：将目前的routes重置为最初的routes设置(默认设置)
    ******************************************************************
    */
    int confirm;
    printf("确定要恢复到默认路线吗？（会覆盖当前所有路线）(1=确定,0=取消): ");
    if (scanf("%d", &confirm) != 1 || confirm != 1) {
        printf("已取消恢复操作。\n");
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
        return;
    }
    // 从默认文件加载并覆盖（需手动创建default.txt）
    load_routes_from_file(DEFAULT_FILE);
}


void display_all_routes() {
    /*
    ************************显示所有路线函数**************************
    输入：无
    输出：无
    功能：打印所有公交线路的详细信息
    ****************************************************************
    */

    //异常处理：若无任何路线信息，反馈给用户
    if (route_count == 0)
    {
        printf("暂无公交线路信息。\n");
        return;
    }
    //打印部分，通过双重循环打印每条路线的详细信息↓
    printf("\n———————————————————————所有公交线路———————————————————————\n");
    for (int i = 0; i < route_count; i++) {
        printf("线路名称: %s\n", routes[i].route_name);
        printf("站点数量: %d  |  首班: %d:00  |  末班: %d:00\n",
            routes[i].station_count,
            routes[i].start_time,
            routes[i].end_time);
        printf("途经站点: ");
        for (int j = 0; j < routes[i].station_count; j++) {
            printf("%s ", routes[i].stations[j]);
        }
        printf("\n\n");
    }
}

int get_current_time() {
    /*
    ************************获取当前时间函数**************************
    输入：无
    输出：返回一个整数，表示当前时间距离当天 00:00 的总分钟数，便于后续时间查询处理
    功能：获取当前系统时间，并将其转换为 “分钟数” 格式返回
    ****************************************************************
    */
    time_t now;
    struct tm* timeinfo;
    time(&now);
    timeinfo = localtime(&now);
    return timeinfo->tm_hour * 60 + timeinfo->tm_min;
}

RouteTime inquire_time(int start_index, int end_index, int i, int current_time) {
    RouteTime time = { 0 };  // 初始化所有成员为0
    int nearest_time = 0;
    int found = 0;
    int before_time = 0;
    BusRoute* route = &routes[i];

    // 计算从首站到起点站的耗时（分方向）
    if (start_index < end_index) {  // 正向
        for (int j = 0; j < start_index; j++) {
            before_time += route->time_between_stations[j];
        }
    }
    else {  // 反向
        for (int j = route->station_count - 2; j >= start_index; j--) {
            before_time += route->time_between_stations[j];
        }
    }

    // 计算起点到终点的行驶时间
    if (start_index < end_index) {  // 正向
        for (int j = start_index; j < end_index; j++) {
            time.sum_time += route->time_between_stations[j];
        }
    }
    else {  // 反向
        for (int j = end_index; j < start_index; j++) {
            time.sum_time += route->time_between_stations[j];
        }
    }

    // 查找最近的可行班次
    for (int j = route->start_time; j < route->end_time; j++)
    {  // 遍历运营小时
        for (int k = 0; k < route->run_count; k++)
        {  // 遍历每小时班次
            nearest_time = j * 60 + route->departure_time[k] + before_time;
            if (nearest_time >= current_time) {
                found = 1;
                break;
            }
        }
        if (found) break;
    }

    // 填充结果
    if (found)
    {
        time.can_arrive = 1;
        time.wait_time = nearest_time - current_time;
        time.sum_time += time.wait_time;
        time.arrive_time = current_time + time.sum_time;
    }
    else
    {
        time.can_arrive = 0;  // 明确标记不可达
    }
    return time;
}

void inquire_route(char* start, char* end, int is_ask) {
    /*
    ***********核心部分：查询路线模块——包含直达和换乘查询******************
    输入：起点、终点
    输出：直达路线信息、换乘路线信息
    功能：根据用户输入的起点和终点，查询并显示所有直达和换乘路线信息
    ******************************************************************
    */

    //-------------------检查用户输入的有效性（防止不存在&重复）--------------------

    // 定义变量接收站点是否存在的结果
    int start_valid, end_valid;
    check_input(start, end, &start_valid, &end_valid);

    // 如果起点或终点不存在，直接返回，不继续查询
    if (!start_valid || !end_valid) {
        return;
    }

    // --------------------------------直达路线查询 --------------------------------

    int found_directly = 0;//found_directly标记是否找到直达路线,找到为1

    printf("\n【直达路线】\n");
    //下面这里起点和终点的查找再同一个for循环下，意味着如果都被找到，就一定再同一个固定的i下，即同一条路线中
    for (int i = 0; i < route_count; i++)
    {
        int start_idx = -1, end_idx = -1;//初始把起点和终点索引设为-1，表示预设不存在
        // 查找起点和终点在当前路线中的索引
        for (int j = 0; j < routes[i].station_count; j++)
        {
            if (strcmp(routes[i].stations[j], start) == 0)
            {
                start_idx = j;
            }
            if (strcmp(routes[i].stations[j], end) == 0)
            {
                end_idx = j;
            }

        }
        //索引更新完毕，检查是否都被找到↓
        if (start_idx != -1 && end_idx != -1) {  // 起点和终点都被找到
            found_directly = 1;
            printf("线路: 坐【%s】：[%s]→[%s]\n",
                routes[i].route_name, start, end);

            // 计算时间并显示，总耗时包含等待时间与坐车时间
            int current_time = get_current_time();
            RouteTime rt = inquire_time(start_idx, end_idx, i, current_time);
            if (rt.can_arrive)
            {
                printf("最近一班车离您还有 %d分钟\n", rt.wait_time);
                printf("预计总耗时: %d分钟\n", rt.sum_time);
                printf("预计到达时间: %d:%02d\n", rt.arrive_time / 60, rt.arrive_time % 60);
            }
            //异常处理：若超出运营时间反馈给用户
            else
            {
                printf("状态: 超出运营时间（%d:00-%d:00）\n", routes[i].start_time, routes[i].end_time);
            }
            printf("------------------------------------------------\n");
        }
    }
    //若未找到任何直达路线，也反馈给用户
    if (!found_directly)
    {
        printf("抱歉，未找到任何直达路线\n");
    }

    // ---------------------------------换乘路线查询 ---------------------------------
    //模仿上面的思想，定义变量found_transform以标记是否找到换乘路线，1为找到
    int found_transform = 0;
    printf("\n【换乘路线】\n");
    //先分别用双层for循环类比上面查找start与end各自所在的路线，并更新索引↓
    for (int start_route = 0; start_route < route_count; start_route++)
    {
        // 查找起点在start_route中的索引
        int start_idx = -1;//预设起点索引不存在，值为-1
        for (int j = 0; j < routes[start_route].station_count; j++)
        {
            if (strcmp(routes[start_route].stations[j], start) == 0)
            {
                start_idx = j;
                break;
            }
        }
        if (start_idx == -1) continue;  // 该路线无起点，再找下一个

        // 同理处理换乘后路线，即终点所在的路线，代码逻辑与查找起点路线类似↓
        for (int end_route = 0; end_route < route_count; end_route++)
        {
            if (start_route == end_route) continue;  // 排除同一条路线

            // 查找终点在end_route中的索引
            int end_idx = -1;
            for (int j = 0; j < routes[end_route].station_count; j++)
            {
                if (strcmp(routes[end_route].stations[j], end) == 0)
                {
                    end_idx = j;
                    break;
                }
            }
            if (end_idx == -1) continue;  // 该路线无终点，再找下一个

            // 查找换乘站（两路线的公共站点，且不是起点/终点）
            for (int s = 0; s < routes[start_route].station_count; s++)
            {
                if (s == start_idx) continue;  // 换乘站不能是起点，遇到就跳过
                for (int e = 0; e < routes[end_route].station_count; e++)
                {
                    if (e == end_idx) continue;  // 换乘站不能是终点，遇到也跳过
                    if (strcmp(routes[start_route].stations[s], routes[end_route].stations[e]) == 0) {
                        found_transform = 1;//找到了就更新found_transform为1
                        char* transfer = routes[start_route].stations[s];
                        printf("换乘方案: \n");
                        printf("先：坐【%s】: [%s]→[%s]\n",
                            routes[start_route].route_name, start, transfer);
                        printf("后：坐【%s】: [%s]→[%s]\n",
                            routes[end_route].route_name, transfer, end);

                        // 计算两段路线时间
                        int current_time = get_current_time();
                        RouteTime rt1 = inquire_time(start_idx, s, start_route, current_time);
                        RouteTime rt2 = inquire_time(e, end_idx, end_route, rt1.arrive_time);

                        if (rt1.can_arrive && rt2.can_arrive) {
                            printf("最近一班车离您还有: %d分钟（首段）\n", rt1.wait_time);
                            printf("预计总耗时: %d分钟（首段%d分钟 + 换乘后%d分钟）\n",
                                rt1.sum_time + rt2.sum_time, rt1.sum_time, rt2.sum_time);
                            printf("预计到达时间: %d:%02d\n",
                                rt2.arrive_time / 60, rt2.arrive_time % 60);
                        }
                        //异常处理：若有任一段超出运营时间，反馈给用户
                        else
                        {
                            printf("状态: 抱歉，存在路线超出运营时间\n");
                        }
                        printf("------------------------------------------------\n");
                    }
                }
            }
        }
    }
    //若未找到任何换乘路线，反馈给用户
    if (!found_transform)
    {
        printf("无换乘路线\n");
    }

    //下面这部分是收藏的调用，在路线查询后问用户是否要收藏
    //is_ask的作用是控制询问用户是否收藏
    int is_star;
    if ((found_directly || found_transform) && is_ask) {
        printf("是否需要收藏该起点终点信息？（1是，其他数字表示退出）：");

        do {
            if (scanf("%d", &is_star) != 1) {
                printf("输入无效，请重新输入（1是，其他数字表示退出）：");
                int c;
                while ((c = getchar()) != '\n' && c != EOF);
                continue;
            }
            if (1) {
                break;
            }
        } while (is_star != 1);

        if (is_star == 1)
        {
            star_routes(start, end);
        }
    }
}

// 交互系统函数实现
void user_menu() {
    /*
    ***************************用户菜单部分***************************
    输入：无
    输出：无
    功能：呈现用户菜单，并且处理用户无效输入
    ******************************************************************
    */
    int choice;
    //呈现用户菜单选项

    do {
        printf("\n【用户菜单】\n");
        printf("1. 查询站点间路线\n");
        printf("2. 查看所有公交线路\n");
        printf("3. 查看收藏夹\n");
        printf("4. 退出\n");
        printf("请选择: ");
        //%d 格式下，scanf 成功读取数字时返回 1，否则返回 0 或 EOF，我们利用这个特点来进行异常处理
        if (scanf("%d", &choice) != 1) {
            printf("输入无效！请输入数字1-4。\n");
            // 清空输入缓冲区（处理残留的非数字字符）
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            choice = 0;  // 给一个默认值，确保switch进入default
            continue;
        }

        switch (choice)
        {
        case 1: {
            char start[MAX_NAME_LENGTH];
            char end[MAX_NAME_LENGTH];
            printf("请输入起点站：");
            scanf("%s", start);
            printf("请输入终点站：");
            scanf("%s", end);
            inquire_route(start, end, 1);
            break;
        }
        case 2:
            display_all_routes();
            show_All();
            printf("是否查看指定线路的地图信息？（1是，其他数字表示退出）\n");
            int if_view_map;
            scanf("%d", &if_view_map);
            if (if_view_map == 1)
            {
                printf("请输入要查看地图的线路名称：");
                char map_route_name[MAX_NAME_LENGTH];
                scanf("%s", map_route_name);

                // 关键：循环查找是否存在该线路，初始化为0表示未找到
                int is_route_found = 0;

                // 类似删除线路中的查找逻辑
                for (int i = 0; i < route_count; i++) {
                    if (strcmp(routes[i].route_name, map_route_name) == 0)
                    {
                        view_map(map_route_name);
                        is_route_found = 1;  // 找到线路后更新为1
                        break;  // 找到后直接退出循环，减少判断
                    }
                }

                // 只有当为0（未找到）时才提示
                if (is_route_found == 0) {
                    printf("未找到该线路\n");
                }
            }
            break;
        case 3: {
            search_star(1);
            break;
        }
        case 4: printf("退出用户菜单。\n"); break;  // 让退出对应case 4
        default:
            printf("无效选择，请输入1-4之间的数字。\n");// 处理有效数字但超出范围的情况
        }
    } while (choice != 4);  // 退出条件对应case 4
}

void admin_menu() {
    /*
    **************************管理员菜单部分**************************
    输入：无
    输出：无
    功能：呈现管理员菜单，并且处理管理员无效输入
    ******************************************************************
    */
    int choice;
    int comfirm = 0;  // 初始化
    do {
        printf("\n【管理员菜单】\n");
        printf("1. 添加线路\n");
        printf("2. 修改线路\n");
        printf("3. 删除线路\n");
        printf("4. 恢复上一次路线\n");  // 新增：恢复上一次（原case4移至case6）
        printf("5. 恢复默认路线\n");     // 新增：恢复默认（原case5移至case7）
        printf("6. 查询站点间路线\n");    // 原case4
        printf("7. 查看所有公交线路\n");  // 原case5
        printf("8. 注册管理员\n");        // 原case6
        printf("9. 初始化所有账号\n");    // 原case7
        printf("10. 退出\n");             // 原case8
        printf("请选择: ");

        // 读取输入并判断是否为有效数字
        if (scanf("%d", &choice) != 1) {
            printf("输入无效！请输入数字1-10。\n");
            // 清空输入缓冲区
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            choice = 0;  // 赋默认值
            continue;
        }

        // 处理选择逻辑
        switch (choice) {
        case 1: add_route(); break;
        case 2: modify_route(); break;
        case 3: delete_route(); break;
        case 4: restore_last_routes(); break;  // 恢复上一次
        case 5: restore_default_routes(); break; // 恢复默认
        case 6: {
            char start[MAX_NAME_LENGTH];
            char end[MAX_NAME_LENGTH];
            printf("请输入起点站：");
            scanf("%s", start);
            printf("请输入终点站：");
            scanf("%s", end);
            int is_ask = 1;
            inquire_route(start, end, is_ask);
            break;
        }
        case 7: display_all_routes(); break;
        case 8: regist_admin(); break;
        case 9:
            printf("确定要初始化所有账号吗？(1=确定,0=取消): ");
            if (scanf("%d", &comfirm) != 1) {
                printf("输入无效，已取消操作。\n");
                int c;
                while ((c = getchar()) != '\n' && c != EOF);
                comfirm = 0;
                break;
            }
            if (comfirm == 1)
            {
                initialize_users();
            }
            else if (comfirm == 0)
            {
                printf("已取消初始化操作。\n");
            }
            else
            {
                printf("无效输入，已取消操作。\n");
            }
            comfirm = 0;
            break;
        case 10: printf("退出管理员菜单。\n"); break;  // 原case8
        default:
            printf("无效选择，请输入1-10之间的数字。\n");
        }
    } while (choice != 10 && comfirm == 0);  // 退出条件对应case10
}

void star_menu(char* filepath, int line_count) {
    /*
    ***************************收藏菜单部分***************************
    输入：无
    输出：无
    功能：呈现收藏夹菜单，并且处理用户无效输入
    ******************************************************************
    */
    int choice;
    do {
        printf("\n【功能选择】\n");
        printf("1. 管理收藏夹\n");
        printf("2. 使用收藏夹\n");
        printf("3. 退出\n");
        printf("请选择: ");

        if (scanf("%d", &choice) != 1) {                   //修改点11
            printf("输入无效！\n\n");
            int c;
            while ((c = getchar()) != '\n' && c != EOF);     // 清空输入缓冲区
            continue;
        }

        switch (choice) {
        case 1: {
            arr_star(filepath, line_count);
            break;
        }
        case 2: {
            use_star(filepath, line_count);
        }
        case 3: {
            break;
        }
        default: printf("无效选择，请重新输入。\n");
        }
    } while (choice != 3);
}


//初始化用户
void initialize_users()
{
    /*
**************************初始化用户函数**************************
输入：无
输出：无
功能：初始化所有用户，并且设置一个默认管理员admin，密码123456
******************************************************************
*/
//重置所有用户
    user_count = 0;
    adm_count = 0;
    //设置默认管理员
    strcpy(users[user_count].username, "admin");
    strcpy(users[user_count].password, "123456");
    users[user_count].is_admin = 1;
    user_count++;
    adm_count++;
    save_users();
    printf("初始管理员账密：\nadmin\n123456\n\n");
}


//登录操作
void login() {
    /*
*****************************登录函数*****************************
输入：无
输出：无
功能：输入用户名、密码，并跳转至对应的管理员/用户界面
******************************************************************
*/
    char username[MAX_NAME_LENGTH], password[MAX_NAME_LENGTH];
    printf("请输入用户名: ");
    scanf("%s", username);
    printf("请输入密码: ");
    get_password(password, MAX_NAME_LENGTH);

    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].username, username) == 0 &&
            strcmp(users[i].password, password) == 0) {
            if (users[i].is_admin) {
                admin_menu();
            }
            else {
                user_menu();
            }
            return;  // 登录成功后退出函数
        }
    }
    printf("用户名或密码错误！\n");
}


//站点合理性判断
int station_exists(char* station) {
    /*
    ************************检验站点存在函数**************************
    输入：站点对应指针
    输出：0或1（不存在与存在）
    功能：检查单个站点在路线中是否存在，便于后续异常处理
    ******************************************************************
    */
    // 我们先写一个函数检查单个站点是否存在，这样用两次这个函数即可判断起点终点是否存在
        // 遍历所有线路
    for (int i = 0; i < route_count; i++) {
        // 遍历当前线路的所有站点
        for (int j = 0; j < routes[i].station_count; j++) {
            // 严格比较站点名称（区分大小写，若需忽略大小写需额外处理）
            if (strcmp(routes[i].stations[j], station) == 0) {
                return 1;  // 存在
            }
        }
    }
    return 0;  // 不存在
}

void check_input(char* start, char* end, int* start_valid, int* end_valid) {
    /*
    ************************检验站点存在函数****************************
    输入: 起点、终点对应指针,并且用两个valid变量接受返回的结果值
    输出: 无
    功能: 检查用户输入的是否是合理的(起点，终点)对, 优先级: 不存在＞重复
    ******************************************************************
    */

    // 检查用户输入的有效性，先检查起点终点是否存在，再看是否重复，基于上面的站点存在函数
        // 初始化默认有效
    *start_valid = 1;
    *end_valid = 1;

    // 检查起点是否存在
    if (!station_exists(start)) {
        printf("您输入的起点“%s”不存在！\n", start);
        *start_valid = 0;
    }

    // 检查终点是否存在
    if (!station_exists(end)) {
        printf("您输入的终点“%s”不存在！\n", end);
        *end_valid = 0;
    }

    // 检查起点和终点是否相同（即使都存在，也需要提示）
    if (strcmp(start, end) == 0 && *start_valid && *end_valid) {
        printf("输入错误：起点和终点不能相同！\n");
        *start_valid = 0;  // 标记为无效，阻止后续查询
        *end_valid = 0;
    }
}


//收藏夹系列函数
int file_exists(const char* file_path) {
    /*
    ************************文件存在函数*******************************
    输入：文本文件的路径
    输出：0/1
    功能：检查用户输入文本名称是否存在
    ******************************************************************
    */
    FILE* fp = fopen(file_path, "r");
    if (fp != NULL) {
        fclose(fp);       // 能打开，说明文件存在
        return 1;
    }
    else {
        return 0;         // 打开失败，说明文件不存在
    }
}

void star_routes(char* start, char* end) {
    /*
    ************************文件存在函数*******************************
    输入：起点站、终点站字符串数组
    输出：无
    功能：将起点终点以一定格式收入collection文件夹内的文本文件中
    ******************************************************************
    */
    char temp_filepath[MAX_NAME_LENGTH];                     // 存储文件路径
    char temp_inputname[MAX_NAME_LENGTH];                    // 临时存储用户输入的文件名
    int choice;

    printf("\n———————————————————————添加收藏———————————————————————\n");
    printf("\n收藏路线操作：\n1. 新建收藏文件\n2. 打开已有收藏文件\n(退出请输入非1、2的任意数字）\n请选择：");

    //输入合法性判断
    do {
        if (scanf("%d", &choice) != 1) {
            printf("输入无效，请重新输入（1/2）：");
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            continue;
        }
    } while (choice != 1 && choice != 2);

    //固定收藏夹名称
    const char* folder = "collection/";

    //新建收藏文件
    if (choice == 1)
    {
        int is_duplicate;                                                  //判断用户的命名是否存在重复的变量
        do {
            printf("\n———————————————————————新建收藏夹———————————————————————\n");
            printf("请输入新收藏文件的名称：");
            scanf("%s", temp_inputname);
            sprintf(temp_filepath, "%s%s.txt", folder, temp_inputname);    //获取用户希望得到的文件地址

            is_duplicate = file_exists(temp_filepath);                     //判断存在与否

            if (is_duplicate) {
                printf("文件 %s 已存在，请重新输入！\n", temp_filepath);
            }
        } while (is_duplicate);
    }


    //已有收藏夹
    else if (choice == 2)
    {
        int is_duplicate;                                                  //判断用户的命名是否存在重复的变量
        do {
            search_star(0);
            printf("请输入收藏文件的名称（无需加.txt,输入BACK可退出）：");
            scanf("%s", temp_inputname);
            sprintf(temp_filepath, "%s%s.txt", folder, temp_inputname);    //获取用户希望得到的文件地址

            if (strcmp(temp_inputname, "BACK") == 0)
            {
                return;
            }

            is_duplicate = file_exists(temp_filepath);                     //判断存在与否

            if (!is_duplicate) {
                printf("文件 %s 不存在，请重新输入！\n", temp_filepath);
            }

        } while (!is_duplicate);
    }

    else
    {
        return;                                                            //非1、2的数字选项，自动退出
    }

    FILE* fp = fopen(temp_filepath, "a");                                  //用追加模式修改txt文件
    if (fp == NULL) {
        printf("错误：无法打开文件！\n");
        return;
    }

    fprintf(fp, "%s -> %s\n", start, end);                                 //写入起点终点:[起点 -> 终点]
    printf("路线已收藏至：%s\n", temp_filepath);                             //提示用户
    fclose(fp);
}

int display_star(const char* search_path, char filenames[][MAX_NAME_LENGTH]) {
    /*
    ************************收藏夹文件展示函数*******************************
    输入：collection文件夹路径  文本文件文件名（二维数组）
    输出：无
    功能：展示collection文件夹中的所有文件名称
    ***********************************************************************
    */

    /*以下方法为豆包提供：使用windows-API实现打开文件夹的操作*/
    WIN32_FIND_DATAA find_data;                                                   //Windows API 中的一个结构体，可获取文件属性、时间戳及文件名等信息           
    HANDLE hFind = FindFirstFileA(search_path, &find_data);                       //句柄。服务于上述结构体 ，其中FindFirstFile函数需要使用ANSI编码，不然无法查找


    // 检查是否成功打开搜索句柄--调试辅助语段
    if (hFind == INVALID_HANDLE_VALUE) {
        DWORD err = GetLastError();
        printf("错误：无法打开收藏文件夹(错误码%d)\n", err);
        if (err == ERROR_PATH_NOT_FOUND) {
            printf("提示：可能collection文件夹不存在，请先创建\n");
        }
        else if (err == ERROR_ACCESS_DENIED) {
            printf("提示：没有访问collection文件夹的权限\n");
        }
        return 0;
    }


    int file_count = 0;                                                            //记录文件数目，便于编号
    printf("\n——————————————————————收藏夹中的收藏文件———————————————————————\n");

    // 遍历文件夹，筛选.txt文件
    // dwFileAttributes 是该结构体中的成员，用于表示文件/目录的属性（如是否为目录、是否为隐藏文件等）
    // FILE_ATTRIBUTE_DIRECTORY 是Windows API定义的宏，代表"目录"属性（值为0x00000010）
    // 使用按位与(&)操作判断属性中是否包含"目录"标志：
    // 若结果非0，表示当前项是目录；若结果为0，表示当前项是文件

    /*排除目录d的影响*/
    do {
        if (!(find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            if (file_count < 100) {
                strncpy(filenames[file_count], find_data.cFileName, MAX_NAME_LENGTH - 1);
                filenames[file_count][MAX_NAME_LENGTH - 1] = '\0';
                printf("%d. %s\n", ++file_count, filenames[file_count - 1]);
            }
            else {
                printf("警告：文件数量超过上限（100），后续文件将被忽略\n");
            }
        }
    } while (FindNextFileA(hFind, &find_data) != 0);

    FindClose(hFind);                                                                 //关闭搜索句柄

    // 处理无.txt文件的情况
    if (file_count == 0) {
        printf("收藏文件夹中没有找到收藏文件\n");
        return 0;
    }

    else {
        return file_count;                                                            //返回文件的数目
    }
}

void open_startxt(char* filepath) {
    /*
    ************************收藏夹文件内容展示函数****************************
    输入：collection文件夹内某文件的路径
    输出：无
    功能：展示collection文件夹中的所有文件名称
    ***********************************************************************
    */
    FILE* fp = fopen(filepath, "r");
    if (fp == NULL) {
        printf("错误：无法打开选中的文件\n");
        return;
    }

    // 提取文件名
    const char* filename = strrchr(filepath, '\\');                    //strrchr函数（返回值为指针）用于在字符串中查找某个字符最后一次出现的位置
    filename = (filename == NULL) ? filepath : filename + 1;           // 如果没有'\'，指针后移动，用完整路径作为文件名

    // 打印文件内容
    printf("\n------- %s 中的收藏路线 -----\n", filename);

    char line[MAX_NAME_LENGTH];                                        //fgets函数的缓冲区
    int line_count = 0;                                                //计录行数

    while (fgets(line, sizeof(line), fp) != NULL) {
        line_count++;
        printf("%d、%s", line_count, line);                            //打序号（行数）与文件名称
    }
    printf("----------------------------------\n");

    fclose(fp);                                                        // 关闭文件

    star_menu(filepath, line_count);                                   //进入收藏菜单函数
    return;
}

void search_star(int is_ask) {
    /*
    ************************收藏夹文件查找函数****************************
    输入：是否使用open函数的逻辑判断参数
    输出：无
    功能：查找收藏夹路径及收藏夹文本文件路径（后续一系列函数的重要参数）、
         查找收藏夹内所有文件、
         打开选中的文件
         （diplay+open两个函数的组合）
    ********************************************************************
    */

    char current_dir[MAX_NAME_LENGTH];                               // 当前路径：当前工作目录
    char search_path[MAX_NAME_LENGTH];                               // 搜索路径：当前工作目录+'\'+文件夹名(collection)
    const char* folder = "collection";                               // 预先创建好collection文件夹

    char filenames[100][MAX_NAME_LENGTH];                            // 最多存储100个文件名
    int file_count = 0;                                              // 文件数目记录

    // 1. 获取当前工作目录
    if (_getcwd(current_dir, sizeof(current_dir)) == NULL)           // _getcwd():获取当前程序执行的工作目录路径
    {
        printf("获取当前工作目录失败...");
        return;
    }

    // 2. 构建搜索路径——_snprintf()函数
    int path_len = _snprintf(
        search_path,                                                 // 存储拼接后的文件夹的搜索路径
        sizeof(search_path),                                         // 最大长度
        "%s\\%s\\*",                                                 // 字符串拼接规则
        current_dir,                                                 // 第一个参数：当前工作目录
        folder);                                                     // 第二个参数：文件夹名——（collection）


    /*检查搜索路径长度是否溢出*/
    /*设置原因：编写代码时由于初始设置大小过小，一直在报错。费尽千辛万苦才发现问题*/
    if (path_len < 0 || path_len >= sizeof(search_path))
    {
        printf("错误：搜索路径过长，超出缓冲区限制\n");
        return;
    }
    search_path[sizeof(search_path) - 1] = '\0';                      // 添加字符串结束符


    // 3. 展示所有收藏文件
    file_count = display_star(search_path, filenames);               //使用display函数：1、打印出所有文件  2、获取文件数量

    /*如果没有查询到txt文件，就自动退出*/
    if (file_count == 0) {
        return;
    }



    /*open控制逻辑参数*/

    /*设置该参数的原因：star_route函数中，使用已有收藏夹时考虑到用户体验
                     需要展示现在已有的文件，但是不需要打开；
                     故需要添加此逻辑控制参数；
    */

    if (is_ask) {

        // 4. 让用户选择要打开的文件
        int choice;
        printf("请选择要打开的文件（输入序号1-%d）：", file_count);

        /*输入合法性判断*/
        while (1) {
            if (scanf("%d", &choice) != 1) {
                printf("输入无效，请输入数字：");
                int c;
                while ((c = getchar()) != '\n' && c != EOF);
                continue;
            }
            if (choice >= 1 && choice <= file_count) {              // 检查序号是否在有效范围
                break;
            }
            printf("序号无效，请输入1-%d之间的数字：", file_count);
        }


        // 5. 拼接目标文件的完整路径
        char target_file[MAX_NAME_LENGTH];                           // 目标文本文件（收藏夹）的路径

        path_len = _snprintf(                                        // 拼接
            target_file,
            sizeof(target_file),
            "%s\\%s\\%s",                                            // 当前目录\collection\文件名.txt
            current_dir,
            folder,
            filenames[choice - 1]
        );

        /*检查路径是否过长*/
        if (path_len < 0 || path_len >= sizeof(target_file)) {
            printf("错误：目标文件路径过长，无法打开\n");
            return;
        }
        target_file[sizeof(target_file) - 1] = '\0';                 // 强制添加结束符

        // 6. 打开文本文件
        open_startxt(target_file);
    }
}

void arr_star(char* filepath, int line_count) {
    /*
    ************************收藏夹管理函数****************************
    输入：文本文件地址、文件内的总行数（即收藏记录条数）
    输出：无
    功能：删除某一条收藏记录，
         随后退出至search_route中的display页面（即收藏夹内文件展示页面）
    ****************************************************************
    */
    int choice;

    /*输入合法性判断*/
    printf("是否需要管理收藏夹？（1是0否）：");
    do {
        if (scanf("%d", &choice) != 1) {
            printf("输入无效！\n");
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            continue;
        }
        if (choice != 1 && choice != 0) {
            printf("请输入0或者1！:");
        }
    } while (choice != 1 && choice != 0);

    /*输入合法且同意管理*/
    if (choice)
    {
        FILE* fp = fopen(filepath, "r");
        if (fp == NULL) {
            printf("错误：无法打开选中的文件\n");
            return;
        }

        printf("请输入您想要删除的路线序号：（退出请输入不存在的路线号码）");

        int route_num;                                           //用于记录选中的序号（行数）
        do {
            /*合法输入判断*/
            if (scanf("%d", &route_num) != 1) {
                printf("输入无效！\n");
                int c;
                while ((c = getchar()) != '\n' && c != EOF);
                continue;
            }
            if (route_num > line_count || route_num < 1) {
                return;
            }
            else {
                break;
            }
        } while (1);

        int current_line = 0;                                    //记录fgets函数处理过的行数
        char line[MAX_NAME_LENGTH] = { 0 };                      //每一行的字符缓存数组
        char temp[MAX_NAME_LENGTH] = { 0 };                      //临时存放数组，保存的缓存数据用来进行覆写操作

        while (fgets(line, sizeof(line), fp) != NULL) {          //缓冲区、缓冲区大小、文件指针
            current_line++;
            if (current_line == route_num) {                     // 找到目标行
                continue;
            }
            strcat(temp, line);                                  //用临时数组存放无需改动的信息:将line数组内的字符存放进入temp末尾
        }

        printf("成功删除！\n");

        fclose(fp);

        fp = fopen(filepath, "w");                               //写入的方式打开
        fputs(temp, fp);                                         //将temp内的字符覆写进入文本文件
        fclose(fp);

        search_star(1);                                          //进入到其中的diplay部分
        printf("\n");
    }
    else {
        return;
    }
}

void use_star(char* filepath, int line_count) {
    /*
    ************************收藏夹使用函数****************************
    输入：文本文件地址、文件内的总行数（即收藏记录条数）
    输出：无
    功能：使用收藏夹内的某一条记录作为起点与终点
    ****************************************************************
    */

    int choice;
    int route_num;
    printf("是否需要使用该收藏夹作为起点与终点的选择样本？（1是/0否）：");

    /*合法输入检测*/
    do {
        if (scanf("%d", &choice) != 1) {
            printf("输入无效！\n");
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            continue;
        }
        if (choice != 1 && choice != 0) {
            printf("请输入0或者1！:");
        }
    } while (choice != 1 && choice != 0);

    if (choice)
    {
        FILE* fp = fopen(filepath, "r");
        if (fp == NULL) {
            printf("错误：无法打开选中的文件\n");
            return;
        }

        printf("请输入您预选的路线序号：");
        /*输入合法性检测*/
        do {
            if (scanf("%d", &route_num) != 1) {
                printf("输入无效！\n");
                int c;
                while ((c = getchar()) != '\n' && c != EOF);
                continue;
            }
            if (route_num > line_count || route_num < 1) {
                printf("请输入正确的路线！\n");
                continue;
            }
            else {
                break;
            }
        } while (1);


        char start[MAX_NAME_LENGTH] = { 0 };
        char end[MAX_NAME_LENGTH] = { 0 };
        char line[MAX_NAME_LENGTH] = { 0 };

        int start_end;                                     // 读取终点字符串的开始下标
        int current_line = 0;

        while (fgets(line, sizeof(line), fp) != NULL) {    // 缓冲区、缓冲区大小、文件指针
            current_line++;
            if (current_line == route_num) {               // 找到目标行
                break;                                     // 此时line中的字符串就是目标行
            }
        }

        /*遍历读取起点与终点*/
        /*
        line中的字符串格式如下：
         [起点 -> 终点\n\0]
         起点最后一个字符和终点第一个字符相隔4个字符
        */

        /*读取起点*/
        for (int i = 0; i < sizeof(line); i++) {
            if (line[i] == ' ') {
                start_end = i + 4;
                start[i] = '\0';                            // 结尾加上'\0'
                break;
            }
            else {
                start[i] = line[i];                        // 正常读取
            }
        }

        /*读取终点*/
        for (int i = 0; i < sizeof(line); i++) {
            if (line[start_end + i] == '\0') {             //遇到字符串结束符
                end[i - 1] = '\0';                         //跳过\n和\0
                break;
            }
            else {
                end[i] = line[start_end + i];              // 正确复制当前字符
            }
        }

        inquire_route(start, end, 0);                       // 进入路线查询函数，确定后续不再询问是否添加收藏
        fclose(fp);
    }
    else
    {
        return;
    }
}

void view_map(char* route_name) {
    // 1. 将ANSI字符串转换为宽字符
    wchar_t w_route_name[MAX_NAME_LENGTH];
    MultiByteToWideChar(CP_ACP, 0, route_name, -1, w_route_name, MAX_NAME_LENGTH);

    // 2. 拼接宽字符图片路径（使用 swprintf_s 并指定缓冲区大小）
    wchar_t img_path[200];
    // 第2个参数是缓冲区大小（200），确保不越界
    swprintf_s(img_path, 200, L"%s.png", w_route_name);
    HWND hConsole = GetConsoleWindow();
    // ---------- 初始化图形窗口并显示地图（修正版） ----------
    initgraph(700, 400);  // 创建800x600窗口

    // 设置窗口标题（使用宽字符函数）
    HWND hWnd = GetHWnd();
    SetWindowTextW(hWnd, L"您所查找的地图如下所示！");  // L表示宽字符，W表示Unicode版本函数
    // 3. 清空图形窗口
    cleardevice();
    ShowWindow(hConsole, SW_RESTORE);  // 从最小化恢复
    ShowWindow(hConsole, SW_SHOW);
    SetForegroundWindow(hConsole);
    // 4. 加载并显示图片
    IMAGE img;
    if (loadimage(&img, img_path, 700, 400, true) == 0) {
        putimage(0, 0, &img);
        settextcolor(WHITE);
        settextstyle(25, 0, L"华文楷体");
        // 调整：Y=350（窗口底部上方50像素，避免贴边），X=150（左右居中）
        outtextxy(450, 350, L"在控制台中按任意键返回");
    }
    else {
        settextcolor(RED);
        settextstyle(40, 0, L"华文楷体");
        wchar_t err[100];
        swprintf_s(err, 60, L"未找到图片：%s.png/(ㄒoㄒ)/~~", w_route_name);
        // 调整：错误提示居中显示，Y=120（上半区）、Y=160（中间），避免超出窗口
        outtextxy(60, 120, err);
        outtextxy(120, 160, L"请检查图片是否在程序目录！");
        // 同上方，返回提示放在底部，保持风格统一
        outtextxy(250, 300, L"按任意键返回");
    }


    _getch();
}

void show_All()
    {
        HWND hConsole = GetConsoleWindow();
        // ---------- 初始化图形窗口并显示地图（修正版） ----------
        initgraph(700, 400);  // 创建800x600窗口

        // 设置窗口标题（使用宽字符函数）
        HWND hWnd = GetHWnd();
        SetWindowTextW(hWnd, L"南京大学城公交路线总图");  // L表示宽字符，W表示Unicode版本函数

        // 加载图片（使用宽字符路径）
        IMAGE mapImg;
        // 注意：图片路径前加 L 表示宽字符，最后一个参数是 bool 类型（true/false）
        if (loadimage(&mapImg, L"map.png", 700, 400, true) == 0) {  // 修正：L"map.jpg" 和 true
            putimage(0, 0, &mapImg);
        }
        else 
        {
            settextcolor(RED);
            settextstyle(40, 0, L"华文楷体");
            wchar_t err[100];
            swprintf_s(err, 60, L"未找到图片：map.png/(ㄒoㄒ)/~~");
            // 调整：错误提示居中显示，Y=120（上半区）、Y=160（中间），避免超出窗口
            outtextxy(60, 120, err);
            outtextxy(120, 160, L"请检查图片是否在程序目录！");
            // 同上方，返回提示放在底部，保持风格统一
            outtextxy(250, 300, L"按任意键返回");
        }
        ShowWindow(hConsole, SW_RESTORE);  // 从最小化恢复
        ShowWindow(hConsole, SW_SHOW);
    }

int main() {

        HWND hConsole = GetConsoleWindow();
        // ---------- 初始化图形窗口并显示地图（修正版） ----------
        initgraph(500, 600);  // 创建800x600窗口

        // 设置窗口标题（使用宽字符函数）
        HWND hWnd = GetHWnd();
        SetWindowTextW(hWnd, L"欢迎进入南京大学城公交查询系统");  // L表示宽字符，W表示Unicode版本函数

        // 加载图片（使用宽字符路径）
        IMAGE coverImg;
        // 注意：图片路径前加 L 表示宽字符，最后一个参数是 bool 类型（true/false）
        if (loadimage(&coverImg, L"cover.png", 500, 600, true) == 0) {  // 修正：L"map.jpg" 和 true
            putimage(0, 0, &coverImg);
        }
        else {
            settextcolor(RED);          // 错误提示用红色，更醒目
            settextstyle(30, 0, L"华文楷体");  // 字体大小16号，适配小窗口
            // 提示1：居中显示“加载失败”核心信息（X=80，Y=150，避免超出500宽的窗口）
            outtextxy(80, 150, L"封面图片加载失败……(T_T)");
            // 提示2：在核心信息下方，补充检查建议（X=50，Y=180，与上一行间距30像素，清晰分层）
            outtextxy(15, 400, L"请检查封面(cover.png)是否在程序目录下");
        }
        ShowWindow(hConsole, SW_RESTORE);  // 从最小化恢复
        ShowWindow(hConsole, SW_SHOW);
        load_routes();   // 加载路线数据
        load_users();    // 加载用户数据

        // 若无用户数据，初始化默认管理员
        if (user_count == 0) {
            strcpy(users[user_count].username, "admin");
            strcpy(users[user_count].password, "123456");
            users[user_count].is_admin = 1;
            user_count++;
            adm_count++;
            save_users();
        }

        int choice;
        do {
            printf("\n【欢迎进入南京大学城公交查询系统】\n");
            printf("1. 登录\n");
            printf("2. 注册普通用户\n");
            printf("3. 退出\n");
            printf("请选择: ");
            if (scanf("%d", &choice) != 1) {
                printf("输入无效！请输入数字1-3。\n");
                int c;
                while ((c = getchar()) != '\n' && c != EOF);
                choice = 0;
                continue;
            }
            switch (choice) {
            case 1: login(); break;
            case 2: regist_user(); break;
            case 3: printf("退出系统。\n");
                printf("感谢您的使用！系统将在3秒后关闭...\n");
                fflush(stdout);
                Sleep(3000);
                printf("退出系统。\n");
                printf("公交路线自动化选择系统————————————制作人：Q24010511 朱勇锬 Q24010516 沈子岩 Q24010518 陈殷骏 \n "); 
                break;
            default: printf("无效选择，请重新输入。\n");
            }
        } while (choice != 3);

        return 0;
    }