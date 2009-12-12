#ifndef __TCIMAGE_H__
#define __TCIMAGE_H__

#include <TCFoundation/TCObject.h>
#include <TCFoundation/TCTypedObjectArray.h>

#include <stdio.h>

class TCImageFormat;
class TCImageOptions;

typedef enum
{
	TCRgb8,
	TCRgba8,
	TCRgb16,
	TCRgba16
} TCImageDataFormat;

typedef bool (*TCImageProgressCallback)(CUCSTR message, float progress,
										void* userData);

typedef TCTypedObjectArray<TCImageFormat> TCImageFormatArray;

class TCExport TCImage : public TCObject
{
public:
	TCImage(void);

	virtual void setDataFormat(TCImageDataFormat format);
	TCImageDataFormat getDataFormat(void) { return dataFormat; }
	virtual void setSize(int xSize, int ySize);
	virtual void getSize(int &xSize, int &ySize);
	int getWidth(void) { return width; }
	int getHeight(void) { return height; }
	virtual void setLineAlignment(int value);
	int getLineAlignment(void) { return lineAlignment; }
	virtual void setFlipped(bool value);
	bool getFlipped(void) { return flipped; }
	virtual void allocateImageData(void);
	virtual void setImageData(TCByte *value);
	TCByte *getImageData(void) { return imageData; }
	virtual int getRowSize(void);
//	virtual TCObject *copy(void) const;
	virtual bool loadData(TCByte *data, long length,
		TCImageProgressCallback progressCallback = NULL,
		void *progressUserData = NULL);
	virtual bool loadFile(FILE *file,
		TCImageProgressCallback progressCallback = NULL,
		void *progressUserData = NULL);
	virtual bool loadFile(const char *filename,
		TCImageProgressCallback progressCallback = NULL,
		void *progressUserData = NULL);
	virtual bool saveFile(const char *filename,
		TCImageProgressCallback progressCallback = NULL,
		void *progressUserData = NULL);
	char *getFormatName(void) { return formatName; }
	virtual void setFormatName(const char *value);
	virtual TCImage *createSubImage(int x, int y, int cx, int cy);
	virtual void setComment(const char *value);
	virtual void autoCrop(TCUShort r, TCUShort g, TCUShort b);
	virtual void autoCrop(void);
	const char *getComment(void) { return comment; }
	virtual TCImageOptions *getCompressionOptions(void);

	static int roundUp(int value, int nearest);
	static void addImageFormat(TCImageFormat *imageFormat,
		bool release = false);

#ifdef WIN32
	static TCImage *createFromResource(HMODULE hModule, int resourceId,
		int lineAlignment = 1, bool flipped = false);
	static HBITMAP createDIBSection(HDC hBitmapDC, int bitmapWidth,
		int bitmapHeight, int hDPI, int vDPI, BYTE **bmBuffer);
	static HICON loadIconFromPngResource(HMODULE hModule, int resourceId);

	void getBmpAndMask(HBITMAP &hBitmap, HBITMAP &hMask,
		bool updateSource = false);
	HBITMAP createMask(bool updateSource = false);
#endif // WIN32
protected:
	virtual ~TCImage(void);
	virtual void dealloc(void);
	virtual void syncImageData(void);

	static TCImageFormat *formatWithName(char *name);
	static TCImageFormat *formatForData(const TCByte *data, long length);
	static TCImageFormat *formatForFile(const char *filename);
	static TCImageFormat *formatForFile(FILE *file);
	static void initStandardFormats(void);

	TCByte *imageData;
	TCImageDataFormat dataFormat;
	int bytesPerPixel;
	int width;
	int height;
	int lineAlignment;
	bool flipped;
	char *formatName;
	bool userImageData;
	char *comment;
	TCImageOptions *compressionOptions;

	static TCImageFormatArray *imageFormats;

	static class TCImageCleanup
	{
	public:
		~TCImageCleanup(void);
	} imageCleanup;
	friend class TCImageCleanup;
};

typedef TCTypedObjectArray<TCImage> TCImageArray;

#endif // __TCIMAGE_H__
