#include "../../libpuer/libpuer.h"

PSP_MODULE_INFO("Google2FactorAuthenticator", 0, 1, 1);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER);

#define printf pspDebugScreenPrintf

static char *t="ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";

static int base32_decode(memstream *in,memstream *out){
	int b=0,c;
	u32 x=0;
	char *p;
	for(;~(c=mgetc(in));){
		if(c=='='){
			break;
		}
		if('a'<=c&&c<='z')c-=0x20;
		if(p=strchr(t,c)){
			x=(x<<5)+(p-t);
			b+=5;
			if(b>=8)b-=8,mputc((x>>b)&0xff,out);
		}
	}
	while(b>=8)b-=8,mputc((x>>b)&0xff,out);
	return 0;
}

#define HMAC_SHA1_DIGESTSIZE 20
#define HMAC_SHA1_BLOCKSIZE  64

//out must have HMAC_SHA1_DIGESTSIZE bytes.
static void hmac_sha1(const u8 *key, int lkey, const u8 *data, int ldata, u8 *out){
	u8 key2[HMAC_SHA1_DIGESTSIZE];
	u8 tmp_digest[HMAC_SHA1_DIGESTSIZE];
	u8 buf[HMAC_SHA1_BLOCKSIZE];
	int i;
	struct sha1_ctxt ctx;

	//truncate
	if(lkey>HMAC_SHA1_BLOCKSIZE){
		sha1_init(&ctx);
		sha1_loop(&ctx,key,lkey);
		sha1_result(&ctx,key2);
		key = key2;
		lkey = HMAC_SHA1_DIGESTSIZE;
	}

	//stage1
	for(i=0;i<lkey;i++)buf[i]=key[i]^0x36;
	for(;i<HMAC_SHA1_BLOCKSIZE;i++)buf[i]=0x36;
	sha1_init(&ctx);
	sha1_loop(&ctx,buf,HMAC_SHA1_BLOCKSIZE);
	sha1_loop(&ctx,data,ldata);
	sha1_result(&ctx,tmp_digest);

	//stage2
	for(i=0;i<lkey;i++)buf[i]=key[i]^0x5c;
	for(;i<HMAC_SHA1_BLOCKSIZE;i++)buf[i]=0x5c;
	sha1_init(&ctx);
	sha1_loop(&ctx,buf,HMAC_SHA1_BLOCKSIZE);
	sha1_loop(&ctx,tmp_digest,HMAC_SHA1_DIGESTSIZE);
	sha1_result(&ctx,out);
}

int main(){
	pspDebugScreenInit();pspDebugScreenSetXY(0,0);
	SetupExitCallback();

	printf(
		"Google TwoFactor Authenticator on PSP\n"
		"Tokens will be updated every 30sec.\n"
		"Please make sure your clock is correct.\n"
		"Adjust frequently using NTP (auto mode).\n"
	);

	u8 key[10][40];
	u32 lkey[10];memset(lkey,0,sizeof(lkey));
	strcpy(myfile,"keys.txt");
	FILE *f=fopen(mypath,"rb");
	if(!f){
		printf("cannot open keys.txt\n");
		for(;;sceKernelDelayThread(250000))if(!running)sceKernelExitGame();
	}

	int i=0;for(;i<10;i++){
		if(!myfgets(libpuer_cbuf,BUFLEN,f))break;
		libpuer_cbuf[64]=0;
		memstream in,out;
		mopen(libpuer_cbuf,strlen(libpuer_cbuf),&in),mopen(key[i],40,&out),
		base32_decode(&in,&out);
		lkey[i]=mtell(&out);
	}
	fclose(f);

	pspTime pt;
	u32 itime_old=0,itime;
	time_t timer;

	u8 T[8],hash[20];
	int otp=0;
	for(;;){ //This program is meant to be terminated with Ctrl+C.
		for(;;sceKernelDelayThread(250000)){
			if(!running)sceKernelExitGame();
			sceRtcGetCurrentClockLocalTime(&pt); 
			itime=pt.hour*3600+pt.minutes*60+pt.seconds;
			pspDebugScreenSetXY(0,5);
			printf("Time: %02d:%02d:%02d\n",pt.hour,pt.minutes,pt.seconds);
			if((pt.seconds%30==0||itime_old==0)&&itime!=itime_old){itime_old=itime;break;}
		}
		sceRtcGetCurrentClock(&pt, 0);
		sceRtcGetTime_t(&pt, &timer); 
		u64 TIMER=(u64)timer/30;
		memcpy(T,&TIMER,8);
		{ //fix endian to little
			u8 z;
			z=T[0],T[0]=T[7],T[7]=z;
			z=T[1],T[1]=T[6],T[6]=z;
			z=T[2],T[2]=T[5],T[5]=z;
			z=T[3],T[3]=T[4],T[4]=z;
		}
		for(i=0;i<10;i++){
			if(!lkey[i])break;
			hmac_sha1(key[i],lkey[i],T,8,hash);
			int offset=hash[19]&0xf;
			otp=( ((hash[offset]&0x7f)<<24) | (hash[offset+1]<<16) | (hash[offset+2]<<8) | hash[offset+3] )%1000000;
			printf("---\n");
			printf("%06d\n",otp);
		}
	}
	return 0;
}
