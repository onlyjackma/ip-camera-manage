#include "wsdd.nsmap"  
#include "wsseapi.h"
#include "devinfo.h"
#include <stdio.h>
#include <uuid/uuid.h>
#include <syslog.h>
#include <getopt.h>
#define MULTICAST_ADDRESS "soap.udp://239.255.255.250:3702"
#define ONVIF_USER "admin"
#define ONVIF_PASSWORD "admin"
#define PROC_NAME "ONVIF"
#define SOAP_TIME_OUT 3

static struct soap* ONVIF_Initsoap(const char *was_To, const char *was_Action, int timeout);
int ONVIF_ClientDiscovery( );
int ONVIF_Capabilities(struct __wsdd__ProbeMatches *resp);
void UserGetProfiles(struct soap *soap, struct _tds__GetCapabilitiesResponse *capa_resp);
void UserGetUri(struct soap *soap, struct _trt__GetProfilesResponse *trt__GetProfilesResponse,struct _tds__GetCapabilitiesResponse *capa_resp,char *token,int flag,t_devinfo *node,t_passwd *pnode);
int UserSetSystemTime(struct soap *soap,struct _tds__GetCapabilitiesResponse *capa_resp,t_passwd *pnode);
int UserGetIpMac(struct soap *soap,struct _tds__GetCapabilitiesResponse *capa_resp,t_devinfo *node,t_passwd *pnode);
int UserSetVideoEncoderConfigure(int pcount,struct tt__Profile tprofile,struct _tds__GetCapabilitiesResponse *capa_resp,t_devinfo *node,t_passwd *pnode);



int HasDev = 0;//the number of devices
t_devinfo *HEAD = NULL;
t_passwd *PWD_HEAD = NULL;
t_passwd *PWD_TMP = NULL;
struct g_config config;



t_passwd *init_passwd()
{
	t_passwd *pnode = malloc(sizeof(t_passwd));
	if(pnode == NULL){
		DBG_MSG("%s","init pnode faild");
		return NULL;
	}
	pnode->passwd = malloc(MAX_PWD_LEN);
	pnode->next = NULL;

	if(pnode->passwd == NULL){
		DBG_MSG("%s","init pnode faild");
	}
	memset(pnode->passwd,0,MAX_PWD_LEN);
	return pnode;
}

void add_passwd(t_passwd *head,t_passwd *pnode)
{
	if(head == NULL && pnode != NULL){
		PWD_HEAD = head = pnode;
		return ;
	}
	t_passwd *pcp = head;
	while(pcp->next != NULL){
		pcp = pcp->next;
	}
	pcp->next = pnode;
}

void print_pwd(t_passwd *head)
{
	t_passwd *pcp = PWD_HEAD;
	while(pcp != NULL){
		printf("passwd is %s\n",pcp->passwd);
		pcp = pcp->next;
	}

	printf("config is config->mheight %d,config->mwidth %d,config->mframerate %d,config->mbitrate %d,config->sheight %d,config->swidth %d,config->sframerate %d,config->sbitrate %d\n",config.mheight,config.mwidth,config.mframerate,config.mbitrate,config.sheight,config.swidth,config.sframerate,config.sbitrate);
	

}
void free_pwd(t_passwd *head)
{
	t_passwd *pcp = head;
	t_passwd *ptmp = NULL;

	while(pcp != NULL){
		if(pcp->passwd != NULL){
			free(pcp->passwd);
			pcp->passwd = NULL;
		}
		ptmp = pcp->next;
		free(pcp);
		pcp = ptmp;
		ptmp = NULL;
	}

}
t_devinfo *init_devinfo()
{
	t_devinfo *node = malloc(sizeof(t_devinfo));
	if(node == NULL){
		DBG_MSG("%s","init node faild");
		return NULL;
	}
	node->ip = malloc(20);
	node->mac = malloc(20);
	node->passwd = malloc(20);
	node->main_frame_rate = 0;
	node->sub_frame_rate = 0;
	node->main_stream_url = malloc(MAX_URL_LEN);
	node->sub_stream_url = malloc(MAX_URL_LEN);
	node->main_video_height = 0;
	node->main_video_width = 0;
	node->sub_video_height = 0;
	node->sub_video_width = 0;
	node->next = NULL;

	if(!node->main_stream_url || !node->sub_stream_url || !node->ip || !node->mac|| !node->passwd){
		DBG_MSG("%s","init node member faild");
		return NULL;
	}

	memset(node->ip,0,20);
	memset(node->mac,0,20);
	memset(node->passwd,0,20);
	memset(node->main_stream_url,0,MAX_URL_LEN);
	memset(node->sub_stream_url,0,MAX_URL_LEN);
	return node;
}

void add_devinfo(t_devinfo *head,t_devinfo *node)
{
	if(head==NULL&&node != NULL){
		HEAD = head = node;
		return ;
	}

	t_devinfo *cp = HEAD;
	while(cp->next != NULL){
		cp = cp->next;
	}
	cp->next = node;
}

