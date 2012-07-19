/*
** HFF.H : Header file for HFF.CPP.
*/

#ifndef __HFF_H
#define __HFF_H

#define HFF_FILE_ID			"Orannge"
#define HFF_FILE_VERSION	(0x0100)


	typedef struct tagHFFHeader
	{
		char				identity[8];
		unsigned short		version;
		unsigned short		flags;
		unsigned long		nCharacters;
		signed long			spacing;
		unsigned long		height;
		unsigned long		yBaseline;
		unsigned long		oName;
		unsigned long		imageWidth;
		unsigned long		imageHeight;
		unsigned long		nColors;
		unsigned long		oPalette;
		unsigned long		oImage;
		char				reserved[16];
		unsigned long		oCharacters[256];
	} HFFHeader;

	enum tagHFFHeaderFlags
	{
		HFFF_Color,
		HFFF_AlphaEnable,

		HFFF_Last_HFFF
	};

	typedef struct tagCharacterHeader
	{
		unsigned short		u;
		unsigned short		v;
		unsigned short		width;
		unsigned short		height;
		unsigned short		xOffset;
		unsigned short		yOffset;
	} CharacterHeader;

	typedef struct tagInternalCharacterHeader
	{
		CharacterHeader		charHeader;

		char *charBuffer;
		unsigned long charBufferSize;

	} InternalCharacterHeader;

	class SaveHomeworldFontFile
	{
	private:
		HFFHeader				fileHeader;
		InternalCharacterHeader	*charHeaders[256];
		char					*fontBitmap;

	protected:
	public:
		SaveHomeworldFontFile(void);
		~SaveHomeworldFontFile(void);

		signed char SaveHFF(CString &fileName);

		void CalculateCharacterExtents(CDC &dc, CFont &fTemp, CString &strTemp, CSize &sTemp);
		int CalculateFontSpacing(void);
		InternalCharacterHeader *CreateIndividualCharacter(unsigned char c);
		void CharacterAntialias(char *dst, char *src, CSize *size);
		void CharacterCopy(char *dst, char *src, CSize *size);
		void AddDropshadow(char *dst, CSize *size);
		signed char CreateCharacters(void);
		signed char CreateCharacterBitmap(void);
		signed char SetupFileHeader(void);
		signed char SetupFileOffsets(void);
		signed char WriteFile(CString &fileName);
	};

#endif