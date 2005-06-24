/*
filefilter.cpp

�������� ������

*/

/* Revision: 1.09 24.06.2005 $ */

/*
Modify:
  24.06.2005 WARP
    ! ����� ������ ������� �������.
  24.04.2005 AY
    ! GCC
  23.04.2005 KM
    ! ����������� ��� ������� ������������� �������� Directory �� �����������
  01.11.2004 SVS
    - ��� ���������� ������
  25.10.2004 SVS
    - "������ 0 - ������������ ��������� - ������������ "�����"."
  06.08.2004 SKV
    ! see 01825.MSVCRT.txt
  14.06.2004 KM
    + ��������� ��������� �������� FILE_ATTRIBUTE_DIRECTORY
    ! ������� BSTATE_UNCHECKED �� BSTATE_3STATE ��� ������ ��������.
  08.12.2003 SVS
    + ������� � Mb, Gb
    + ������ Reset
    ! �������� ������������� (����� ���� ������� ����� :-)
  11.10.2003 KM
    ! ��������� �������� ���������� ������.
  04.10.2003 KM
    ! �������� � ����� ������� ��������. ������ ����� ��� :-)
*/

#include "headers.hpp"
#pragma hdrstop

#include "filefilter.hpp"
#include "fn.hpp"
#include "lang.hpp"
#include "dialog.hpp"
#include "global.hpp"

static DWORD CompEnabled,EncrEnabled;
static DWORD SizeType,DateType;

// ������ �� ������������� �������� Directory ��� ������������
static int DisableDir=FALSE;

FileFilter::FileFilter(int DisableDirAttr):
  FmtMask1("99%c99%c9999"),
  FmtMask2("9999%c99%c99"),
  FmtMask3("99%c99%c99"),
  DigitMask("99999999999999999999"),
  FilterMasksHistoryName("FilterMasks")
{
  int I;

  // �������� ������������� �������� Directory ��� ������������
  DisableDir=DisableDirAttr;

  // ����������� ���������� ���� � ������� � �������.
  DateSeparator=GetDateSeparator();
  TimeSeparator=GetTimeSeparator();
  DateFormat=GetDateFormat();

  switch(DateFormat)
  {
    case 0:
      sprintf(DateMask,FmtMask1,DateSeparator,DateSeparator);
      break;
    case 1:
      sprintf(DateMask,FmtMask1,DateSeparator,DateSeparator);
      break;
    default:
      sprintf(DateMask,FmtMask2,DateSeparator,DateSeparator);
      break;
  }
  sprintf(TimeMask,FmtMask3,TimeSeparator,TimeSeparator);

  // ��������� ������ ���������� ��� ���� �������
  TableItemSize=(FarListItem *)xf_malloc(sizeof(FarListItem)*FSIZE_IN_LAST);
  SizeList.Items=TableItemSize;
  SizeList.ItemsNumber=FSIZE_IN_LAST;

  memset(TableItemSize,0,sizeof(FarListItem)*FSIZE_IN_LAST);
  for(I=0; I < FSIZE_IN_LAST; ++I)
    xstrncpy(TableItemSize[I].Text,MSG(MFileFilterSizeInBytes+I),sizeof(TableItemSize[I].Text)-1);

  // ��������� ������ ����� ��� �����
  TableItemDate=(FarListItem *)xf_malloc(sizeof(FarListItem)*DATE_COUNT);
  DateList.Items=TableItemDate;
  DateList.ItemsNumber=DATE_COUNT;

  memset(TableItemDate,0,sizeof(FarListItem)*DATE_COUNT);
  xstrncpy(TableItemDate[0].Text,MSG(MFileFilterModified),sizeof(TableItemDate[0].Text)-1);
  xstrncpy(TableItemDate[1].Text,MSG(MFileFilterCreated),sizeof(TableItemDate[1].Text)-1);
  xstrncpy(TableItemDate[2].Text,MSG(MFileFilterOpened),sizeof(TableItemDate[2].Text)-1);

  // ��� ������������ ���������� � ��������?
  unsigned long FSFlags=0;
  if (GetVolumeInformation(NULL,NULL,0,NULL,NULL,&FSFlags,NULL,0))
  {
    if (FSFlags & FS_FILE_COMPRESSION)
      CompEnabled=TRUE;
    else
      CompEnabled=FALSE;

    if ((IsCryptFileASupport) && (FSFlags & FS_FILE_ENCRYPTION))
      EncrEnabled=TRUE;
    else
      EncrEnabled=FALSE;
  }

  // ��������� ������� ��������� ������� �� ��������� ��������,
  // � ������� � ����� ��������
  FF=Opt.OpFilter;

  // �������� �� ���������� ������� �������� �������
  if ((*FF.FMask.Mask==0) || (!FilterMask.Set(FF.FMask.Mask,FMF_SILENT)))
    xstrncpy(FF.FMask.Mask,"*.*",sizeof(FF.FMask.Mask));

  // �������� ����� ������� � ����� ������ ��� ��������� �������� �������� �����.
  FilterMask.Set(FF.FMask.Mask,FMF_SILENT);

  SizeType=FF.FSize.SizeType;
  if (SizeType>FSIZE_INMBYTES || SizeType<FSIZE_INBYTES)
  {
    SizeType=0;
    FF.FSize.SizeType=(FSizeType) SizeType;
  }

  DateType=FF.FDate.DateType;
  if (DateType>FDATE_OPENED || DateType<FDATE_MODIFIED)
  {
    DateType=0;
    FF.FDate.DateType=(FDateType) DateType;
  }

}