void print_dev_info(t_devinfo * head)
{
	t_devinfo *cp = head;

	while(cp != NULL){
		//printf("Device ip :%s \n",cp->ip);
		printf("Main,%s,%s,%s,%d,%d,%d,%s\n",
				cp->ip,cp->mac,cp->main_stream_url,cp->main_video_height,cp->main_video_width,cp->main_frame_rate,cp->passwd);
		printf("Sub,%s,%s,%s,%d,%d,%d,%s\n",
				cp->ip,cp->mac,cp->sub_stream_url,cp->sub_video_height,cp->sub_video_width,cp->sub_frame_rate,cp->passwd);
		cp = cp->next;
	}
}

void free_devinfo(t_devinfo *head)
{
	t_devinfo *cp = head;
	t_devinfo *tmp_d = NULL;
		
	while(cp != NULL){
		if(cp->main_stream_url != NULL){
			free(cp->main_stream_url);
			cp->main_stream_url = NULL;
		}
		if(cp->sub_stream_url != NULL){
			free(cp->sub_stream_url);
			cp->sub_stream_url= NULL;
		}
		if(cp->ip != NULL){
			free(cp->ip);
			cp->ip = NULL;
		}
		if(cp->mac != NULL){
			free(cp->mac);
			cp->mac = NULL;
		}
		if(cp->passwd!= NULL){
			free(cp->passwd);
			cp->passwd = NULL;
		}
		tmp_d = cp->next;
		cp->next = NULL;
		free(cp);
		cp = tmp_d;
		tmp_d = NULL;
	}


}

int parse_passwd(int argc,char **argv)
{
	//DBG_MSG("======================%d",argc);
	int count = 1;
	if(argc >1){
		for(;count < argc;count++){
			t_passwd *pnode = init_passwd();
			if(pnode != NULL){
				memcpy(pnode->passwd,argv[count],strlen(argv[count]));
				add_passwd(PWD_HEAD,pnode);
			}
		}

	}else{
		printf("usage :%s pwd1 [pwd2 ...]",PROC_NAME);
		exit(-1);
	}
	return 0;
	
}

int parse_passwd_v2(char *pass)
{
	//printf("======================%s\n",pass);
	
	t_passwd *pnode = init_passwd();
	if(pnode != NULL){
		memcpy(pnode->passwd,pass,strlen(pass));
		add_passwd(PWD_HEAD,pnode);
	}
	return 0;
	
}

void usage()
{

}
void parse_parameters(int argc ,char **argv)
{
	int opt;
	int optind = 0;
	char *opt_str="f:s:t:o:p:q:r:u:v:w:x:h";
	static struct option long_options[] ={
		{"passwd1",1,NULL,'f'},
		{"passwd2",1,NULL,'s'},
		{"passwd3",1,NULL,'t'},
		{"mheight",1,NULL,'o'},
		{"mwidth",1,NULL,'p'},
		{"mframerate",1,NULL,'q'},
		{"mbitrate",1,NULL,'r'},
		{"sheight",1,NULL,'u'},
		{"swidth",1,NULL,'v'},
		{"sframerate",1,NULL,'w'},
		{"sbitrate",1,NULL,'x'},
		{"help",0,NULL,'h'},
		{NULL,0,NULL,0}
	};

	while((opt=getopt_long(argc,argv,opt_str,long_options,&optind)) != -1){
		switch(opt){
			case 'f':
				parse_passwd_v2(strdup(optarg));
				break;
			case 's':
				parse_passwd_v2(strdup(optarg));
				break;
			case 't':
				parse_passwd_v2(strdup(optarg));
				break;
			case 'o':
				config.mheight = atoi(optarg);
				break;
			case 'p':
				config.mwidth = atoi(optarg);
				break;
			case 'q':
				config.mframerate = atoi(optarg);
				break;
			case 'r':
				config.mbitrate = atoi(optarg);
				break;
			case 'u':
				config.sheight = atoi(optarg);
				break;
			case 'v':
				config.swidth = atoi(optarg);
				break;
			case 'w':
				config.sframerate = atoi(optarg);
				break;
			case 'x':
				config.sbitrate = atoi(optarg);
				break;
			case 'h':
				usage();
				exit(0);
			default:
				printf("This is a default");
				break;

		}

	}
	
}

int main(int argc ,char *argv[])
{
	//发现协议
	//parse_passwd(argc,argv);
	parse_parameters(argc,argv);
	//print_pwd(PWD_HEAD);
	openlog(PROC_NAME, LOG_CONS | LOG_PID, 0);
	if (ONVIF_ClientDiscovery() != 0 )
	{
		printf("discovery failed!,show run this command :ip route add 239.255.255.250/32 dev br-lan1\n");
		return -1;
	}
	print_dev_info(HEAD);
	free_devinfo(HEAD);
	free_pwd(PWD_HEAD);
	closelog();
	return 0;
}


