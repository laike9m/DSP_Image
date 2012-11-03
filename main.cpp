#include <Windows.h>
#include <atlimage.h>
#include <iostream>
using namespace std;

int once(char* pic_name,char* output);
void GuassFilter(int *pGrayMat, int* pFilterMat, int nWidth, int nHeight, double dSigma); //高斯滤波
int main(){
	char* pic_name = "pics\\test0.jpg";
	char* output = "pics\\newimage0_2.jpg";
	once(pic_name,output);
	pic_name = "pics\\test1.jpg";
	output = "pics\\newimage1_2.jpg";
	once(pic_name,output);
	pic_name = "pics\\test2.jpg";
	output = "pics\\newimage2_2.jpg";
	once(pic_name,output);
	pic_name = "pics\\test3.jpg";
	output = "pics\\newimage3_2.jpg";
	once(pic_name,output);
	return 0;
}


int once(char* pic_name,char* output){
	double histogram[256];
	double histogram_c[256];	//cumulative 累积直方图
	double ratio[256];			//记录每一种灰度需要增加的值，可正可负
	for (int i=0;i<256;i++){
		histogram[i] = 0;
		histogram_c[i] = 0;
	}
	CImage img_load;
	CImage img_new;
	img_load.Load(pic_name);
	img_new = img_load;
	int width = img_load.GetWidth();
	int height = img_load.GetHeight();
	int nRowbytes = img_load.GetPitch();
	int pixel_num = width*height;
	BYTE *p;
	p = (BYTE*)img_load.GetBits();
	int* old_avg = new int[pixel_num];	//保存原来的均值
	int* new_avg = new int[pixel_num];	//保存中值滤波之后的新均值 
	int* d_avg = new int[pixel_num];		//保存差值，d=新值-老值 
	int* low = new int[pixel_num];		//过guass后得到的低频部分
	int* high = new int[pixel_num];		//原图-low
	//获取原始灰度图，保存在old_avg中
	for (int i=0;i<height;i++){
		for (int j=0;j<width;j++){
			int average = (p[j*3]+p[j*3+1]+p[j*3+2])/3;
			old_avg[width*i+j] = average;
		}
		p += nRowbytes;
	}
	//高斯滤波得到低频分量，保存在low_avg中
	GuassFilter(old_avg,low,width,height,0.4);

	//得到高频部分
	for (int i=0;i<pixel_num;i++)
		high[i] = old_avg[i] - low[i];

	//获取低频分量滤波后灰度图的直方图
	for (int i=0;i<height;i++){
		for (int j=0;j<width;j++){
			histogram[low[i*width+j]]++;
		}
	}
	//累积直方图+取整扩展
	for (int i=0;i<256;i++){
		histogram[i] = histogram[i]/pixel_num;
		if (i==0)
			histogram_c[i] = histogram[i];
		else
			histogram_c[i] = histogram_c[i-1] + histogram[i];
	}
	for (int i=0;i<256;i++){
		histogram_c[i] = (int)(histogram_c[i]*255 + 0.5);	//均衡化后的新灰度
	}
	//做HE，计算低频部分新的灰度值，依然保存在low中。然后把low和high相加得到最终灰度图保存在high中。然后减去old_avg计算差灰度图d_avg
	for (int i=0;i<height;i++){
		for (int j=0;j<width;j++){
			int temp = i*width + j;
			low[temp] = histogram_c[low[temp]];//低频部分新的灰度值
			high[temp] += low[temp];	//最终灰度
			d_avg[temp] = high[temp] - old_avg[temp];//差灰度图
		}
	}
	//把差灰度叠加到原RGB图的每一个像素上
	p = (BYTE*)img_new.GetBits();
	for (int i=0;i<height;i++){
		for (int j=0;j<width;j++){
			int temp = i*width + j;
			//要考虑上下溢出的情形
			if (p[j*3] + d_avg[temp]<256 && p[j*3] + d_avg[temp]>-1)
				p[j*3] += d_avg[temp];
			else if (p[j*3] + d_avg[temp]>255)
				p[j*3] = 255;
			else
				p[j*3] = 0;

			if (p[j*3+1] + d_avg[temp]<256 && p[j*3+1] + d_avg[temp]>-1)
				p[j*3+1] += d_avg[temp];
			else if (p[j*3+1] + d_avg[temp]>255)
				p[j*3+1] = 255;
			else
				p[j*3+1] = 0;

			if (p[j*3+2] + d_avg[temp]<256 && p[j*3+2] + d_avg[temp]>-1)
				p[j*3+2] += d_avg[temp];
			else if (p[j*3+2] + d_avg[temp]>255)
				p[j*3+2] = 255;
			else
				p[j*3+2] = 0;

			int average = (p[j*3]+p[j*3+1]+p[j*3+2])/3;
			old_avg[width*i+j] = average;
			new_avg[width*i+j] = average;
		}
		p += nRowbytes;
	}

	
	//中值滤波，取3*3的十字形模版
	p = (BYTE*)img_new.GetBits();
	for (int i=2;i<height-2;i++){
		for (int j=2;j<width-2;j++){
			int point = i*width + j;	//当前点在数组中的位置
			int model[9];
			//model[0] = old_avg[point-width*2];
			model[1] = old_avg[point-width];
			//model[2] = old_avg[point+width*2];
			model[3] = old_avg[point+width];
			model[4] = old_avg[point];
			model[5] = old_avg[point-1];
			//model[6] = old_avg[point-2];
			model[7] = old_avg[point+1];
			//model[8] = old_avg[point+2];
			for (int k=0;k<5;k++){
				for (int l=4;l>0;l--){
					if (model[l]<model[l-1]){
						int tempoint;
						tempoint = model[l];
						model[l] = model[l-1];
						model[l-1] = tempoint;         //从小到大排序
					}
				}
				if (k==2)
					break;
			}
			new_avg[point] = model[2];	//用模版中值代替原来的灰度值
			new_avg[point] -= old_avg[point];	//节省存储空间用new_avg直接保存差值 
			
			if (p[j*3] + new_avg[point]<256 && p[j*3] + new_avg[point]>-1)
				p[j*3] += new_avg[point];
			else if (p[j*3] + new_avg[point]>255)
				p[j*3] = 255;
			else
				p[j*3] = 0;

			if (p[j*3+1] + new_avg[point]<256 && p[j*3+1] + new_avg[point]>-1)
				p[j*3+1] += new_avg[point];
			else if (p[j*3+1] + new_avg[point]>255)
				p[j*3+1] = 255;
			else
				p[j*3+1] = 0;

			if (p[j*3+2] + new_avg[point]<256 && p[j*3+2] + new_avg[point]>-1)
				p[j*3+2] += new_avg[point];
			else if (p[j*3+2] + new_avg[point]>255)
				p[j*3+2] = 255;
			else
				p[j*3+2] = 0;
		}
		p += nRowbytes;
	}
	
	HRESULT hResult = img_new.Save(output);
    if(FAILED(hResult)){
		cout<<"保存图像文件失败！"<<endl;
    }


	return 0;
}


