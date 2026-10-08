#include <cstdio>
#include <cstdlib>
#include <vector>
#include <fstream>
#include "../LilyGo_GifPlayer/GifCanvas.h"
static GifCanvas c;
static void draw(GIFDRAW*d){c.draw(*d);}
static void* openFile(const char*name,int32_t*size){FILE*f=fopen(name,"rb");if(!f)return nullptr;fseek(f,0,SEEK_END);*size=ftell(f);fseek(f,0,SEEK_SET);return f;}
static void closeFile(void*p){if(p)fclose((FILE*)p);}
static int32_t readFile(GIFFILE*f,uint8_t*b,int32_t n){if(n<=0)return 0;int left=f->iSize-f->iPos;if(left<=0)return 0;if(n>left)n=left;int got=fread(b,1,n,(FILE*)f->fHandle);f->iPos=ftell((FILE*)f->fHandle);return got;}
static int32_t seekFile(GIFFILE*f,int32_t p){if(p<0||p>f->iSize)return -1;if(fseek((FILE*)f->fHandle,p,SEEK_SET))return -1;f->iPos=ftell((FILE*)f->fHandle);return f->iPos;}

int main(int argc,char**argv){
 if(argc!=3)return 2;
 std::ifstream in(argv[1],std::ios::binary);std::vector<uint8_t>b((std::istreambuf_iterator<char>(in)),{});
 if(b.size()<13)return 3;
 std::vector<uint16_t>pixels(480*480),saved(480*480);c.pixels=pixels.data();c.saved=saved.data();
 AnimatedGIF gif;gif.begin(GIF_PALETTE_RGB565_LE);
 if(!gif.open(argv[1],openFile,closeFile,readFile,seekFile,draw)){printf("OPEN ERROR %d\n",gif.getLastError());return 4;}
 int w=gif.getCanvasWidth(),h=gif.getCanvasHeight();uint16_t bg=0;
 if(b[10]&128){int p=13+3*b[11];if(p+2<(int)b.size())bg=((b[p]&248)<<8)|((b[p+1]&252)<<3)|(b[p+2]>>3);}
 c.configure(w,h,bg);
 for(int i=0;i<20;++i){c.startDecode();int delay=0;int more=gif.playFrame(false,&delay);if(more<0||c.invalid){printf("DECODE ERROR %d\n",gif.getLastError());return 5;}
  if(c.sawLine){char p[1024];snprintf(p,sizeof(p),"%s-%d.raw",argv[2],i);std::ofstream o(p,std::ios::binary);o.write((char*)pixels.data(),pixels.size()*2);printf("%d %d\n",i,delay);}
  if(!more)break;
 }
 gif.close();
 return 0;
}