//初始化soap函数
static struct soap* ONVIF_Initsoap(const char *was_To, const char *was_Action, 
		int timeout)
{
	struct SOAP_ENV__Header *header;	
	struct soap *soap = NULL; 
	unsigned char macaddr[6];
	char _HwId[1024];
	unsigned int Flagrand;
	soap = soap_new();
	soap_header(soap);
	header = soap->header;
	if(soap == NULL)
	{
		DBG_MSG("[%d]soap = NULL\n", __LINE__);
		return NULL;
	}
	soap_set_namespaces( soap, namespaces);
	//超过5秒钟没有数据就退出
	if (timeout > 0)
	{
		soap->recv_timeout = timeout;
		soap->send_timeout = timeout;
		soap->connect_timeout = timeout;
	}
	else
	{
		//如果外部接口没有设备默认超时时间的话，我这里给了一个默认值10s
		soap->recv_timeout    = 10;
		soap->send_timeout    = 10;
		soap->connect_timeout = 10;
	}
	//soap->sndbuf = 1024;

	soap_default_SOAP_ENV__Header(soap, header);

	// 为了保证每次搜索的时候MessageID都是不相同的！因为简单，直接取了随机值
	/*
	   srand((int)time(0));
	   Flagrand = rand()%9000 + 1000; //保证四位整数
	   macaddr[0] = 0x1; macaddr[1] = 0x2; macaddr[2] = 0x3; macaddr[3] = 0x4; macaddr[4] = 0x5; macaddr[5] = 0x6;
	   sprintf(_HwId,"urn:uuid:%ud68a-1dd2-11b2-a105-%02X%02X%02X%02X%02X%02X", 
	   Flagrand, macaddr[0], macaddr[1], macaddr[2], macaddr[3], macaddr[4], macaddr[5]);
	   header->wsa__MessageID =(char *)malloc( 100);
	   memset(header->wsa__MessageID, 0, 100);
	   strncpy(header->wsa__MessageID, _HwId, strlen(_HwId));
	   */
	uuid_t uuid;
	char guid_string[100]={0};
	uuid_generate(uuid);
	uuid_unparse(uuid, guid_string);

	header->wsa__MessageID =(char *)malloc( 100);
	memset(header->wsa__MessageID,0,100);
	//header->wsa__MessageID = guid_string; 
	strncpy(header->wsa__MessageID, guid_string, strlen(guid_string));
	//printf("header->wsa__MessageID is %s ,guid_string is %s\n",header->wsa__MessageID, guid_string);

	if (was_Action != NULL)
	{
		header->wsa__Action =(char *)malloc(1024);
		memset(header->wsa__Action, '\0', 1024);
		strncpy(header->wsa__Action, was_Action, 1024);//"http://schemas.xmlsoap.org/ws/2005/04/discovery/Probe";
	}
	if (was_To != NULL)
	{
		header->wsa__To =(char *)malloc(1024);
		memset(header->wsa__To, '\0', 1024);
		strncpy(header->wsa__To,  was_To, 1024);//"urn:schemas-xmlsoap-org:ws:2005:04:discovery";	
	}
	soap->header = header;
	return soap;
}

int ONVIF_ClientDiscovery( )
{
	int retval = SOAP_FAULT;
	wsdd__ProbeType req;       
	struct __wsdd__ProbeMatches resp;
	wsdd__ScopesType sScope;
	struct soap *soap = NULL; 

	const char *was_To = "urn:schemas-xmlsoap-org:ws:2005:04:discovery";
	const char *was_Action = "http://schemas.xmlsoap.org/ws/2005/04/discovery/Probe";
	//这个就是传递过去的组播的ip地址和对应的端口发送广播信息	
	const char *soap_endpoint = "soap.udp://239.255.255.250:3702/";

	//这个接口填充一些信息并new返回一个soap对象，本来可以不用额外接口，
	// 但是后期会作其他操作，此部分剔除出来后面的操作就相对简单了,只是调用接口就好
	soap = ONVIF_Initsoap(was_To, was_Action, SOAP_TIME_OUT);

	//soap_default_SOAP_ENV__Header(soap, &header);
	//soap->header = &header;

	soap_default_wsdd__ScopesType(soap, &sScope);
	sScope.__item = "";
	soap_default_wsdd__ProbeType(soap, &req);
	req.Scopes = &sScope;
	req.Types = "tdn:NetworkVideoTransmitter";

	retval = soap_send___wsdd__Probe(soap, soap_endpoint, NULL, &req);		
	//发送组播消息成功后，开始循环接收各位设备发送过来的消息
	while (retval == SOAP_OK)
	{
		retval = soap_recv___wsdd__ProbeMatches(soap, &resp);//这个函数用来接受probe消息，存在resp里面
		if (retval == SOAP_OK) 
		{
			if (soap->error)
			{
				DBG_MSG("[%d]: recv soap error :%d, %s, %s\n", __LINE__, soap->error, *soap_faultcode(soap), *soap_faultstring(soap)); 
				retval = soap->error;
			}
			else //成功接收某一个设备的消息
			{
				HasDev ++;

				if (resp.wsdd__ProbeMatches->ProbeMatch != NULL && resp.wsdd__ProbeMatches->ProbeMatch->XAddrs != NULL)
				{
					//printf(" ################  recv  %d devices info #### \n", HasDev );

					//printf("Target Service Address  : %s\n", resp.wsdd__ProbeMatches->ProbeMatch->XAddrs);	
					//printf("Target EP Address       : %s\n", resp.wsdd__ProbeMatches->ProbeMatch->wsa__EndpointReference.Address);  
					//printf("Target Type             : %s\n", resp.wsdd__ProbeMatches->ProbeMatch->Types);  
					//printf("Target Metadata Version : %d\n", resp.wsdd__ProbeMatches->ProbeMatch->MetadataVersion); 
					ONVIF_Capabilities(&resp);//这里可以调用能力值获取函数
					//sleep(1);
				}

			}
		}
		else if (soap->error)  
		{  
			if (HasDev == 0)
			{
				DBG_MSG("[%s][%s][Line:%d] Thers Device discovery or soap error: %d, %s, %s \n",__FILE__, __func__, __LINE__, soap->error, *soap_faultcode(soap), *soap_faultstring(soap)); 
				retval = soap->error;  
			}
			else 
			{
				//DBG_MSG(" [%s]-[%d] Search end! It has Searched %d devices! \n", __func__, __LINE__, HasDev);
				retval = 0;
			}
			break;
		}  
	}

	soap_destroy(soap); 
	soap_end(soap); 
	soap_free(soap);

	return retval;
}