void GuassFilter(int *pGrayMat, int* pFilterMat, int nWidth, int nHeight, double dSigma)
{
	////////////////////////参数说明///////////////////////////////////
	//pGrayMat:待处理图像数组
	//pFilterMat:保存高斯滤波结果
	//nWidth:图像宽度
	//nHeight:图像高度
	//dSigma:高斯滤波参数，方差
	int i;
	int nWidowSize = (int)(1+2*ceil(3*dSigma));  //定义滤波窗口的大小
	int nCenter = (nWidowSize)/2;                //定义滤波窗口中心的索引
	//生成二维的高斯滤波系数
	double* pdKernal = new double[nWidowSize*nWidowSize]; //定义一维高斯核数组
	double  dSum = 0.0;	                                  //求和，进行归一化		
	/////////////二维高斯函数公式//////////////////////     
	//	                         x*x+y*y        ///////
	//	                   -1*--------------	///////
	//          1               2*Sigma*Sigma	///////
	//   ---------------- e						///////
	//   2*pi*Sigma*Sigma						///////
	///////////////////////////////////////////////////
	for(i=0; i<nWidowSize; i++)
	{
		for(int j=0; j<nWidowSize; j++)
		{
			int nDis_x = i-nCenter;
			int nDis_y = j-nCenter;
			pdKernal[i+j*nWidowSize]=exp(-(1/2)*(nDis_x*nDis_x+nDis_y*nDis_y)
				/(dSigma*dSigma))/(2*3.1415926*dSigma*dSigma);
			dSum += pdKernal[i+j*nWidowSize];
		}
	}
	//进行归一化
	for(i=0; i<nWidowSize; i++)
	{
		for(int j=0; j<nWidowSize; j++)
		{
			pdKernal[i+j*nWidowSize] /= dSum;
		}
	}

	for(i=0; i<nHeight; i++)
	{
		for(int j=0; j<nWidth; j++)
		{
			double dFilter=0.0;
			double dSum = 0.0;
			for(int x=(-nCenter); x<=nCenter; x++)			//行
			{
				for(int y=(-nCenter); y<=nCenter; y++)		//列
				{
					if( (j+x)>=0 && (j+x)<nWidth && (i+y)>=0 && (i+y)<nHeight)	//判断边缘
					{
						double ImageData = pGrayMat[(i+y)*nWidth+j+x];
						dFilter += ImageData * pdKernal[(y+nCenter)*nWidowSize+(x+nCenter)];
						dSum += pdKernal[(y+nCenter)*nWidowSize+(x+nCenter)];
					}
				}
			}
			pFilterMat[i*nWidth+j] = dFilter/dSum;
		}
	}
	delete[]pdKernal;
}