FileFilter::~FileFilter()
{
  xf_free(TableItemSize);
  xf_free(TableItemDate);

  // �������� ������� ��������� � ���������� �������
  Opt.OpFilter=FF;
}

enum enumFileFilterConfigure {
    ID_FF_TITLE,

    ID_FF_MATCHMASK,
    ID_FF_MASKEDIT,

    ID_FF_SEPARATOR1,

    ID_FF_MATCHSIZE,
    ID_FF_SIZEDIVIDER,
    ID_FF_SIZEFROM,
    ID_FF_SIZEFROMEDIT,
    ID_FF_SIZETO,
    ID_FF_SIZETOEDIT,

    ID_FF_SEPARATOR2,

    ID_FF_MATCHDATE,
    ID_FF_DATETYPE,
    ID_FF_DATEAFTER,
    ID_FF_DATEAFTEREDIT,
    ID_FF_TIMEAFTEREDIT,
    ID_FF_DATEBEFORE,
    ID_FF_DATEBEFOREEDIT,
    ID_FF_TIMEBEFOREEDIT,
    ID_FF_CURRENT,
    ID_FF_BLANK,

    ID_FF_SEPARATOR3,

    ID_FF_MATCHATTRIBUTES,
    ID_FF_READONLY,
    ID_FF_ARCHIVE,
    ID_FF_HIDDEN,
    ID_FF_SYSTEM,
    ID_FF_COMPRESSED,
    ID_FF_ENCRYPTED,
    ID_FF_DIRECTORY,

    ID_FF_SEPARATOR4,

    ID_FF_OK,
    ID_FF_RESET,
    ID_FF_CANCEL
    };


long WINAPI FileFilter::FilterDlgProc(HANDLE hDlg,int Msg,int Param1,long Param2)
{
  switch(Msg)
  {
    case DN_LISTCHANGE:
    {
      if (Param1==ID_FF_SIZEDIVIDER)  // In bytes or kilobytes?
        SizeType=Param2;
      if (Param1==ID_FF_DATETYPE) // Modified,created or accessed?
        DateType=Param2;
      return TRUE;
    }

    case DN_BTNCLICK:
    {
      if (Param1==ID_FF_MATCHATTRIBUTES) // Attributes
      {
        Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,FALSE,0);

        if (Param2)
        {
          if (CompEnabled==FALSE)
            Dialog::SendDlgMessage(hDlg,DM_ENABLE,ID_FF_COMPRESSED,FALSE);
          else
            Dialog::SendDlgMessage(hDlg,DM_ENABLE,ID_FF_COMPRESSED,TRUE);

          if (EncrEnabled==FALSE)
            Dialog::SendDlgMessage(hDlg,DM_ENABLE,ID_FF_ENCRYPTED,FALSE);
          else
            Dialog::SendDlgMessage(hDlg,DM_ENABLE,ID_FF_ENCRYPTED,TRUE);
        }

        if (DisableDir)
        {
          Dialog::SendDlgMessage(hDlg,DM_ENABLE,ID_FF_DIRECTORY,FALSE);
        }

        Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,TRUE,0);
      }
      if (Param1==ID_FF_CURRENT || Param1==ID_FF_BLANK) //Current � Blank
      {
        FILETIME ft;
        char Date[16],Time[16];

        if (Param1==ID_FF_CURRENT)
        {
          GetSystemTimeAsFileTime(&ft);
          ConvertDate(ft,Date,Time,8,FALSE,FALSE,TRUE);
        }
        else
          *Date=*Time=0;

        Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,FALSE,0);

        Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,ID_FF_DATEAFTEREDIT,(long)Date);
        Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,ID_FF_TIMEAFTEREDIT,(long)Time);
        Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,ID_FF_DATEBEFOREEDIT,(long)Date);
        Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,ID_FF_TIMEBEFOREEDIT,(long)Time);

        Dialog::SendDlgMessage(hDlg,DM_SETFOCUS,ID_FF_DATEAFTEREDIT,0);
        COORD r;
        r.X=r.Y=0;
        Dialog::SendDlgMessage(hDlg,DM_SETCURSORPOS,ID_FF_DATEAFTEREDIT,(long)&r);

        Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,TRUE,0);
      }
      if (Param1==ID_FF_OK || Param1==ID_FF_CANCEL) // Ok � Cancel
        return FALSE;

      if (Param1==ID_FF_RESET) // Reset
      {
        // ������� �������
        Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,FALSE,0);

        Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,ID_FF_MASKEDIT,(long)"*.*");
        Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,ID_FF_SIZEFROMEDIT,(long)"");
        Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,ID_FF_SIZETOEDIT,(long)"");
        Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,ID_FF_DATEAFTEREDIT,(long)"");
        Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,ID_FF_TIMEAFTEREDIT,(long)"");
        Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,ID_FF_DATEBEFOREEDIT,(long)"");
        Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,ID_FF_TIMEBEFOREEDIT,(long)"");

        /* 14.06.2004 KM
           ������� BSTATE_UNCHECKED �� BSTATE_3STATE, � ������
           ������ ��� ����� ��������, �.�. ��������� ��������
        */
        for(int I=ID_FF_READONLY; I <= ID_FF_DIRECTORY; ++I)
          Dialog::SendDlgMessage(hDlg,DM_SETCHECK,I,BSTATE_3STATE);

        // 6, 13 - ������� � ������
        struct FarListPos LPos={0,0};
        SizeType=0;
        Dialog::SendDlgMessage(hDlg,DM_LISTSETCURPOS,ID_FF_SIZEDIVIDER,(long)&LPos);
        DateType=0;
        Dialog::SendDlgMessage(hDlg,DM_LISTSETCURPOS,ID_FF_DATETYPE,(long)&LPos);

        Dialog::SendDlgMessage(hDlg,DM_SETCHECK,ID_FF_MATCHMASK,BSTATE_UNCHECKED);
        Dialog::SendDlgMessage(hDlg,DM_SETCHECK,ID_FF_MATCHSIZE,BSTATE_UNCHECKED);
        Dialog::SendDlgMessage(hDlg,DM_SETCHECK,ID_FF_MATCHDATE,BSTATE_UNCHECKED);
        Dialog::SendDlgMessage(hDlg,DM_SETCHECK,ID_FF_MATCHATTRIBUTES,BSTATE_UNCHECKED);

        Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,TRUE,0);
        break;;
      }
      return TRUE;
    }
  }
  return Dialog::DefDlgProc(hDlg,Msg,Param1,Param2);
}