int ONVIF_Capabilities(struct __wsdd__ProbeMatches *resp)
{  
	struct _tds__GetCapabilities capa_req;
	struct _tds__GetCapabilitiesResponse capa_resp;

	struct soap *soap = NULL; 
	//struct SOAP_ENV__Header header;		
	int retval = 0;  
	soap = ONVIF_Initsoap(NULL, NULL, 10);
	char *soap_endpoint = (char *)malloc(256);  
	memset(soap_endpoint, '\0', 256);  
	//海康的设备，固定ip连接设备获取能力值 ,实际开发的时候，"172.18.14.22"地址以及80端口号需要填写在动态搜索到的具体信息  
	sprintf(soap_endpoint, resp->wsdd__ProbeMatches->ProbeMatch->XAddrs);  
	capa_req.Category = (enum tt__CapabilityCategory *)soap_malloc(soap, sizeof(int));  

	capa_req.__sizeCategory = 1;  
	*(capa_req.Category) = (enum tt__CapabilityCategory)0;  
	//此句也可以不要，因为在接口soap_call___tds__GetCapabilities中判断了，如果此值为NULL,则会给它赋值  
	const char *soap_action = "http://www.onvif.org/ver10/device/wsdl/GetCapabilities";  
	capa_resp.Capabilities = (struct tt__Capabilities*)soap_malloc(soap,sizeof(struct tt__Capabilities)) ;

	//这里处理鉴权函数
	
	//soap_wsse_add_UsernameTokenDigest(soap, "user", ONVIF_USER, ONVIF_PASSWORD);
	do  
	{  
		int result = soap_call___tds__GetCapabilities(soap, soap_endpoint, soap_action, &capa_req, &capa_resp);  
		if (soap->error)  
		{  
			DBG_MSG("[%s][%d]--->>> soap error: %d, %s, %s\n", __func__, __LINE__, soap->error, *soap_faultcode(soap), *soap_faultstring(soap));  
			retval = soap->error;
			break;  
		}  
		else   //获取参数成功  
		{  
			// 走到这里的时候，已经就是验证成功了，可以获取到参数了，  
			// 在实际开发的时候，可以把capa_resp结构体的那些需要的值匹配到自己的私有协议中去，简单的赋值操作就好              
			//printf("[%s][%d] Get capabilities success !\n", __func__, __LINE__);  
			if(capa_resp.Capabilities == NULL)
				DBG_MSG("GetCapabilities failed! result = %d\n", result);
			else
			{
				//printf(" Media->XAddr=%s \n", capa_resp.Capabilities->Media->XAddr);
				UserGetProfiles(soap, &capa_resp);//
			}
		}
	}while(0);  

	free(soap_endpoint);  
	soap_endpoint = NULL;  
	soap_destroy(soap);  
	return retval;  
}  


