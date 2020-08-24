#include <3000\bios.h>
#include <3000\dos.h>
#include <3000\urm.h>
#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <process.h>
#include <dos.h>


#define CUSTOMERSIZE 144
#define SERIALSIZE 14
#define ADDRESCODESIZE 14

#define SCANSEARCH 0
#define SUBSCRSEARCH 1
#define SERIALSEARCH 2
#define ADRESCODE 3
#define ADRSEARCH 4
#define NOTREAD 5

int iSearchMethod;

unsigned char* pgFontPtr;
long lThisInOut;

int iBackLightTimeOut;

struct customer
{
//WhiteList:
        char cSubscriptionCode[16];
        char cName[19];
        char cFamily[23];
        char cAddressCode[11];
        char cCounterSerial[11];
        long lConsumptionKind;        
        long lCapacity;
        char cPrevDate[7];
        char cPrevCounter[11];
//Reading Counter:
        char cCurCounter[11];
        char cCurDate[7];
        char cCurTime[7];
        char cReader;
//Errors:
        char cErrors[4];
};

struct customer Customer;
int iCustomersNum;
int iReadNum;
int iToday;
char sToday[11];
char cLastReadAddressCode[11];
char cCityCode[3];

char cFileDate[9];
int iMustDoItAReader;
int iReadersNum;
char cReaderCodes[20];
int iReaderPasses[20];
char cActiveReaderCode;
int iIsDataTransfered;
int iCanReadManualySub;
int iCanReadManualyAdr;

FILE* fDataset;
FILE* fOut;
FILE* fErrors;

char sSettingsPassCode[5];

void LoadFont(char *strFilePath);
void AnounceDate(void);
void ShamsiDate(char* sDayOfWeek, char* sDay, char* sMonth, char *sYear);
void DrawBox(int iMinrow, int iMincol, int iMaxrow, int iMaxcol, int iAttr);
void ShowDateMenuItem(int ItemNum, int HighLight);
void gregorian_to_jalali(int *j_y, int *j_m, int *j_d, const int g_y, const int g_m, const int g_d);
void ShowMainMenuItem(int ItemNum, int HighLight);
int DoMainMenu();
void InitPersianEnvironment();
void ShowBatteryStatus();
int DoCradleMenu();
int IsInTheCradle();
int ConfigureLine(int *pComHandle);
void SetupComReady();
int DoSettingsMenu();
void ShowSettingsMenuItem(int ItemNum, int HighLight);
void ChangeDate();
int dayofweek(int dayofmonth, int month, int year);
int DoClockMenu();
void AnounceClock(void);
void ChangeClock();
void SetKybdTimeout();
int DoFindRecordBinary(struct customer* pCustomer);
int GetSubCode(char *sSubCode);
void OpenDataset();
int ReadValue(int iFound,struct customer* pCustomer);
int ScanSub(char *sInput);
void CalConsumption(char* pCur, char *pPrev, char *pRes);
void CalConsumption2(char* pCur, char *pPrev, char *pRes);
int CompareArrays(char* pCur, char* pPrev);
void DoErrorMenu(char* cSubscriptionCode);
void ShowErrorMenuItem(int ItemNum, int HighLight, int Checked);
void ShowErrorPage(int ErrorPage, int* iChecked);
void SetToday();
void CheckToday();
void GetToday(char* cdate);
void GetPassCode(char *sInput);
void ShowVersion();
void DoDeleteRead();
int GetCounterSerial(char *sInput);
void ShowFreeSpace();
int DoLastReadMenu(struct customer* pCustomer);
int CheckOperator();
void ShowFileDate();
void ShowSearchMethodMenuItem(int ItemNum, int HighLight);
int DoSearchMethodMenu();
int GoNext();
void FindAddressCode(char *sMyAddressCode, struct customer* pCustomer);
int GetAddressCode(char *sInput);
void ShowHelp();
void DoBackLightTimeOutSelect();
void ShowBackLightMenuItem(int ItemNum, int HighLight, int Checked);


void main(void)
{
        int iMainSelection;
        int iSettingsSelection;
        char sPass[5];
        unsigned int chEnter;
        int iRet;
        FILE* fProperties;
        FILE* fResult;
        char cWhatToDo;
        char cRes;
        pgFontPtr=NULL;
        cCityCode[0]='0';
        cCityCode[1]='1';
        cCityCode[2]='\0';

        iSearchMethod=SCANSEARCH;

        iBackLightTimeOut=0;
        BiosSetKybdTimeout(30);

        strcpy(cFileDate,"00/00/00");
        strcpy(cLastReadAddressCode, "aaaaaaaaaa");
        strcpy(sSettingsPassCode, "1384");

        iMustDoItAReader=0;//no operator pass is required
        iReadersNum=0; 
        cActiveReaderCode=1; //reading is reported as KONTORKHAN

        iIsDataTransfered=1; //TRUE;

        iCanReadManualySub=0;//FALSE;
        iCanReadManualyAdr=0;//FALSE;


        InitPersianEnvironment();
        do
        {
        iMainSelection=DoMainMenu();
        switch(iMainSelection)
        {
                case 0:
                     {
                     if(fDataset)
                     {
                        iRet=1;
                        if(CheckOperator())
                        while(CheckFreeSpace()&&iRet)
                        {
                               iRet=ReadValue(DoFindRecordBinary(&Customer), &Customer);
                        }
                     }
                     }
                     break;
                case 2:
                        ShowFileDate();
                     break;
                case 3:
                     GetPassCode(sPass);
                     if(strcmp(sPass, sSettingsPassCode)==0)
                     {
                     do
                     {
                     iSettingsSelection=DoSettingsMenu();
                     switch(iSettingsSelection)
                     {
                        case 0://date
                                if(DoDateMenu()==1)//change
                                        ChangeDate();
                                break;
                        case 1://clock
                                if(DoClockMenu()==1)//change
                                        ChangeClock();
                                break;
                        case 2://free space
                                ShowFreeSpace();
                                break;
                        case 3://delete
                                DoDeleteRead();
                                break;
                        case 4://backlight
                                DoBackLightTimeOutSelect();
                                switch(iBackLightTimeOut)
                                {
                                case 0:BiosSetKybdTimeout(30);break;
                                case 1:BiosSetKybdTimeout(40);break;
                                case 2:BiosSetKybdTimeout(60);break;
                                case 3:BiosSetKybdTimeout(100);break;
                                case 4:BiosSetKybdTimeout(200);break;
                                }
                                break;
                     }
                     }
                     while(iSettingsSelection!=5);
                     }
                     break;
                case 4:
                        ShowVersion();
                        break;
                case 1:
                     {


                     if(DoCradleMenu())
                     {/*
                        BiosClrScr(0x07);
                        DrawBox(0, 0, 6, 19,0x07);
                        BiosPutStrMove(7, 1, 18, "    ENTER=˜Ûã£ã   ", 0x70);
                        BiosPutStrMove(1, 1, 18, "å˙˜£ıãˆ¢ êÍ§ü ˚ã•ì", 0x07);
                        BiosPutStrMove(2, 1, 18, "åÛóü ¸ÒìÏ ˚å˙å‡¢ ˆ", 0x07);
                        BiosPutStrMove(3, 1, 18, "£˝ıÌ ¯˝Ò¢óã•˜åÓó©£", 0x07);
                        BiosPutStrMove(4, 1, 18, "˚£˝ÒÌ êñ´Ó¶åì ˚ã•ì", 0x07);
                        BiosPutStrMove(5, 1, 18, "!£˝ı¶ì ENTER ¶ã•˝È" , 0x07);
                        BiosBeep(500);
                        chEnter=BiosGetChar();*/
//                        if(chEnter==0x1C0D)
                        {
                                if(fOut)
                                        fclose(fOut);
                                if(fErrors)
                                        fclose(fErrors);

                                _spawnlp(_P_WAIT, "TL3100", NULL);

                                //Check to see what is going to be done:                                
                                fProperties=fopen("D:\\DO.DAT", "r");
                                fread(&cWhatToDo, sizeof(char), 1, fProperties);
                                fclose(fProperties);

                                if(cWhatToDo=='r')
                                {
                                if(iIsDataTransfered)
                                {
                                        if(fDataset)
                                                fclose(fDataset);

                                        //deleting old data
                                        fOut=fopen("D:\\OUTPUT.DAT","w");
                                        fclose(fOut);
                                        fErrors=fopen("D:\\ERRORS.DAT", "w");
                                        fclose(fErrors);

                                        _spawnlp(_P_WAIT, "TL3100", NULL);
                                        OpenDataset();
                                }
                                else
                                {
                                        BiosClrScr(0x07);
                                        DrawBox(0, 0, 6, 19,0x07);
                                        BiosPutStrMove(7, 1, 18, "    ENTER=˜Ûã£ã   ", 0x70);
                                        BiosPutStrMove(1, 1, 18, "êñåÂ‡ã £©•¸Û•·ı¯ì", 0x07);
                                        BiosPutStrMove(2, 1, 18, "˜£´ı¯˝Ò¢ó˜£´˜£ıãˆ¢", 0x07);
                                        BiosPutStrMove(3, 1, 18, "ã•¸ÒìÏ êñåÂ‡ãåÍ‡Ò", 0x07);
                                        BiosPutStrMove(4, 1, 18, "åó £˝ıÌêÍ§üå˛¯˝Ò¢ó", 0x07);
                                        BiosPutStrMove(5, 1, 18, "!£ˆ´•˛§ïıåÌÛã¯˛§Ëó" , 0x07);
                                        BiosBeep(1000);
                                        chEnter=BiosGetChar();
                                }

                                }
                                else
                                {
                                        //Check for correct transformation
                                        iIsDataTransfered=1;//TRUE;
                                        fResult=fopen("D:\\RES.DAT", "w");
                                        cRes='n';
                                        fwrite(&cRes, sizeof(char), 1, fResult);
                                        fclose(fResult);

                                        _spawnlp(_P_WAIT, "TL3100", NULL);

                                        fResult=fopen("D:\\RES.DAT", "r");
                                        fread(&cRes, sizeof(char), 1, fResult);
                                        fclose(fResult);
                                        if(cRes=='n')
                                        {
                                                iIsDataTransfered=0;//FALSE;
                                                BiosClrScr(0x07);
                                                DrawBox(0, 0, 6, 19,0x07);
                                                BiosPutStrMove(7, 1, 18, "    ENTER=˜Ûã£ã   ", 0x70);
                                                BiosPutStrMove(1, 1, 18, "     ¯ì ÔåÏóıã    ", 0x07);
                                                BiosPutStrMove(2, 1, 18, " .£´ı Úåõıã ¸ó©•£ ", 0x07);
                                                BiosPutStrMove(3, 1, 18, "------------------", 0x07);
                                                BiosPutStrMove(4, 1, 18, " †™ó ˜•åìˆ£ åÍ‡Ò ", 0x07);
                                                BiosPutStrMove(5, 1, 18, "       !£˝ıÌ      ", 0x07);
                                                BiosBeep(1000);
                                                chEnter=BiosGetChar();
                                        }



                                        fOut=fopen("D:\\OUTPUT.DAT", "r+");
                                        fErrors=fopen("D:\\ERRORS.DAT", "a+");
                                }
                        }
                     }

                     }
                     break;

        }
        }
        while(1);
        BiosClrScr(0x07);
        BiosShowCursor();
};


int DoMainMenu()
{
        int iRet;
        int iSel;
        int iOld;
        unsigned int ch;
        char cToday[20];

        BiosClrScr(0x07);
        BiosHideCursor();
        BiosSetCursorPos(0, 0);


        iRet=-2;
        iSel=0;

        DrawBox(0, 0, 6, 19,0x07);
        BiosPutStrMove(0, 2, 16, "[¶åÓ•ˆóıÌêñëã•Ï]", 0x07);
        ShowBatteryStatus();
        GetToday(cToday);
        BiosPutStrMove(7, 9, 10, &cToday[1], 0x70);
        ShowMainMenuItem(0, 1);
        ShowMainMenuItem(1, 0);
        ShowMainMenuItem(2, 0);
        ShowMainMenuItem(3, 0);
        ShowMainMenuItem(4, 0);
        do
        {
                ch=BiosGetChar();
                switch(ch)
                {
                        case 0x1C0D://enter
                                iRet=iSel;
                                break;
                        case 0x4800://up
                                {
                                iOld=iSel;
                                if(iSel==0)
                                {
                                        iSel=4;
                                }
                                else
                                {
                                        iSel--;
                                }
                                ShowMainMenuItem(iOld, 0);
                                ShowMainMenuItem(iSel, 1);
                                break;
                                }
                        case 0x5000://down
                                {
                                iOld=iSel;
                                if(iSel==4)
                                {
                                        iSel=0;
                                }
                                else
                                {
                                        iSel++;
                                }
                                ShowMainMenuItem(iOld, 0);
                                ShowMainMenuItem(iSel, 1);
                                break;
                                }
                         default:
                                BiosBeep(500);
                }
        }
        while(iRet==-2);
        return iRet;
}


void ShowMainMenuItem(int ItemNum, int HighLight)
{
        int iMod;
        int MenuPos[]={1, 2, 3, 4, 5};
        char MenuTitle[][19]={"    ¸ıãˆ¢•ˆóıÌ    ", "      ‡åìó•ã      ", "     êñåÉÂ‡ã     ", "     êñåÛ˝·ıó     ", "     †™˛ã•É˛ˆ     "};
        iMod=0x07;
        if(HighLight)  iMod=0x70;
        BiosPutStrMove(MenuPos[ItemNum], 1, 18, MenuTitle[ItemNum], iMod);
}


int DoDateMenu()
{
        int iRet;
        int iSel;
        int iOld;
        unsigned int ch;

        iRet=-2;
        iSel=0;

        BiosClrScr(0x07);
        BiosHideCursor();
        BiosSetCursorPos(0, 0);

        AnounceDate();
        ShowDateMenuItem(0, 1);
        ShowDateMenuItem(1, 0);
        do
        {
                ch=BiosGetChar();
                switch(ch)
                {
                        case 0x1C0D://enter
                                iRet=iSel;
                                break;
                        case 0x4800://up
                                {
                                iOld=iSel;
                                if(iSel==0)
                                {
                                        iSel=1;
                                }
                                else
                                {
                                        iSel--;
                                }
                                ShowDateMenuItem(iOld, 0);
                                ShowDateMenuItem(iSel, 1);
                                break;
                                }
                        case 0x5000://down
                                {
                                iOld=iSel;
                                if(iSel==1)
                                {
                                        iSel=0;
                                }
                                else
                                {
                                        iSel++;
                                }
                                ShowDateMenuItem(iOld, 0);
                                ShowDateMenuItem(iSel, 1);
                                break;
                                }
                         default:
                                BiosBeep(500);
                }
        }
        while(iRet==-2);;
}

void AnounceDate(void)
{
        char sdw[10];
        char sd[3];
        char sm[3];
        char sy[5];
        char sc[3];
        char sdate[200];
        int y;
        int m;
        int d;
        int wrow;

        ShamsiDate(sdw, sd, sm, sy);      
       

        DrawBox(0, 0, 5, 19,0x07);
        strcpy(sdate, sy);
        strcat(sdate, "/");
        strcat(sdate, sm);
        strcat(sdate, "/");
        strcat(sdate, sd);
        BiosPutStrMove(0, 5, 10, "::°˛•åÉó::", 0x07);
        wrow=10-(int)strlen(sdw)/2;
        BiosPutStrMove(2, wrow, strlen(sdw), sdw, 0x07);
        BiosPutStrMove(3, 5, strlen(sdate), sdate, 0x07);
};

void ShowDateMenuItem(int ItemNum, int HighLight)
{
        int iMod;
        int MenuPos[]={6, 7};
        char MenuTitle[][19]={"      £˝˛åÉó      ", "      •É˝˝Ëó      "};
        iMod=0x07;
        if(HighLight)  iMod=0x70;
        BiosPutStrMove(MenuPos[ItemNum], 1, 18, MenuTitle[ItemNum], iMod);
}

void ShamsiDate(char* sDayOfWeek, char* sDay, char* sMonth, char *sYear)
{

        unsigned char dayofweek[2];
        unsigned char day[2];
        unsigned char month[2];
        unsigned char year[2];
        unsigned char century[2];

        char sTemp[3];

        int iDay;
        int iMonth;
        int iYear;
        int iCentury;
        int iDayOfWeek;

        int jDay;
        int jMonth;
        int jYear;

        BiosGetDate(dayofweek, day, month, year, century);

        iDayOfWeek=dayofweek[0]&0x0F;
        iDay=(day[0]>>4)*10+(day[0]&0x0F);
        iMonth=(month[0]>>4)*10+(month[0]&0x0F);
        iCentury=century[0];
        if(iCentury==25)
        {
                iCentury=19;
        }
        else
        if(iCentury==32)
        {
                iCentury=20;
        }
        iYear=iCentury*100+((year[0]>>4)*10)+(year[0]&0x0F);

        switch(iDayOfWeek)
        {
        case 0:
                strcpy(sDayOfWeek, "¯ìı´Ì˛");
                break;
        case 1:
                strcpy(sDayOfWeek, "¯ìı´ˆ£");
                break;
        case 2:
                strcpy(sDayOfWeek, "¯ìı´¯©");
                break;
        case 3:
                strcpy(sDayOfWeek, "¯ìı´•å˘ù");
                break;
        case 4:
                strcpy(sDayOfWeek, "¯ìı´õıï");
                break;
        case 5:
                strcpy(sDayOfWeek, "¯‰Ûõ");
                break;
        default:
                strcpy(sDayOfWeek, "¯ìı´");
        };

        gregorian_to_jalali(&jYear, &jMonth, &jDay, iYear, iMonth, iDay);

        itoa(jDay, sDay, 10);
        if(jDay<10)
        {
                strcpy(sTemp, "0");
                strcat(sTemp, sDay);
                strcpy(sDay, sTemp);
        }
        itoa(jMonth, sMonth, 10);
        if(jMonth<10)
        {
                strcpy(sTemp, "0");
                strcat(sTemp, sMonth);
                strcpy(sMonth, sTemp);
        }
        itoa(jYear, sYear, 10);

}

void DrawBox(int iMinrow, int iMincol, int iMaxrow, int iMaxcol, int iAttr)
{

        unsigned char minrow;
        unsigned char mincol;
        unsigned char maxrow;
        unsigned char maxcol;
        unsigned char attr;

        unsigned char oldrow;
        unsigned char oldcol;
        unsigned char row;

        minrow=iMinrow;
        mincol=iMincol;
        maxrow=iMaxrow;
        maxcol=iMaxcol;
        attr=iAttr;

        if((maxrow-minrow)<2)
                return;
        BiosGetCursorPos(&oldrow, &oldcol);

        /*      Corners */
        BiosSetCursorPos(minrow, mincol);
        BiosPutCharAttr((char)218, attr, 1);
        BiosSetCursorPos(maxrow, mincol);
        BiosPutCharAttr((char)192, attr, 1);
        BiosSetCursorPos(minrow, maxcol);
        BiosPutCharAttr((char)191, attr, 1);
        BiosSetCursorPos(maxrow, maxcol);
        BiosPutCharAttr((char)217, attr, 1);

        /* Top */
        BiosSetCursorPos(minrow, (unsigned char)(mincol+1));
        if(maxcol>(unsigned char)(mincol+1))
                BiosPutCharAttr((char)196, attr, (unsigned char)(maxcol-mincol-(unsigned char)1));
        /* Sides */
        for(row=(unsigned char)(minrow+1);row<maxrow;row++)
        {
                BiosSetCursorPos(row, mincol);
                BiosPutCharAttr((char)179, attr, 1);
                BiosSetCursorPos(row, maxcol);
                BiosPutCharAttr((char)179, attr, 1);
        }

        /* Bottom */
        BiosSetCursorPos(maxrow, (unsigned char)(mincol+1));
        if(maxcol>(unsigned char)(mincol+1))
                BiosPutCharAttr((char)196, attr, (unsigned char)(maxcol-mincol-1));
        BiosSetCursorPos(oldrow, oldcol);
}


/* This file is part of:
 *    Jalali, a Gregorian to Jalali and inverse date convertor
 * Copyright (C) 2001  Roozbeh Pournader <roozbeh@sharif.edu>
 * Copyright (C) 2001  Mohammad Toossi <mohammad@bamdad.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You can receive a copy of GNU Lesser General Public License at the
 * World Wide Web address <http://www.gnu.org/licenses/lgpl.html>.
 *
 * For licensing issues, contact The FarsiWeb Project Group,
 * Computing Center, Sharif University of Technology,
 * PO Box 11365-8515, Tehran, Iran, or contact us the
 * email address <FWPG@sharif.edu>.
 */

/* Changes:
 * 
 * 2001-Sep-21:
 *	Fixed a bug with "30 Esfand" dates, reported by Mahmoud Ghandi
 *
 * 2001-Sep-20:
 *	First LGPL release, with both sides of conversions
 */
 
 