void FileFilter::Configure()
{
/*
    0000000000111111111122222222223333333333444444444455555555556666666
    0123456789012345678901234567890123456789012345678901234567890123456
00  """""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
01  "  +-------------------- Operations filter --------------------+  "
02  "  | +-[ ] Match file mask(s) -------------------------------+ |  "
03  "  | | *.*################################################## | |  "
04  "  | +-------------------------------------------------------+ |  "
05  "  | +-[ ] File size-----------------------------------------+ |  "
06  "  | | In bytes#################                             | |  "
07  "  | | Greater than or equal to:        #################### | |  "
08  "  | | Less than or equal to:           #################### | |  "
09  "  | +-------------------------------------------------------+ |  "
10  "  | +-[ ] File date-----------------------------------------+ |  "
11  "  | | Last modified time#######                             | |  "
12  "  | | After:                            ##.##.#### ##:##:## | |  "
13  "  | | Before:                           ##.##.#### ##:##:## | |  "
14  "  | | [ Current ] [ Blank ]                                 | |  "
15  "  | +-------------------------------------------------------+ |  "
16  "  | +-[ ] Attributes----------------------------------------+ |  "
17  "  | | [x] ������ ������ [x] �������       [x] ������        | |  "
18  "  | | [x] ��������      [x] ���������     [x] ������������� | |  "
19  "  | | [x] Directory                                         | |  "
20  "  | +-------------------------------------------------------+ |  "
21  "  |                [ Ok ]  [ Reset]  [ Cancel ]               |  "
22  "  +-----------------------------------------------------------+  "
23  "                                                                 "
24  """""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
*/

  // ��������� �����.
  CFileMask FileMask;
  int I;

  struct DialogData FilterDlgData[]={
  /* 00 */DI_DOUBLEBOX,3,1,61,22,0,0,DIF_SHOWAMPERSAND,0,(char *)MFileFilterTitle,

  /* 01 */DI_CHECKBOX,5,2,0,0,1,0,DIF_AUTOMATION,0,(char *)MFileFilterMatchMask,
  /* 02 */DI_EDIT,7,3,59,3,0,(DWORD)FilterMasksHistoryName,DIF_HISTORY,0,"",

  /* 03 */DI_TEXT,0,4,0,0,0,0,DIF_SEPARATOR,0,"",

  /* 04 */DI_CHECKBOX,5,5,0,0,0,0,DIF_AUTOMATION,0,(char *)MFileFilterSize,
  /* 05 */DI_COMBOBOX,7,6,31,6,0,0,DIF_DROPDOWNLIST|DIF_LISTNOAMPERSAND,0,"",
  /* 06 */DI_TEXT,7,7,38,7,0,0,0,0,(char *)MFileFilterSizeFrom,
  /* 07 */DI_FIXEDIT,40,7,59,7,0,(DWORD)DigitMask,DIF_MASKEDIT,0,"",
  /* 08 */DI_TEXT,7,8,38,8,0,0,0,0,(char *)MFileFilterSizeTo,
  /* 09 */DI_FIXEDIT,40,8,59,8,0,(DWORD)DigitMask,DIF_MASKEDIT,0,"",

  /* 10 */DI_TEXT,0,9,0,0,0,0,DIF_SEPARATOR,0,"",

  /* 11 */DI_CHECKBOX,5,10,0,0,0,0,DIF_AUTOMATION,0,(char *)MFileFilterDate,
  /* 12 */DI_COMBOBOX,7,11,31,11,0,0,DIF_DROPDOWNLIST|DIF_LISTNOAMPERSAND,0,"",
  /* 13 */DI_TEXT,7,12,38,12,0,0,0,0,(char *)MFileFilterAfter,
  /* 14 */DI_FIXEDIT,41,12,50,12,0,(DWORD)DateMask,DIF_MASKEDIT,0,"",
  /* 15 */DI_FIXEDIT,52,12,59,12,0,(DWORD)TimeMask,DIF_MASKEDIT,0,"",
  /* 16 */DI_TEXT,7,13,40,13,0,0,0,0,(char *)MFileFilterBefore,
  /* 17 */DI_FIXEDIT,41,13,50,13,0,(DWORD)DateMask,DIF_MASKEDIT,0,"",
  /* 18 */DI_FIXEDIT,52,13,59,13,0,(DWORD)TimeMask,DIF_MASKEDIT,0,"",
  /* 19 */DI_BUTTON,7,14,0,14,0,0,DIF_BTNNOCLOSE,0,(char *)MFileFilterCurrent,
  /* 20 */DI_BUTTON,19,14,0,14,0,0,DIF_BTNNOCLOSE,0,(char *)MFileFilterBlank,

  /* 21 */DI_TEXT,0,15,0,0,0,0,DIF_SEPARATOR,0,"",

  /* 22 */DI_CHECKBOX, 5,16,0,0,0,0,DIF_AUTOMATION,0,(char *)MFileFilterAttr,
  /* 23 */DI_CHECKBOX, 7,17,0,0,0,0,DIF_3STATE,0,(char *)MFileFilterAttrR,
  /* 24 */DI_CHECKBOX, 7,18,0,0,0,0,DIF_3STATE,0,(char *)MFileFilterAttrA,
  /* 25 */DI_CHECKBOX,25,17,0,0,0,0,DIF_3STATE,0,(char *)MFileFilterAttrH,
  /* 26 */DI_CHECKBOX,25,18,0,0,0,0,DIF_3STATE,0,(char *)MFileFilterAttrS,
  /* 27 */DI_CHECKBOX,43,17,0,0,0,0,DIF_3STATE,0,(char *)MFileFilterAttrC,
  /* 28 */DI_CHECKBOX,43,18,0,0,0,0,DIF_3STATE,0,(char *)MFileFilterAttrE,
  /* 29 */DI_CHECKBOX, 7,19,0,0,0,0,DIF_3STATE,0,(char *)MFileFilterAttrD,

  /* 30 */DI_TEXT, 0, 20, 0, 0, 0, 0, DIF_SEPARATOR, 0, "",

  /* 31 */DI_BUTTON,0,21,0,21,0,0,DIF_CENTERGROUP,1,(char *)MFileFilterOk,
  /* 32 */DI_BUTTON,0,21,0,21,0,0,DIF_CENTERGROUP|DIF_BTNNOCLOSE,0,(char *)MFileFilterReset,
  /* 33 */DI_BUTTON,0,21,0,21,0,0,DIF_CENTERGROUP,0,(char *)MFileFilterCancel,
  };

  MakeDialogItems(FilterDlgData,FilterDlg);

  FilterDlg[ID_FF_MATCHMASK].Selected=FF.FMask.Used;
  xstrncpy(FilterDlg[ID_FF_MASKEDIT].Data,FF.FMask.Mask,sizeof(FilterDlg[3].Data));
  if (!FilterDlg[ID_FF_MATCHMASK].Selected)
    FilterDlg[ID_FF_MASKEDIT].Flags|=DIF_DISABLE;

  FilterDlg[ID_FF_MATCHSIZE].Selected=FF.FSize.Used;
  FilterDlg[ID_FF_SIZEDIVIDER].ListItems=&SizeList;

  SizeType=FF.FSize.SizeType;

  xstrncpy(FilterDlg[ID_FF_SIZEDIVIDER].Data,TableItemSize[SizeType].Text,sizeof(FilterDlg[ID_FF_SIZEDIVIDER].Data));

  if (FF.FSize.SizeAbove != _i64(-1))
    _ui64toa(FF.FSize.SizeAbove,FilterDlg[ID_FF_SIZEFROMEDIT].Data,10);

  if (FF.FSize.SizeBelow != _i64(-1))
    _ui64toa(FF.FSize.SizeBelow,FilterDlg[ID_FF_SIZETOEDIT].Data,10);

  if (!FilterDlg[ID_FF_MATCHSIZE].Selected)
    for(I=ID_FF_SIZEDIVIDER; I <= ID_FF_SIZETOEDIT; ++I)
      FilterDlg[I].Flags|=DIF_DISABLE;

  FilterDlg[ID_FF_MATCHDATE].Selected=FF.FDate.Used;
  FilterDlg[ID_FF_DATETYPE].ListItems=&DateList;

  DateType=FF.FDate.DateType;

  xstrncpy(FilterDlg[ID_FF_DATETYPE].Data,TableItemDate[DateType].Text,sizeof(FilterDlg[ID_FF_DATETYPE].Data));

  ConvertDate(FF.FDate.DateAfter,FilterDlg[ID_FF_DATEAFTEREDIT].Data,FilterDlg[ID_FF_TIMEAFTEREDIT].Data,8,FALSE,FALSE,TRUE);
  ConvertDate(FF.FDate.DateBefore,FilterDlg[ID_FF_DATEBEFOREEDIT].Data,FilterDlg[ID_FF_TIMEBEFOREEDIT].Data,8,FALSE,FALSE,TRUE);

  if (!FilterDlg[ID_FF_MATCHDATE].Selected)
    for(I=ID_FF_DATETYPE; I <= ID_FF_BLANK; ++I)
      FilterDlg[I].Flags|=DIF_DISABLE;

  DWORD AttrSet=FF.FAttr.AttrSet;
  DWORD AttrClear=FF.FAttr.AttrClear;

  FilterDlg[ID_FF_MATCHATTRIBUTES].Selected=FF.FAttr.Used;
  FilterDlg[ID_FF_READONLY].Selected=(AttrSet & FILE_ATTRIBUTE_READONLY?1:AttrClear & FILE_ATTRIBUTE_READONLY?0:2);
  FilterDlg[ID_FF_ARCHIVE].Selected=(AttrSet & FILE_ATTRIBUTE_ARCHIVE?1:AttrClear & FILE_ATTRIBUTE_ARCHIVE?0:2);
  FilterDlg[ID_FF_HIDDEN].Selected=(AttrSet & FILE_ATTRIBUTE_HIDDEN?1:AttrClear & FILE_ATTRIBUTE_HIDDEN?0:2);
  FilterDlg[ID_FF_SYSTEM].Selected=(AttrSet & FILE_ATTRIBUTE_SYSTEM?1:AttrClear & FILE_ATTRIBUTE_SYSTEM?0:2);
  FilterDlg[ID_FF_COMPRESSED].Selected=(AttrSet & FILE_ATTRIBUTE_COMPRESSED?1:AttrClear & FILE_ATTRIBUTE_COMPRESSED?0:2);
  FilterDlg[ID_FF_ENCRYPTED].Selected=(AttrSet & FILE_ATTRIBUTE_ENCRYPTED?1:AttrClear & FILE_ATTRIBUTE_ENCRYPTED?0:2);
  FilterDlg[ID_FF_DIRECTORY].Selected=(AttrSet & FILE_ATTRIBUTE_DIRECTORY?1:AttrClear & FILE_ATTRIBUTE_DIRECTORY?0:2);

  if (!FilterDlg[ID_FF_MATCHATTRIBUTES].Selected)
  {
    for(I=ID_FF_READONLY; I <= ID_FF_DIRECTORY; ++I)
      FilterDlg[I].Flags|=DIF_DISABLE;
  }
  else
  {
    if (CompEnabled==FALSE)
    {
      FilterDlg[ID_FF_COMPRESSED].Flags|=DIF_DISABLE;
      FilterDlg[ID_FF_COMPRESSED].Selected=0;
    }
    if (EncrEnabled==FALSE)
    {
      FilterDlg[ID_FF_ENCRYPTED].Flags|=DIF_DISABLE;
      FilterDlg[ID_FF_ENCRYPTED].Selected=0;
    }
  }

  Dialog Dlg(FilterDlg,sizeof(FilterDlg)/sizeof(FilterDlg[0]),FilterDlgProc);

  Dlg.SetHelp("OpFilter");
  Dlg.SetPosition(-1,-1,65,24);

  Dlg.SetAutomation(ID_FF_MATCHMASK,ID_FF_MASKEDIT,DIF_DISABLE,0,0,DIF_DISABLE);

  Dlg.SetAutomation(ID_FF_MATCHSIZE,ID_FF_SIZEDIVIDER,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHSIZE,ID_FF_SIZEFROM,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHSIZE,ID_FF_SIZEFROMEDIT,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHSIZE,ID_FF_SIZETO,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHSIZE,ID_FF_SIZETOEDIT,DIF_DISABLE,0,0,DIF_DISABLE);

  Dlg.SetAutomation(ID_FF_MATCHDATE,ID_FF_DATETYPE,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHDATE,ID_FF_DATEAFTER,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHDATE,ID_FF_DATEAFTEREDIT,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHDATE,ID_FF_TIMEAFTEREDIT,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHDATE,ID_FF_DATEBEFORE,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHDATE,ID_FF_DATEBEFOREEDIT,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHDATE,ID_FF_TIMEBEFOREEDIT,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHDATE,ID_FF_CURRENT,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHDATE,ID_FF_BLANK,DIF_DISABLE,0,0,DIF_DISABLE);

  Dlg.SetAutomation(ID_FF_MATCHATTRIBUTES,ID_FF_READONLY,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHATTRIBUTES,ID_FF_ARCHIVE,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHATTRIBUTES,ID_FF_HIDDEN,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHATTRIBUTES,ID_FF_SYSTEM,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHATTRIBUTES,ID_FF_COMPRESSED,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHATTRIBUTES,ID_FF_ENCRYPTED,DIF_DISABLE,0,0,DIF_DISABLE);

  if (DisableDir)
    FilterDlg[ID_FF_DIRECTORY].Flags|=DIF_DISABLE;
  else
    Dlg.SetAutomation(ID_FF_MATCHATTRIBUTES,ID_FF_DIRECTORY,DIF_DISABLE,0,0,DIF_DISABLE);

  for (;;)
  {
    Dlg.ClearDone();
    Dlg.Process();
    int ExitCode=Dlg.GetExitCode();

    if (ExitCode==ID_FF_OK) // Ok
    {
      // ���� �������� ������������� ����� �� ���������, ����� �������� � ������
      if (FilterDlg[ID_FF_MATCHMASK].Selected && !FileMask.Set(FilterDlg[ID_FF_MASKEDIT].Data,0))
        continue;

      FF.FMask.Used=FilterDlg[ID_FF_MATCHMASK].Selected;
      xstrncpy(FF.FMask.Mask,FilterDlg[ID_FF_MASKEDIT].Data,sizeof(FF.FMask.Mask));

      // �������� ����� ������� � ����� ������ ��� ��������� �������� �������� �����.
      FilterMask.Set(FF.FMask.Mask,FMF_SILENT);

      FF.FSize.Used=FilterDlg[ID_FF_MATCHSIZE].Selected;
      FF.FSize.SizeType=(FSizeType) SizeType;
      if(!*RemoveExternalSpaces(FilterDlg[ID_FF_SIZEFROMEDIT].Data))
        FF.FSize.SizeAbove=_i64(-1);
      else
        FF.FSize.SizeAbove=_atoi64(FilterDlg[ID_FF_SIZEFROMEDIT].Data);

      if(!*RemoveExternalSpaces(FilterDlg[ID_FF_SIZETOEDIT].Data))
        FF.FSize.SizeBelow=_i64(-1);
      else
        FF.FSize.SizeBelow=_atoi64(FilterDlg[ID_FF_SIZETOEDIT].Data);

      FF.FDate.Used=FilterDlg[ID_FF_MATCHDATE].Selected;
      FF.FDate.DateType=(FDateType) DateType;
      StrToDateTime(FilterDlg[ID_FF_DATEAFTEREDIT].Data,FilterDlg[ID_FF_TIMEAFTEREDIT].Data,FF.FDate.DateAfter);
      StrToDateTime(FilterDlg[ID_FF_DATEBEFOREEDIT].Data,FilterDlg[ID_FF_TIMEBEFOREEDIT].Data,FF.FDate.DateBefore);

      DWORD AttrSet=0;
      DWORD AttrClear=0;

      FF.FAttr.Used=FilterDlg[ID_FF_MATCHATTRIBUTES].Selected;

      AttrSet|=(FilterDlg[ID_FF_READONLY].Selected==1?FILE_ATTRIBUTE_READONLY:0);
      AttrSet|=(FilterDlg[ID_FF_ARCHIVE].Selected==1?FILE_ATTRIBUTE_ARCHIVE:0);
      AttrSet|=(FilterDlg[ID_FF_HIDDEN].Selected==1?FILE_ATTRIBUTE_HIDDEN:0);
      AttrSet|=(FilterDlg[ID_FF_SYSTEM].Selected==1?FILE_ATTRIBUTE_SYSTEM:0);
      AttrSet|=(FilterDlg[ID_FF_COMPRESSED].Selected==1?FILE_ATTRIBUTE_COMPRESSED:0);
      AttrSet|=(FilterDlg[ID_FF_ENCRYPTED].Selected==1?FILE_ATTRIBUTE_ENCRYPTED:0);
      AttrSet|=(FilterDlg[ID_FF_DIRECTORY].Selected==1?FILE_ATTRIBUTE_DIRECTORY:0);
      AttrClear|=(FilterDlg[ID_FF_READONLY].Selected==0?FILE_ATTRIBUTE_READONLY:0);
      AttrClear|=(FilterDlg[ID_FF_ARCHIVE].Selected==0?FILE_ATTRIBUTE_ARCHIVE:0);
      AttrClear|=(FilterDlg[ID_FF_HIDDEN].Selected==0?FILE_ATTRIBUTE_HIDDEN:0);
      AttrClear|=(FilterDlg[ID_FF_SYSTEM].Selected==0?FILE_ATTRIBUTE_SYSTEM:0);
      AttrClear|=(FilterDlg[ID_FF_COMPRESSED].Selected==0?FILE_ATTRIBUTE_COMPRESSED:0);
      AttrClear|=(FilterDlg[ID_FF_ENCRYPTED].Selected==0?FILE_ATTRIBUTE_ENCRYPTED:0);
      AttrClear|=(FilterDlg[ID_FF_DIRECTORY].Selected==0?FILE_ATTRIBUTE_DIRECTORY:0);
      FF.FAttr.AttrSet=AttrSet;
      FF.FAttr.AttrClear=AttrClear;

      break;
    }
    else
      break;
  }
}