void UserGetProfiles(struct soap *soap, struct _tds__GetCapabilitiesResponse *capa_resp)  
{  
	struct _trt__GetProfiles trt__GetProfiles;
	struct _trt__GetProfilesResponse trt__GetProfilesResponse;

	int result= SOAP_OK ; 
	int flag = 0;
	t_devinfo *node = NULL;
	//printf("\n-------------------Getting Onvif Devices Profiles--------------\n\n");
	
	PWD_TMP = PWD_HEAD;
pwd_again:
	if(PWD_TMP !=NULL){
		soap_wsse_add_UsernameTokenDigest(soap,"user", ONVIF_USER, PWD_TMP->passwd);
	}else{
		soap_wsse_add_UsernameTokenDigest(soap,"user", ONVIF_USER, ONVIF_PASSWORD);
		PWD_TMP = init_passwd();
		memcpy(PWD_TMP->passwd,ONVIF_PASSWORD,strlen(ONVIF_PASSWORD));
		flag = 1;
	}
	
	//soap_wsse_add_UsernameTokenDigest(soap,"user", ONVIF_USER, pnode->passwd);

	result = soap_call___trt__GetProfiles(soap, capa_resp->Capabilities->Media->XAddr, NULL, &trt__GetProfiles, &trt__GetProfilesResponse);  
	if (result != SOAP_OK)  
		//NOTE: it may be regular if result isn't SOAP_OK.Because some attributes aren't supported by server.  
	{  
		//DBG_MSG("soap error: %d, %s, %s\n", soap->error, *soap_faultcode(soap), *soap_faultstring(soap)); 
		result = soap->error; 	 
		if(soap->error == 12 && flag != 1){
			//DBG_MSG("passwd error! try an other one passwd is %s ",PWD_TMP->passwd);
			if(PWD_TMP != NULL){
				PWD_TMP=PWD_TMP->next;
				goto pwd_again;
			}
		}
		
		return ;  
	}else{  
		//printf("soap error: %d, %s, %s\n", soap->error, *soap_faultcode(soap), *soap_faultstring(soap));  
		//printf("\n-------------------Profiles Get OK--------------\n\n");  
		if(trt__GetProfilesResponse.Profiles!=NULL)  
		{  
			
			//printf("size of profiles %d \n",trt__GetProfilesResponse.__sizeProfiles);
			int pcount = 0;
			if (trt__GetProfilesResponse.__sizeProfiles >0){
				node = init_devinfo();
				if(node == NULL){
					DBG_MSG("%s","init devinfo faild");
					return;
				}
			}
			
			for (;pcount < trt__GetProfilesResponse.__sizeProfiles ;pcount++){
				/*
				if(trt__GetProfilesResponse.Profiles[pcount].Name!=NULL){  
					printf("Profiles Name:%s  \n",trt__GetProfilesResponse.Profiles[pcount].Name);
				}
				
				if(trt__GetProfilesResponse.Profiles[pcount].token!=NULL){  
					printf("Profiles Taken:%s\n",trt__GetProfilesResponse.Profiles[pcount].token);  
				} 
				*/
				
				if(trt__GetProfilesResponse.Profiles[pcount].VideoEncoderConfiguration->RateControl != NULL){
					//printf("Profiles Frame limit is :%d\n",trt__GetProfilesResponse.Profiles[pcount].VideoEncoderConfiguration->RateControl->FrameRateLimit);  
					if(pcount == 0){
						node->main_frame_rate = trt__GetProfilesResponse.Profiles[pcount].VideoEncoderConfiguration->RateControl->FrameRateLimit;
						node->main_video_height = trt__GetProfilesResponse.Profiles[pcount].VideoEncoderConfiguration->Resolution->Height;
						node->main_video_width = trt__GetProfilesResponse.Profiles[pcount].VideoEncoderConfiguration->Resolution->Width;
					}else{
						node->sub_frame_rate = trt__GetProfilesResponse.Profiles[pcount].VideoEncoderConfiguration->RateControl->FrameRateLimit;
						node->sub_video_height = trt__GetProfilesResponse.Profiles[pcount].VideoEncoderConfiguration->Resolution->Height;
						node->sub_video_width = trt__GetProfilesResponse.Profiles[pcount].VideoEncoderConfiguration->Resolution->Width;
					}
				}
				UserSetVideoEncoderConfigure(pcount,trt__GetProfilesResponse.Profiles[pcount],capa_resp,node,PWD_TMP);		
				UserGetUri(soap, &trt__GetProfilesResponse, capa_resp,trt__GetProfilesResponse.Profiles[pcount].token,pcount,node,PWD_TMP);
			}
		}else{  
			DBG_MSG("Profiles Get inner Error\n");  
		}  
		//printf("Profiles Get Procedure over\n");
		UserGetIpMac(soap,capa_resp,node,PWD_TMP);
		UserSetSystemTime(soap,capa_resp,PWD_TMP);
		add_devinfo(HEAD,node);
	}
}  

