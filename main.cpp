#include <Windows.h>
#include <atlimage.h>
#include <iostream>
using namespace std;

int once(char* pic_name,char* output);
void GuassFilter(int *pGrayMat, int* pFilterMat, int nWidth, int nHeight, double dSigma); //��˹�˲�
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
	double histogram_c[256];	//cumulative �ۻ�ֱ��ͼ
	double ratio[256];			//��¼ÿһ�ֻҶ���Ҫ���ӵ�ֵ�������ɸ�
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
	int* old_avg = new int[pixel_num];	//����ԭ���ľ�ֵ
	int* new_avg = new int[pixel_num];	//������ֵ�˲�֮����¾�ֵ 
	int* d_avg = new int[pixel_num];		//�����ֵ��d=��ֵ-��ֵ 
	int* low = new int[pixel_num];		//��guass��õ��ĵ�Ƶ����
	int* high = new int[pixel_num];		//ԭͼ-low
	//��ȡԭʼ�Ҷ�ͼ��������old_avg��
	for (int i=0;i<height;i++){
		for (int j=0;j<width;j++){
			int average = (p[j*3]+p[j*3+1]+p[j*3+2])/3;
			old_avg[width*i+j] = average;
		}
		p += nRowbytes;
	}
	//��˹�˲��õ���Ƶ������������low_avg��
	GuassFilter(old_avg,low,width,height,0.4);

	//�õ���Ƶ����
	for (int i=0;i<pixel_num;i++)
		high[i] = old_avg[i] - low[i];

	//��ȡ��Ƶ�����˲���Ҷ�ͼ��ֱ��ͼ
	for (int i=0;i<height;i++){
		for (int j=0;j<width;j++){
			histogram[low[i*width+j]]++;
		}
	}
	//�ۻ�ֱ��ͼ+ȡ����չ
	for (int i=0;i<256;i++){
		histogram[i] = histogram[i]/pixel_num;
		if (i==0)
			histogram_c[i] = histogram[i];
		else
			histogram_c[i] = histogram_c[i-1] + histogram[i];
	}
	for (int i=0;i<256;i++){
		histogram_c[i] = (int)(histogram_c[i]*255 + 0.5);	//���⻯����»Ҷ�
	}
	//��HE�������Ƶ�����µĻҶ�ֵ����Ȼ������low�С�Ȼ���low��high��ӵõ����ջҶ�ͼ������high�С�Ȼ���ȥold_avg�����Ҷ�ͼd_avg
	for (int i=0;i<height;i++){
		for (int j=0;j<width;j++){
			int temp = i*width + j;
			low[temp] = histogram_c[low[temp]];//��Ƶ�����µĻҶ�ֵ
			high[temp] += low[temp];	//���ջҶ�
			d_avg[temp] = high[temp] - old_avg[temp];//��Ҷ�ͼ
		}
	}
	//�Ѳ�Ҷȵ��ӵ�ԭRGBͼ��ÿһ��������
	p = (BYTE*)img_new.GetBits();
	for (int i=0;i<height;i++){
		for (int j=0;j<width;j++){
			int temp = i*width + j;
			//Ҫ�����������������
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

	
	//��ֵ�˲���ȡ3*3��ʮ����ģ��
	p = (BYTE*)img_new.GetBits();
	for (int i=2;i<height-2;i++){
		for (int j=2;j<width-2;j++){
			int point = i*width + j;	//��ǰ���������е�λ��
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
						model[l-1] = tempoint;         //��С��������
					}
				}
				if (k==2)
					break;
			}
			new_avg[point] = model[2];	//��ģ����ֵ����ԭ���ĻҶ�ֵ
			new_avg[point] -= old_avg[point];	//��ʡ�洢�ռ���new_avgֱ�ӱ����ֵ 
			
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
		cout<<"����ͼ���ļ�ʧ�ܣ�"<<endl;
    }


	return 0;
}


void GuassFilter(int *pGrayMat, int* pFilterMat, int nWidth, int nHeight, double dSigma)
{
	////////////////////////����˵��///////////////////////////////////
	//pGrayMat:������ͼ������
	//pFilterMat:�����˹�˲����
	//nWidth:ͼ����
	//nHeight:ͼ��߶�
	//dSigma:��˹�˲�����������
	int i;
	int nWidowSize = (int)(1+2*ceil(3*dSigma));  //�����˲����ڵĴ�С
	int nCenter = (nWidowSize)/2;                //�����˲��������ĵ�����
	//���ɶ�ά�ĸ�˹�˲�ϵ��
	double* pdKernal = new double[nWidowSize*nWidowSize]; //����һά��˹������
	double  dSum = 0.0;	                                  //��ͣ����й�һ��		
	/////////////��ά��˹������ʽ//////////////////////     
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
	//���й�һ��
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
			for(int x=(-nCenter); x<=nCenter; x++)			//��
			{
				for(int y=(-nCenter); y<=nCenter; y++)		//��
				{
					if( (j+x)>=0 && (j+x)<nWidth && (i+y)>=0 && (i+y)<nHeight)	//�жϱ�Ե
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