FILETIME &FileFilter::StrToDateTime(const char *CDate,const char *CTime,FILETIME &ft)
{
  unsigned DateN[3],TimeN[3];
  SYSTEMTIME st;
  FILETIME lft;

  // ����������� �������� ������������� ���� � �����
  GetFileDateAndTime(CDate,DateN,DateSeparator);
  GetFileDateAndTime(CTime,TimeN,GetTimeSeparator());
  if(DateN[0] == -1 || DateN[1] == -1 || DateN[2] == -1)
  {
    // ������������ ������� ���� ������, ������ ������� ���� � �����.
    ZeroMemory(&ft,sizeof(ft));
    return ft;
  }

  ZeroMemory(&st,sizeof(st));

  // "�������"
  switch(DateFormat)
  {
    case 0:
      st.wMonth=DateN[0]!=(unsigned)-1?DateN[0]:0;
      st.wDay  =DateN[1]!=(unsigned)-1?DateN[1]:0;
      st.wYear =DateN[2]!=(unsigned)-1?DateN[2]:0;
      break;
    case 1:
      st.wDay  =DateN[0]!=(unsigned)-1?DateN[0]:0;
      st.wMonth=DateN[1]!=(unsigned)-1?DateN[1]:0;
      st.wYear =DateN[2]!=(unsigned)-1?DateN[2]:0;
      break;
    default:
      st.wYear =DateN[0]!=(unsigned)-1?DateN[0]:0;
      st.wMonth=DateN[1]!=(unsigned)-1?DateN[1]:0;
      st.wDay  =DateN[2]!=(unsigned)-1?DateN[2]:0;
      break;
  }
  st.wHour   = TimeN[0]!=(unsigned)-1?(TimeN[0]):0;
  st.wMinute = TimeN[1]!=(unsigned)-1?(TimeN[1]):0;
  st.wSecond = TimeN[2]!=(unsigned)-1?(TimeN[2]):0;

  if (st.wYear<100)
    if (st.wYear<80)
      st.wYear+=2000;
    else
      st.wYear+=1900;

  // �������������� � "������������" ������
  SystemTimeToFileTime(&st,&lft);
  LocalFileTimeToFileTime(&lft,&ft);
  return ft;
}