void UserGetUri(struct soap *soap, struct _trt__GetProfilesResponse *trt__GetProfilesResponse,struct _tds__GetCapabilitiesResponse *capa_resp,char *token,int flag, t_devinfo *node, t_passwd *pnode)  
{  
	struct _trt__GetStreamUri *trt__GetStreamUri = (struct _trt__GetStreamUri *)malloc(sizeof(struct _trt__GetStreamUri));
	struct _trt__GetStreamUriResponse *trt__GetStreamUriResponse = (struct _trt__GetStreamUriResponse *)malloc(sizeof(struct _trt__GetStreamUriResponse));
	int result=0 ;  
	trt__GetStreamUri->StreamSetup = (struct tt__StreamSetup*)soap_malloc(soap,sizeof(struct tt__StreamSetup));//初始化，分配空间  
	trt__GetStreamUri->StreamSetup->Stream = 0;//stream type  

	trt__GetStreamUri->StreamSetup->Transport = (struct tt__Transport *)soap_malloc(soap, sizeof(struct tt__Transport));//初始化，分配空间  
	trt__GetStreamUri->StreamSetup->Transport->Protocol = 0;  
	trt__GetStreamUri->StreamSetup->Transport->Tunnel = 0;  
	trt__GetStreamUri->StreamSetup->__size = 1;  
	trt__GetStreamUri->StreamSetup->__any = NULL;  
	trt__GetStreamUri->StreamSetup->__anyAttribute =NULL;  


	//trt__GetStreamUri->ProfileToken = trt__GetProfilesResponse->Profiles->token ;  
	trt__GetStreamUri->ProfileToken = token ;  

	//printf("\n\n---------------Getting Uri----------------\n\n");  

	soap_wsse_add_UsernameTokenDigest(soap,"user", ONVIF_USER, pnode->passwd);  
	soap_call___trt__GetStreamUri(soap, capa_resp->Capabilities->Media->XAddr, NULL, trt__GetStreamUri, trt__GetStreamUriResponse);  


	if (soap->error) {  
		DBG_MSG("soap error: %d, %s, %s\n", soap->error, *soap_faultcode(soap), *soap_faultstring(soap));  
		result = soap->error;  

	} else {  
		if(trt__GetStreamUriResponse->MediaUri->Uri != NULL){
			//printf("!!!!NOTE: RTSP Addr Get Done is :%s \n",trt__GetStreamUriResponse->MediaUri->Uri);
			if(flag == 0){
				memcpy(node->main_stream_url,trt__GetStreamUriResponse->MediaUri->Uri,strlen(trt__GetStreamUriResponse->MediaUri->Uri));
			}else{
				memcpy(node->sub_stream_url,trt__GetStreamUriResponse->MediaUri->Uri,strlen(trt__GetStreamUriResponse->MediaUri->Uri));
			}
		}else{
			DBG_MSG("%s","Get MediaUri->Uri faild");
		}
	}  
}  
int UserSetSystemTime(struct soap *soap,struct _tds__GetCapabilitiesResponse *capa_resp,t_passwd *pnode)
{
	soap = ONVIF_Initsoap(NULL, NULL, SOAP_TIME_OUT);
	struct tm *tt;
	time_t now;
	//printf("------------------------UserSetSystemTime----------------------------\n");
	struct _tds__SetSystemDateAndTime *tds__SetSystemDateAndTime = (struct _tds__SetSystemDateAndTime*)soap_malloc(soap ,sizeof(struct _tds__SetSystemDateAndTime));
	struct _tds__SetSystemDateAndTimeResponse *tds__SetSystemDateAndTimeResponse =  (struct _tds__SetSystemDateAndTimeResponse*)soap_malloc(soap ,sizeof(struct _tds__SetSystemDateAndTimeResponse));
	tds__SetSystemDateAndTime->TimeZone = (struct tt__TimeZone*) soap_malloc(soap,sizeof(struct tt__TimeZone));
	tds__SetSystemDateAndTime->TimeZone->TZ = (char *) malloc(12);
	memset(tds__SetSystemDateAndTime->TimeZone->TZ,0,12);
	strncpy(tds__SetSystemDateAndTime->TimeZone->TZ,"UTC-0",strlen("UTC-0"));
	tds__SetSystemDateAndTime->DateTimeType = tt__SetDateTimeType__Manual ;
	tds__SetSystemDateAndTime->DaylightSavings = 0;
	tds__SetSystemDateAndTime->UTCDateTime=(struct tt__DateTime*) soap_malloc(soap,sizeof(struct tt__DateTime));
	tds__SetSystemDateAndTime->UTCDateTime->Date = (struct tt__Date*) soap_malloc(soap,sizeof(struct tt__Date));
	tds__SetSystemDateAndTime->UTCDateTime->Time = (struct tt__Time*) soap_malloc(soap,sizeof(struct tt__Time));

	time(&now);
	tt = localtime(&now);
	//printf("%d %d %d %d %d %d \n",tt->tm_year+1900,tt->tm_mon,tt->tm_mday,tt->tm_hour,tt->tm_min,tt->tm_sec);
	tds__SetSystemDateAndTime->UTCDateTime->Date->Year = tt->tm_year+1900;
	tds__SetSystemDateAndTime->UTCDateTime->Date->Month = tt->tm_mon+1;
	tds__SetSystemDateAndTime->UTCDateTime->Date->Day = tt->tm_mday;
	tds__SetSystemDateAndTime->UTCDateTime->Time->Hour = tt->tm_hour;
	tds__SetSystemDateAndTime->UTCDateTime->Time->Minute = tt->tm_min;
	tds__SetSystemDateAndTime->UTCDateTime->Time->Second = tt->tm_sec;
	//printf("-11-----------------------UserSetSystemTime---------------------------%s-\n",capa_resp->Capabilities->Device->XAddr);
	soap_wsse_add_UsernameTokenDigest(soap,"user", ONVIF_USER, pnode->passwd);  

	int ret = soap_call___tds__SetSystemDateAndTime(soap , capa_resp->Capabilities->Device->XAddr, NULL,tds__SetSystemDateAndTime ,tds__SetSystemDateAndTimeResponse);
	if (ret == -1){
		if(soap->error){
			DBG_MSG("soap_call___tds__SetSystemDateAndTime faild %d ,%s ,%s\n",soap->error, *soap_faultcode(soap), *soap_faultstring(soap));
			ret = soap->error;
		}
	}else{
		//DBG_MSG("soap_call___tds__SetSystemDateAndTime OK !!!!!!!!!\n");
		;
	}
	return soap->error;
}

