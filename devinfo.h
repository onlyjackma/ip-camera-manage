#ifndef __DEVINFO_H
#define __DEVINFO_H
struct devinfo{
	char *ip;
	char *mac;
	char *passwd;
	int main_frame_rate;
	int sub_frame_rate;
	char *main_stream_url;
	char *sub_stream_url;
	int main_video_height;
	int main_video_width;
	int sub_video_height;
	int sub_video_width;
	struct devinfo *next;
};

struct passwd_info{
	char *passwd;
	struct passwd_info *next;
};

struct g_config{
	int mheight;
	int mwidth;
	int sheight;
	int swidth;
	int mframerate;
	int sframerate;
	int mbitrate;
	int sbitrate;
};


struct devinfo *init_devinfo();
void add_devinfo(struct devinfo *HEAD ,struct devinfo *node);
void print_dev_info(struct devinfo *head);
typedef struct devinfo t_devinfo;
typedef struct passwd_info t_passwd;

#define DBG_MSG(fmt,...) syslog(LOG_INFO,"%s [%d] [%s]:"#fmt"\n",__FILE__, __LINE__, __FUNCTION__,##__VA_ARGS__)
//#define DBG_MSG(fmt,...) syslog(LOG_INFO,#fmt"\n",##__VA_ARGS__)


#define MAX_URL_LEN 256
#define MAX_PWD_LEN 32

#endif