void FileFilter::GetFileDateAndTime(const char *Src,unsigned *Dst,int Separator)
{
  char Temp[32], Digit[16],*PtrDigit;
  int I;

  xstrncpy(Temp,Src,sizeof(Temp)-1);
  Dst[0]=Dst[1]=Dst[2]=(unsigned)-1;
  I=0;
  const char *Ptr=Temp;
  while((Ptr=GetCommaWord(Ptr,Digit,Separator)) != NULL)
  {
    PtrDigit=Digit;
    while (*PtrDigit && !isdigit(*PtrDigit))
      PtrDigit++;
    if(*PtrDigit)
      Dst[I]=atoi(PtrDigit);
    ++I;
  }
}

int FileFilter::FileInFilter(WIN32_FIND_DATA *fd)
{
  // ������ ��������?
  if (fd==NULL)
    return FALSE;

  // ����� �������� ����� ����� �������?
  if (FF.FMask.Used)
  {
    // ���� �� �������� ��� ����� �������� � �������?
    if (!FilterMask.Compare(fd->cFileName))
      // �� ���������� ���� ����
      return FALSE;
  }

  // ����� �������� ������� ����� �������?
  if (FF.FSize.Used)
  {
    unsigned __int64 sizeabove=FF.FSize.SizeAbove;
    unsigned __int64 sizebelow=FF.FSize.SizeBelow;

    // ����������� ������ �� ���� DWORD � ����������� __int64
    unsigned __int64 fsize=(unsigned __int64)fd->nFileSizeLow|((unsigned __int64)fd->nFileSizeHigh<<32);

    if (sizeabove != (unsigned __int64)-1)
    {
      switch (FF.FSize.SizeType)
      {
        case FSIZE_INBYTES:
          // ������ ����� � ������, ������ ������ �� ������.
          break;
        case FSIZE_INKBYTES:
          // ������ ����� � ����������, �������� ��� � �����.
          // !!! �������� �� ���������� ������������� �������� �� �������� !!!
          sizeabove=sizeabove*_i64(1024);
          break;
        case FSIZE_INMBYTES:
          // ����� // ������ ����� � ����������, �������� ��� � �����.
          // !!! �������� �� ���������� ������������� �������� �� �������� !!!
          sizeabove=sizeabove*_i64(1024)*_i64(1024);
          break;
        case FSIZE_INGBYTES:
          // ����� // ������ ����� � ����������, �������� ��� � �����.
          // !!! �������� �� ���������� ������������� �������� �� �������� !!!
          sizeabove=sizeabove*_i64(1024)*_i64(1024)*_i64(1024);
          break;
        default:
          break;
      }

      if (//sizeabove < 0 &&        // ���� �������� ������������� ����������� ������ �����?      "!= 0" ???
          fsize < sizeabove)       // ������ ����� ������ ������������ ������������ �� �������?
         return FALSE;             // �� ���������� ���� ����
    }

    if (sizebelow != (unsigned __int64)-1)
    {
      switch (FF.FSize.SizeType)
      {
        case FSIZE_INBYTES:
          // ������ ����� � ������, ������ ������ �� ������.
          break;
        case FSIZE_INKBYTES:
          // ������ ����� � ����������, �������� ��� � �����.
          // !!! �������� �� ���������� ������������� �������� �� �������� !!!
          sizebelow=sizebelow*_i64(1024);
          break;
        case FSIZE_INMBYTES:
          // ����� // ������ ����� � ����������, �������� ��� � �����.
          // !!! �������� �� ���������� ������������� �������� �� �������� !!!
          sizebelow=sizebelow*_i64(1024)*_i64(1024);
          break;
        case FSIZE_INGBYTES:
          // ����� // ������ ����� � ����������, �������� ��� � �����.
          // !!! �������� �� ���������� ������������� �������� �� �������� !!!
          sizebelow=sizebelow*_i64(1024)*_i64(1024)*_i64(1024);
          break;
        default:
          break;
      }

      if (//sizebelow < 0 &&        // ���� �������� ������������� ������������ ������ �����?     "!= 0" ???
          fsize > sizebelow)       // ������ ����� ������ ������������� ������������ �� �������?
         return FALSE;             // �� ���������� ���� ����
    }
  }

  // ����� �������� ������� ����� �������?
  if (FF.FDate.Used)
  {
    // ����������� FILETIME � ����������� __int64
    unsigned __int64 &after=(unsigned __int64 &)FF.FDate.DateAfter;
    unsigned __int64 &before=(unsigned __int64 &)FF.FDate.DateBefore;

    if (after!=0 || before!=0)
    {
      unsigned __int64 ftime=0;

      switch (FF.FDate.DateType)
      {
        case FDATE_MODIFIED:
          (unsigned __int64 &)ftime=(unsigned __int64 &)fd->ftLastWriteTime;
          break;
        case FDATE_CREATED:
          (unsigned __int64 &)ftime=(unsigned __int64 &)fd->ftCreationTime;
          break;
        case FDATE_OPENED:
          (unsigned __int64 &)ftime=(unsigned __int64 &)fd->ftLastAccessTime;
          break;
      }

      // ���� �������� ������������� ��������� ����?
      if (after!=0)
        // ���� ����� ������ ��������� ���� �� �������?
        if (ftime<after)
          // �� ���������� ���� ����
          return FALSE;

      // ���� �������� ������������� �������� ����?
      if (before!=0)
        // ���� ����� ������ �������� ���� �� �������?
        if (ftime>before)
          return FALSE;
    }
  }

  // ����� �������� ��������� ����� �������?
  if (FF.FAttr.Used)
  {
    // �������� ��������� ����� �� ������������� ���������
    DWORD AttrSet=FF.FAttr.AttrSet;
    if ((AttrSet & FILE_ATTRIBUTE_READONLY) && (!(fd->dwFileAttributes & FILE_ATTRIBUTE_READONLY)))
      return FALSE;
    if ((AttrSet & FILE_ATTRIBUTE_ARCHIVE) && (!(fd->dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE)))
      return FALSE;
    if ((AttrSet & FILE_ATTRIBUTE_HIDDEN) && (!(fd->dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)))
      return FALSE;
    if ((AttrSet & FILE_ATTRIBUTE_SYSTEM) && (!(fd->dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)))
      return FALSE;
    if ((AttrSet & FILE_ATTRIBUTE_COMPRESSED) && (!(fd->dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED)))
      return FALSE;
    if ((AttrSet & FILE_ATTRIBUTE_ENCRYPTED) && (!(fd->dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED)))
      return FALSE;

    if (!DisableDir)
    {
      if ((AttrSet & FILE_ATTRIBUTE_DIRECTORY) && (!(fd->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)))
        return FALSE;
    }

    // �������� ��������� ����� �� ������������� ���������
    DWORD AttrClear=FF.FAttr.AttrClear;
    if ((AttrClear & FILE_ATTRIBUTE_READONLY) && (fd->dwFileAttributes & FILE_ATTRIBUTE_READONLY))
      return FALSE;
    if ((AttrClear & FILE_ATTRIBUTE_ARCHIVE) && (fd->dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE))
      return FALSE;
    if ((AttrClear & FILE_ATTRIBUTE_HIDDEN) && (fd->dwFileAttributes & FILE_ATTRIBUTE_HIDDEN))
      return FALSE;
    if ((AttrClear & FILE_ATTRIBUTE_SYSTEM) && (fd->dwFileAttributes & FILE_ATTRIBUTE_SYSTEM))
      return FALSE;
    if ((AttrClear & FILE_ATTRIBUTE_COMPRESSED) && (fd->dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED))
      return FALSE;
    if ((AttrClear & FILE_ATTRIBUTE_ENCRYPTED) && (fd->dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED))
      return FALSE;

    if (!DisableDir)
    {
      if ((AttrClear & FILE_ATTRIBUTE_DIRECTORY) && (fd->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        return FALSE;
    }
  }

  // ��! ���� �������� ��� ��������� � ����� ������� � �������������
  // � ��������� ��� ������� ��������.
  return TRUE;
}
