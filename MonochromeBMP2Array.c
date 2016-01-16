/*
* Name: MonochromeBMP2Array.c
* Author: Michael Kruger
* Brief:
*
* This simple program converts a monochrome bitmap image to an array sorted by rows
* In a monchrome image all pixels are defined by a bit. 
* To draw the image from the array simple loop through each bit the value is 0 the pixel is black, 
* and if the value is 1 the pixel is white. Each outer element represents a row.
*
* This is a very simple bitmap parser that was quickly written to store a bitmap as an array on a microcontroller,
* where an SD card was not available. It might not work with some monochrome bitmaps and does not handle compression.
* Feel free to fix it and let me know of issues.
*
* NOTE: It makes sense to put the array in ROM. In some compiler the const keyword is enough.
* You might need to change it to ensure it ends up in ROM. 
* For Arduino change to const char ImgArray[...][...] PROGMEM = instead of const char ImgArray[...][...] =
* See https://www.arduino.cc/en/Reference/PROGMEM for info. (note you cannot use it like normal RAM) 
*
* For images less than 200 pixels the image will be drawn in the file and on the terminal.
*
* usage Linux/Cygwin:
*
* Compile:
* gcc -o MonochromeBMP2Array MonochromeBMP2Array.c
*
* Run:
* ./MonochromeBMP2Array InputFile.bmp OutputFile.h
*
* Copyright 2016 Krugaroo Technology
* More Info: www.krugaroo.com#openSource/MonochromeBMP2Array
*
* License: MIT License
*
* Copyright (c) 2015 Michael Kruger, Krugaroo Interactive Technology
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
* 
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define FREAD_ERROR(dptr,size,count,fptr,error) if(fread((void*)dptr, size, count, fptr)!=count){printf(error); return 0;}bytesread+=size*count;
#define FSEEK_ERROR(fptr, bytestoseek, error) if(fseek(fptr, bytestoseek, SEEK_CUR)){ printf(error); return 0;}bytesread+=bytestoseek;

typedef struct{
	unsigned int filesize;
	unsigned int offset;
	unsigned int dibHeaderSize;
	int width;
	int height;
	short colorplane;
	unsigned int bitsPerPixel;
	unsigned int compression;
	unsigned int pixelSize;
	unsigned int dpihor;
	unsigned int dpiver;
	unsigned int colorsInPallete;
	unsigned int ImportantColors;
	
} BMPdata;

int main(int argc, char **argv) 
{
	/* used to read sizes*/
	BMPdata bmpData = {0};
	unsigned int bytesread = 0;
	
	int j=0;

	printf("Starting monochrome bitmap analyzer...\r\n");
	
	if ( argc < 2) 
    {
        printf( "No arguments given! Please provide bmp file name\r\n");
        return 0;
    }
    
    printf("Opening %s\r\n", argv[1]);
    
    FILE* bmp = fopen(argv[1],"r");
    
    if(bmp==0)
    {
		printf("Could not open file!\r\n");
		return 0;
	}
	
	/* seek over BM */
	FSEEK_ERROR(bmp, 2, "File does not start with BM");
	
	FREAD_ERROR((void*)&bmpData.filesize,sizeof(unsigned int),1,bmp,"File does not have size");

	/* seek over app specific */
	FSEEK_ERROR(bmp, 4, "No app specific bytes");
	
	FREAD_ERROR((void*)&bmpData.offset,sizeof(unsigned int),1,bmp,"File does not have pixel offset");
	
	FSEEK_ERROR(bmp, 4, "No app dib header");
	
	FREAD_ERROR((void*)&bmpData.width,sizeof(int),1,bmp,"File does not have width");
	FREAD_ERROR((void*)&bmpData.height,sizeof(int),1,bmp,"File does not have height");
	FREAD_ERROR((void*)&bmpData.colorplane,sizeof(short),1,bmp,"File does not have colorplane");
	FREAD_ERROR((void*)&bmpData.bitsPerPixel,sizeof(short),1,bmp,"File does not have bits per pixel");
	FREAD_ERROR((void*)&bmpData.compression,sizeof(unsigned int),1,bmp,"File does not have compression field");
	FREAD_ERROR((void*)&bmpData.pixelSize,sizeof(unsigned int),1,bmp,"File does not have pixelsize");
	FREAD_ERROR((void*)&bmpData.dpihor,sizeof(unsigned int),1,bmp,"File does not have dpi hor");
	FREAD_ERROR((void*)&bmpData.dpiver,sizeof(unsigned int),1,bmp,"File does not have dpi ver");
	FREAD_ERROR((void*)&bmpData.colorsInPallete,sizeof(unsigned int),1,bmp,"File does not have # colours in pallete");
	FREAD_ERROR((void*)&bmpData.ImportantColors,sizeof(unsigned int),1,bmp,"File does not have important colours");

	printf("Filesize: %d bytes \r\n", bmpData.filesize); 
	printf("Pixeldata starts at: %d %X\r\n", bmpData.offset, bmpData.offset);
	printf("Width: %d\r\n", bmpData.width);
	printf("Height: %d\r\n", bmpData.height); 
	printf("Colorplane: %d\r\n", bmpData.colorplane); 
	printf("Bits Per Pixel: %d\r\n", bmpData.bitsPerPixel); 
	printf("Compression: %d\r\n", bmpData.compression); 
	printf("Pixel size: %d bytes %X\r\n", bmpData.pixelSize, bmpData.pixelSize); 
	printf("DPI horizontal: %u %X\r\n", bmpData.dpihor,bmpData.dpihor); 
	printf("DPI vertical: %u %X\r\n", bmpData.dpiver,bmpData.dpiver);
	printf("Number of colors: %d\r\n", bmpData.colorsInPallete);
	printf("Important colors: %d\r\n", bmpData.ImportantColors);
	
	printf("Header Bytes Read: %d\r\n", bytesread);
	
	if(bmpData.bitsPerPixel != 1)
	{
		printf("WARNING: Image is not a monochrome image! Will try to continue...\r\n");
	}
	
	if(bmpData.compression != 0)
	{
		printf("WARNING: Image has compression! Will try to continue...\r\n");
	}
	
	if(bytesread<bmpData.offset)
	{
		printf("Seeking %d bytes to pixeldata...\r\n",(bmpData.offset-bytesread));
		FSEEK_ERROR(bmp, (bmpData.offset-bytesread), "No pixeldata found");
	}
	else if(bytesread>bmpData.offset)
	{
		printf("WARNING: Image header too small! Will try to continue...\r\n");
	}
	
	/* get number of bytes we need from each row */
	int widthPixBytes = bmpData.width/8 + ((bmpData.width%8)?1:0);
	printf("Each row has %d bytes of pixeldata\r\n", widthPixBytes);
	
	/* get number of bytes for each row with padding */
	int totalWidth = widthPixBytes + ((widthPixBytes%4)?(4-(widthPixBytes%4)):0);
	printf("Each row with padding is %d bytes\r\n", totalWidth);
	
	/* since bmp is upside down malloc memory to fill array upside down */
	char* pixelBuff = malloc(widthPixBytes*bmpData.height);
	
	if(pixelBuff == NULL)
	{
		printf("Error: Could not allocate memory for pixel buffer!");
		return 0;
	}
	
	/* address the pointer to the last row */
	char* rowPointer = pixelBuff+(widthPixBytes*(bmpData.height-1));
	
	int pixelByte;
	int columnoffset = 0;
	int pixelBytesRead = 0;
	
	while(((pixelByte = fgetc(bmp)) != EOF) && pixelBytesRead<(totalWidth*bmpData.height))
	{
		pixelBytesRead++;
		columnoffset++;
		
		if(columnoffset<=widthPixBytes)
		{
			rowPointer[columnoffset-1] = pixelByte;
		}
		
		if(columnoffset>=totalWidth)
		{
			/* move pointer to next row*/
			rowPointer -= (widthPixBytes);
			
			columnoffset=0;
		}
	}
	
	/* Draw the image on the screen and generate the output file */
	FILE* outputFile = NULL; 
	
	if ( argc < 3) 
    {
        printf( "No output arguments given! will not generate output file\r\n");
    }
    else
    {
		outputFile = fopen(argv[2],"w");
		
		if(outputFile==NULL)
		{
			printf("ERROR: Could not open output file for writing!\r\n");
		}
		else
		{
			fprintf(outputFile,"/* Image data for %s %dx%d */\r\n", argv[1], bmpData.width, bmpData.height);
			fprintf(outputFile,"/* Generated by MonochromeBMP2Array */\r\n");
			fprintf(outputFile,"/* Source code available at https://github.com/Krugaroo/MonochromeBMP2Array  */\r\n");
			fprintf(outputFile,"/*\r\n");
		}
	}
	
	unsigned char pixelByteDraw;
	columnoffset=0;
	pixelBytesRead=0;	
	
	/* Draw the image on screen and to the file */
	if(bmpData.width<200)
	{
		do 
		{
			pixelByteDraw = *(pixelBuff+pixelBytesRead);
		  
			if(columnoffset<widthPixBytes)
			{     
				for(j=1;j<=8;j++)
				{
					if(pixelByteDraw&(1<<(8-j)))
					{
						printf("*");
						
						if(outputFile!=NULL)
						{
							fprintf(outputFile,"*");
						}
					}
					else
					{
						printf(" ");
						
						if(outputFile!=NULL)
						{
							fprintf(outputFile," ");
						}
					}
				}
			}
			
			pixelBytesRead++;
			columnoffset++;
			
			if(columnoffset==widthPixBytes)
			{
				if(outputFile!=NULL)
				{
					fprintf(outputFile,"\r\n");
				}
				
				columnoffset=0;
				printf("\r\n");	
			}
		  
		} while (pixelBytesRead<widthPixBytes*bmpData.height);
		
	}
	else
	{
		printf("Image is wider than 200px and is therefore not drawn!\r\n");
		if(outputFile!=NULL)
		{
			fprintf(outputFile,"\r\nImage is wider than 200px and is therefore not drawn!\r\n");
		}
	}
	
    if(outputFile!=NULL)
	{
		fprintf(outputFile,"*/\r\n");
	}
    
	columnoffset=0;
	pixelBytesRead=0;	
	
	/* Generate the array and data to file */
	if(outputFile!=NULL)
	{
		fprintf(outputFile,"\r\n/* Image data */\r\n");
		fprintf(outputFile,"unsigned int ImgWidthBytes = %d;\r\n", widthPixBytes);
		fprintf(outputFile,"unsigned int ImgWidthPixels = %d;\r\n", bmpData.width);
		fprintf(outputFile,"unsigned int ImgHeightPixels = %d;\r\n", bmpData.height);
		fprintf(outputFile,"unsigned int ImgTotalBytes = %d;\r\n\r\n", widthPixBytes*bmpData.height);
		fprintf(outputFile,"const char ImgArray[%d][%d] = {\r\n{", bmpData.height,widthPixBytes);
	}
	
	do 
	{
		pixelByteDraw = *(pixelBuff+pixelBytesRead);
      
		if(columnoffset<widthPixBytes)
		{
			if(outputFile!=NULL)
			{
				if(columnoffset<widthPixBytes-1)
				{
					fprintf(outputFile,"0x%02X,", pixelByteDraw);
				}
				else
				{
					fprintf(outputFile,"0x%02X", pixelByteDraw);
				}
			}
		}
		
		pixelBytesRead++;
		columnoffset++;
		
		if(columnoffset==widthPixBytes)
		{
			if(outputFile!=NULL && pixelBytesRead<widthPixBytes*bmpData.height)
			{
				fprintf(outputFile,"},\r\n{");
			}
			
			columnoffset=0;	
		}
		
		printf("\rProgress: %3d %%", pixelBytesRead*100/(widthPixBytes*bmpData.height));
      
    } while (pixelBytesRead<widthPixBytes*bmpData.height);
    
    if(outputFile!=NULL)
	{
		fprintf(outputFile,"}\r\n};\r\n\r\n");
	}

    /* Free the allocated buffer */
	free(pixelBuff);	
	
	printf("\r\nFile written to %s.\r\n", argv[2]);
	printf("All done\r\n");
	
	return 0;
}