int UserGetIpMac(struct soap *soap,struct _tds__GetCapabilitiesResponse *capa_resp,t_devinfo *node,t_passwd *pnode)
{
	struct _tds__GetNetworkInterfaces *tds__GetNetworkInterfaces = (struct _tds__GetNetworkInterfaces*) soap_malloc(soap,sizeof(struct _tds__GetNetworkInterfaces));
	struct _tds__GetNetworkInterfacesResponse *tds__GetNetworkInterfacesResponse = (struct _tds__GetNetworkInterfacesResponse*) soap_malloc(soap,sizeof(struct _tds__GetNetworkInterfacesResponse));
	
	soap_wsse_add_UsernameTokenDigest(soap,"user", ONVIF_USER, pnode->passwd);
	
	//tds__GetNetworkInterfaces->tds__GetNetworkInterfaces
	int ret = soap_call___tds__GetNetworkInterfaces(soap,capa_resp->Capabilities->Device->XAddr,NULL,tds__GetNetworkInterfaces,tds__GetNetworkInterfacesResponse);
	if(soap->error){
		DBG_MSG("soap_call___tds__GetNetworkInterfaces faild %d ,%s ,%s\n",soap->error, *soap_faultcode(soap), *soap_faultstring(soap));
		return soap->error;
	}else{
		//printf("ip is %s",tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4->Config->FromDHCP->Address?tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4->Config->Manual->Address :NULL);
		if(tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4->Config->FromDHCP != NULL){
			memcpy(node->ip,tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4->Config->FromDHCP->Address,strlen(tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4->Config->FromDHCP->Address));
		}else if (tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4->Config->Manual != NULL ){
			memcpy(node->ip,tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4->Config->Manual->Address,strlen(tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4->Config->Manual->Address ));
		}
		//printf("mac is %s",tds__GetNetworkInterfacesResponse->NetworkInterfaces->Info->HwAddress);
		if(tds__GetNetworkInterfacesResponse->NetworkInterfaces->Info->HwAddress  !=NULL){
			memcpy(node->mac,tds__GetNetworkInterfacesResponse->NetworkInterfaces->Info->HwAddress,strlen(tds__GetNetworkInterfacesResponse->NetworkInterfaces->Info->HwAddress));
		}
		memcpy(node->passwd,pnode->passwd,strlen(pnode->passwd));
	}
	
	return ret;
}

