#include <Windows.h>
#include <atlimage.h>
#include <iostream>
using namespace std;

#define pic_name "pics\\test2.jpg"
#define output "pics\\newimage2.jpg"

int main(){
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
	//ֱ��ͼ
	for (int i=0;i<height;i++){
		for (int j=0;j<width;j++){
			int average = (p[j*3]+p[j*3+1]+p[j*3+2])/3;
			histogram[average]++;
		}
		p += nRowbytes;
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
		histogram_c[i] = (int)(histogram_c[i]*255 + 0.5);
		ratio[i] = histogram_c[i] - i;	//ratio = �»Ҷ�/�ϻҶ�
	}
	//�����µĻҶ�ֵ,��������浽�µ�ͼ��
	p = (BYTE*)img_new.GetBits();
	for (int i=0;i<height;i++){
		for (int j=0;j<width;j++){
			int average = (p[j*3]+p[j*3+1]+p[j*3+2])/3;
			//Ҫ�����������������
			if (histogram_c[p[j*3]] + ratio[average]<256 && histogram_c[p[j*3]] + ratio[average]>-1)
				p[j*3] = histogram_c[p[j*3]] + ratio[average];
			else if (histogram_c[p[j*3]] + ratio[average]>255)
				p[j*3] = 255;
			else
				p[j*3] = 0;

			if (histogram_c[p[j*3+1]] + ratio[average]<256 && histogram_c[p[j*3+1]] + ratio[average]>-1)
				p[j*3+1] = histogram_c[p[j*3+1]] + ratio[average];
			else if (histogram_c[p[j*3+1]] + ratio[average]>255)
				p[j*3+1] = 255;
			else
				p[j*3+1] = 0;

			if (histogram_c[p[j*3+2]] + ratio[average]<256 && histogram_c[p[j*3+2]] + ratio[average]>-1)
				p[j*3+2] = histogram_c[p[j*3+2]] + ratio[average];
			else if (histogram_c[p[j*3+2]] + ratio[average]>255)
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



}