void gregorian_to_jalali(
	int *j_y,
	int *j_m,
	int *j_d,
	const int g_y,
	const int g_m,
	const int g_d)
{
   int gy, gm, gd;
   int jy, jm, jd;
   long g_day_no, j_day_no;
   int j_np;
   long l365;
   int g_days_in_month[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
   int j_days_in_month[12] = {31, 31, 31, 31, 31, 31, 30, 30, 30, 30, 30, 29};

 
   int i;
/* 
   printf("Current Gregorian date: %04d-%02d-%02d\n", g.da_year,
                                                      g.da_mon, g.da_day);
*/ 
   gy = g_y-1600;
   gm = g_m-1;
   gd = g_d-1;

   l365=365;
   g_day_no = l365*gy+(gy+3)/4-(gy+99)/100+(gy+399)/400;
   for (i=0;i<gm;++i)
      g_day_no += g_days_in_month[i];
   if (gm>1 && ((gy%4==0 && gy%100!=0) || (gy%400==0)))
      /* leap and after Feb */
      ++g_day_no;
   g_day_no += gd;
 
   j_day_no = g_day_no-79;
 
   j_np = j_day_no / 12053;
   j_day_no %= 12053;
 
   jy = 979+33*j_np+4*(j_day_no/1461);
   j_day_no %= 1461;
 
   if (j_day_no >= 366) {
      jy += (j_day_no-1)/365;
      j_day_no = (j_day_no-1)%365;
   }
 
   for (i = 0; i < 11 && j_day_no >= j_days_in_month[i]; ++i) {
      j_day_no -= j_days_in_month[i];
   }
   jm = i+1;
   jd = j_day_no+1;
   *j_y = jy;
   *j_m = jm;
   *j_d = jd;
}

void jalali_to_gregorian(
	int *g_y,
	int *g_m,
	int *g_d,
	const int j_y,
	const int j_m,
	const int j_d)
{
   int gy, gm, gd;
   int jy, jm, jd;
   long g_day_no, j_day_no;
   int leap;
   long l365;
   long l8;
   long l4;
   long l400;
   long l100;

   int i;

   int g_days_in_month[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
   int j_days_in_month[12] = {31, 31, 31, 31, 31, 31, 30, 30, 30, 30, 30, 29};


   l365=365;
   jy = j_y-979;
   jm = j_m-1;
   jd = j_d-1;

   l8=8;
   j_day_no = l365*jy + (jy/33)*l8 + (jy%33+3)/4;
   for (i=0; i < jm; ++i)
      j_day_no += j_days_in_month[i];

   j_day_no += jd;

   g_day_no = j_day_no+79;

   l400=400;
   gy = 1600 + l400*(g_day_no/146097); /* 146097 = 365*400 + 400/4 - 400/100 + 400/400 */
   g_day_no = g_day_no % 146097;

   leap = 1;
   if (g_day_no >= 36525) /* 36525 = 365*100 + 100/4 */
   {
      g_day_no--;
      l100=100;
      gy += l100*(g_day_no/36524); /* 36524 = 365*100 + 100/4 - 100/100 */
      g_day_no = g_day_no % 36524;
      
      if (g_day_no >= l365)
         g_day_no++;
      else
         leap = 0;
   }

   l4=4;
   gy += 4*(g_day_no/1461); /* 1461 = 365*4 + 4/4 */
   g_day_no %= 1461;

   if (g_day_no >= 366) {
      leap = 0;

      g_day_no--;
      gy += g_day_no/365;
      g_day_no = g_day_no % 365;
   }

   for (i = 0; g_day_no >= g_days_in_month[i] + (i == 1 && leap); i++)
      g_day_no -= g_days_in_month[i] + (i == 1 && leap);
   gm = i+1;
   gd = g_day_no+1;

   *g_y = gy;
   *g_m = gm;
   *g_d = gd;
}

void InitPersianEnvironment()
{
        LoadFont("B:\\FARSI.FNT");
        BiosSelectFont(1 ,pgFontPtr, 0, 0);        
}

void ShowBatteryStatus()
{
        char sBattery[2];
        unsigned int iStatus;

        iStatus=BiosChkBattery();

        iStatus=(iStatus & 0x000F);

        sBattery[1]='\0';

        switch(iStatus)
        {
                case 0://good
                        sBattery[0]=0x26;
                        break;
                case 1://low
                        sBattery[0]=0x0C;
                        break;
                case 2://dead
                        sBattery[0]=0x0B;
                        break;
        }
        BiosPutStrMove(7, 1, 1, sBattery, 0x07);
}

void LoadFont(char *strFilePath)
{
        int iHandle;
        unsigned int size;
        char strErrorMessage[]="Error Openning Font File!";

        if(pgFontPtr)
        {
                free(pgFontPtr);
                pgFontPtr=NULL;
        }
        iHandle=open(strFilePath, O_RDONLY);

        if(iHandle)
        {
                size=(unsigned int)filelength(iHandle);
                pgFontPtr=malloc(size);
                read(iHandle, pgFontPtr, size);
                close(iHandle);
        }
        else
        {
                BiosPutStrMove(0, 0, strlen(strErrorMessage), strErrorMessage, 0x07);
        }
};

int DoCradleMenu()
{
        if(!IsInTheCradle())
                return 0;
        //SetupComReady();
        return 1;
}

int IsInTheCradle()
{
        int iSec;
        int iWasNot;
        iSec=0;
        iWasNot=0;
        BiosGetAbortStatus();
        for(;;)
        {
        if(BiosGetAbortStatus())
        {
                return 0;//not in the cradle and not want to put it in
        }
        if(BiosGetPowerSource()==1)
        {

                if(++iSec>=2)
                {
                        if (iWasNot) BiosClrScr(0x07);
                        return 1;
                }
                else
                        BiosDelay(2000L);
        }
        else
        {
                iWasNot=1;
                BiosClrScr(0x07);
                DrawBox(0, 0, 6, 19,0x07);
                BiosPutStrMove(7, 1, 18, "  CLEAR= êñ´Ó¶åì  ", 0x70);
                BiosPutStrMove(3, 1, 18, "¯˛åï ˚ˆ• ã• ˜åÓó©£", 0x07);
                BiosPutStrMove(4, 1, 18, "     !£˛•ã§Óì     ", 0x07);
                BiosBeep(500);
        }
        }
        return 0;
}

void SetupComReady()
{
    int ComHandle;
    ConfigureLine(&ComHandle);
//    OpenCommLine();
} 

int ConfigureLine(int *pComHandle)
{
  IoctlT ioctl;
  unsigned int DeviceInfo;
  int CommError;

      if ( DosOpen("COM1", READWRITE, pComHandle) )  return 0;
      
      DeviceInfo = DosIoCtrlGetInfo(*pComHandle);
      if ( (DeviceInfo & ISDEVICE) == 0 )            return 0;
      DosIoCtrlSetInfo(*pComHandle, DeviceInfo | RAWMODE);

      /*--- Select protocol.  Read to set up ioctl ---*/
      ioctl.funcode = ComIoctlSelectProtCmd;            /*--- Fn 0x22 ---*/
      DosIoCtrlRdData(*pComHandle, (char *)&ioctl, ComIoctlSelectProtLen);

      /*--- Write out the selected protocol ---*/
      ioctl.funcode = ComIoctlSelectProtCmd;
      ioctl.data.selectprotocol.protocol = MSISTD;
      if ( DosIoCtrlWrData(*pComHandle, (char *)&ioctl, ComIoctlSelectProtLen) !=
	   ComIoctlSelectProtLen )
	 CommError = -1;
      else
	 CommError = ioctl.errcode;

      if ( CommError )   return 0;

      /*--- Get the current line parameters ---*/
      ioctl.funcode = ComIoctlComParamCmd;
      DosIoCtrlRdData(*pComHandle, (char *)&ioctl, ComIoctlComParamLen);

      ioctl.data.comparameters.databits      = DATABITS8;
      ioctl.data.comparameters.parity        = PARITYNONE;
      ioctl.data.comparameters.flowctl       = NOFLOWCTL;
      ioctl.data.comparameters.baudrate      = BAUD9600;
      ioctl.data.comparameters.stopbits      = STOPBITS1;
      ioctl.data.comparameters.duplex        = 0x02;//MULTIACCESS;
      ioctl.data.comparameters.modemdelay    = 0;
      ioctl.data.comparameters.rxcharwait    = 10;    /* 10 sec timeout */
      ioctl.data.comparameters.dsrwait       = 0;
      ioctl.data.comparameters.cdwait        = 0;
      ioctl.data.comparameters.spacetime     = 0;
      ioctl.data.comparameters.marktime      = 0;

      ioctl.funcode = ComIoctlComParamCmd;
      if ( DosIoCtrlWrData(*pComHandle, (char *)&ioctl, ComIoctlComParamLen) !=
	    ComIoctlComParamLen )
	  CommError = -1;
      else
	  CommError = ioctl.errcode;
      if ( CommError ) return 0;
     
      return 1;         //OK
}; 

int DoSettingsMenu()
{
        int iRet;
        int iSel;
        int iOld;
        unsigned int ch;

        BiosGetAbortStatus();

        BiosClrScr(0x07);
        BiosHideCursor();
        BiosSetCursorPos(0, 0);


        iRet=-2;
        iSel=0;

        DrawBox(0, 0, 6, 19,0x07);
        BiosPutStrMove(0, 3,13, "::êñåÛÉ˝·ıó::", 0x07);
        BiosPutStrMove(7, 1, 18, "  CLEAR= êñ´Ó¶åì  ", 0x70);
        ShowSettingsMenuItem(0, 1);
        ShowSettingsMenuItem(1, 0);
        ShowSettingsMenuItem(2, 0);
        ShowSettingsMenuItem(3, 0);
        ShowSettingsMenuItem(4, 0);
        do
        {
                ch=BiosGetChar();
                switch(ch)
                {
                        case 0x1C0D://enter
                                iRet=iSel;
                                break;
                        case 0x4800://up
                                {
                                iOld=iSel;
                                if(iSel==0)
                                {
                                        iSel=4;
                                }
                                else
                                {
                                        iSel--;
                                }
                                ShowSettingsMenuItem(iOld, 0);
                                ShowSettingsMenuItem(iSel, 1);
                                break;
                                }
                        case 0x5000://down
                                {
                                iOld=iSel;
                                if(iSel==4)
                                {
                                        iSel=0;
                                }
                                else
                                {
                                        iSel++;
                                }
                                ShowSettingsMenuItem(iOld, 0);
                                ShowSettingsMenuItem(iSel, 1);
                                break;
                                }
                         default:
                                if(BiosGetAbortStatus())
                                        iRet=5;
                                else
                                        BiosBeep(500);
                }
        }
        while(iRet==-2);
        return iRet;
}

void ShowSettingsMenuItem(int ItemNum, int HighLight)
{
        int iMod;
        int MenuPos[]={1, 2, 3, 4, 5};
//        char MenuTitle[][19]={"      °É˛•åó      ", "      êñÉÂå©      ", "  ˜åÓó©££ã¶â˚åØÍ  " ,"å˙å‡¢ˆå˙˜£ıãˆ¢êÍ§ü" ,"     êñ´ÉÓ¶åì    "};
        char MenuTitle[][19]={"      °É˛•åó      ", "      êñÉÂå©      ", "  ˜åÓó©££ã¶â˚åØÍ  " ,"å˙å‡¢ˆå˙˜£ıãˆ¢êÍ§ü" ,"    ¸´ˆÛå¢ÙåÛ¶    "};
        iMod=0x07;                                                                        
        if(HighLight)  iMod=0x70;
        BiosPutStrMove(MenuPos[ItemNum], 1, 18, MenuTitle[ItemNum], iMod);
}

void ChangeDate()
{
        unsigned int ch;
        int iCurrentInput;
        int iChanged;
        char sInput[3][3];
        unsigned char row;
        unsigned char col;
        int id;
        int im;
        int iy;
        unsigned char d;
        unsigned char m;
        unsigned char y;
        unsigned char c;
        int i2000;
        char sItems[][19]={"      --:¶ˆ•      ", "      --:˜åÛ      ", "      --:Ôå©      ", "      £É˝˛åó      "};
        iCurrentInput=0;
        iChanged=0;
        strcpy(sInput[0], "");
        strcpy(sInput[1], "");
        strcpy(sInput[2], "");
//        sInput[0][2]='\0';
//        sInput[1][2]='\0';
//        sInput[2][2]='\0';
        BiosClrScr(0x07);
        DrawBox(0, 0, 6, 19,0x07);
        BiosPutStrMove(2, 1, 18, sItems[0], 0x07);
        BiosPutStrMove(3, 1, 18, sItems[1], 0x07);
        BiosPutStrMove(4, 1, 18, sItems[2], 0x07);
        BiosPutStrMove(7, 1, 18, sItems[3], 0x07);
        BiosSetCursorPos(2, 7);
        BiosShowCursor();
        do
        {
                ch=BiosGetChar();
                switch(ch)
                {
                        case 0x1C0D://enter
                                if(iCurrentInput<3)
                                {

                                                if(iCurrentInput==0)
                                                {
                                                        if((atoi(sInput[0])>31)||(atoi(sInput[0])<1))
                                                        {
                                                                BiosBeep(500);
                                                        }
                                                        else
                                                        {
                                                                iCurrentInput++;
                                                        }

                                                }
                                                else
                                                if(iCurrentInput==1)
                                                {
                                                        if((atoi(sInput[1])>12)||(atoi(sInput[1])<1))
                                                        {
                                                                BiosBeep(500);
                                                        }
                                                        else
                                                        {
                                                                iCurrentInput++;
                                                        }
                                                }
                                                else
                                                {
                                                        if(atoi(sInput[2])==0)
                                                        {
                                                                BiosBeep(500);
                                                        }
                                                        else
                                                        {
                                                                iCurrentInput++;
                                                        }

                                                }

                                        if(iCurrentInput<3)
                                        {
                                                BiosPutStrMove(7, 1, 18, sItems[3], 0x07);
                                                BiosPutStrMove(iCurrentInput+2, 1, 18, sItems[iCurrentInput], 0x07);
                                                strcpy(sInput[iCurrentInput],"00");
                                                BiosSetCursorPos(iCurrentInput+2, 7);
                                                BiosShowCursor();
                                        }
                                        else
                                        {
                                                BiosHideCursor();
                                                BiosPutStrMove(7, 1, 18, sItems[3], 0x70);
                                        }

                                }
                                else
                                {
                                        jalali_to_gregorian(&iy, &im, &id, 1300+atoi(sInput[2]), atoi(sInput[1]), atoi(sInput[0]));

                                        d=(unsigned char)id/10;
                                        d=d<<4;
                                        d=d+((unsigned char)id%10);
                                        m=(unsigned char)im/10;
                                        m=m<<4;
                                        m=m+((unsigned char)im%10);
                                        if(iy>=2000)
                                        {
                                                c=35;
                                                i2000=2000;
                                        }
                                        else
                                        {
                                                c=25;
                                                i2000=1900;
                                        }
                                        y=(unsigned char)(iy-i2000)/10;
                                        y=y<<4;
                                        y=y+((unsigned char)(iy-i2000)%10);
                                        BiosSetDate((unsigned char)dayofweek(id, im, iy), d, m, y, c);                                        
                                        iChanged=1;
                                }
                                break;
                        case 0x4800://up
                                {
                                                if(iCurrentInput==0)
                                                {
                                                        if((atoi(sInput[0])>31)||(atoi(sInput[0])<1))
                                                        {
                                                                BiosBeep(500);
                                                        }
                                                        else
                                                        {
                                                        if(iCurrentInput==0)
                                                                iCurrentInput=3;
                                                        else
                                                                iCurrentInput--;
                                                        }

                                                }
                                                else
                                                if(iCurrentInput==1)
                                                {
                                                        if((atoi(sInput[1])>12)||(atoi(sInput[1])<1))
                                                        {
                                                                BiosBeep(500);
                                                        }
                                                        else
                                                        {
                                                        if(iCurrentInput==0)
                                                                iCurrentInput=3;
                                                        else
                                                                iCurrentInput--;
                                                        }
                                                }
                                                else
                                                {
                                                        if(atoi(sInput[2])==0)
                                                        {
                                                                BiosBeep(500);
                                                        }
                                                        else
                                                        {
                                                        if(iCurrentInput==0)
                                                                iCurrentInput=3;
                                                        else
                                                                iCurrentInput--;
                                                        }

                                                }

                                if(iCurrentInput<3)
                                        {
                                        BiosPutStrMove(7, 1, 18, sItems[3], 0x07);
                                        BiosPutStrMove(iCurrentInput+2, 1, 18, sItems[iCurrentInput], 0x07);
                                        strcpy(sInput[iCurrentInput],"00");
                                        BiosSetCursorPos(iCurrentInput+2, 7);
                                        BiosShowCursor();
                                        }
                                else
                                {
                                        BiosHideCursor();
                                        BiosPutStrMove(7, 1, 18, sItems[3], 0x70);
                                }
                                break;
                                }
                        case 0x5000://down
                                {
                                                if(iCurrentInput==0)
                                                {
                                                        if((atoi(sInput[0])>31)||(atoi(sInput[0])<1))
                                                        {
                                                                BiosBeep(500);
                                                        }
                                                        else
                                                        {
                                                        if(iCurrentInput==3)
                                                                iCurrentInput=0;
                                                        else
                                                                iCurrentInput++;
                                                        }

                                                }
                                                else
                                                if(iCurrentInput==1)
                                                {
                                                        if((atoi(sInput[1])>12)||(atoi(sInput[1])<1))
                                                        {
                                                                BiosBeep(500);
                                                        }
                                                        else
                                                        {
                                                        if(iCurrentInput==3)
                                                                iCurrentInput=0;
                                                        else
                                                                iCurrentInput++;
                                                        }
                                                }
                                                else
                                                {
                                                        if(atoi(sInput[2])==0)
                                                        {
                                                                BiosBeep(500);
                                                        }
                                                        else
                                                        {
                                                        if(iCurrentInput==3)
                                                                iCurrentInput=0;
                                                        else
                                                                iCurrentInput++;
                                                        }

                                                }
                                if(iCurrentInput<3)
                                        {
                                        BiosPutStrMove(7, 1, 18, sItems[3], 0x07);
                                        BiosPutStrMove(iCurrentInput+2, 1, 18, sItems[iCurrentInput], 0x07);
                                        strcpy(sInput[iCurrentInput],"00");  
                                        BiosSetCursorPos(iCurrentInput+2, 7);
                                        BiosShowCursor();
                                        }
                                else
                                {
                                        BiosHideCursor();
                                        BiosPutStrMove(7, 1, 18, sItems[3], 0x70);
                                }
                                break;
                                }
                         default:
                                switch((char)ch)
                                {
                                 case '0':
                                 case '1':
                                 case '2':
                                 case '3':
                                 case '4':
                                 case '5':
                                 case '6':
                                 case '7':
                                 case '8':
                                 case '9':
                                 {
                                        if(iCurrentInput<3)
                                        {
                                        putch(ch);                                        
                                        BiosGetCursorPos(&row, &col);
                                        if(col==8) sInput[iCurrentInput][0]=(char)ch;
                                        if(col==9)
                                        {
                                                sInput[iCurrentInput][1]=(char)ch;
                                                if(iCurrentInput==0)
                                                {
                                                        if((atoi(sInput[0])>31)||(atoi(sInput[0])<1))
                                                        {
                                                                BiosBeep(500);
                                                        }
                                                        else
                                                                iCurrentInput++;
                                                }
                                                else
                                                if(iCurrentInput==1)
                                                {
                                                        if((atoi(sInput[1])>12)||(atoi(sInput[1])<1))
                                                        {
                                                                BiosBeep(500);
                                                        }
                                                        else
                                                                iCurrentInput++;
                                                }
                                                else
                                                {
                                                        if(atoi(sInput[2])==0)
                                                        {
                                                                BiosBeep(500);
                                                        }
                                                        else
                                                                iCurrentInput++;
                                                }

                                                if(iCurrentInput<3)
                                                {
                                                        BiosPutStrMove(7, 1, 18, sItems[3], 0x07);
                                                        BiosPutStrMove(iCurrentInput+2, 1, 18, sItems[iCurrentInput], 0x07);
                                                        strcpy(sInput[iCurrentInput],"00");
                                                        BiosSetCursorPos(iCurrentInput+2, 7);
                                                        BiosShowCursor();
                                                }
                                                else
                                                {
                                                        BiosHideCursor();
                                                        BiosPutStrMove(7, 1, 18, sItems[3], 0x70);
                                                }
                                        }
                                        }
                                        else
                                                  BiosBeep(500);
                                 break;
                                 }
                                 default:
                                        BiosBeep(500);
                                }
                }
        }
        while(iChanged!=1);
}

int dayofweek(int dayofmonth, int month, int year)
{
        int a, b, c, day, febdays, t;
        int isleapyear;


        isleapyear= year%100 ? !(year%4) : !(year%400);
        if(isleapyear)
                febdays=29;
        else
                febdays=28;
        a=year-2000;
        t=a>0;
        b=(a-t)/4+t;
        c=(a-t)/100;
        c-=c/4;
        day=a+b-c;
        while(--month)
        if(month==1||month==3||month==5||month==7||month==8||month==10)
                day+=31;
        else if (month!=2) day+=30;
                else day+=febdays;
        day+=dayofmonth+4;
        day%=7;
        if(day<0) day+=7;

        if(day==6)
                day=0;
        else
                day++;
        return day;
}

int DoClockMenu()
{
        int iRet;
        int iSel;
        int iOld;
        unsigned int ch;

        iRet=-2;
        iSel=0;

        BiosClrScr(0x07);
        BiosHideCursor();
        BiosSetCursorPos(0, 0);

        AnounceClock();
        ShowDateMenuItem(0, 1);
        ShowDateMenuItem(1, 0);
        do
        {
                ch=BiosGetChar();
                switch(ch)
                {
                        case 0x1C0D://enter
                                iRet=iSel;
                                break;
                        case 0x4800://up
                                {
                                iOld=iSel;
                                if(iSel==0)
                                {
                                        iSel=1;
                                }
                                else
                                {
                                        iSel--;
                                }
                                ShowDateMenuItem(iOld, 0);
                                ShowDateMenuItem(iSel, 1);
                                break;
                                }
                        case 0x5000://down
                                {
                                iOld=iSel;
                                if(iSel==1)
                                {
                                        iSel=0;
                                }
                                else
                                {
                                        iSel++;
                                }
                                ShowDateMenuItem(iOld, 0);
                                ShowDateMenuItem(iSel, 1);
                                break;
                                }
                         default:
                                BiosBeep(500);
                }
        }
        while(iRet==-2);;
}


void AnounceClock(void)
{
        unsigned char hours[2];
        unsigned char mins[2];
        unsigned char secs[2];
        char sh[3];
        char sm[3];
        char ss[3];
        char sclock[9];
        int h;
        int m;
        int s;
        char sTemp[3];
      
        BiosGetTime(hours, mins, secs);
        h=((hours[0]>>4)*10)+(hours[0]&0x0F);
        m=((mins[0]>>4)*10)+(mins[0]&0x0F);
        s=((secs[0]>>4)*10)+(secs[0]&0x0F);
        itoa(h, sh, 10);

        if(h<10)
        {
                strcpy(sTemp, "0");
                strcat(sTemp,sh);
                strcpy(sh, sTemp);
        }

        itoa(m, sm, 10);

        if(m<10)
        {
                strcpy(sTemp, "0");
                strcat(sTemp,sm);
                strcpy(sm, sTemp);
        }

        itoa(s, ss, 10);

        if(s<10)
        {
                strcpy(sTemp, "0");
                strcat(sTemp,ss);
                strcpy(ss, sTemp);
        }

        DrawBox(0, 0, 5, 19,0x07);
        strcpy(sclock, sh);
        strcat(sclock, ":");
        strcat(sclock, sm);
        strcat(sclock, ":");
        strcat(sclock, ss);
        BiosPutStrMove(0, 5, 10, "::êñÉÂå©::", 0x07);
        BiosPutStrMove(3, 6, strlen(sclock), sclock, 0x07);
};


void ChangeClock()
{
        unsigned int ch;
        int iCurrentInput;
        int iChanged;
        char sInput[3][3];
        unsigned char row;
        unsigned char col;
        int id;
        int im;
        int iy;
        unsigned char d;
        unsigned char m;
        unsigned char y;
        unsigned char c;
        int i2000;
        char sItems[][19]={"     --:êñÂå©     ", "     --:¯Ï˝Ï£     ", "     --:¯˝ıåô     ", "      £É˝˛åó      "};
        iCurrentInput=0;
        iChanged=0;
        strcpy(sInput[0], "");
        strcpy(sInput[1], "");
        strcpy(sInput[2], "");
//        sInput[0][2]='\0';
//        sInput[1][2]='\0';
//        sInput[2][2]='\0';
        BiosClrScr(0x07);
        DrawBox(0, 0, 6, 19,0x07);
        BiosPutStrMove(2, 1, 18, sItems[0], 0x07);
        BiosPutStrMove(3, 1, 18, sItems[1], 0x07);
        BiosPutStrMove(4, 1, 18, sItems[2], 0x07);
        BiosPutStrMove(7, 1, 18, sItems[3], 0x07);
        BiosSetCursorPos(2, 6);
        BiosShowCursor();
        do
        {
                ch=BiosGetChar();
                switch(ch)
                {
                        case 0x1C0D://enter
                                if(iCurrentInput<3)
                                {

                                                if(iCurrentInput==0)
                                                {
                                                        if((atoi(sInput[0])>23))
                                                        {
                                                                BiosBeep(500);
                                                        }
                                                        else
                                                        {
                                                                iCurrentInput++;
                                                        }

                                                }
                                                else
                                                if(iCurrentInput==1)
                                                {
                                                        if((atoi(sInput[1])>59))
                                                        {
                                                                BiosBeep(500);
                                                        }
                                                        else
                                                        {
                                                                iCurrentInput++;
                                                        }
                                                }
                                                else
                                                {
                                                        if(atoi(sInput[2])>59)
                                                        {
                                                                BiosBeep(500);
                                                        }
                                                        else
                                                        {
                                                                iCurrentInput++;
                                                        }

                                                }

                                        if(iCurrentInput<3)
                                        {
                                                BiosPutStrMove(7, 1, 18, sItems[3], 0x07);
                                                BiosPutStrMove(iCurrentInput+2, 1, 18, sItems[iCurrentInput], 0x07);
                                                strcpy(sInput[iCurrentInput],"00");
                                                BiosSetCursorPos(iCurrentInput+2, 6);
                                                BiosShowCursor();
                                        }
                                        else
                                        {
                                                BiosHideCursor();
                                                BiosPutStrMove(7, 1, 18, sItems[3], 0x70);
                                        }

                                }
                                else
                                {
                                        iy=atoi(sInput[0]);
                                        im=atoi(sInput[1]);
                                        id=atoi(sInput[2]);

                                        d=(unsigned char)id/10;
                                        d=d<<4;
                                        d=d+((unsigned char)id%10);
                                        m=(unsigned char)im/10;
                                        m=m<<4;
                                        m=m+((unsigned char)im%10);
                                        y=(unsigned char)(iy)/10;
                                        y=y<<4;
                                        y=y+((unsigned char)(iy)%10);
                                        BiosSetTime(y, m, d);
                                        iChanged=1;
                                }
                                break;
                        case 0x4800://up
                                {
                                                if(iCurrentInput==0)
                                                {
                                                        if((atoi(sInput[0])>23)||(atoi(sInput[0])<1))
                                                        {
                                                                BiosBeep(500);
                                                        }
                                                        else
                                                        {
                                                        if(iCurrentInput==0)
                                                                iCurrentInput=3;
                                                        else
                                                                iCurrentInput--;
                                                        }

                                                }
                                                else
                                                if(iCurrentInput==1)
                                                {
                                                        if((atoi(sInput[1])>59))
                                                        {
                                                                BiosBeep(500);
                                                        }
                                                        else
                                                        {
                                                        if(iCurrentInput==0)
                                                                iCurrentInput=3;
                                                        else
                                                                iCurrentInput--;
                                                        }
                                                }
                                                else
                                                {
                                                        if(atoi(sInput[2])>59)
                                                        {
                                                                BiosBeep(500);
                                                        }
                                                        else
                                                        {
                                                        if(iCurrentInput==0)
                                                                iCurrentInput=3;
                                                        else
                                                                iCurrentInput--;
                                                        }

                                                }

                                if(iCurrentInput<3)
                                        {
                                        BiosPutStrMove(7, 1, 18, sItems[3], 0x07);
                                        BiosPutStrMove(iCurrentInput+2, 1, 18, sItems[iCurrentInput], 0x07);
                                        strcpy(sInput[iCurrentInput],"00");
                                        BiosSetCursorPos(iCurrentInput+2, 6);
                                        BiosShowCursor();
                                        }
                                else
                                {
                                        BiosHideCursor();
                                        BiosPutStrMove(7, 1, 18, sItems[3], 0x70);
                                }
                                break;
                                }
                        case 0x5000://down
                                {
                                                if(iCurrentInput==0)
                                                {
                                                        if((atoi(sInput[0])>23)||(atoi(sInput[0])<1))
                                                        {
                                                                BiosBeep(500);
                                                        }
                                                        else
                                                        {
                                                        if(iCurrentInput==3)
                                                                iCurrentInput=0;
                                                        else
                                                                iCurrentInput++;
                                                        }

                                                }
                                                else
                                                if(iCurrentInput==1)
                                                {
                                                        if(atoi(sInput[1])>59)
                                                        {
                                                                BiosBeep(500);
                                                        }
                                                        else
                                                        {
                                                        if(iCurrentInput==3)
                                                                iCurrentInput=0;
                                                        else
                                                                iCurrentInput++;
                                                        }
                                                }
                                                else
                                                {
                                                        if(atoi(sInput[2])>59)
                                                        {
                                                                BiosBeep(500);
                                                        }
                                                        else
                                                        {
                                                        if(iCurrentInput==3)
                                                                iCurrentInput=0;
                                                        else
                                                                iCurrentInput++;
                                                        }

                                                }
                                if(iCurrentInput<3)
                                        {
                                        BiosPutStrMove(7, 1, 18, sItems[3], 0x07);
                                        BiosPutStrMove(iCurrentInput+2, 1, 18, sItems[iCurrentInput], 0x07);
                                        strcpy(sInput[iCurrentInput],"00");  
                                        BiosSetCursorPos(iCurrentInput+2, 6);
                                        BiosShowCursor();
                                        }
                                else
                                {
                                        BiosHideCursor();
                                        BiosPutStrMove(7, 1, 18, sItems[3], 0x70);
                                }
                                break;
                                }
                         default:
                                switch((char)ch)
                                {
                                 case '0':
                                 case '1':
                                 case '2':
                                 case '3':
                                 case '4':
                                 case '5':
                                 case '6':
                                 case '7':
                                 case '8':
                                 case '9':
                                 {
                                        if(iCurrentInput<3)
                                        {
                                        putch(ch);                                        
                                        BiosGetCursorPos(&row, &col);
                                        if(col==7) sInput[iCurrentInput][0]=(char)ch;
                                        if(col==8)
                                        {
                                                sInput[iCurrentInput][1]=(char)ch;
                                                if(iCurrentInput==0)
                                                {
                                                        if((atoi(sInput[0])>23)||(atoi(sInput[0])<1))
                                                        {
                                                                BiosBeep(500);
                                                        }
                                                        else
                                                                iCurrentInput++;
                                                }
                                                else
                                                if(iCurrentInput==1)
                                                {
                                                        if((atoi(sInput[1])>59))
                                                        {
                                                                BiosBeep(500);
                                                        }
                                                        else
                                                                iCurrentInput++;
                                                }
                                                else
                                                {
                                                        if(atoi(sInput[2])>59)
                                                        {
                                                                BiosBeep(500);
                                                        }
                                                        else
                                                                iCurrentInput++;
                                                }

                                                if(iCurrentInput<3)
                                                {
                                                        BiosPutStrMove(7, 1, 18, sItems[3], 0x07);
                                                        BiosPutStrMove(iCurrentInput+2, 1, 18, sItems[iCurrentInput], 0x07);
                                                        strcpy(sInput[iCurrentInput],"00");
                                                        BiosSetCursorPos(iCurrentInput+2, 6);
                                                        BiosShowCursor();
                                                }
                                                else
                                                {
                                                        BiosHideCursor();
                                                        BiosPutStrMove(7, 1, 18, sItems[3], 0x70);
                                                }
                                        }
                                        }
                                        else
                                                  BiosBeep(500);
                                 break;
                                 }
                                 default:
                                        BiosBeep(500);
                                }
                }
        }
        while(iChanged!=1);
}

void SetKybdTimeout()
{
        unsigned int my;
        BiosClrScr(0x07);
        my=35;
        BiosSetKybdTimeout(my);
        printf("%d", BiosGetKybdTimeout());
        getch();
}

int GetSubCode(char *sInput)
{
        int iCurrentInput;
        unsigned int ch;
        unsigned char row;
        unsigned char col;
        char cCode[17];


        int iEntered=0;
        iCurrentInput=6;
        BiosClrScr(0x07);
        DrawBox(0, 0, 6, 19,0x07);

        BiosPutStrMove(0, 5, 10, "::ˆÉõó©õ::", 0x07);
        BiosPutStrMove(7, 1, 18, " SPACE=ˆÉõó©õ†™ˆ• ", 0x70);
        BiosPutStrMove(2, 3, 14, ":êÌã•ó´ã ˜•åÛ´", 0x07);

        strcpy(cCode, "121213----------");
        cCode[4]=cCityCode[0];
        cCode[5]=cCityCode[1];
        BiosPutStrMove(3, 2, 16, cCode, 0x07);
        sInput[0]='1';
        sInput[1]='2';
        sInput[2]='1';
        sInput[3]='2';
        sInput[4]=cCityCode[0];
        sInput[5]=cCityCode[1];
        sInput[6]='\0';
        BiosSetCursorPos(3, 8);
        BiosShowCursor();
        do
        {
                ch=BiosGetChar();
                switch(ch)
                {
                        case 0x1C0D://enter
                                if(iCurrentInput==15)
                                {
                                        sInput[15]='\0';
                                        iEntered=1;
                                }
                                break;
                         default:
                                switch((char)ch)
                                {
                                 case ' ':
                                        return 0;
                                        break;
                                 case '0':
                                 case '1':
                                 case '2':
                                 case '3':
                                 case '4':
                                 case '5':
                                 case '6':
                                 case '7':
                                 case '8':
                                 case '9':
                                 {
                                        if(iCurrentInput<15)
                                        {
                                                putch(ch);                                        
                                                sInput[iCurrentInput]=(char)ch;
                                                iCurrentInput++;

                                        }
                                        else
                                                BiosBeep(500);
                                 break;
                                 }
                                 default:
                                        BiosBeep(500);
                                }
                }
        }
        while(iEntered!=1);
        BiosHideCursor();
        return 2;
}



/*void LoadToMemory()
{
        char cName[23];
        unsigned datahandle;
        unsigned bytesread;
        int i;
        int j;
        

        if(!DosOpen("D:\\CONTOR.DAT", 0x00, &datahandle))
        {
                DosRead(datahandle, &lCustomersNumber, sizeof(long), &bytesread);
                if(Customers)
                        free(Customers);
                Customers=malloc(lCustomersNumber*sizeof(struct customer));
                for(j=0; j<lCustomersNumber; j++)
                {
                        DosRead(datahandle, &Customers[j].cSubscriptionCode, 15, &bytesread);
                        Customers[j].cSubscriptionCode[15]='\0';
                        if(bytesread==15)
                        {
                                DosRead(datahandle, Customers[j].cName, 18, &bytesread);
                                Customers[j].cName[18]='\0';
                                DosRead(datahandle, Customers[j].cFamily, 22, &bytesread);
                                Customers[j].cFamily[22]='\0';
                                DosRead(datahandle, Customers[j].cAddressCode, 10, &bytesread);
                                Customers[j].cAddressCode[10]='\0';
                                DosRead(datahandle, Customers[j].cCounterSerial, 10, &bytesread);
                                Customers[j].cCounterSerial[10]='\0';
                                DosRead(datahandle, &Customers[j].lConsumptionKind, sizeof(long), &bytesread);
                                DosRead(datahandle, &Customers[j].lCapacity, sizeof(long), &bytesread);
                                DosRead(datahandle, Customers[j].cPrevDate, 6, &bytesread);
                                Customers[j].cPrevDate[6]='\0';
                                DosRead(datahandle, Customers[j].cPrevCounter, 10, &bytesread);
                        }
                }
        }        
        DosClose(datahandle);

}
*/

int ScanSub(char *sInput)
{
        int hscanner;
        IoctlT iob;
        char databuf[80];
        int count;
        int labellen;
        char sdw[10];
        char sd[3];
        char sm[3];
        char sy[5];
        char sc[3];
        char sdate[200];
        int y;
        int m;
        int d;
        int wrow;
        int iRet;

        iRet=0;

        ShamsiDate(sdw, sd, sm, sy);      
       

        strcpy(sdate, sy);
        strcat(sdate, "/");
        strcat(sdate, sm);
        strcat(sdate, "/");
        strcat(sdate, sd);


        BiosClrScr(0x07);
        DrawBox(0, 0, 6, 19,0x07);
        BiosPutStrMove(0, 3, 14, "::ÙÌ©ã ˜£åÛâ::", 0x70);
        BiosPutStrMove(1, 1, 18, "        :    °˛•åó", 0x07);
        BiosPutStrMove(1, 1, 8, &sdate[2], 0x07);
        BiosPutStrMove(2, 1, 18, "        :   ÔÌ•åÛâ", 0x07);
        itoa(iCustomersNum, sdate, 10);
        BiosPutStrMove(2, 1, strlen(sdate), sdate, 0x07);
        BiosPutStrMove(3, 1, 18, "        : å˙˜£ıãˆ¢", 0x07);
        itoa(iReadNum, sdate, 10);
        BiosPutStrMove(3, 1, strlen(sdate), sdate, 0x07);
        BiosPutStrMove(4, 1, 18, "        :å˙˜£ıãˆ¢ı", 0x07);
        itoa(iCustomersNum-iReadNum, sdate, 10);
        BiosPutStrMove(4, 1, strlen(sdate), sdate, 0x07);
        BiosPutStrMove(5, 1, 18, "        :¶ˆ•Ûã•åÛâ", 0x07);
        itoa(iToday, sdate, 10);
        BiosPutStrMove(5, 1, strlen(sdate), sdate, 0x07);
//        BiosPutStrMove(7, 1, 18, "   SPACE=å‡¢ö•£   ", 0x70);
        BiosPutStrMove(7, 1, 18, " SPACE=ˆÉõó©õ†™ˆ• ", 0x70);


        DosOpen("CON", READONLY, &hscanner);
        DosIoCtrlSetInfo(hscanner, DosIoCtrlGetInfo(hscanner)|RAWMODE);
        BiosGetAbortStatus();
        for(;;)
        {
                 //input mode+label timeout
                iob.funcode=ConsIoctlSetInputMode;
                iob.data.inputmode.inmode=INMODE_KEYANDLABELS;
                DosIoCtrlWrData(hscanner, &iob, ConsIoctlSetInputModeLen);

                //read input:
                DosRead(hscanner, databuf, 1, &count);
                if(count)
                {
                        iob.funcode=ConsIoctlGetCharStatus;
                        DosIoCtrlRdData(hscanner, &iob, ConsIoctlGetCharStatusLen);
                        if((iob.data.charstatus.source!=SOURCE_CONTACT)&&(iob.data.charstatus.source!=SOURCE_LASER))
                        {
                                if(databuf[0]==0x20)
                                        iRet=2;
                                if(BiosGetAbortStatus())
                                        iRet=3;
                                break;//key
                        }
                        labellen=iob.data.charstatus.labellen;
                        if(labellen>1)
                        {
                                DosRead(hscanner, databuf+1, labellen-1, &count);
                                databuf[labellen]=0;
                                BiosBeep(100);
                                iRet=1;
                                break;
                        }
                        
                }
        }
        iob.funcode=ConsIoctlSetInputMode;
        iob.data.inputmode.inmode=INMODE_KEYSONLY;
        DosIoCtrlWrData(hscanner, &iob, ConsIoctlSetInputModeLen);

        DosClose(hscanner);
        strcpy(sInput, "121");
        strcat(sInput, databuf);
        return iRet;
}

int ReadValue(int iFound, struct customer* pCustomer)
{
        unsigned int ch;
        int iCurrentInput;
        int iChanged;
        char sInput[11];
        char sTemp[11];
        char sResult[12];
        unsigned char row;
        unsigned char col;
        char sItems[][19]={"          :¸ÒìÏ££Â", "----------:¸Ò‰Í££Â", "---------- : êÍ•≠Û", "      £É˝˛åó      ","     å‡¢  ö•£     ","      êÍã•≠ıã     "};
        int iInputIndex;
        int iPrevInput;
        int iPrevRead;
        unsigned char hours[2];
        unsigned char mins[2];
        unsigned char secs[2];
        char sh[3];
        char sm[3];
        char ss[3];
        int h;
        int m;
        int s;
        char cName[16];

        char sdw[10];
        char sd[3];
//        char sm[3];
        char sy[5];

        fpos_t pos;
        int iNewSearch;

        char b110[110];


        iCurrentInput=0;
        iPrevInput=0;
        iChanged=0;
        iPrevRead=0;

        if(iFound<0)
        {
                BiosClrScr(0x07);
                DrawBox(0, 0, 6, 19,0x07);
                BiosPutStrMove(7, 1, 18, "    £˝ı¶ì˚£˝ÒÌ    ", 0x70);
                BiosPutStrMove(3, 1, 18, " ˜•åÛ´Ù˛ãåì¸Ì•ó´Û ", 0x07);
                BiosPutStrMove(4, 1, 18, "     !£´ıêñÍå˛    ", 0x07);
                BiosBeep(1000);
                BiosGetChar();
                return 1;
        }

        if(iFound==3)
                return 0;

        if(iFound==9)
                return 1;


        if( (iFound==2) && (iCanReadManualySub==0) )
        {
              DoErrorMenu(pCustomer->cSubscriptionCode);
              return 1;
        }

        if( (iFound==5) && (iCanReadManualyAdr==0) )
        {
              DoErrorMenu(pCustomer->cSubscriptionCode);
              return 1;
        }

        if( (iFound==2) || (iFound==5) )
        {
                        iNewSearch=0;
                        strcpy(pCustomer->cCurCounter, "");
                        fseek(fOut, 0, SEEK_SET);
                        while((fread(cName, sizeof(char), 15, fOut)==15)&&!iNewSearch)
                        {
                        cName[15]='\0';
                        if(strcmp(pCustomer->cSubscriptionCode, cName)==0)
                        {
                                iNewSearch=1;
                                lThisInOut=ftell(fOut)-15;
                                fread(pCustomer->cCurCounter, sizeof(char), 10, fOut);
                                fread(pCustomer->cCurDate, sizeof(char), 6, fOut);
                                fread(pCustomer->cCurTime, sizeof(char), 6, fOut);
                        }
                        else
                                fread(b110, sizeof(char), 23, fOut);
                        }

        }

        strcpy(sInput, "");
        strcpy(sTemp, "0");

        BiosClrScr(0x07);
        DrawBox(0, 0, 4, 19,0x07);        
        BiosPutStrMove(1, 1, 18, sItems[0], 0x07);//prev
        BiosPutStrMove(1, 1, 10, pCustomer->cPrevCounter, 0x70);//prev
        BiosPutStrMove(2, 1, 18, sItems[1], 0x07);//cur
        BiosPutStrMove(3, 1, 18, sItems[2], 0x07);//consumption
        iCurrentInput=0;
        if(strlen(pCustomer->cCurCounter))
        {
                BiosPutStrMove(2, 1, 10, pCustomer->cCurCounter, 0x07);//cur
                strcpy(sInput, pCustomer->cCurCounter);
                CalConsumption(pCustomer->cCurCounter, pCustomer->cPrevCounter, sResult);
                BiosPutStrMove(3, 1, strlen(sResult), sResult, 0x70);
                iPrevRead=1;
                iCurrentInput=1;
                fseek(fOut, lThisInOut, SEEK_SET);
        }
        if(iCurrentInput==0)
                BiosPutStrMove(5, 1, 18, sItems[3], 0x07);//enter
        else
                BiosPutStrMove(5, 1, 18, sItems[3], 0x70);//enter
        BiosPutStrMove(6, 1, 18, sItems[4], 0x07);//error insertion
        BiosPutStrMove(7, 1, 18, sItems[5], 0x07);//cancel
        BiosSetCursorPos(2, 1);
        if(!iPrevRead)                
                BiosShowCursor();


        iInputIndex=0;

        do
        {
                ch=BiosGetChar();
                switch(ch)
                {
                        case 0x1C0D://enter
                                if(iCurrentInput==1)
                                        iChanged=1;
                                else
                                {
                                        if(iCurrentInput==0)
                                        {
                                                iCurrentInput=1;

                                                while(strlen(sInput)<10)
                                                {
                                                        strcpy(sTemp, "0");
                                                        strcat(sTemp, sInput);
                                                        strcpy(sInput, sTemp);
                                                }
                                                BiosPutStrMove(2, 1, 10, sInput, 0x07);
                                                strcpy(sResult,"");
                                                CalConsumption(sInput, pCustomer->cPrevCounter, sResult);
                                                BiosPutStrMove(3, 1, strlen(sResult), sResult, 0x70);
                                                BiosHideCursor();
                                                BiosPutStrMove(5, 1, 18, sItems[3], 0x07);//enter
                                                BiosPutStrMove(6, 1, 18, sItems[4], 0x07);//error insertion
                                                BiosPutStrMove(7, 1, 18, sItems[5], 0x07);//cancel
                                                BiosPutStrMove(iCurrentInput+4, 1, 18, sItems[iCurrentInput+2], 0x70);//current
                                        }
                                        else
                                        if(iCurrentInput==2)
                                        {
                                                DoErrorMenu(pCustomer->cSubscriptionCode);
                                                iPrevRead=1;
                                                iChanged=1;
                                        }
                                        else
                                                return 1;

                                }
                                break;
                        case 0x4800://up
                                iPrevInput=iCurrentInput;
                                if(iCurrentInput==0)
                                        iCurrentInput=3;
                                else
                                if( (iCurrentInput==1) )
                                {
                                        if(strlen(pCustomer->cCurCounter))
                                                iCurrentInput==3;
                                        else
                                                iCurrentInput==1;                                                
                                }
                                else
                                        iCurrentInput--;
                                if(iCurrentInput==0)
                                        {
                                        strcpy(sInput, "");
                                        strcpy(sResult, "");
                                        BiosPutStrMove(2, 1, 18, sItems[1], 0x07);//cur
                                        BiosPutStrMove(3, 1, 18, sItems[2], 0x07);//consumption
                                        BiosPutStrMove(7, 1, 18, sItems[5], 0x07);//cancel
                                        BiosSetCursorPos(2, 1);
                                        iInputIndex=0;
                                        BiosShowCursor();
                                        }
                                else
                                        {
                                        if(iPrevInput==0)
                                        {
                                        while(strlen(sInput)<10)
                                                {
                                                        strcpy(sTemp, "0");
                                                        strcat(sTemp, sInput);
                                                        strcpy(sInput, sTemp);
                                                }
                                        BiosPutStrMove(2, 1, 10, sInput, 0x07);
                                        strcpy(sResult,"");
                                        CalConsumption(sInput, pCustomer->cPrevCounter, sResult);
                                        BiosPutStrMove(3, 1, strlen(sResult), sResult, 0x70);
                                        }
                                        BiosHideCursor();
                                        BiosPutStrMove(5, 1, 18, sItems[3], 0x07);//enter
                                        BiosPutStrMove(6, 1, 18, sItems[4], 0x07);//error insertion
                                        BiosPutStrMove(7, 1, 18, sItems[5], 0x07);//cancel
                                        BiosPutStrMove(iCurrentInput+4, 1, 18, sItems[iCurrentInput+2], 0x70);//current
                                        }
                                break;
                        case 0x5000://down
                                iPrevInput=iCurrentInput;
                                if(iCurrentInput==3)
                                {
                                //if(strlen(pCustomer->cCurCounter))
                                //        iCurrentInput=1;
                                //else
                                        iCurrentInput=0;
                                }
                                else
                                        iCurrentInput++;
                                if(iCurrentInput==0)
                                        {
                                        strcpy(sInput, "");
                                        strcpy(sResult, "");
                                        BiosPutStrMove(2, 1, 18, sItems[1], 0x07);//cur
                                        BiosPutStrMove(3, 1, 18, sItems[2], 0x07);//consumption
                                        BiosPutStrMove(7, 1, 18, sItems[5], 0x07);//cancel
                                        BiosSetCursorPos(2, 1);
                                        iInputIndex=0;
                                        BiosShowCursor();
                                        }
                                else
                                        {
                                        if(iPrevInput==0)
                                        {
                                        while(strlen(sInput)<10)
                                                {
                                                        strcpy(sTemp, "0");
                                                        strcat(sTemp, sInput);
                                                        strcpy(sInput, sTemp);
                                                }
                                        BiosPutStrMove(2, 1, 10, sInput, 0x07);
                                        strcpy(sResult, "");
                                        CalConsumption(sInput, pCustomer->cPrevCounter, sResult);
                                        BiosPutStrMove(3, 1, strlen(sResult), sResult, 0x70);
                                        }
                                        BiosHideCursor();
                                        BiosPutStrMove(5, 1, 18, sItems[3], 0x07);//enter
                                        BiosPutStrMove(6, 1, 18, sItems[4], 0x07);//error insertion
                                        BiosPutStrMove(7, 1, 18, sItems[5], 0x07);//cancel
                                        BiosPutStrMove(iCurrentInput+4, 1, 18, sItems[iCurrentInput+2], 0x70);//current
                                        }
                                break;
                         default:
                                //if(BiosGetAbortStatus())
                                        //return 0;
                                switch((char)ch)
                                {
                                 case '0':
                                 case '1':
                                 case '2':
                                 case '3':
                                 case '4':
                                 case '5':
                                 case '6':
                                 case '7':
                                 case '8':
                                 case '9':
                                 {
                                        if(iCurrentInput==0)
                                        {
                                        putch(ch);
                                        BiosGetCursorPos(&row, &col);
                                        if(col<11)
                                        {
                                                sInput[iInputIndex]=(char)ch;
                                                sInput[iInputIndex+1]='\0';
                                                iInputIndex++;
                                        }
                                        
                                        if(col==11)
                                        {
                                                sInput[iInputIndex+1]='\0';
                                                sInput[iInputIndex]=(char)ch;
                                                iCurrentInput=1;
                                                while(strlen(sInput)<10)
                                                {
                                                        strcpy(sTemp, "0");
                                                        strcat(sTemp, sInput);
                                                        strcpy(sInput, sTemp);
                                                }
                                                BiosPutStrMove(2, 1, 10, sInput, 0x07);
                                                strcpy(sResult, "");
                                                CalConsumption(sInput, pCustomer->cPrevCounter, sResult);
                                                BiosPutStrMove(3, 1, strlen(sResult), sResult, 0x70);
                                                BiosHideCursor();
                                                BiosPutStrMove(5, 1, 18, sItems[3], 0x07);//enter
                                                BiosPutStrMove(6, 1, 18, sItems[4], 0x07);//error insertion
                                                BiosPutStrMove(7, 1, 18, sItems[5], 0x07);//cancel
                                                BiosPutStrMove(iCurrentInput+4, 1, 18, sItems[iCurrentInput+2], 0x70);//current
                                        }
                                 }
                                        else
                                                  BiosBeep(500);
                                 break;
                                 }
                                 default:
                                        BiosBeep(500);
                                }
                }
        }
        while(iChanged!=1);
        //if(!iPrevRead)
        {
                iIsDataTransfered=0;//FALSE;
                fwrite(pCustomer->cSubscriptionCode, sizeof(char), 15, fOut);
                pCustomer->cAddressCode[10]='\0';
                strcpy(cLastReadAddressCode, pCustomer->cAddressCode);
                fwrite(sInput, sizeof(char), 10, fOut);
                ShamsiDate(sdw, sd, sm, sy);

                fwrite(&sy[2], sizeof(char), 2, fOut);
                fwrite(sm, sizeof(char), 2, fOut);
                fwrite(sd, sizeof(char), 2, fOut);

                BiosGetTime(hours, mins, secs);
                h=((hours[0]>>4)*10)+(hours[0]&0x0F);
                m=((mins[0]>>4)*10)+(mins[0]&0x0F);
                s=((secs[0]>>4)*10)+(secs[0]&0x0F);
                itoa(h, sh, 10);

                if(h<10)
                {
                        strcpy(sTemp, "0");
                        strcat(sTemp,sh);
                        strcpy(sh, sTemp);
                }

                itoa(m, sm, 10);
        
                if(m<10)
                {
                        strcpy(sTemp, "0");
                        strcat(sTemp,sm);
                        strcpy(sm, sTemp);
                }

                itoa(s, ss, 10);

                if(s<10)
                {
                        strcpy(sTemp, "0");
                        strcat(sTemp,ss);
                        strcpy(ss, sTemp);
                }

                fwrite(sh, sizeof(char), 2, fOut);
                fwrite(sm, sizeof(char), 2, fOut);
                fwrite(ss, sizeof(char), 2, fOut);

                fwrite(&cActiveReaderCode, sizeof(char), 1, fOut);

                fflush(fOut);
                fclose(fOut);
                fOut=fopen("D:\\OUTPUT.DAT", "r+");

                if(!iPrevRead)
                {
                        iReadNum++;
                        CheckToday();
                }

                if(iFound==5)
                    GoNext();

        }
        return 1;
}

void CalConsumption(char* pCur, char *pPrev, char *pRes)
{
        int i,j;
        char sTemp[11];
        if(CompareArrays(pCur, pPrev)<0)
        {
                CalConsumption2(pPrev, pCur, pRes);
                strcpy(sTemp, "-");
                strcat(sTemp, pRes);
                strcpy(pRes, sTemp);
        }
        else
        {
                strcpy(sTemp, pCur);
                pRes[10]='\0';
                for(i=9;i>-1;i--)
                {
                        if(sTemp[i]>=pPrev[i])
                                pRes[i]=sTemp[i]-pPrev[i]+48;
                        else
                        {
                                j=i-1;
                                while(sTemp[j]=='0') j--;
                                sTemp[j]--;
                                j++;
                                while(j<i)
                                {
                                        sTemp[j]+=9;
                                        j++;
                                }
                                sTemp[j]+=10;
                                pRes[i]=sTemp[i]-pPrev[i]+48;
                        }
                }
        }
}

void CalConsumption2(char* pCur, char *pPrev, char *pRes)
{
        int i,j;
        char sTemp[11];

                strcpy(sTemp, pCur);
                pRes[10]='\0';
                for(i=9;i>-1;i--)
                {
                        if(sTemp[i]>=pPrev[i])
                                pRes[i]=sTemp[i]-pPrev[i]+48;
                        else
                        {
                                j=i-1;
                                while(sTemp[j]=='0') j--;
                                sTemp[j]--;
                                j++;
                                while(j<i)
                                {
                                        sTemp[j]+=9;
                                        j++;
                                }
                                sTemp[j]+=10;
                                pRes[i]=sTemp[i]-pPrev[i]+48;
                        }
                }
}


int CompareArrays(char* pCur, char* pPrev)
{
        int i=0;
        while((pCur[i]==pPrev[i])&&(i<10)) i++;
        if(i==10)
                return 0;//equal;
        if(pCur[i]>pPrev[i])
                return 1;
        return -1;
}

void OpenDataset()
{
        char sBuf[150];
        FILE* myfAddressCodes;
        FILE* myConfigFile;
        char cConfig;
        char cNum[5];
        int i;
        iCustomersNum=0;

        BiosHideCursor();
        BiosClrScr(0x07);
        DrawBox(0, 0, 7, 19,0x07);
        BiosPutStrMove(3, 1, 18, "...êñåÂ‡ã¸ıãˆ¢¶åì", 0x07);
        BiosPutStrMove(4, 1, 18, "  !£˝ıÌ •ì≠ åÍ‡Ò  ", 0x07);

        fDataset=fopen("D:\\CONTOR.DAT", "r");
        if(fDataset)
        {
                while(fread(sBuf, sizeof(char), CUSTOMERSIZE, fDataset)==CUSTOMERSIZE)
                        iCustomersNum++;

                cCityCode[0]=sBuf[4];
                cCityCode[1]=sBuf[5];
                cCityCode[2]='\0';

                
                myfAddressCodes=fopen("D:\\ADRSCDS.DAT", "r");
                if(myfAddressCodes)
                {
                        fread(cLastReadAddressCode, sizeof(char), 10, myfAddressCodes);
                        cLastReadAddressCode[10]='\0';
                }
                fclose(myfAddressCodes);


                strcpy(cFileDate,"00/00/00");

                iMustDoItAReader=0;//no operator pass is required
                iReadersNum=0; 
                cActiveReaderCode=1; //reading is reported as KONTORKHAN

                iCanReadManualySub=0;//FALSE;
                iCanReadManualyAdr=0;//FALSE;

                myConfigFile=fopen("D:\\CONFIG.DAT", "r");
                if(myConfigFile)
                {
                        fread(&cConfig, sizeof(char), 1, myConfigFile);//Enter using ad code
                        iSearchMethod=SCANSEARCH;
                        if(cConfig=='1')
                        {
                                iSearchMethod=ADRESCODE;
                                iCanReadManualyAdr=1;//TRUE;
                        }
                        fread(&cConfig, sizeof(char), 1, myConfigFile);//Enter using sub code
                        if(cConfig=='1')
                                iCanReadManualySub=1;//TRUE;
                        fread(&cConfig, sizeof(char), 1, myConfigFile);//Check Pass
                        if(cConfig=='1')
                                iMustDoItAReader=1;
                        fread(cFileDate, sizeof(char), 8, myConfigFile);//Check Pass
                        fread(cNum, sizeof(char), 3, myConfigFile);//Number of Operators
                        iReadersNum=atoi(cNum);

                        for(i=0; i<iReadersNum; i++)
                        {
                                fread(cNum, sizeof(char), 3, myConfigFile);
                                cNum[3]='\0';
                                cReaderCodes[i]=atoi(cNum);
                                fread(cNum, sizeof(char), 4, myConfigFile);
                                cNum[4]='\0';
                                iReaderPasses[i]=atoi(cNum);
                        }

                        
                        fread(sSettingsPassCode, sizeof(char), 4, myConfigFile);
                        sSettingsPassCode[4]='\0';
                }
                fclose(myConfigFile);



                iReadNum=0;
                iToday=0;
                SetToday();
                fOut=fopen("D:\\OUTPUT.DAT", "r+");
                fErrors=fopen("D:\\ERRORS.DAT", "a+");
        }
}

void DoErrorMenu(char* cSubscriptionCode)
{
        int iActivePage;
        int iRet;
        int iOld;
        char iSel;
        char iSelOld;
        unsigned int ch;
        char cFileSub[16];
        char index;


        unsigned char hours[2];
        unsigned char mins[2];
        unsigned char secs[2];
        char sh[3];
        char sm[3];
        char ss[3];
        int h;
        int m;
        int s;

        char sdw[10];
        char sd[3];
//        char sm[3];
        char sy[5];

        char sTemp[11];

        char cErrDate[6];
        char cErrTime[6];
        char cWhoRead;


        int iChecked[25]={0};
        iActivePage=0;
        iRet=-2;

        fseek(fErrors, 0, SEEK_SET);
        while((fread(cFileSub, sizeof(char), 15, fErrors)==15))
        {
                cFileSub[15]='\0';
                //BiosClrScr(0x07);
                //BiosPutStrMove(0, 0, strlen(cFileSub), cFileSub, 0x07);
                //getch();
                if(strcmp(cSubscriptionCode, cFileSub)==0)
                {
                        BiosClrScr(0x07);
                        DrawBox(0, 0, 6, 19,0x07);
                        BiosPutStrMove(7, 1, 18, "    £˝ı¶ì˚£˝ÒÌ    ", 0x70);
                        BiosPutStrMove(3, 1, 18, "  êñ•ˆ≠å‡¢ö•£ìÏ  ", 0x07);
                        BiosPutStrMove(4, 1, 18, "   !êñ©ã¯óÍ•˛§ï   ", 0x07);
                        BiosBeep(2000);
                        BiosGetChar();
                        return;

                }
                else
                {
                        fread(&index, sizeof(char), 1, fErrors);
                        fread(cErrDate, sizeof(char), 6, fErrors);
                        fread(cErrTime, sizeof(char), 6, fErrors);
                        fread(&cWhoRead, sizeof(char), 1, fErrors);
                }
        }
        ShowErrorPage(0, iChecked);
        iSel=0;
        ShowErrorMenuItem(iSel, 1, iChecked[iSel]);

        do
        {
                ch=BiosGetChar();
                switch(ch)                     
                {
                        case 0x1C0D://enter
                        {
                                if(iChecked[iSel]==1)
                                        iChecked[iSel]=0;
                                else
                                        iChecked[iSel]=1;
                                ShowErrorMenuItem(iSel, 1, iChecked[iSel]);
                                break;
                        }
                        case 0x4800://up
                                {
                                iOld=iSel;
                                if(iSel==0)
                                {
                                        iSel=24;
                                }
                                else
                                {
                                        iSel--;
                                }
                                if( (iOld<5) && (iSel>19) )
                                {
                                        iActivePage=4;
                                        ShowErrorPage(iActivePage, iChecked);
                                }
                                else
                                if( (iOld>19) && (iSel<20) )
                                {
                                        iActivePage=3;
                                        ShowErrorPage(iActivePage, iChecked);
                                }
                                else
                                if( (iOld>14) && (iSel<15) )
                                {
                                        iActivePage=2;
                                        ShowErrorPage(iActivePage, iChecked);
                                }
                                else
                                if( (iOld>9) && (iSel<10) )
                                {
                                        iActivePage=1;
                                        ShowErrorPage(iActivePage, iChecked);
                                }
                                else
                                if( (iOld>4) && (iSel<5) )
                                {
                                        iActivePage=0;
                                        ShowErrorPage(iActivePage, iChecked);
                                }
                                else
                                if( (iOld>19) && (iSel<5) )
                                {
                                        iActivePage=0;
                                        ShowErrorPage(iActivePage, iChecked);
                                }
                                else        
                                        ShowErrorMenuItem(iOld, 0, iChecked[iOld]);


                                ShowErrorMenuItem(iSel, 1, iChecked[iSel]);
                                break;
                                }
                        case 0x5000://down
                                {
                                iOld=iSel;
                                if(iSel==24)
                                {
                                        iSel=0;
                                }
                                else
                                {
                                        iSel++;
                                }
                                if( (iOld<5) && (iSel>4) )
                                {
                                        iActivePage=1;
                                        ShowErrorPage(iActivePage, iChecked);
                                }
                                else
                                if( (iOld<10) && (iSel>9) )
                                {
                                        iActivePage=2;
                                        ShowErrorPage(iActivePage, iChecked);
                                }
                                else
                                if( (iOld<15) && (iSel>14) )
                                {
                                        iActivePage=3;
                                        ShowErrorPage(iActivePage, iChecked);
                                }
                                else        
                                if( (iOld<20) && (iSel>19) )
                                {
                                        iActivePage=4;
                                        ShowErrorPage(iActivePage, iChecked);
                                }
                                else
                                if( (iOld>4) && (iSel<4) )
                                {
                                        iActivePage=0;
                                        ShowErrorPage(iActivePage, iChecked);
                                }
                                else        
                                        ShowErrorMenuItem(iOld, 0, iChecked[iOld]);

                                ShowErrorMenuItem(iSel, 1, iChecked[iSel]);
                                break;
                                }
                         default:
                                if(BiosGetAbortStatus())
                                        iRet=0;
                                else
                                        BiosBeep(500);
                }
        }
        while(iRet==-2);
        for(iSel=0; iSel<25; iSel++)
        {
                if(iChecked[iSel]!=0)
                {
                        iIsDataTransfered=0;//FALSE;
                        fwrite(cSubscriptionCode, sizeof(char), 15, fErrors);
                        iSelOld=iSel;
                        if(iSelOld>9) iSelOld+=4;
                        fwrite(&iSelOld, sizeof(char), 1, fErrors);
                        ShamsiDate(sdw, sd, sm, sy);

                        fwrite(&sy[2], sizeof(char), 2, fErrors);
                        fwrite(sm, sizeof(char), 2, fErrors);
                        fwrite(sd, sizeof(char), 2, fErrors);

                        BiosGetTime(hours, mins, secs);
                        h=((hours[0]>>4)*10)+(hours[0]&0x0F);
                        m=((mins[0]>>4)*10)+(mins[0]&0x0F);
                        s=((secs[0]>>4)*10)+(secs[0]&0x0F);
                        itoa(h, sh, 10);

                        if(h<10)
                        {
                                strcpy(sTemp, "0");
                                strcat(sTemp,sh);
                                strcpy(sh, sTemp);
                        }

                        itoa(m, sm, 10);
        
                        if(m<10)
                        {
                                strcpy(sTemp, "0");
                                strcat(sTemp,sm);
                                strcpy(sm, sTemp);
                        }

                        itoa(s, ss, 10);

                        if(s<10)
                        {
                                strcpy(sTemp, "0");
                                strcat(sTemp,ss);
                                strcpy(ss, sTemp);
                        }

                        fwrite(sh, sizeof(char), 2, fErrors);
                        fwrite(sm, sizeof(char), 2, fErrors);
                        fwrite(ss, sizeof(char), 2, fErrors);

                        fwrite(&cActiveReaderCode, sizeof(char), 1, fErrors);

//                      fflush(fErrors);
                        fclose(fErrors);
                        fErrors=fopen("D:\\ERRORS.DAT", "a+");


                }
        }
}

void ShowErrorMenuItem(int ItemNum, int HighLight, int Checked)
{
        int iMod;
        int MenuPos[]={1, 2, 3, 4, 5, 1, 2, 3, 4, 5, 1, 2, 3, 4, 5, 1, 2, 3, 4, 5, 1, 2, 3, 4, 5};
        char sItems[][18]={"£Ì•åìåìêí©ù•ì£ˆìı", "£Ì•åì¸ìêí©ù•ì£ˆìı", "    êÌ•ó´Û•ˆØüÚ£Â", "            ¯˝Ò¢ó", "          êñ‰ıåÛÛ", "           ˜£˝´ˆï", "          ¯ó´ã£•ì", "              •£Ì", "         ¶åÓêñÏ•©", "          ¶åÓêñ´ı", "           ¶åÓ„‡Ï", "          ¸˛åõìåõ", "           ˚¶å©ˆı", "      ¸´Ì¯ÒˆÒêÍ¢", "      †®ÌÂ•ì•ˆóıÌ", "     •Ó´•åÛ´¸ìã•¢", "      êîÛÒïÙó´ã£ı", "      •ˆóıÌ¸Óó©Ì´", "       ¯´˝´¸Óó©Ì´", "      •ˆóıÌ†Æ˛ˆ‰ó", "      êí©åıÛåıÔüÛ", "      åıìêñã•˝Û‰ó", "     •ˆóıÌ˚•åÌó©£", "      êÌˆÌ´Û•ˆóıÌ", "      ˜£¶•ˆ£•ˆóıÌ", "                 ", "                 ", "                 ", "                 "};
        char sChecked[2];
        sChecked[0]=0x24;
        sChecked[1]='\0';
        iMod=0x07;
        if(HighLight)  iMod=0x70;
        if(Checked)
                sChecked[0]=0x23;
        BiosPutStrMove(MenuPos[ItemNum], 2, 17, sItems[ItemNum], iMod);
        BiosPutStrMove(MenuPos[ItemNum], 1, 1, sChecked, iMod);
}

void ShowErrorPage(int iErrorPage, int* iChecked)
{
        int i;
        int j;
        char sItems[][18]={"£Ì•åìåìêí©ù•ì£ˆìı", "£Ì•åì¸ìêí©ù•ì£ˆìı", "    êÌ•ó´Û•ˆØüÚ£Â", "            ¯˝Ò¢ó", "          êñ‰ıåÛÛ", "           ˜£˝´ˆï", "          ¯ó´ã£•ì", "              •£Ì", "         ¶åÓêñÏ•©", "          ¶åÓêñ´ı", "           ¶åÓ„‡Ï", "          ¸˛åõìåõ", "           ˚¶å©ˆı", "      ¸´Ì¯ÒˆÒêÍ¢", "      †®ÌÂ•ì•ˆóıÌ", "     •Ó´•åÛ´¸ìã•¢", "      êîÛÒïÙó´ã£ı", "      •ˆóıÌ¸Óó©Ì´", "       ¯´˝´¸Óó©Ì´", "      •ˆóıÌ†Æ˛ˆ‰ó", "      êí©åıÛåıÔüÛ", "      åıìêñã•˝Û‰ó", "     •ˆóıÌ˚•åÌó©£", "      êÌˆÌ´Û•ˆóıÌ", "      ˜£¶•ˆ£•ˆóıÌ", "                 ", "                 ", "                 ", "                 "};
        BiosClrScr(0x07);
        DrawBox(0, 0, 6, 19,0x07);
        j=5;
//        if(iErrorPage==4)
//                j=4;
        for(i=0+iErrorPage*5; i<j+iErrorPage*5; i++)
        {
                ShowErrorMenuItem(i, 0, iChecked[i]);
        }
        BiosPutStrMove(7, 1, 18, "  CLEAR= êñ´Ó¶åì  ", 0x70);
}

void SetToday()
{
        char sdw[10];
        char sd[3];
        char sm[3];
        char sy[5];
        char sc[3];
        char sdate[200];
        int y;
        int m;
        int d;
        int wrow;

        ShamsiDate(sdw, sd, sm, sy);      
       

        strcpy(sdate, sy);
        strcat(sdate, "/");
        strcat(sdate, sm);
        strcat(sdate, "/");
        strcat(sdate, sd);
        strcpy(sToday, sdate);
}

void CheckToday()
{
        char sdw[10];
        char sd[3];
        char sm[3];
        char sy[5];
        char sc[3];
        char sdate[200];
        int y;
        int m;
        int d;
        int wrow;

        ShamsiDate(sdw, sd, sm, sy);      
       

        strcpy(sdate, sy);
        strcat(sdate, "/");
        strcat(sdate, sm);
        strcat(sdate, "/");
        strcat(sdate, sd);
        if(strcmp(sdate, sToday)==0)
        {
                iToday++;
                return;
        }
        strcpy(sToday, sdate);
        iToday=1;
}

void GetToday(char* cdate)
{
        char sdw[10];
        char sd[3];
        char sm[3];
        char sy[5];
        char sc[3];
        int y;
        int m;
        int d;
        int wrow;

        ShamsiDate(sdw, sd, sm, sy);      
       

        strcpy(cdate, sy);
        strcat(cdate, "/");
        strcat(cdate, sm);
        strcat(cdate, "/");
        strcat(cdate, sd);
        cdate[1]='[';
        strcat(cdate, "]");
}

void GetPassCode(char *sInput)
{
        int iCurrentInput;
        unsigned int ch;
        unsigned char row;
        unsigned char col;
        int iEntered=0;
        iCurrentInput=0;
        BiosClrScr(0x07);
        DrawBox(0, 0, 6, 19,0x07);
        BiosPutStrMove(0, 3, 14, ":: £ˆ•ˆ ¶Û• ::", 0x70);
        BiosPutStrMove(3, 2, 16, "      ----      ", 0x07);
        BiosSetCursorPos(3, 8);
        BiosShowCursor();
        do
        {
                ch=BiosGetChar();
                switch(ch)
                {
                        case 0x1C0D://enter
                                if(iCurrentInput==4)
                                {
                                        sInput[4]='\0';
                                        iEntered=1;
                                }
                                break;
                         default:
                                switch((char)ch)
                                {
                                 case '0':
                                 case '1':
                                 case '2':
                                 case '3':
                                 case '4':
                                 case '5':
                                 case '6':
                                 case '7':
                                 case '8':
                                 case '9':
                                 {
                                        if(iCurrentInput<4)
                                        {
                                                sInput[iCurrentInput]=(char)ch;
                                                ch='*';
                                                putch(ch);                                        
                                                iCurrentInput++;

                                        }
                                        else
                                                BiosBeep(500);
                                 break;
                                 }
                                 default:
                                        BiosBeep(500);
                                }
                }
        }
        while(iEntered!=1);
        BiosHideCursor();
}


int DoFindRecordBinary(struct customer* pCustomer)
{

        char cSearchCode[16];
        char cName[23];
        char b110[110];
        int iEnd, i;
        int iFound;
        int iRet;
        int iScan;
        long iWhere;
        int iStartIndex;
        int iEndIndex;
        int iOldStart;
        int iOldEnd;
        int iResult;
        char sSerial[11];
        long lIndex;

        FILE* fSerials;


        iRet=-1;
        if(!fDataset)
                return iRet;
/*        iRet=1;
        if(iCanReadManualyAdr)
              iRet=DoLastReadMenu(pCustomer);
        if(iRet==5)
                return iRet;
        if(iRet==6)
                return 3;
        iRet=1;
        iScan=ScanSub(cSearchCode);*/
        iRet=1;
        iScan=0;
        switch(iSearchMethod)
        {
                case SCANSEARCH:
                        iScan=ScanSub(cSearchCode);
                        break;
                case SUBSCRSEARCH:
                        iRet=GetSubCode(cSearchCode);
                        if(iRet!=2)
                                iScan=2;
                        break;
                case SERIALSEARCH:
                        iRet=GetCounterSerial(cSearchCode);
                        if(iRet==1)
                                iScan=2;
                        break;
                case ADRESCODE:
                        iRet=DoLastReadMenu(pCustomer);
                        if(iRet!=5)
                                iScan=2;
                        else
                                return 5;
                        break;
                case ADRSEARCH:
                        break;
                case NOTREAD:
                        break;
        }
        if(iScan==2)
        {
                do
                {
                        iScan=DoSearchMethodMenu();
                        if(iScan==4)
                                ShowHelp();
                }
                while(iScan==4);

                if(iScan==5)
                        return 3;
                else
                if(iScan!=-2)
                {
                        iSearchMethod=iScan;
                        return 9;
                }
        }
        else
        if(iScan==3)
        {
                return 3;
        }
        /*
        if(iScan==2)
        {
                iRet=-1;
                do
                {
                    iRet=GetSubCode(cSearchCode);
                    if( (iRet!=2) )
                      iRet=GetCounterSerial(cSearchCode);
                      if( (iRet!=4) && (iRet!=2) )
                        iRet=DoLastReadMenu(pCustomer);
                }
                while( ! ( (iRet==2) || (iRet==4) || (iRet==5) ) );
                if(iRet==5)
                        return iRet;

        }
        else
        if(iScan==3)
        {
                return 3;
        }
        */

        if(iRet==4)
        {
        //Find Counter Serial:
               if(strlen(cSearchCode)==0)
                return -1;
               if(atoi(cSearchCode)==0)
                return -1;
                
               fSerials=fopen("D:\\SERIALS.DAT", "r");
               if(!fSerials)
                return -1;

               iStartIndex=0;
               iEndIndex=iCustomersNum-1;

               iOldStart=iStartIndex;
               iOldEnd=iEndIndex;

               iWhere=iEndIndex;

               fseek(fSerials, iWhere*SERIALSIZE, SEEK_SET);
               fread(sSerial, sizeof(char), 10, fSerials);
               sSerial[10]='\0';

               iResult=50;
               iResult=strcmp(sSerial, cSearchCode);

               if(iResult!=0)
                     iWhere=iCustomersNum/2;
               do
                {

                        if(iResult!=0)
                        {
                        fseek(fSerials, iWhere*SERIALSIZE, SEEK_SET);
                        fread(sSerial, sizeof(char), 10, fSerials);
                        sSerial[10]='\0';

                        iResult=strcmp(sSerial, cSearchCode);
                        }
                        if(iResult==0)
                        {//iResult
                        BiosClrScr(0x07);
                        DrawBox(0, 0, 6, 19,0x07);
                        fread(&lIndex, sizeof(long), 1, fSerials);
                        fseek(fDataset, lIndex*CUSTOMERSIZE, SEEK_SET);
                        fread(pCustomer->cSubscriptionCode, sizeof(char), 15, fDataset);
                        pCustomer->cSubscriptionCode[15]='\0';
                        BiosPutStrMove(0, 2, 15, pCustomer->cSubscriptionCode, 0x70);
                        fread(pCustomer->cName, sizeof(char), 18, fDataset);
                        pCustomer->cName[18]='\0';
                        strcpy(cName, pCustomer->cName);
                        i=0;
                        while((cName[i]==' ')&&(i<18))
                                {i++;}
                        BiosPutStrMove(1, 10-(18-i)/2, 18-i, &cName[i], 0x07);
                        fread(pCustomer->cFamily, sizeof(char), 22, fDataset);
                        pCustomer->cFamily[22]='\0';
                        strcpy(cName, pCustomer->cFamily); 
                        i=0;
                        while((cName[i]==' ')&&(i<22))
                                {i++;}
                        BiosPutStrMove(2, 10-(22-i)/2, 22-i, &cName[i], 0x07);
                        BiosPutStrMove(5, 11, 8, ":†®•£â£Ì", 0x07);
                        fread(pCustomer->cAddressCode, sizeof(char), 10, fDataset);
                        BiosPutStrMove(5, 1, 10, pCustomer->cAddressCode, 0x07);
                        BiosPutStrMove(4, 11, 8, ":ÔåÉÉ˛•©", 0x07);
                        fread(pCustomer->cCounterSerial, sizeof(char), 10, fDataset);
        
                        fread(&pCustomer->lConsumptionKind, sizeof(long), 1, fDataset);
                        fread(&pCustomer->lCapacity, sizeof(long), 1, fDataset);
                        fread(pCustomer->cPrevDate, sizeof(char), 6, fDataset);
                        fread(pCustomer->cPrevCounter, sizeof(char), 10, fDataset);    


                        BiosPutStrMove(4, 1, 10, pCustomer->cCounterSerial, 0x07);
                        BiosPutStrMove(7, 1, 18,"      £˝˛åÉó      ", 0x70);
                        fclose(fSerials);

                        iFound=0;
                        strcpy(pCustomer->cCurCounter, "");
                        fseek(fOut, 0, SEEK_SET);
                        while((fread(cName, sizeof(char), 15, fOut)==15)&&!iFound)
                        {
                        cName[15]='\0';
                        if(strcmp(pCustomer->cSubscriptionCode, cName)==0)
                        {
                                iFound=1;
                                fread(pCustomer->cCurCounter, sizeof(char), 10, fOut);
                                fread(pCustomer->cCurDate, sizeof(char), 6, fOut);
                                fread(pCustomer->cCurTime, sizeof(char), 6, fOut);
                        }
                        else
                                fread(b110, sizeof(char), 23, fOut);
                        }

                        getch();   
                        return iRet;
                        }
                        else
                        if(iResult<0)
                        {
                                iOldStart=iStartIndex;
                                iOldEnd=iEndIndex;
                                iStartIndex=iWhere;
                                iWhere=(iEndIndex-iStartIndex)/2+iStartIndex;
                        }
                        else
                        {
                                iOldStart=iStartIndex;
                                iOldEnd=iEndIndex;
                                iEndIndex=iWhere;
                                iWhere=(iEndIndex-iStartIndex)/2+iStartIndex;
                        }                

                }
                while( (iStartIndex<iEndIndex) && ( !((iOldStart==iStartIndex) && (iOldEnd==iEndIndex)) )  );
               fclose(fSerials);
               return -1;
        }

        if( !( (cSearchCode[0]=='1') && (cSearchCode[1]=='2') && (cSearchCode[2]=='1') && (cSearchCode[3]=='2') ) )
                return -1;


        iStartIndex=0;
        iEndIndex=iCustomersNum-1;

        iOldStart=iStartIndex;
        iOldEnd=iEndIndex;


        iWhere=iEndIndex;
        fseek(fDataset, iWhere*CUSTOMERSIZE, SEEK_SET);
        fread(pCustomer->cSubscriptionCode, sizeof(char), 15, fDataset);
        pCustomer->cSubscriptionCode[15]='\0';
        iResult=strcmp(pCustomer->cSubscriptionCode, cSearchCode);
        if(iResult!=0)
                iWhere=iCustomersNum/2;
        do
        {
                if(iResult!=0)
                {
                fseek(fDataset, iWhere*CUSTOMERSIZE, SEEK_SET);
//                BiosPutStrMove(0, 0, 1, " ", 0x70);
                fread(pCustomer->cSubscriptionCode, sizeof(char), 15, fDataset);
                pCustomer->cSubscriptionCode[15]='\0';
//                BiosClrScr(0x07);
//                printf("s=%d\n e=%d\n w=%d\n%s",iStartIndex, iEndIndex, iWhere,pCustomer->cSubscriptionCode);
//                getch();

                iResult=strcmp(pCustomer->cSubscriptionCode, cSearchCode);
                }
                if(iResult==0)
                {//iResult
                BiosClrScr(0x07);
                DrawBox(0, 0, 6, 19,0x07);
                BiosPutStrMove(0, 2, 15, pCustomer->cSubscriptionCode, 0x70);
                fread(pCustomer->cName, sizeof(char), 18, fDataset);
                pCustomer->cName[18]='\0';
                strcpy(cName, pCustomer->cName);
                i=0;
                while((cName[i]==' ')&&(i<18))
                        {i++;}
                BiosPutStrMove(1, 10-(18-i)/2, 18-i, &cName[i], 0x07);
                fread(pCustomer->cFamily, sizeof(char), 22, fDataset);
                pCustomer->cFamily[22]='\0';
                strcpy(cName, pCustomer->cFamily); 
                i=0;
                while((cName[i]==' ')&&(i<22))
                        {i++;}
                BiosPutStrMove(2, 10-(22-i)/2, 22-i, &cName[i], 0x07);
                if(iScan!=2)
                        BiosPutStrMove(5, 11, 8, ":†®•£â£Ì", 0x07);
                else
                        BiosPutStrMove(4, 11, 8, ":†®•£â£Ì", 0x07);
                fread(pCustomer->cAddressCode, sizeof(char), 10, fDataset);
                if(iScan!=2)
                        BiosPutStrMove(5, 1, 10, pCustomer->cAddressCode, 0x07);
                else
                        BiosPutStrMove(4, 1, 10, pCustomer->cAddressCode, 0x07);
                if(iScan!=2)
                        BiosPutStrMove(4, 11, 8, ":ÔåÉÉ˛•©", 0x07);
                fread(pCustomer->cCounterSerial, sizeof(char), 10, fDataset);

                fread(&pCustomer->lConsumptionKind, sizeof(long), 1, fDataset);
                fread(&pCustomer->lCapacity, sizeof(long), 1, fDataset);
                fread(pCustomer->cPrevDate, sizeof(char), 6, fDataset);
                fread(pCustomer->cPrevCounter, sizeof(char), 10, fDataset);


                iFound=0;
                strcpy(pCustomer->cCurCounter, "");
                fseek(fOut, 0, SEEK_SET);
                while((fread(cName, sizeof(char), 15, fOut)==15)&&!iFound)
                {
                cName[15]='\0';
                if(strcmp(pCustomer->cSubscriptionCode, cName)==0)
                {
                        iFound=1;
                        fread(pCustomer->cCurCounter, sizeof(char), 10, fOut);
                        fread(pCustomer->cCurDate, sizeof(char), 6, fOut);
                        fread(pCustomer->cCurTime, sizeof(char), 6, fOut);
                }
                else
                        fread(b110, sizeof(char), 23, fOut);
                }

                if(iScan!=2)
                        BiosPutStrMove(4, 1, 10, pCustomer->cCounterSerial, 0x07);
                BiosPutStrMove(7, 1, 18,"      £˝˛åÉó      ", 0x70);
                getch();   
                return iRet;
                }
                else
                if(iResult<0)
                {
                        iOldStart=iStartIndex;
                        iOldEnd=iEndIndex;
                        iStartIndex=iWhere;
                        iWhere=(iEndIndex-iStartIndex)/2+iStartIndex;
                }
                else
                {
                        iOldStart=iStartIndex;
                        iOldEnd=iEndIndex;
                        iEndIndex=iWhere;
                        iWhere=(iEndIndex-iStartIndex)/2+iStartIndex;
                }                

        }
        while( (iStartIndex<iEndIndex) && ( !((iOldStart==iStartIndex) && (iOldEnd==iEndIndex)) )  );
        return -1;
}

void ShowVersion()
{
          BiosClrScr(0x07);
          DrawBox(0, 0, 6, 19,0x07);
          BiosPutStrMove(7, 1, 18, "   !£˝ı¶ì ˚£˝ÒÌ   ", 0x70);
          BiosPutStrMove(4, 1, 18, "       1.1.1      ", 0x07);
          BiosPutStrMove(3, 1, 18, "  :¯Ûåı•ì†™˛ã•˛ˆ  ", 0x07);
          BiosBeepFreq(1000);
          BiosBeep(500);
          BiosBeepOff();
          BiosGetChar();
}

void DoDeleteRead()
{
        unsigned int chEnter;
        char sPass[5];
        if(DoCradleMenu())
        {
          BiosClrScr(0x07);
          DrawBox(0, 0, 6, 19,0x07);
          BiosPutStrMove(7, 1, 18, "    ENTER=˜Ûã£ã   ", 0x70);
          BiosPutStrMove(1, 1, 18, "  êÍ§üêòÂåìÔÛÂÙ˛ã ", 0x07);
          BiosPutStrMove(2, 1, 18, "  å˙å‡¢ˆ å˙˜£ıãˆ¢ ", 0x07);
          BiosPutStrMove(3, 1, 18, "   ÙëÛ‡Û•Óã£ˆ´¸Û  ", 0x07);
          BiosPutStrMove(4, 1, 18, " ¶ã•˝È˚£˝ÒÌ£˝ó©˝ı ", 0x07);
          BiosPutStrMove(5, 1, 18, "  £˝ı¶ìã• ENTER   ", 0x07);
          BiosBeep(500);
          chEnter=BiosGetChar();
          if(chEnter==0x1C0D)
          {

          GetPassCode(sPass);
          if(strcmp(sPass, "5831")!=0)
          {
                  BiosClrScr(0x07);
                  DrawBox(0, 0, 6, 19,0x07);
                  BiosPutStrMove(7, 1, 18, "    £˝ı¶ì˚£˝ÒÌ    ", 0x70);
                  BiosPutStrMove(3, 1, 18, "  !êñ©ã˜åìó´ã¶Û•  ", 0x07);
                  BiosBeep(1500);
                  BiosGetChar();
          }
          else
          {

             if(fOut)
                  fclose(fOut);
             if(fErrors)
                  fclose(fErrors);
             fOut=fopen("D:\\OUTPUT.DAT","w");
             fclose(fOut);
             fErrors=fopen("D:\\ERRORS.DAT", "w");
             fclose(fErrors);
             iIsDataTransfered=1; //TRUE;
             fOut=fopen("D:\\OUTPUT.DAT", "r+");
             fErrors=fopen("D:\\ERRORS.DAT", "a+");
             BiosClrScr(0x07);
             DrawBox(0, 0, 6, 19,0x07);
             BiosPutStrMove(7, 1, 18, "   !£˝ı¶ì ˚£˝ÒÌ   ", 0x70);
             BiosPutStrMove(3, 1, 18, " å˙å‡¢ ˆ å˙˜£ıãˆ¢ ", 0x07);
             BiosPutStrMove(4, 1, 18, "      £´ êÌåï     ", 0x07);
             BiosBeepFreq(1000);
             BiosBeep(500);
             BiosBeepOff();
             BiosGetChar();

             }

          }

        }
}


int GetCounterSerial(char *sInput)
{
        int iCurrentInput;
        unsigned int ch;
        unsigned char row;
        unsigned char col;

        int iEntered=0;
        iCurrentInput=0;
        BiosClrScr(0x07);
        DrawBox(0, 0, 6, 19,0x07);
        BiosPutStrMove(0, 5, 10, "::ˆÉõó©õ::", 0x07);
//        BiosPutStrMove(7, 1, 18, "  SPACE=˚£‰ì†™ˆ•  ", 0x70);
        BiosPutStrMove(7, 1, 18, " SPACE=ˆÉõó©õ†™ˆ• ", 0x70);
        BiosPutStrMove(2, 4, 12, ":•ˆóıÌ Ôå˛•©", 0x07);
        BiosPutStrMove(3, 5, 10, "----------", 0x07);
        BiosSetCursorPos(3, 5);
        BiosShowCursor();
        do
        {
                ch=BiosGetChar();
                switch(ch)
                {
                        case 0x1C0D://enter
                                while(iCurrentInput<10)
                                        sInput[iCurrentInput++]=' ';
                                sInput[10]='\0';
                                iEntered=1;
                                break;
                         default:
                                switch((char)ch)
                                {
                                 case ' ':
                                        return 1;
                                 break;
                                 case '0':
                                 case '1':
                                 case '2':
                                 case '3':
                                 case '4':
                                 case '5':
                                 case '6':
                                 case '7':
                                 case '8':
                                 case '9':
                                 {
                                        if(iCurrentInput<10)
                                        {
                                                putch(ch);                                        
                                                sInput[iCurrentInput]=(char)ch;
                                                iCurrentInput++;

                                        }
                                        else
                                                BiosBeep(500);
                                 break;
                                 }
                                 default:
                                        BiosBeep(500);
                                }
                }
        }
        while(iEntered!=1);
        BiosHideCursor();
        return 4;
}

void ShowFreeSpace()
{
        unsigned long bytesfree;
        struct diskfree_t diskinfo;

        _dos_getdiskfree(0, &diskinfo);
        bytesfree = (unsigned long)diskinfo.avail_clusters;
        bytesfree = bytesfree * (unsigned long)diskinfo.sectors_per_cluster;
        bytesfree = bytesfree * (unsigned long)diskinfo.bytes_per_sector;
        BiosClrScr(0x07);
        DrawBox(0, 0, 6, 19,0x07);
        BiosPutStrMove(0, 3, 14, "::£ã¶â ˚åÉØÍ::", 0x07);
        BiosPutStrMove(7, 1, 18, "   !£˝ı¶ì ˚£˝ÒÌ   ", 0x70);
        BiosSetCursorPos(3, 4);
        printf("êñ˛åì%lu", bytesfree);
        BiosGetChar();
}

void ShowFileDate()
{
        BiosClrScr(0x07);
        DrawBox(0, 0, 6, 19,0x07);
        BiosPutStrMove(0, 3, 14, ":: êñåÉÂ‡ã ::", 0x07);
        BiosPutStrMove(7, 1, 18, "   !£˝ı¶ì ˚£˝ÒÌ   ", 0x70);
        BiosSetCursorPos(3, 1);
        printf("%s:Ô˛åÍ°˛•åó", cFileDate);
        BiosGetChar();
}

int CheckFreeSpace()
{
        unsigned long bytesfree;
        struct diskfree_t diskinfo;

        _dos_getdiskfree(0, &diskinfo);
        bytesfree = (unsigned long)diskinfo.avail_clusters;
        bytesfree = bytesfree * (unsigned long)diskinfo.sectors_per_cluster;
        bytesfree = bytesfree * (unsigned long)diskinfo.bytes_per_sector;
        if(bytesfree <20000)
        {
                BiosClrScr(0x07);
                DrawBox(0, 0, 6, 19,0x07);
                BiosPutStrMove(0, 5, 10, "::•åÉ‡¢ã::", 0x07);
                BiosPutStrMove(2, 1, 18, "  êñìô˚ã•ì¸˛åØÍ   ", 0x07);
                BiosPutStrMove(3, 1, 18, " !êñ©ã ˜£ıåÛı ¸Ïåì", 0x07);
                BiosPutStrMove(4, 1, 18, "£ˆ´¯˝Ò¢ó˜åÓó©££˛åì", 0x07);
                BiosPutStrMove(7, 1, 18, "   !£˝ı¶ì ˚£˝ÒÌ   ", 0x70);
                BiosBeepFreq(2000);
                BiosBeep(1000);
                BiosGetChar();
                BiosBeepOff();
                return 0;
        }
        return 1;
}


int DoLastReadMenu(struct customer* pCustomer)
{
        FILE* fAddressCodes;

        char cName[23];
        long iWhere;
        long iOldWhere;
        int iStartIndex;
        int iEndIndex;
        int iOldStart;
        int iOldEnd;
        int iResult;
        char sAddressCode[11];
        long lIndex;
        int i,j,k;
        int iFound;
        char sKeys[50];
        char sSign[10];
        long lLastRecord;
        char sSearching[11];
        

        unsigned int ch;

        char cAddress[46];
        char cAddressPart[19];

        char cPreviousDate[9];

        char cMySubscriptionCode[16];

        int iFoundUnread;

        char cMyRegion[6];


        iFound=0;
        cMyRegion[5]='\0';
        BiosHideCursor();
        if(strcmp(cLastReadAddressCode, "aaaaaaaaaa")==0)
        {
                BiosClrScr(0x07);
                DrawBox(0, 0, 6, 19,0x07);
                BiosPutStrMove(0, 5, 10, "::•˛© ‡¢::", 0x07);
//                BiosPutStrMove(7, 1, 18, "  SPACE=˚£‰ì†™ˆ•  ", 0x70);
                BiosPutStrMove(7, 1, 18, " SPACE=ˆÉõó©õ†™ˆ• ", 0x70);
                BiosPutStrMove(3, 2, 16, "˚•ã£ÏÛ úÉ˝˙ ¶ˆı˙", 0x07);
                BiosPutStrMove(4, 2, 16, "êñ©ã ˜£´ı ˜£ıãˆ¢", 0x07);
                BiosBeep(1000);
                BiosGetChar();
                return -1;
        }

        fAddressCodes=fopen("D:\\ADRSCDS.DAT", "r");
        if(!fAddressCodes)
                return -1;


               iStartIndex=0;
               iEndIndex=iCustomersNum-1;

               iOldStart=iStartIndex;
               iOldEnd=iEndIndex;

               iWhere=iEndIndex;

               fseek(fAddressCodes, iWhere*ADDRESCODESIZE, SEEK_SET);
               fread(sAddressCode, sizeof(char), 10, fAddressCodes);
               sAddressCode[10]='\0';

               iResult=50;
               iResult=strcmp(sAddressCode, cLastReadAddressCode);

               if(iResult!=0)
                     iWhere=iCustomersNum/2;
               do
                {

                        if(iResult!=0)
                        {
                        fseek(fAddressCodes, iWhere*ADDRESCODESIZE, SEEK_SET);
                        fread(sAddressCode, sizeof(char), 10, fAddressCodes);
                        sAddressCode[10]='\0';

                        iResult=strcmp(sAddressCode, cLastReadAddressCode);
                        }
                        if(iResult==0)
                        {//iResult
                        BiosClrScr(0x07);
                        DrawBox(0, 0, 6, 19,0x07);
                        BiosPutStrMove(0, 5, 10, "::•˛© ‡¢::", 0x07);
                        fread(&lIndex, sizeof(long), 1, fAddressCodes);
                        fseek(fDataset, lIndex*CUSTOMERSIZE, SEEK_SET);
                        fread(pCustomer->cSubscriptionCode, sizeof(char), 15, fDataset);
                        pCustomer->cSubscriptionCode[15]='\0';
                        //if(strcmp(sAddressCode, cLastReadAddressCode))
                                BiosPutStrMove(1, 2, 15, pCustomer->cSubscriptionCode, 0x07);
                        //else
                        //        BiosPutStrMove(1, 4, 12, "˜£ıãˆ¢ Ù˛•¢â", 0x70);
                        fread(pCustomer->cName, sizeof(char), 18, fDataset);
                        pCustomer->cName[18]='\0';
                        strcpy(cName, pCustomer->cName);
                        i=0;
                        while((cName[i]==' ')&&(i<18))
                                {i++;}
                        BiosPutStrMove(2, 10-(18-i)/2, 18-i, &cName[i], 0x07);
                        fread(pCustomer->cFamily, sizeof(char), 22, fDataset);
                        pCustomer->cFamily[22]='\0';
                        strcpy(cName, pCustomer->cFamily); 
                        i=0;
                        while((cName[i]==' ')&&(i<22))
                                {i++;}
                        BiosPutStrMove(3, 10-(22-i)/2, 22-i, &cName[i], 0x07);
                        BiosPutStrMove(4, 11, 8, ":†®•£â£Ì", 0x07);
                        fread(pCustomer->cAddressCode, sizeof(char), 10, fDataset);
                        BiosPutStrMove(4, 1, 10, pCustomer->cAddressCode, 0x07);
                        fread(pCustomer->cCounterSerial, sizeof(char), 10, fDataset);
        
                        fread(&pCustomer->lConsumptionKind, sizeof(long), 1, fDataset);
                        fread(&pCustomer->lCapacity, sizeof(long), 1, fDataset);
                        fread(pCustomer->cPrevDate, sizeof(char), 6, fDataset);
                        fread(pCustomer->cPrevCounter, sizeof(char), 10, fDataset);
                        if(iCanReadManualyAdr)
                        {
                                BiosPutStrMove(5, 11, 8, ":ÔåÉÉ˛•©", 0x07);
                                BiosPutStrMove(5, 1, 10, pCustomer->cCounterSerial, 0x07);
                        }
                        else
                        {
                        sSign[1]='\0';
                        sSign[0]=0x1F;
                        strcpy(sKeys, sSign);
                        strcat(sKeys, ":˚£‰ì");
                        strcat(sKeys,"  ");
                        sSign[0]=0x1E;
                        strcat(sKeys, sSign);
                        strcat(sKeys, ":¸ÒìÏ");
                        BiosPutStrMove(5, 3, 14, sKeys, 0x07);
                        }

                        BiosPutStrMove(7, 1, 18, " SPACE=ˆÉõó©õ†™ˆ• ", 0x70);

                        break;
                        }
                        else
                        if(iResult<0)
                        {
                                iOldStart=iStartIndex;
                                iOldEnd=iEndIndex;
                                iStartIndex=iWhere;
                                iWhere=(iEndIndex-iStartIndex)/2+iStartIndex;
                        }
                        else
                        {
                                iOldStart=iStartIndex;
                                iOldEnd=iEndIndex;
                                iEndIndex=iWhere;
                                iWhere=(iEndIndex-iStartIndex)/2+iStartIndex;
                        }                

                }
                while( (iStartIndex<iEndIndex) && ( !((iOldStart==iStartIndex) && (iOldEnd==iEndIndex)) )  );

                do
                {
                        ch=BiosGetChar();
                        switch(ch)
                        {
                                case 0x1C0D://enter
                                        iFound=1;
                                        break;
                                case 0x5000://down


                                        if(iWhere<(iCustomersNum-1))
                                          iWhere=iWhere+1;
                                        else
                                           BiosBeep(500);
                                        fseek(fAddressCodes, iWhere*ADDRESCODESIZE, SEEK_SET);
                                        fread(sAddressCode, sizeof(char), 10, fAddressCodes);
                                        sAddressCode[10]='\0';

                                        BiosClrScr(0x07);
                                        DrawBox(0, 0, 6, 19,0x07);
                                        BiosPutStrMove(0, 5, 10, "::•˛© ‡¢::", 0x07);
                                        fread(&lIndex, sizeof(long), 1, fAddressCodes);

                                        if(!(lIndex<iCustomersNum)) lIndex=0;

                                        fseek(fDataset, lIndex*CUSTOMERSIZE, SEEK_SET);
                                        fread(pCustomer->cSubscriptionCode, sizeof(char), 15, fDataset);
                                        pCustomer->cSubscriptionCode[15]='\0';
                                        //if(strcmp(sAddressCode, cLastReadAddressCode)!=0)
                                        {
                                        //if(iWhere==(iCustomersNum-1))
                                        //        BiosPutStrMove(1, 5, 10, "•˝© ‡¢ •¢â", 0x70);
                                        //else
                                        //if(iWhere==0)
                                        //        BiosPutStrMove(1, 5, 10, "•˝© ‡¢ Ôˆã", 0x70);
                                        //else
                                                BiosPutStrMove(1, 2, 15, pCustomer->cSubscriptionCode, 0x07);
                                        }
                                        //else
                                        //        BiosPutStrMove(1, 4, 12, "˜£ıãˆ¢ Ù˛•¢â", 0x70);
                                        fread(pCustomer->cName, sizeof(char), 18, fDataset);
                                        pCustomer->cName[18]='\0';
                                        strcpy(cName, pCustomer->cName);
                                        i=0;
                                        while((cName[i]==' ')&&(i<18))
                                                {i++;}
                                        BiosPutStrMove(2, 10-(18-i)/2, 18-i, &cName[i], 0x07);
                                        fread(pCustomer->cFamily, sizeof(char), 22, fDataset);
                                        pCustomer->cFamily[22]='\0';
                                        strcpy(cName, pCustomer->cFamily); 
                                        i=0;
                                        while((cName[i]==' ')&&(i<22))
                                                {i++;}
                                        BiosPutStrMove(3, 10-(22-i)/2, 22-i, &cName[i], 0x07);
                                        BiosPutStrMove(4, 11, 8, ":†®•£â£Ì", 0x07);
                                        fread(pCustomer->cAddressCode, sizeof(char), 10, fDataset);
                                        BiosPutStrMove(4, 1, 10, pCustomer->cAddressCode, 0x07);
                                        fread(pCustomer->cCounterSerial, sizeof(char), 10, fDataset);
        
                                        fread(&pCustomer->lConsumptionKind, sizeof(long), 1, fDataset);
                                        fread(&pCustomer->lCapacity, sizeof(long), 1, fDataset);
                                        fread(pCustomer->cPrevDate, sizeof(char), 6, fDataset);
                                        fread(pCustomer->cPrevCounter, sizeof(char), 10, fDataset);    
                        if(iCanReadManualyAdr)
                        {
                                BiosPutStrMove(5, 11, 8, ":ÔåÉÉ˛•©", 0x07);
                                BiosPutStrMove(5, 1, 10, pCustomer->cCounterSerial, 0x07);
                        }
                        else
                        {
                        sSign[1]='\0';
                        sSign[0]=0x1F;
                        strcpy(sKeys, sSign);
                        strcat(sKeys, ":˚£‰ì");
                        strcat(sKeys,"  ");
                        sSign[0]=0x1E;
                        strcat(sKeys, sSign);
                        strcat(sKeys, ":¸ÒìÏ");
                        BiosPutStrMove(5, 3, 14, sKeys, 0x07);
                        }
//                                        BiosPutStrMove(7, 1, 18, "  SPACE=˚£‰ì†™ˆ•  ", 0x70);
                                        BiosPutStrMove(7, 1, 18, " SPACE=ˆÉõó©õ†™ˆ• ", 0x70);

                                        break;

                                case 0x4800://up


                                        if(iWhere>0)
                                          iWhere=iWhere-1;
                                        else
                                           BiosBeep(500);

                                        fseek(fAddressCodes, iWhere*ADDRESCODESIZE, SEEK_SET);
                                        fread(sAddressCode, sizeof(char), 10, fAddressCodes);
                                        sAddressCode[10]='\0';

                                        BiosClrScr(0x07);
                                        DrawBox(0, 0, 6, 19,0x07);
                                        BiosPutStrMove(0, 5, 10, "::•˛© ‡¢::", 0x07);

                                        if(!(lIndex<iCustomersNum)) lIndex=0;

                                        fread(&lIndex, sizeof(long), 1, fAddressCodes);

                                        fseek(fDataset, lIndex*CUSTOMERSIZE, SEEK_SET);
                                        fread(pCustomer->cSubscriptionCode, sizeof(char), 15, fDataset);
                                        pCustomer->cSubscriptionCode[15]='\0';
                                        if(strcmp(sAddressCode, cLastReadAddressCode)!=0)
                                        {
                                        if(iWhere==(iCustomersNum-1))
                                                BiosPutStrMove(1, 5, 10, "•˝© ‡¢ •¢â", 0x70);
                                        else
                                        if(iWhere==0)
                                                BiosPutStrMove(1, 5, 10, "•˝© ‡¢ Ôˆã", 0x70);
                                        else
                                                BiosPutStrMove(1, 2, 15, pCustomer->cSubscriptionCode, 0x07);
                                        }
                                        else
                                                BiosPutStrMove(1, 4, 12, "˜£ıãˆ¢ Ù˛•¢â", 0x70);
                                        fread(pCustomer->cName, sizeof(char), 18, fDataset);
                                        pCustomer->cName[18]='\0';
                                        strcpy(cName, pCustomer->cName);
                                        i=0;
                                        while((cName[i]==' ')&&(i<18))
                                                {i++;}
                                        BiosPutStrMove(2, 10-(18-i)/2, 18-i, &cName[i], 0x07);
                                        fread(pCustomer->cFamily, sizeof(char), 22, fDataset);
                                        pCustomer->cFamily[22]='\0';
                                        strcpy(cName, pCustomer->cFamily); 
                                        i=0;
                                        while((cName[i]==' ')&&(i<22))
                                                {i++;}
                                        BiosPutStrMove(3, 10-(22-i)/2, 22-i, &cName[i], 0x07);
                                        BiosPutStrMove(4, 11, 8, ":†®•£â£Ì", 0x07);
                                        fread(pCustomer->cAddressCode, sizeof(char), 10, fDataset);
                                        BiosPutStrMove(4, 1, 10, pCustomer->cAddressCode, 0x07);
                                        fread(pCustomer->cCounterSerial, sizeof(char), 10, fDataset);
        
                                        fread(&pCustomer->lConsumptionKind, sizeof(long), 1, fDataset);
                                        fread(&pCustomer->lCapacity, sizeof(long), 1, fDataset);
                                        fread(pCustomer->cPrevDate, sizeof(char), 6, fDataset);
                                        fread(pCustomer->cPrevCounter, sizeof(char), 10, fDataset);

                        if(iCanReadManualyAdr)
                        {
                                BiosPutStrMove(5, 11, 8, ":ÔåÉÉ˛•©", 0x07);
                                BiosPutStrMove(5, 1, 10, pCustomer->cCounterSerial, 0x07);
                        }
                        else
                        {
                        sSign[1]='\0';
                        sSign[0]=0x1F;
                        strcpy(sKeys, sSign);
                        strcat(sKeys, ":˚£‰ì");
                        strcat(sKeys,"  ");
                        sSign[0]=0x1E;
                        strcat(sKeys, sSign);
                        strcat(sKeys, ":¸ÒìÏ");
                        BiosPutStrMove(5, 3, 14, sKeys, 0x07);
                        }
                                        BiosPutStrMove(7, 1, 18, " SPACE=ˆÉõó©õ†™ˆ• ", 0x70);

                                        break;

                                        case 0x4D00://right


                                          BiosClrScr(0x07);
                                          DrawBox(0, 0, 7, 19,0x07);
                                          BiosPutStrMove(3, 1, 18, "å˙ ˜£ıãˆ¢ •£ ˆõó©õ", 0x07);
                                          BiosPutStrMove(4, 1, 18, "  !£˝ıÌ •ì≠ åÍ‡Ò  ", 0x07);

                                        iFoundUnread=0;
                                        while( (iWhere<(iCustomersNum-1)) && !iFoundUnread )
                                        {
                                          iWhere=iWhere+1;

                                          fseek(fAddressCodes, iWhere*ADDRESCODESIZE, SEEK_SET);
                                          fread(sAddressCode, sizeof(char), 10, fAddressCodes);
                                          sAddressCode[10]='\0';

                                          fread(&lIndex, sizeof(long), 1, fAddressCodes);

                                          fseek(fDataset, lIndex*CUSTOMERSIZE, SEEK_SET);
                                          fread(cMySubscriptionCode, sizeof(char), 15, fDataset);
                                          cMySubscriptionCode[15]='\0';
                                          if(!IsThisCustomerRead(cMySubscriptionCode))
                                                iFoundUnread=1;
                                         }


                                        if(!iFoundUnread)
                                        {
                                          BiosClrScr(0x07);
                                          DrawBox(0, 0, 7, 19,0x07);
                                          BiosPutStrMove(3, 1, 18, " ˚£‰ì Ù˝Ì•ó´Û ¯Û˙ ", 0x07);
                                          BiosPutStrMove(4, 1, 18, "  !£ıã˜£´˜£ıãˆÉ¢  ", 0x07);
                                          BiosBeep(500);
                                          BiosGetChar();

                                        }

                                        fseek(fAddressCodes, iWhere*ADDRESCODESIZE, SEEK_SET);
                                        fread(sAddressCode, sizeof(char), 10, fAddressCodes);
                                        sAddressCode[10]='\0';

                                        BiosClrScr(0x07);
                                        DrawBox(0, 0, 6, 19,0x07);
                                        BiosPutStrMove(0, 5, 10, "::•˛© ‡¢::", 0x07);
                                        fread(&lIndex, sizeof(long), 1, fAddressCodes);

                                        if(!(lIndex<iCustomersNum)) lIndex=0;

                                        fseek(fDataset, lIndex*CUSTOMERSIZE, SEEK_SET);
                                        fread(pCustomer->cSubscriptionCode, sizeof(char), 15, fDataset);
                                        pCustomer->cSubscriptionCode[15]='\0';
                                        //if(strcmp(sAddressCode, cLastReadAddressCode)!=0)
                                        {
                                        /*if(iWhere==(iCustomersNum-1))
                                                BiosPutStrMove(1, 5, 10, "•˝© ‡¢ •¢â", 0x70);
                                        else
                                        if(iWhere==0)
                                                BiosPutStrMove(1, 5, 10, "•˝© ‡¢ Ôˆã", 0x70);
                                        else*/
                                                BiosPutStrMove(1, 2, 15, pCustomer->cSubscriptionCode, 0x07);
                                        }
                                        //else
                                        //        BiosPutStrMove(1, 4, 12, "˜£ıãˆ¢ Ù˛•¢â", 0x70);
                                        fread(pCustomer->cName, sizeof(char), 18, fDataset);
                                        pCustomer->cName[18]='\0';
                                        strcpy(cName, pCustomer->cName);
                                        i=0;
                                        while((cName[i]==' ')&&(i<18))
                                                {i++;}
                                        BiosPutStrMove(2, 10-(18-i)/2, 18-i, &cName[i], 0x07);
                                        fread(pCustomer->cFamily, sizeof(char), 22, fDataset);
                                        pCustomer->cFamily[22]='\0';
                                        strcpy(cName, pCustomer->cFamily); 
                                        i=0;
                                        while((cName[i]==' ')&&(i<22))
                                                {i++;}
                                        BiosPutStrMove(3, 10-(22-i)/2, 22-i, &cName[i], 0x07);
                                        BiosPutStrMove(4, 11, 8, ":†®•£â£Ì", 0x07);
                                        fread(pCustomer->cAddressCode, sizeof(char), 10, fDataset);
                                        BiosPutStrMove(4, 1, 10, pCustomer->cAddressCode, 0x07);
                                        fread(pCustomer->cCounterSerial, sizeof(char), 10, fDataset);
        
                                        fread(&pCustomer->lConsumptionKind, sizeof(long), 1, fDataset);
                                        fread(&pCustomer->lCapacity, sizeof(long), 1, fDataset);
                                        fread(pCustomer->cPrevDate, sizeof(char), 6, fDataset);
                                        fread(pCustomer->cPrevCounter, sizeof(char), 10, fDataset);    
                        if(iCanReadManualyAdr)
                        {
                                BiosPutStrMove(5, 11, 8, ":ÔåÉÉ˛•©", 0x07);
                                BiosPutStrMove(5, 1, 10, pCustomer->cCounterSerial, 0x07);
                        }
                        else
                        {
                        sSign[1]='\0';
                        sSign[0]=0x1F;
                        strcpy(sKeys, sSign);
                        strcat(sKeys, ":˚£‰ì");
                        strcat(sKeys,"  ");
                        sSign[0]=0x1E;
                        strcat(sKeys, sSign);
                        strcat(sKeys, ":¸ÒìÏ");
                        BiosPutStrMove(5, 3, 14, sKeys, 0x07);
                        }
                                        BiosPutStrMove(7, 1, 18, " SPACE=ˆÉõó©õ†™ˆ• ", 0x70);
                                        break;


                                        case 0x04B00://left


                                          BiosClrScr(0x07);
                                          DrawBox(0, 0, 7, 19,0x07);
                                          BiosPutStrMove(3, 1, 18, "å˙ ˜£ıãˆ¢ •£ ˆõó©õ", 0x07);
                                          BiosPutStrMove(4, 1, 18, "  !£˝ıÌ •ì≠ åÍ‡Ò  ", 0x07);


                                        iFoundUnread=0;
                                        while( (iWhere>0) && !iFoundUnread )
                                        {
                                          iWhere=iWhere-1;
                                          fseek(fAddressCodes, iWhere*ADDRESCODESIZE, SEEK_SET);
                                          fread(sAddressCode, sizeof(char), 10, fAddressCodes);
                                          sAddressCode[10]='\0';
                                          fread(&lIndex, sizeof(long), 1, fAddressCodes);

                                          fseek(fDataset, lIndex*CUSTOMERSIZE, SEEK_SET);
                                          fread(cMySubscriptionCode, sizeof(char), 15, fDataset);
                                          cMySubscriptionCode[15]='\0';
                                          if(!IsThisCustomerRead(cMySubscriptionCode))
                                                iFoundUnread=1;
                                         }
                                        if(!iFoundUnread)
                                        {
                                          BiosClrScr(0x07);
                                          DrawBox(0, 0, 7, 19,0x07);
                                          BiosPutStrMove(3, 1, 18, " ¸ÒìÏ Ù˝Ì•ó´Û ¯Û˙ ", 0x07);
                                          BiosPutStrMove(4, 1, 18, "  !£ıã˜£´˜£ıãˆÉ¢  ", 0x07);
                                          BiosBeep(500);
                                          BiosGetChar();

                                        }

                                        fseek(fAddressCodes, iWhere*ADDRESCODESIZE, SEEK_SET);
                                        fread(sAddressCode, sizeof(char), 10, fAddressCodes);
                                        sAddressCode[10]='\0';

                                        BiosClrScr(0x07);
                                        DrawBox(0, 0, 6, 19,0x07);
                                        BiosPutStrMove(0, 5, 10, "::•˛© ‡¢::", 0x07);
                                        fread(&lIndex, sizeof(long), 1, fAddressCodes);

                                        if(!(lIndex<iCustomersNum)) lIndex=0;

                                        fseek(fDataset, lIndex*CUSTOMERSIZE, SEEK_SET);
                                        fread(pCustomer->cSubscriptionCode, sizeof(char), 15, fDataset);
                                        pCustomer->cSubscriptionCode[15]='\0';
                                        //if(strcmp(sAddressCode, cLastReadAddressCode)!=0)
                                        {
                                        /*
                                        if(iWhere==(iCustomersNum-1))
                                                BiosPutStrMove(1, 5, 10, "•˝© ‡¢ •¢â", 0x70);
                                        else
                                        if(iWhere==0)
                                                BiosPutStrMove(1, 5, 10, "•˝© ‡¢ Ôˆã", 0x70);
                                        else*/
                                                BiosPutStrMove(1, 2, 15, pCustomer->cSubscriptionCode, 0x07);
                                        }
                                        //else
                                          //      BiosPutStrMove(1, 4, 12, "˜£ıãˆ¢ Ù˛•¢â", 0x70);
                                        fread(pCustomer->cName, sizeof(char), 18, fDataset);
                                        pCustomer->cName[18]='\0';
                                        strcpy(cName, pCustomer->cName);
                                        i=0;
                                        while((cName[i]==' ')&&(i<18))
                                                {i++;}
                                        BiosPutStrMove(2, 10-(18-i)/2, 18-i, &cName[i], 0x07);
                                        fread(pCustomer->cFamily, sizeof(char), 22, fDataset);
                                        pCustomer->cFamily[22]='\0';
                                        strcpy(cName, pCustomer->cFamily); 
                                        i=0;
                                        while((cName[i]==' ')&&(i<22))
                                                {i++;}
                                        BiosPutStrMove(3, 10-(22-i)/2, 22-i, &cName[i], 0x07);
                                        BiosPutStrMove(4, 11, 8, ":†®•£â£Ì", 0x07);
                                        fread(pCustomer->cAddressCode, sizeof(char), 10, fDataset);
                                        BiosPutStrMove(4, 1, 10, pCustomer->cAddressCode, 0x07);
                                        fread(pCustomer->cCounterSerial, sizeof(char), 10, fDataset);
        
                                        fread(&pCustomer->lConsumptionKind, sizeof(long), 1, fDataset);
                                        fread(&pCustomer->lCapacity, sizeof(long), 1, fDataset);
                                        fread(pCustomer->cPrevDate, sizeof(char), 6, fDataset);
                                        fread(pCustomer->cPrevCounter, sizeof(char), 10, fDataset);    
                        if(iCanReadManualyAdr)
                        {
                                BiosPutStrMove(5, 11, 8, ":ÔåÉÉ˛•©", 0x07);
                                BiosPutStrMove(5, 1, 10, pCustomer->cCounterSerial, 0x07);
                        }
                        else
                        {
                        sSign[1]='\0';
                        sSign[0]=0x1F;
                        strcpy(sKeys, sSign);
                        strcat(sKeys, ":˚£‰ì");
                        strcat(sKeys,"  ");
                        sSign[0]=0x1E;
                        strcat(sKeys, sSign);
                        strcat(sKeys, ":¸ÒìÏ");
                        BiosPutStrMove(5, 3, 14, sKeys, 0x07);
                        }
                                        BiosPutStrMove(7, 1, 18, " SPACE=ˆÉõó©õ†™ˆ• ", 0x70);



                                        break;


                                        case 0x5032: //shift down arrow
                                        iOldWhere=iWhere;
                                                fseek(fAddressCodes, iWhere*ADDRESCODESIZE, SEEK_SET);
                                                fread(cMyRegion, sizeof(char), 5, fAddressCodes);
                                                cMyRegion[5]='\0';

                                                BiosClrScr(0x07);
                                                DrawBox(0, 0, 7, 19,0x07);
                                                BiosPutStrMove(3, 1, 18, "˚£‰ì ¯‰‡Ï˚ã•ìˆõó©õ", 0x07);
                                                BiosPutStrMove(4, 1, 18, "  !£˝ıÌ •ì≠ åÍ‡Ò  ", 0x07);

                                        iFoundUnread=0;
                                        while( (iWhere<(iCustomersNum-1)) && !iFoundUnread )
                                        {
                                          iWhere=iWhere+1;

                                          fseek(fAddressCodes, iWhere*ADDRESCODESIZE, SEEK_SET);
                                          fread(sAddressCode, sizeof(char), 5, fAddressCodes);
                                          sAddressCode[5]='\0';

                                          if(strcmp(cMyRegion, sAddressCode)!=0)
                                             iFoundUnread=1;
                                        }


                                        if(!iFoundUnread)
                                        {
                                         iWhere=iOldWhere;
                                          BiosClrScr(0x07);
                                          DrawBox(0, 0, 7, 19,0x07);
                                          BiosPutStrMove(3, 1, 18, "     •¢â¯É‰‡Ï     ", 0x07);
                                          BiosBeep(500);
                                          BiosGetChar();

                                        }

                                        fseek(fAddressCodes, iWhere*ADDRESCODESIZE, SEEK_SET);
                                        fread(sAddressCode, sizeof(char), 10, fAddressCodes);
                                        sAddressCode[10]='\0';

                                        BiosClrScr(0x07);
                                        DrawBox(0, 0, 6, 19,0x07);
                                        BiosPutStrMove(0, 5, 10, "::•˛© ‡¢::", 0x07);
                                        fread(&lIndex, sizeof(long), 1, fAddressCodes);

                                        if(!(lIndex<iCustomersNum)) lIndex=0;

                                        fseek(fDataset, lIndex*CUSTOMERSIZE, SEEK_SET);
                                        fread(pCustomer->cSubscriptionCode, sizeof(char), 15, fDataset);
                                        pCustomer->cSubscriptionCode[15]='\0';
                                        //if(strcmp(sAddressCode, cLastReadAddressCode)!=0)
                                        {
                                        /*if(iWhere==(iCustomersNum-1))
                                                BiosPutStrMove(1, 5, 10, "•˝© ‡¢ •¢â", 0x70);
                                        else
                                        if(iWhere==0)
                                                BiosPutStrMove(1, 5, 10, "•˝© ‡¢ Ôˆã", 0x70);
                                        else*/
                                                BiosPutStrMove(1, 2, 15, pCustomer->cSubscriptionCode, 0x07);
                                        }
                                        //else
                                        //        BiosPutStrMove(1, 4, 12, "˜£ıãˆ¢ Ù˛•¢â", 0x70);
                                        fread(pCustomer->cName, sizeof(char), 18, fDataset);
                                        pCustomer->cName[18]='\0';
                                        strcpy(cName, pCustomer->cName);
                                        i=0;
                                        while((cName[i]==' ')&&(i<18))
                                                {i++;}
                                        BiosPutStrMove(2, 10-(18-i)/2, 18-i, &cName[i], 0x07);
                                        fread(pCustomer->cFamily, sizeof(char), 22, fDataset);
                                        pCustomer->cFamily[22]='\0';
                                        strcpy(cName, pCustomer->cFamily); 
                                        i=0;
                                        while((cName[i]==' ')&&(i<22))
                                                {i++;}
                                        BiosPutStrMove(3, 10-(22-i)/2, 22-i, &cName[i], 0x07);
                                        BiosPutStrMove(4, 11, 8, ":†®•£â£Ì", 0x07);
                                        fread(pCustomer->cAddressCode, sizeof(char), 10, fDataset);
                                        BiosPutStrMove(4, 1, 10, pCustomer->cAddressCode, 0x07);
                                        fread(pCustomer->cCounterSerial, sizeof(char), 10, fDataset);
        
                                        fread(&pCustomer->lConsumptionKind, sizeof(long), 1, fDataset);
                                        fread(&pCustomer->lCapacity, sizeof(long), 1, fDataset);
                                        fread(pCustomer->cPrevDate, sizeof(char), 6, fDataset);
                                        fread(pCustomer->cPrevCounter, sizeof(char), 10, fDataset);    
                        if(iCanReadManualyAdr)
                        {
                                BiosPutStrMove(5, 11, 8, ":ÔåÉÉ˛•©", 0x07);
                                BiosPutStrMove(5, 1, 10, pCustomer->cCounterSerial, 0x07);
                        }
                        else
                        {
                        sSign[1]='\0';
                        sSign[0]=0x1F;
                        strcpy(sKeys, sSign);
                        strcat(sKeys, ":˚£‰ì");
                        strcat(sKeys,"  ");
                        sSign[0]=0x1E;
                        strcat(sKeys, sSign);
                        strcat(sKeys, ":¸ÒìÏ");
                        BiosPutStrMove(5, 3, 14, sKeys, 0x07);
                        }
                                        BiosPutStrMove(7, 1, 18, " SPACE=ˆÉõó©õ†™ˆ• ", 0x70);


                                        break;

                                        case 0x4838: //shift up arrow



                                                fseek(fAddressCodes, iWhere*ADDRESCODESIZE, SEEK_SET);
                                                fread(cMyRegion, sizeof(char), 5, fAddressCodes);
                                                cMyRegion[5]='\0';

                                                BiosClrScr(0x07);
                                                DrawBox(0, 0, 7, 19,0x07);
                                                BiosPutStrMove(3, 1, 18, "¸ÒìÏ ¯‰‡Ï˚ã•ìˆõó©õ", 0x07);
                                                BiosPutStrMove(4, 1, 18, "  !£˝ıÌ •ì≠ åÍ‡Ò  ", 0x07);

                                        iFoundUnread=0;
                                        while( (iWhere>0) && !iFoundUnread )
                                        {
                                          iWhere=iWhere-1;

                                          fseek(fAddressCodes, iWhere*ADDRESCODESIZE, SEEK_SET);
                                          fread(sAddressCode, sizeof(char), 5, fAddressCodes);
                                          sAddressCode[5]='\0';

                                          if(strcmp(cMyRegion, sAddressCode)!=0)
                                             iFoundUnread=1;
                                        }

                                        if(!iFoundUnread)
                                        {
                                          BiosClrScr(0x07);
                                          DrawBox(0, 0, 7, 19,0x07);
                                          BiosPutStrMove(3, 1, 18, "     Ôˆã¯É‰‡Ï     ", 0x07);
                                          BiosBeep(500);
                                          BiosGetChar();

                                        }
                                        else
                                        {

                                        fseek(fAddressCodes, iWhere*ADDRESCODESIZE, SEEK_SET);
                                        fread(cMyRegion, sizeof(char), 5, fAddressCodes);
                                        cMyRegion[5]='\0';

                                        iFoundUnread=0;
                                        while( (iWhere>0) && !iFoundUnread )
                                        {
                                          iWhere=iWhere-1;

                                          fseek(fAddressCodes, iWhere*ADDRESCODESIZE, SEEK_SET);
                                          fread(sAddressCode, sizeof(char), 5, fAddressCodes);
                                          sAddressCode[5]='\0';

                                          if(strcmp(cMyRegion, sAddressCode)!=0)
                                             iFoundUnread=1;
                                        }
                                        if(iFoundUnread)
                                          if(iWhere>0)
                                            iWhere=iWhere+1;
                                        }


                                        fseek(fAddressCodes, iWhere*ADDRESCODESIZE, SEEK_SET);
                                        fread(sAddressCode, sizeof(char), 10, fAddressCodes);
                                        sAddressCode[10]='\0';

                                        BiosClrScr(0x07);
                                        DrawBox(0, 0, 6, 19,0x07);
                                        BiosPutStrMove(0, 5, 10, "::•˛© ‡¢::", 0x07);
                                        fread(&lIndex, sizeof(long), 1, fAddressCodes);

                                        if(!(lIndex<iCustomersNum)) lIndex=0;

                                        fseek(fDataset, lIndex*CUSTOMERSIZE, SEEK_SET);
                                        fread(pCustomer->cSubscriptionCode, sizeof(char), 15, fDataset);
                                        pCustomer->cSubscriptionCode[15]='\0';
                                        //if(strcmp(sAddressCode, cLastReadAddressCode)!=0)
                                        {
                                        /*if(iWhere==(iCustomersNum-1))
                                                BiosPutStrMove(1, 5, 10, "•˝© ‡¢ •¢â", 0x70);
                                        else
                                        if(iWhere==0)
                                                BiosPutStrMove(1, 5, 10, "•˝© ‡¢ Ôˆã", 0x70);
                                        else*/
                                                BiosPutStrMove(1, 2, 15, pCustomer->cSubscriptionCode, 0x07);
                                        }
                                        //else
                                        //        BiosPutStrMove(1, 4, 12, "˜£ıãˆ¢ Ù˛•¢â", 0x70);
                                        fread(pCustomer->cName, sizeof(char), 18, fDataset);
                                        pCustomer->cName[18]='\0';
                                        strcpy(cName, pCustomer->cName);
                                        i=0;
                                        while((cName[i]==' ')&&(i<18))
                                                {i++;}
                                        BiosPutStrMove(2, 10-(18-i)/2, 18-i, &cName[i], 0x07);
                                        fread(pCustomer->cFamily, sizeof(char), 22, fDataset);
                                        pCustomer->cFamily[22]='\0';
                                        strcpy(cName, pCustomer->cFamily); 
                                        i=0;
                                        while((cName[i]==' ')&&(i<22))
                                                {i++;}
                                        BiosPutStrMove(3, 10-(22-i)/2, 22-i, &cName[i], 0x07);
                                        BiosPutStrMove(4, 11, 8, ":†®•£â£Ì", 0x07);
                                        fread(pCustomer->cAddressCode, sizeof(char), 10, fDataset);
                                        BiosPutStrMove(4, 1, 10, pCustomer->cAddressCode, 0x07);
                                        fread(pCustomer->cCounterSerial, sizeof(char), 10, fDataset);
        
                                        fread(&pCustomer->lConsumptionKind, sizeof(long), 1, fDataset);
                                        fread(&pCustomer->lCapacity, sizeof(long), 1, fDataset);
                                        fread(pCustomer->cPrevDate, sizeof(char), 6, fDataset);
                                        fread(pCustomer->cPrevCounter, sizeof(char), 10, fDataset);    
                        if(iCanReadManualyAdr)
                        {
                                BiosPutStrMove(5, 11, 8, ":ÔåÉÉ˛•©", 0x07);
                                BiosPutStrMove(5, 1, 10, pCustomer->cCounterSerial, 0x07);
                        }
                        else
                        {
                        sSign[1]='\0';
                        sSign[0]=0x1F;
                        strcpy(sKeys, sSign);
                        strcat(sKeys, ":˚£‰ì");
                        strcat(sKeys,"  ");
                        sSign[0]=0x1E;
                        strcat(sKeys, sSign);
                        strcat(sKeys, ":¸ÒìÏ");
                        BiosPutStrMove(5, 3, 14, sKeys, 0x07);
                        }
                                        BiosPutStrMove(7, 1, 18, " SPACE=ˆÉõó©õ†™ˆ• ", 0x70);



                                        break;

                                 default:
                                                if(BiosGetAbortStatus())
                                                {
                                                       fclose(fAddressCodes);
                                                        return 6;
                                                }

                                                switch((char)ch)
                                                {
                                                case '2':
                                                        if(GetAddressCode(sSearching))
                                                                FindAddressCode(sSearching, pCustomer);


               iStartIndex=0;
               iEndIndex=iCustomersNum-1;

               iOldStart=iStartIndex;
               iOldEnd=iEndIndex;

               iWhere=iEndIndex;

               fseek(fAddressCodes, iWhere*ADDRESCODESIZE, SEEK_SET);
               fread(sAddressCode, sizeof(char), 10, fAddressCodes);
               sAddressCode[10]='\0';

               iResult=50;
               iResult=strcmp(sAddressCode, cLastReadAddressCode);

               if(iResult!=0)
                     iWhere=iCustomersNum/2;
               do
                {

                        if(iResult!=0)
                        {
                        fseek(fAddressCodes, iWhere*ADDRESCODESIZE, SEEK_SET);
                        fread(sAddressCode, sizeof(char), 10, fAddressCodes);
                        sAddressCode[10]='\0';

                        iResult=strcmp(sAddressCode, cLastReadAddressCode);
                        }
                        if(iResult==0)
                        {//iResult
                        BiosClrScr(0x07);
                        DrawBox(0, 0, 6, 19,0x07);
                        BiosPutStrMove(0, 5, 10, "::•˛© ‡¢::", 0x07);
                        fread(&lIndex, sizeof(long), 1, fAddressCodes);
                        fseek(fDataset, lIndex*CUSTOMERSIZE, SEEK_SET);
                        fread(pCustomer->cSubscriptionCode, sizeof(char), 15, fDataset);
                        pCustomer->cSubscriptionCode[15]='\0';
                        //if(strcmp(sAddressCode, cLastReadAddressCode))
                                BiosPutStrMove(1, 2, 15, pCustomer->cSubscriptionCode, 0x07);
                        //else
                        //        BiosPutStrMove(1, 4, 12, "˜£ıãˆ¢ Ù˛•¢â", 0x70);
                        fread(pCustomer->cName, sizeof(char), 18, fDataset);
                        pCustomer->cName[18]='\0';
                        strcpy(cName, pCustomer->cName);
                        i=0;
                        while((cName[i]==' ')&&(i<18))
                                {i++;}
                        BiosPutStrMove(2, 10-(18-i)/2, 18-i, &cName[i], 0x07);
                        fread(pCustomer->cFamily, sizeof(char), 22, fDataset);
                        pCustomer->cFamily[22]='\0';
                        strcpy(cName, pCustomer->cFamily); 
                        i=0;
                        while((cName[i]==' ')&&(i<22))
                                {i++;}
                        BiosPutStrMove(3, 10-(22-i)/2, 22-i, &cName[i], 0x07);
                        BiosPutStrMove(4, 11, 8, ":†®•£â£Ì", 0x07);
                        fread(pCustomer->cAddressCode, sizeof(char), 10, fDataset);
                        BiosPutStrMove(4, 1, 10, pCustomer->cAddressCode, 0x07);
                        fread(pCustomer->cCounterSerial, sizeof(char), 10, fDataset);
        
                        fread(&pCustomer->lConsumptionKind, sizeof(long), 1, fDataset);
                        fread(&pCustomer->lCapacity, sizeof(long), 1, fDataset);
                        fread(pCustomer->cPrevDate, sizeof(char), 6, fDataset);
                        fread(pCustomer->cPrevCounter, sizeof(char), 10, fDataset);    
                        if(iCanReadManualyAdr)
                        {
                                BiosPutStrMove(5, 11, 8, ":ÔåÉÉ˛•©", 0x07);
                                BiosPutStrMove(5, 1, 10, pCustomer->cCounterSerial, 0x07);
                        }
                        else
                        {
                        sSign[1]='\0';
                        sSign[0]=0x1F;
                        strcpy(sKeys, sSign);
                        strcat(sKeys, ":˚£‰ì");
                        strcat(sKeys,"  ");
                        sSign[0]=0x1E;
                        strcat(sKeys, sSign);
                        strcat(sKeys, ":¸ÒìÏ");
                        BiosPutStrMove(5, 3, 14, sKeys, 0x07);
                        }
//                        BiosPutStrMove(7, 1, 18, "  SPACE=˚£‰ì†™ˆ•  ", 0x70);
                        BiosPutStrMove(7, 1, 18, " SPACE=ˆÉõó©õ†™ˆ• ", 0x70);
                        //BiosPutStrMove(5, 1, 18, "      £˝˛åÉó      ", 0x70);
                        break;
                        }
                        else
                        if(iResult<0)
                        {
                                iOldStart=iStartIndex;
                                iOldEnd=iEndIndex;
                                iStartIndex=iWhere;
                                iWhere=(iEndIndex-iStartIndex)/2+iStartIndex;
                        }
                        else
                        {
                                iOldStart=iStartIndex;
                                iOldEnd=iEndIndex;
                                iEndIndex=iWhere;
                                iWhere=(iEndIndex-iStartIndex)/2+iStartIndex;
                        }                

                }
                while( (iStartIndex<iEndIndex) && ( !((iOldStart==iStartIndex) && (iOldEnd==iEndIndex)) )  );


                                                break;
                                                case '0':
                                                        fread(cAddress, sizeof(char), 45, fDataset);
                                                        cAddress[45]='\0';
                                                        BiosClrScr(0x07);
                                                        DrawBox(0, 0, 6, 19,0x07);
                                                        BiosPutStrMove(0, 5, 10, "::¸ıåÉ´ı::", 0x07);

                                                        for(j=0; j<33;j+=16)
                                                        {
                                                        for(i=j; (i<j+16)&&(i<45); i++)
                                                                cAddressPart[i-j]=cAddress[i];
                                                        cAddressPart[i-j]='\0';
                                                         BiosPutStrMove(4-j/16, 18-strlen(cAddressPart), strlen(cAddressPart), cAddressPart, 0x07);
                                                        BiosPutStrMove(7, 1, 18, "   !£˝ı¶ì ˚£˝ÒÌ   ", 0x70);
                                                        }
                                                        BiosGetChar();



                                        fseek(fAddressCodes, iWhere*ADDRESCODESIZE, SEEK_SET);
                                        fread(sAddressCode, sizeof(char), 10, fAddressCodes);
                                        sAddressCode[10]='\0';

                                        BiosClrScr(0x07);
                                        DrawBox(0, 0, 6, 19,0x07);
                                        BiosPutStrMove(0, 5, 10, "::•˛© ‡¢::", 0x07);
                                        fread(&lIndex, sizeof(long), 1, fAddressCodes);

                                        if(!(lIndex<iCustomersNum)) lIndex=0;

                                        fseek(fDataset, lIndex*CUSTOMERSIZE, SEEK_SET);
                                        fread(pCustomer->cSubscriptionCode, sizeof(char), 15, fDataset);
                                        pCustomer->cSubscriptionCode[15]='\0';
                                        if(strcmp(sAddressCode, cLastReadAddressCode)!=0)
                                        {
                                        if(iWhere==(iCustomersNum-1))
                                                BiosPutStrMove(1, 5, 10, "•˝© ‡¢ •¢â", 0x70);
                                        else
                                        if(iWhere==0)
                                                BiosPutStrMove(1, 5, 10, "•˝© ‡¢ Ôˆã", 0x70);
                                        else
                                                BiosPutStrMove(1, 2, 15, pCustomer->cSubscriptionCode, 0x07);
                                        }
                                        else
                                                BiosPutStrMove(1, 4, 12, "˜£ıãˆ¢ Ù˛•¢â", 0x70);
                                        fread(pCustomer->cName, sizeof(char), 18, fDataset);
                                        pCustomer->cName[18]='\0';
                                        strcpy(cName, pCustomer->cName);
                                        i=0;
                                        while((cName[i]==' ')&&(i<18))
                                                {i++;}
                                        BiosPutStrMove(2, 10-(18-i)/2, 18-i, &cName[i], 0x07);
                                        fread(pCustomer->cFamily, sizeof(char), 22, fDataset);
                                        pCustomer->cFamily[22]='\0';
                                        strcpy(cName, pCustomer->cFamily); 
                                        i=0;
                                        while((cName[i]==' ')&&(i<22))
                                                {i++;}
                                        BiosPutStrMove(3, 10-(22-i)/2, 22-i, &cName[i], 0x07);
                                        BiosPutStrMove(4, 11, 8, ":†®•£â£Ì", 0x07);
                                        fread(pCustomer->cAddressCode, sizeof(char), 10, fDataset);
                                        BiosPutStrMove(4, 1, 10, pCustomer->cAddressCode, 0x07);
                                        fread(pCustomer->cCounterSerial, sizeof(char), 10, fDataset);
        
                                        fread(&pCustomer->lConsumptionKind, sizeof(long), 1, fDataset);
                                        fread(&pCustomer->lCapacity, sizeof(long), 1, fDataset);
                                        fread(pCustomer->cPrevDate, sizeof(char), 6, fDataset);
                                        fread(pCustomer->cPrevCounter, sizeof(char), 10, fDataset);    
                        if(iCanReadManualyAdr)
                        {
                                BiosPutStrMove(5, 11, 8, ":ÔåÉÉ˛•©", 0x07);
                                BiosPutStrMove(5, 1, 10, pCustomer->cCounterSerial, 0x07);
                        }
                        else
                        {
                        sSign[1]='\0';
                        sSign[0]=0x1F;
                        strcpy(sKeys, sSign);
                        strcat(sKeys, ":˚£‰ì");
                        strcat(sKeys,"  ");
                        sSign[0]=0x1E;
                        strcat(sKeys, sSign);
                        strcat(sKeys, ":¸ÒìÏ");
                        BiosPutStrMove(5, 3, 14, sKeys, 0x07);
                        }
//                                        BiosPutStrMove(7, 1, 18, "  SPACE=˚£‰ì†™ˆ•  ", 0x70);
                                        BiosPutStrMove(7, 1, 18, " SPACE=ˆÉõó©õ†™ˆ• ", 0x70);



                                                        break;

                                                case '1':
                                                        BiosClrScr(0x07);
                                                        DrawBox(0, 0, 6, 19,0x07);
                                                        BiosPutStrMove(0, 3, 14, "::¸ÒìÏ °˛•åó::", 0x07);

                                                        cPreviousDate[0]=pCustomer->cPrevDate[0];
                                                        cPreviousDate[1]=pCustomer->cPrevDate[1];
                                                        cPreviousDate[2]='/';
                                                        cPreviousDate[3]=pCustomer->cPrevDate[2];
                                                        cPreviousDate[4]=pCustomer->cPrevDate[3];
                                                        cPreviousDate[5]='/';
                                                        cPreviousDate[6]=pCustomer->cPrevDate[4];
                                                        cPreviousDate[7]=pCustomer->cPrevDate[5];
                                                        cPreviousDate[8]='\0';
                                                        BiosPutStrMove(3, 6, 8, cPreviousDate, 0x07);
                                                        BiosPutStrMove(7, 1, 18, "   !£˝ı¶ì ˚£˝ÒÌ   ", 0x70);
                                                        BiosGetChar();



                                        fseek(fAddressCodes, iWhere*ADDRESCODESIZE, SEEK_SET);
                                        fread(sAddressCode, sizeof(char), 10, fAddressCodes);
                                        sAddressCode[10]='\0';

                                        BiosClrScr(0x07);
                                        DrawBox(0, 0, 6, 19,0x07);
                                        BiosPutStrMove(0, 5, 10, "::•˛© ‡¢::", 0x07);
                                        fread(&lIndex, sizeof(long), 1, fAddressCodes);

                                        if(!(lIndex<iCustomersNum)) lIndex=0;

                                        fseek(fDataset, lIndex*CUSTOMERSIZE, SEEK_SET);
                                        fread(pCustomer->cSubscriptionCode, sizeof(char), 15, fDataset);
                                        pCustomer->cSubscriptionCode[15]='\0';
                                        if(strcmp(sAddressCode, cLastReadAddressCode)!=0)
                                        {
                                        if(iWhere==(iCustomersNum-1))
                                                BiosPutStrMove(1, 5, 10, "•˝© ‡¢ •¢â", 0x70);
                                        else
                                        if(iWhere==0)
                                                BiosPutStrMove(1, 5, 10, "•˝© ‡¢ Ôˆã", 0x70);
                                        else
                                                BiosPutStrMove(1, 2, 15, pCustomer->cSubscriptionCode, 0x07);
                                        }
                                        else
                                                BiosPutStrMove(1, 4, 12, "˜£ıãˆ¢ Ù˛•¢â", 0x70);
                                        fread(pCustomer->cName, sizeof(char), 18, fDataset);
                                        pCustomer->cName[18]='\0';
                                        strcpy(cName, pCustomer->cName);
                                        i=0;
                                        while((cName[i]==' ')&&(i<18))
                                                {i++;}
                                        BiosPutStrMove(2, 10-(18-i)/2, 18-i, &cName[i], 0x07);
                                        fread(pCustomer->cFamily, sizeof(char), 22, fDataset);
                                        pCustomer->cFamily[22]='\0';
                                        strcpy(cName, pCustomer->cFamily); 
                                        i=0;
                                        while((cName[i]==' ')&&(i<22))
                                                {i++;}
                                        BiosPutStrMove(3, 10-(22-i)/2, 22-i, &cName[i], 0x07);
                                        BiosPutStrMove(4, 11, 8, ":†®•£â£Ì", 0x07);
                                        fread(pCustomer->cAddressCode, sizeof(char), 10, fDataset);
                                        BiosPutStrMove(4, 1, 10, pCustomer->cAddressCode, 0x07);
                                        fread(pCustomer->cCounterSerial, sizeof(char), 10, fDataset);
        
                                        fread(&pCustomer->lConsumptionKind, sizeof(long), 1, fDataset);
                                        fread(&pCustomer->lCapacity, sizeof(long), 1, fDataset);
                                        fread(pCustomer->cPrevDate, sizeof(char), 6, fDataset);
                                        fread(pCustomer->cPrevCounter, sizeof(char), 10, fDataset);    
                        if(iCanReadManualyAdr)
                        {
                                BiosPutStrMove(5, 11, 8, ":ÔåÉÉ˛•©", 0x07);
                                BiosPutStrMove(5, 1, 10, pCustomer->cCounterSerial, 0x07);
                        }
                        else
                        {
                        sSign[1]='\0';
                        sSign[0]=0x1F;
                        strcpy(sKeys, sSign);
                        strcat(sKeys, ":˚£‰ì");
                        strcat(sKeys,"  ");
                        sSign[0]=0x1E;
                        strcat(sKeys, sSign);
                        strcat(sKeys, ":¸ÒìÏ");
                        BiosPutStrMove(5, 3, 14, sKeys, 0x07);
                        }
//                                        BiosPutStrMove(7, 1, 18, "  SPACE=˚£‰ì†™ˆ•  ", 0x70);
                                        BiosPutStrMove(7, 1, 18, " SPACE=ˆÉõó©õ†™ˆ• ", 0x70);


                                                        break;
                                                case ' ':
                                                       fclose(fAddressCodes);
                                                        return -1;
                                                        break;
                                                }                        
                                }

                }while(!iFound);

               fclose(fAddressCodes);
               if(iFound)
               {
                return 5;
               }
        return -1;
}

void GetOpCode(char *sInput)
{
        int iCurrentInput;
        unsigned int ch;
        unsigned char row;
        unsigned char col;
        int iEntered=0;
        iCurrentInput=0;
        BiosClrScr(0x07);
        DrawBox(0, 0, 6, 19,0x07);
        BiosPutStrMove(0, 3, 14, ":: •ì•åÌ £Ì ::", 0x70);
        BiosPutStrMove(3, 2, 16, "      ---       ", 0x07);
        BiosSetCursorPos(3, 8);
        BiosShowCursor();
        do
        {
                ch=BiosGetChar();
                switch(ch)
                {
                        case 0x1C0D://enter
                                if(iCurrentInput==3)
                                {
                                        sInput[3]='\0';
                                        iEntered=1;
                                }
                                break;
                         default:
                                switch((char)ch)
                                {
                                 case '0':
                                 case '1':
                                 case '2':
                                 case '3':
                                 case '4':
                                 case '5':
                                 case '6':
                                 case '7':
                                 case '8':
                                 case '9':
                                 {
                                        if(iCurrentInput<3)
                                        {
                                                sInput[iCurrentInput]=(char)ch;
                                                //ch='*';
                                                putch(ch);                                        
                                                iCurrentInput++;

                                        }
                                        else
                                                BiosBeep(500);
                                 break;
                                 }
                                 default:
                                        BiosBeep(500);
                                }
                }
        }
        while(iEntered!=1);
        BiosHideCursor();
}

void GetOpPassCode(char *sInput)
{
        int iCurrentInput;
        unsigned int ch;
        unsigned char row;
        unsigned char col;
        int iEntered=0;
        iCurrentInput=0;
        BiosClrScr(0x07);
        DrawBox(0, 0, 6, 19,0x07);
        BiosPutStrMove(0, 3, 14, ":: £ˆ•ˆ ¶Û• ::", 0x70);
        BiosPutStrMove(3, 2, 16, "      ----      ", 0x07);
        BiosSetCursorPos(3, 8);
        BiosShowCursor();
        do
        {
                ch=BiosGetChar();
                switch(ch)
                {
                        case 0x1C0D://enter
                                if(iCurrentInput==4)
                                {
                                        sInput[4]='\0';
                                        iEntered=1;
                                }
                                break;
                         default:
                                switch((char)ch)
                                {
                                 case '0':
                                 case '1':
                                 case '2':
                                 case '3':
                                 case '4':
                                 case '5':
                                 case '6':
                                 case '7':
                                 case '8':
                                 case '9':
                                 {
                                        if(iCurrentInput<4)
                                        {
                                                sInput[iCurrentInput]=(char)ch;
                                                ch='*';
                                                putch(ch);                                        
                                                iCurrentInput++;

                                        }
                                        else
                                                BiosBeep(500);
                                 break;
                                 }
                                 default:
                                        BiosBeep(500);
                                }
                }
        }
        while(iEntered!=1);
        BiosHideCursor();
}


int CheckOperator()
{
        char sOpCode[5];
        char sOpPassCode[5];
        int iOpCode;
        int iOpPassCode;
        int i;

        if(iMustDoItAReader)
        {
             GetOpCode(sOpCode);
             GetOpPassCode(sOpPassCode);
             iOpCode=atoi(sOpCode);
             iOpPassCode=atoi(sOpPassCode);

             for(i=0; i<iReadersNum;i++)
             {
                if(cReaderCodes[i]==iOpCode)
                if(iReaderPasses[i]==iOpPassCode)
                {
                        cActiveReaderCode=(char)iOpCode;
                        return 1;
                }
             }
                return 0;
        }
        return 1;
}


int DoSearchMethodMenu()
{
        int iRet;
        int iSel;
        int iOld;
        unsigned int ch;

        BiosClrScr(0x07);
        BiosHideCursor();
        BiosSetCursorPos(0, 0);


        iRet=-2;
        iSel=iSearchMethod;

        DrawBox(0, 0, 7, 19,0x07);
        BiosPutStrMove(0, 3,14, "::ˆõó©õ †™ˆ•::", 0x07);
        ShowSearchMethodMenuItem(0, 0);
        ShowSearchMethodMenuItem(1, 0);
        ShowSearchMethodMenuItem(2, 0);
        ShowSearchMethodMenuItem(3, 0);
        ShowSearchMethodMenuItem(4, 0);
        ShowSearchMethodMenuItem(5, 0);
        ShowSearchMethodMenuItem(iSel, 1);
        do
        {
                ch=BiosGetChar();
                switch(ch)
                {
                        case 0x1C0D://enter
                                iRet=iSel;
                                break;
                        case 0x4800://up
                                {
                                iOld=iSel;
                                if(iSel==0)
                                {
                                        iSel=5;
                                }
                                else
                                {
                                        iSel--;
                                }
                                ShowSearchMethodMenuItem(iOld, 0);
                                ShowSearchMethodMenuItem(iSel, 1);
                                break;
                                }
                        case 0x5000://down
                                {
                                iOld=iSel;
                                if(iSel==5)
                                {
                                        iSel=0;
                                }
                                else
                                {
                                        iSel++;
                                }
                                ShowSearchMethodMenuItem(iOld, 0);
                                ShowSearchMethodMenuItem(iSel, 1);
                                break;
                                }
                         default:
                                BiosBeep(500);
                }
        }
        while(iRet==-2);
        return iRet;
}

void ShowSearchMethodMenuItem(int ItemNum, int HighLight)
{
        int iMod;
        int MenuPos[]={1, 2, 3, 4, 5, 6};
        char MenuTitle[][19]={"    £ÉÌ•åìÙÌ©ã    ", "   êÌã•ó´ã˜•åÛ´   ", "   •ˆóıÌÔåÉÉ˛•©   " ,"   •˝© ‡¢êí˝ó•ó   " ,"      åÛı˙ã•      ", "¸ÉÒ≠ã˚ˆıÛ¯ìêñ´Ó¶åì"};
        iMod=0x07;                                                                        
        if(HighLight)  iMod=0x70;
        BiosPutStrMove(MenuPos[ItemNum], 1, 18, MenuTitle[ItemNum], iMod);
}


int GoNext()
{


        FILE* fAddressCodes;

        long iWhere;
        int iStartIndex;
        int iEndIndex;
        int iOldStart;
        int iOldEnd;
        int iResult;
        char sAddressCode[11];
        long lIndex;
        int i,j,k;
        int iFound;
        char sKeys[50];
        char sSign[10];
        long lLastRecord;
        

        unsigned int ch;

        char cAddress[46];
        char cAddressPart[19];



        iFound=0;
        BiosHideCursor();
        if(strcmp(cLastReadAddressCode, "aaaaaaaaaa")==0)
        {
                return 0;
        }

        fAddressCodes=fopen("D:\\ADRSCDS.DAT", "r");
        if(!fAddressCodes)
                return 0;


               iStartIndex=0;
               iEndIndex=iCustomersNum-1;

               iOldStart=iStartIndex;
               iOldEnd=iEndIndex;

               iWhere=iEndIndex;

               fseek(fAddressCodes, iWhere*ADDRESCODESIZE, SEEK_SET);
               fread(sAddressCode, sizeof(char), 10, fAddressCodes);
               sAddressCode[10]='\0';

               iResult=50;
               iResult=strcmp(sAddressCode, cLastReadAddressCode);

               if(iResult!=0)
                     iWhere=iCustomersNum/2;
               do
                {

                        if(iResult!=0)
                        {
                        fseek(fAddressCodes, iWhere*ADDRESCODESIZE, SEEK_SET);
                        fread(sAddressCode, sizeof(char), 10, fAddressCodes);
                        sAddressCode[10]='\0';

                        iResult=strcmp(sAddressCode, cLastReadAddressCode);
                        }
                        if(iResult==0)
                        {//iResult
                                if(iWhere<iCustomersNum)
                                        iWhere++;
                                fseek(fAddressCodes, iWhere*ADDRESCODESIZE, SEEK_SET);
                                fread(cLastReadAddressCode, sizeof(char), 10, fAddressCodes);
                                fclose(fAddressCodes);
                                return 1;
                                break;
                        }
                        else
                        if(iResult<0)
                        {
                                iOldStart=iStartIndex;
                                iOldEnd=iEndIndex;
                                iStartIndex=iWhere;
                                iWhere=(iEndIndex-iStartIndex)/2+iStartIndex;
                        }
                        else
                        {
                                iOldStart=iStartIndex;
                                iOldEnd=iEndIndex;
                                iEndIndex=iWhere;
                                iWhere=(iEndIndex-iStartIndex)/2+iStartIndex;
                        }                

                }
                while( (iStartIndex<iEndIndex) && ( !((iOldStart==iStartIndex) && (iOldEnd==iEndIndex)) )  );
                fclose(fAddressCodes);
                return 0;
}


int GetAddressCode(char *sInput)
{
        int iCurrentInput;
        unsigned int ch;
        unsigned char row;
        unsigned char col;

        int iEntered=0;
        iCurrentInput=0;
        BiosClrScr(0x07);
        DrawBox(0, 0, 6, 19,0x07);
        BiosPutStrMove(0, 5, 10, "::ˆÉõó©õ::", 0x07);
        BiosPutStrMove(7, 1, 18,"      £˝˛åÉó      ", 0x70);
        BiosPutStrMove(2, 4, 12, "  :†®•£â£Ì  ", 0x07);
        BiosPutStrMove(3, 5, 10, "----------", 0x07);
        BiosSetCursorPos(3, 5);
        BiosShowCursor();
        do
        {
                ch=BiosGetChar();
                switch(ch)
                {
                        case 0x1C0D://enter
                                while(iCurrentInput<10)
                                        sInput[iCurrentInput++]=' ';
                                sInput[10]='\0';
                                iEntered=1;
                                break;
                         default:
                                switch((char)ch)
                                {
                                 case ' ':
                                        return 0;
                                 break;
                                 case '0':
                                 case '1':
                                 case '2':
                                 case '3':
                                 case '4':
                                 case '5':
                                 case '6':
                                 case '7':
                                 case '8':
                                 case '9':
                                 {
                                        if(iCurrentInput<10)
                                        {
                                                putch(ch);                                        
                                                sInput[iCurrentInput]=(char)ch;
                                                iCurrentInput++;

                                        }
                                        else
                                                BiosBeep(500);
                                 break;
                                 }
                                 default:
                                        BiosBeep(500);
                                }
                }
        }
        while(iEntered!=1);
        BiosHideCursor();
        return 1;
}

void FindAddressCode(char *sMyAddressCode, struct customer* pCustomer)
{

        FILE* fAddressCodes;

        char cName[23];
        long iWhere;
        int iStartIndex;
        int iEndIndex;
        int iOldStart;
        int iOldEnd;
        int iResult;
        char sAddressCode[11];
        long lIndex;
        int i,j,k;
        int iFound;
        char sKeys[50];
        char sSign[10];
        long lLastRecord;
        char sSearching[11];
        

        unsigned int ch;

        char cAddress[46];
        char cAddressPart[19];

        char cPreviousDate[9];

        iFound=-1;
        fAddressCodes=fopen("D:\\ADRSCDS.DAT", "r");
        if(!fAddressCodes)
                return;


               iStartIndex=0;
               iEndIndex=iCustomersNum-1;

               iOldStart=iStartIndex;
               iOldEnd=iEndIndex;

               iWhere=iEndIndex;

               fseek(fAddressCodes, iWhere*ADDRESCODESIZE, SEEK_SET);
               fread(sAddressCode, sizeof(char), 10, fAddressCodes);
               sAddressCode[10]='\0';

               iResult=50;
               iResult=strcmp(sAddressCode, sMyAddressCode);

               if(iResult!=0)
                     iWhere=iCustomersNum/2;
               do
                {

                        if(iResult!=0)
                        {
                        fseek(fAddressCodes, iWhere*ADDRESCODESIZE, SEEK_SET);
                        fread(sAddressCode, sizeof(char), 10, fAddressCodes);
                        sAddressCode[10]='\0';

                        iResult=strcmp(sAddressCode, sMyAddressCode);
                        }
                        if(iResult==0)
                        {//iResult
                                strcpy(cLastReadAddressCode, sAddressCode);
                                iFound=0;
                                break;
                        }
                        else
                        if(iResult<0)
                        {
                                iOldStart=iStartIndex;
                                iOldEnd=iEndIndex;
                                iStartIndex=iWhere;
                                iWhere=(iEndIndex-iStartIndex)/2+iStartIndex;
                        }
                        else
                        {
                                iOldStart=iStartIndex;
                                iOldEnd=iEndIndex;
                                iEndIndex=iWhere;
                                iWhere=(iEndIndex-iStartIndex)/2+iStartIndex;
                        }                

                }
                while( (iStartIndex<iEndIndex) && ( !((iOldStart==iStartIndex) && (iOldEnd==iEndIndex)) )  );

        if(iFound<0)
        {
                BiosClrScr(0x07);
                DrawBox(0, 0, 6, 19,0x07);
                BiosPutStrMove(7, 1, 18, "    £˝ı¶ì˚£˝ÒÌ    ", 0x70);
                BiosPutStrMove(3, 1, 18, "†®•£â£ÌÙ˛ãåì¸Ì•ó´Û", 0x07);
                BiosPutStrMove(4, 1, 18, "     !£´ıêñÍå˛    ", 0x07);
                BiosBeep(1000);
                BiosGetChar();
        }

          fclose(fAddressCodes);
}

void ShowHelp()
{
      char sSign[2];
      char sKeys[18];
      BiosClrScr(0x07);
      DrawBox(0, 0, 6, 19,0x07);
      BiosPutStrMove(0, 5, 10, "::åÛı˙ã•::", 0x07);
      BiosPutStrMove(2, 1, 18, "0£˝ÒÌ       :¸ıå´ı", 0x07);
      BiosPutStrMove(3, 1, 18, "1£˝ÒÌ   :¸ÒìÏ°˛•åó", 0x07);
      BiosPutStrMove(4, 1, 18, "2£˝ÒÌ:†®•£â£Ìˆõó©õ", 0x07);

      BiosPutStrMove(7, 1, 18, "   !£˝ı¶ì ˚£˝ÒÌ   ", 0x70);

      BiosGetChar();

      sSign[1]='\0';
      sSign[0]=0x1F;
      strcpy(sKeys, "SHIFT+");
      strcat(sKeys, sSign);
      strcat(sKeys, "  :˚£‰ì¯‰‡Ï");
      BiosPutStrMove(1, 1, 18, sKeys, 0x07);

      sSign[1]='\0';
      sSign[0]=0x1E;
      strcpy(sKeys, "SHIFT+");
      strcat(sKeys, sSign);
      strcat(sKeys, "  :¸ÒìÏ¯‰‡Ï");
      BiosPutStrMove(2, 1, 18, sKeys, 0x07);

      sSign[1]='\0';
      sSign[0]=0x10;
      strcpy(sKeys, sSign);
      strcat(sKeys, "      ");
      strcat(sKeys, " :˚£‰ì˜£˛•ï");
      BiosPutStrMove(3, 1, 18, sKeys, 0x07);

      sSign[1]='\0';
      sSign[0]=0x11;
      strcpy(sKeys, sSign);
      strcat(sKeys, "      ");
      strcat(sKeys, " :¸ÒìÏ˜£˛•ï");
      BiosPutStrMove(4, 1, 18, sKeys, 0x07);

      sSign[1]='\0';
      sSign[0]=0x1F;
      strcpy(sKeys, sSign);
      strcat(sKeys, ":˚£‰ì");
      strcat(sKeys,"  ");
      sSign[0]=0x1E;
      strcat(sKeys, sSign);
      strcat(sKeys, ":¸ÒìÏ");
      BiosPutStrMove(5, 3, 14, sKeys, 0x07);


      BiosPutStrMove(7, 1, 18, "   !£˝ı¶ì ˚£˝ÒÌ   ", 0x70);

      BiosGetChar();

}

int IsThisCustomerRead(char *cMySub)
{
        char cSub[16];
        char b110[110];
        int iNewSearch;

                        fseek(fOut, 0, SEEK_SET);
                        while((fread(cSub, sizeof(char), 15, fOut)==15))
                        {
                        cSub[15]='\0';
                        if(strcmp(cMySub, cSub)==0)
                        {
                                return 1;
                        }
                        else
                                fread(b110, sizeof(char), 23, fOut);
                        }
        return 0;
}

void DoBackLightTimeOutSelect()
{
        int iRet;
        int iSel;
        int iOld;
        unsigned int ch;

        BiosGetAbortStatus();

        BiosClrScr(0x07);
        BiosHideCursor();
        BiosSetCursorPos(0, 0);


        iRet=-2;
        iSel=0;

        DrawBox(0, 0, 6, 19,0x07);
        BiosPutStrMove(0, 3,14, "::¸´ˆÛå¢ÙåÛ¶::", 0x07);
        BiosPutStrMove(7, 1, 18, "   CLEAR= ÔåÛÂã   ", 0x70);
        ShowBackLightMenuItem(0, 1, iBackLightTimeOut==0);
        ShowBackLightMenuItem(1, 0, iBackLightTimeOut==1);
        ShowBackLightMenuItem(2, 0, iBackLightTimeOut==2);
        ShowBackLightMenuItem(3, 0, iBackLightTimeOut==3);
        ShowBackLightMenuItem(4, 0, iBackLightTimeOut==4);
        do
        {
                ch=BiosGetChar();
                switch(ch)
                {
                        case 0x1C0D://enter
                                ShowBackLightMenuItem(iBackLightTimeOut, 0, 0);
                                iBackLightTimeOut=iSel;
                                ShowBackLightMenuItem(iBackLightTimeOut, 1, 1);
                                //iRet=iSel;
                                break;
                        case 0x4800://up
                                {
                                iOld=iSel;
                                if(iSel==0)
                                {
                                        iSel=4;
                                }
                                else
                                {
                                        iSel--;
                                }
                                ShowBackLightMenuItem(iOld, 0, iBackLightTimeOut==iOld);
                                ShowBackLightMenuItem(iSel, 1, iBackLightTimeOut==iSel);
                                break;
                                }
                        case 0x5000://down
                                {
                                iOld=iSel;
                                if(iSel==4)
                                {
                                        iSel=0;
                                }
                                else
                                {
                                        iSel++;
                                }
                                ShowBackLightMenuItem(iOld, 0, iBackLightTimeOut==iOld);
                                ShowBackLightMenuItem(iSel, 1, iBackLightTimeOut==iSel);
                                break;
                                }
                         default:
                                if(BiosGetAbortStatus())
                                        iRet=5;
                                else
                                        BiosBeep(500);
                }
        }
        while(iRet==-2);
}


void ShowBackLightMenuItem(int ItemNum, int HighLight, int Checked)
{
        int iMod;
        int MenuPos[]={1, 2, 3, 4, 5};
        char sItems[][18]={"         ¯˝ıåô 30", "         ¯˝ıåô 40", "         ¯˝ıåô 60", "        ¯˝ıåô 100", "        ¯˝ıåô 200"};
        char sChecked[2];
        sChecked[0]=0x24;
        sChecked[1]='\0';
        iMod=0x07;
        if(HighLight)  iMod=0x70;
        if(Checked)
                sChecked[0]=0x23;
        BiosPutStrMove(MenuPos[ItemNum], 2, 17, sItems[ItemNum], iMod);
        BiosPutStrMove(MenuPos[ItemNum], 1, 1, sChecked, iMod);
}