int UserSetVideoEncoderConfigure(int pcount,struct tt__Profile tprofile,struct _tds__GetCapabilitiesResponse *capa_resp,t_devinfo *node,t_passwd *pnode)
{
	if(pcount == 0 ){
		if(!(config.mheight || config.mwidth || config.mframerate || config.mbitrate)){
			return 0;
		}
		
	}else if(pcount == 1){
		if(!(config.sheight || config.swidth || config.sframerate || config.sbitrate)){
			return 0;
		}
	}
	
	struct soap *soap = ONVIF_Initsoap(NULL, NULL, SOAP_TIME_OUT);
	
	struct _trt__SetVideoEncoderConfiguration *trt__SetVideoEncoderConfiguration = (struct _trt__SetVideoEncoderConfiguration*)soap_malloc(soap,sizeof(struct _trt__SetVideoEncoderConfiguration ));
	struct _trt__SetVideoEncoderConfigurationResponse *trt__SetVideoEncoderConfigurationResponse = (struct _trt__SetVideoEncoderConfigurationResponse*)soap_malloc(soap,sizeof(struct _trt__SetVideoEncoderConfigurationResponse));

	trt__SetVideoEncoderConfiguration->Configuration = (struct tt__VideoEncoderConfiguration*) soap_malloc(soap,sizeof(struct tt__VideoEncoderConfiguration));

	trt__SetVideoEncoderConfiguration->Configuration->Encoding = tprofile.VideoEncoderConfiguration->Encoding;
	trt__SetVideoEncoderConfiguration->Configuration->token = tprofile.VideoEncoderConfiguration->token;
	trt__SetVideoEncoderConfiguration->Configuration->Name = tprofile.VideoEncoderConfiguration->Name;
	trt__SetVideoEncoderConfiguration->Configuration->Quality = tprofile.VideoEncoderConfiguration->Quality;
	if(tprofile.VideoEncoderConfiguration->H264 != NULL){
		//trt__SetVideoEncoderConfiguration->Configuration->H264 = tprofile.VideoEncoderConfiguration->H264;
		trt__SetVideoEncoderConfiguration->Configuration->H264 = (struct tt__H264Configuration*) soap_malloc(soap,sizeof(struct tt__H264Configuration));
		//memcpy(trt__SetVideoEncoderConfiguration->Configuration->H264,tprofile.VideoEncoderConfiguration->H264,sizeof(struct tt__H264Configuration));
		trt__SetVideoEncoderConfiguration->Configuration->H264->GovLength = tprofile.VideoEncoderConfiguration->H264->GovLength;
		trt__SetVideoEncoderConfiguration->Configuration->H264->H264Profile = tprofile.VideoEncoderConfiguration->H264->H264Profile;
		trt__SetVideoEncoderConfiguration->Configuration->MPEG4 = NULL;
	}else if(tprofile.VideoEncoderConfiguration->MPEG4 != NULL){
		//printf("tprofile.VideoEncoderConfiguration->H264 is NULL\n");
		trt__SetVideoEncoderConfiguration->Configuration->MPEG4 = (struct tt__Mpeg4Configuration*) soap_malloc(soap,sizeof(struct tt__Mpeg4Configuration));
		trt__SetVideoEncoderConfiguration->Configuration->MPEG4->GovLength = tprofile.VideoEncoderConfiguration->MPEG4->GovLength;
		trt__SetVideoEncoderConfiguration->Configuration->MPEG4->Mpeg4Profile = tprofile.VideoEncoderConfiguration->MPEG4->Mpeg4Profile;
		trt__SetVideoEncoderConfiguration->Configuration->H264 = NULL;
		
		//trt__SetVideoEncoderConfiguration->Configuration->MPEG4 = tprofile.VideoEncoderConfiguration->MPEG4;
	}
	trt__SetVideoEncoderConfiguration->Configuration->UseCount = tprofile.VideoEncoderConfiguration->UseCount;
	trt__SetVideoEncoderConfiguration->Configuration->Multicast = tprofile.VideoEncoderConfiguration->Multicast;
	trt__SetVideoEncoderConfiguration->Configuration->SessionTimeout = tprofile.VideoEncoderConfiguration->SessionTimeout;
	trt__SetVideoEncoderConfiguration->ForcePersistence = xsd__boolean__true_;
	trt__SetVideoEncoderConfiguration->Configuration->__any = NULL;
	trt__SetVideoEncoderConfiguration->Configuration->__anyAttribute = NULL;
	

	trt__SetVideoEncoderConfiguration->Configuration->Resolution = (struct tt__VideoResolution*) soap_malloc(soap,sizeof(struct tt__VideoResolution));
	trt__SetVideoEncoderConfiguration->Configuration->RateControl = (struct tt__VideoRateControl*) soap_malloc(soap,sizeof(struct tt__VideoRateControl));
	if(pcount == 0){
		if(config.mheight != 0){
			trt__SetVideoEncoderConfiguration->Configuration->Resolution->Height = config.mheight;
		}else{
			trt__SetVideoEncoderConfiguration->Configuration->Resolution->Height = node->main_video_height;
		}
		
		if(config.mwidth !=0 ){
			trt__SetVideoEncoderConfiguration->Configuration->Resolution->Width = config.mwidth;
		}else{
			trt__SetVideoEncoderConfiguration->Configuration->Resolution->Width = node->main_video_width;
		}
		
		if(config.mframerate != 0){
			trt__SetVideoEncoderConfiguration->Configuration->RateControl->FrameRateLimit = config.mframerate;
		}else{
			trt__SetVideoEncoderConfiguration->Configuration->RateControl->FrameRateLimit = node->main_frame_rate;
		}

		if(config.mbitrate != 0){
			trt__SetVideoEncoderConfiguration->Configuration->RateControl->BitrateLimit= config.mbitrate;
		}else{
			trt__SetVideoEncoderConfiguration->Configuration->RateControl->BitrateLimit= tprofile.VideoEncoderConfiguration->RateControl->BitrateLimit;
		}
	
	}else if (pcount == 1){
		
		if(config.sheight != 0){
			trt__SetVideoEncoderConfiguration->Configuration->Resolution->Height = config.sheight;
		}else{
			trt__SetVideoEncoderConfiguration->Configuration->Resolution->Height = node->sub_video_height;
		}
		
		if(config.swidth !=0 ){
			trt__SetVideoEncoderConfiguration->Configuration->Resolution->Width = config.swidth;
		}else{
			trt__SetVideoEncoderConfiguration->Configuration->Resolution->Width = node->sub_video_width;
		}
		
		if(config.sframerate != 0){
			trt__SetVideoEncoderConfiguration->Configuration->RateControl->FrameRateLimit = config.sframerate;
		}else{
			trt__SetVideoEncoderConfiguration->Configuration->RateControl->FrameRateLimit = node->sub_frame_rate;
		}

		if(config.sbitrate != 0){
			trt__SetVideoEncoderConfiguration->Configuration->RateControl->BitrateLimit= config.sbitrate;
		}else{
			trt__SetVideoEncoderConfiguration->Configuration->RateControl->BitrateLimit= tprofile.VideoEncoderConfiguration->RateControl->BitrateLimit;
		}

	}
	
	trt__SetVideoEncoderConfiguration->Configuration->RateControl->EncodingInterval = 1;

	soap_wsse_add_UsernameTokenDigest(soap,"user", ONVIF_USER, pnode->passwd); 
	int ret = soap_call___trt__SetVideoEncoderConfiguration(soap,capa_resp->Capabilities->Media->XAddr,NULL,trt__SetVideoEncoderConfiguration,trt__SetVideoEncoderConfigurationResponse);

	if(soap->error){
		DBG_MSG("soap_call___trt__SetVideoEncoderConfiguration faild %d ,%s ,%s token %s\n",soap->error, *soap_faultcode(soap), *soap_faultstring(soap),trt__SetVideoEncoderConfiguration->Configuration->token);
		ret = soap->error;
	}else{
		//printf("Set ok token is %s \n",trt__SetVideoEncoderConfiguration->Configuration->token);
		if(pcount == 0){
			if(config.mheight != 0){
				node->main_video_height = config.mheight;
			}
			
			if(config.mwidth !=0 ){
				node->main_video_width= config.mwidth;
			}
			
			if(config.mframerate != 0){
				node->main_frame_rate= config.mframerate;
			}

		}else if(pcount ==1){
			if(config.sheight != 0){
				node->sub_video_height = config.sheight;
			}
			
			if(config.swidth !=0 ){
				node->sub_video_width= config.swidth;
			}
			
			if(config.sframerate != 0){
				node->sub_frame_rate= config.sframerate;
			}
		}
		ret =  SOAP_OK;
	}
	soap_destroy(soap);
	return ret;

}

