#define UNICODE
#define _UNICODE

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <string.h>
#include "../dll/vlstr.h"
#include "../dll/pathStack.h"
#include "../dll/langUtil.h"
#include "../dll/stringReader.h"
#include "../dll/utf8Format.h"

HVLSTR fbuf,errmsg;

#include "nsfaa_lang.txt"
#include "6502ins.h"
#include "macroEnum.h"
#include "imageTypeEnum.h"
#include "bankType.h"
#include "bank.h"
#include "rom.h"
#include "image.h"
#include "refStack.h"
#include "labelList.h"
#include "aaFileStack.h"
#include "aaStatus.h"

HPS hps;
struct aaStatus *aas;
