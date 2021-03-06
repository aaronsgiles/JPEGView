/*********************************************************//* This source code copyright (c) 1991-2001, Aaron Giles *//* See the Read Me file for licensing information.       *//* Contact email: mac@aarongiles.com                     *//*********************************************************/#if THINK_C#include "THINK.Header"#elif applec#pragma load ":Headers:MPW.Header"#elif __MWERKS__//#include "MW.Header"#else#include "JPEGView.h"#endif/* * WindowAccessor(classWanted, container, containerClass, keyForm, keyData, *				  resultToken, theRefCon) * *     Purpose: Finds a window according to the specified keyForm and returns the *					corresponding window token *      Inputs: classWanted = the element class desired (cWindow) *				container = token for this element's container *				containerClass = class of this element's container (typeNull) *				keyForm = the search key type *				keyData = descriptor containing search key data *				resultToken = descriptor containing the resulting token *				theRefCon = reference constant *     Returns: an OSErr describing the result * */pascal OSErr WindowAccessor(DescType classWanted, AEDesc *container, 	DescType containerClass, DescType keyForm, AEDesc *keyData, 	AEDesc *resultToken, long theRefCon){#if applec#pragma unused(classWanted, theRefCon)#endif	ObjectTokenHandle theList;	OSErr theErr = noErr;	switch (containerClass) {		case cWindow:			theList = (ObjectTokenHandle)container->dataHandle;			if ((theErr = HandToHand((Handle *)&theList)) != noErr) return theErr;			break;		case typeNull:			if (!(theList = MakeAllWindowsToken())) return memFullErr;			break;		default:			return errAENoSuchObject;	}	switch (keyForm) {		case formAbsolutePosition:			theErr = GetAbsoluteWindow(keyData, &theList);			break;		case formName:			theErr = GetNamedWindow(keyData, &theList);			break;		case formRelativePosition:		case formTest:		case formRange:		case formPropertyID:		default:			return errAEBadKeyForm;			break;	}	if (!theList || !(*theList)->count || (theErr != noErr)) {		DisposeHandle((Handle)theList);		if (theErr == noErr) theErr = errAENoSuchObject;	} else {		theErr = AECreateHandleDesc(cWindow, (Handle)theList, resultToken);		DisposeHandle((Handle)theList);	}	return theErr;}/* * WinPropertyAccessor(classWanted, container, containerClass, keyForm, keyData, *					   resultToken, theRefCon) * *     Purpose: Finds a window property and returns the corresponding property token *      Inputs: classWanted = the element class desired (typeProperty) *				container = token for this element's container *				containerClass = class of this element's container (cWindow) *				keyForm = the search key type *				keyData = descriptor containing search key data *				resultToken = descriptor containing the resulting token *				theRefCon = reference constant *     Returns: an OSErr describing the result * */pascal OSErr WinPropertyAccessor(DescType classWanted, AEDesc *container, 	DescType containerClass, DescType keyForm, AEDesc *keyData, 	AEDesc *resultToken, long theRefCon){	ObjectTokenHandle theList = (ObjectTokenHandle)container->dataHandle;	DescType propertyType;	OSErr theErr;		if ((keyForm != formPropertyID) || (keyData->descriptorType != typeType))		return errAECantSupplyType;	propertyType = *(DescType *)*keyData->dataHandle;	switch (propertyType) {		// Non-Modifiable properties		case pBestType:		case pClass:		case pDefaultType:		case pHasCloseBox:		case pHasTitleBar:		case pIsFloating:		case pIsModal:		case pIsResizable:		case pIsZoomable:		case pWindowType:		// Modifiable properties		case pBounds:		case pFullScreen:		case pIndex:		case pIsZoomed:		case pName:		case pVisible:			break;		// Tricky properties		case pSelection:			return SelectionAccessor(classWanted, container, containerClass, keyForm,				keyData, resultToken, theRefCon);		default:			return errAEEventNotHandled;	}	theErr = HandToHand((Handle *)&theList);	if (theErr != noErr) return theErr;	(*theList)->property = propertyType;	(*theList)->objclass = cWindow;	theErr = AECreateHandleDesc(typeProperty, (Handle)theList, resultToken);	DisposeHandle((Handle)theList);	return theErr;}/* * GetWindowData(theWindow, typeWanted, theData) * *     Purpose: Extracts the data from the window *      Inputs: theWindow = the window associated with this object *				typeWanted = the type we're asking for *				theData = the data we save *     Returns: an OSErr describing the result * */OSErr GetWindowData(WindowPtr theWindow, DescType typeWanted, AEDesc *theData){	if (typeWanted == typeWildCard) typeWanted = typeObjectSpecifier;	if (typeWanted == typeBest) typeWanted = typeObjectSpecifier;	if (typeWanted != typeObjectSpecifier) return errAECantSupplyType;	return MakeWindowObject(theWindow, theData);}/* * GetWinPropertyData(theWindow, theProperty, typeWanted, theData) * *     Purpose: Extracts the data from the window's properties *      Inputs: theWindow = the window associated with this object *				theProperty = the property wanted *				typeWanted = the type we're asking for *				theData = the data we save *     Returns: an OSErr describing the result * */OSErr GetWinPropertyData(WindowPtr theWindow, DescType theProperty, DescType typeWanted, 	AEDesc *theData){	Boolean theBoolean;	ImageHandle theImage;	DescType theType;	Str255 theString;	AEDesc bestData;	long theLong;	Rect theRect;	OSErr theErr;		switch (theProperty) {		case pBestType:		case pDefaultType:			theType = typeObjectSpecifier;			theErr = AECreateDesc(typeType, (void *)&theType, sizeof(DescType), &bestData);			break;		case pBounds:			theRect = theWindow->portRect;			GlobalRect(&theRect, theWindow);			theErr = AECreateDesc(typeQDRectangle, (void *)&theRect, sizeof(Rect), &bestData);			break;		case pClass:			theType = cWindow;			theErr = AECreateDesc(typeType, (void *)&theType, sizeof(DescType), &bestData);			break;		case pHasCloseBox:			theBoolean = HasGoAway(theWindow);			theErr = AECreateDesc(typeBoolean, (void *)&theBoolean, sizeof(Boolean), &bestData);			break;		case pHasTitleBar:			theLong = GetWVariant(theWindow);			theBoolean = (theLong != dBoxProc) && (theLong != altDBoxProc) &&						 (theLong != plainDBox);			theErr = AECreateDesc(typeBoolean, (void *)&theBoolean, sizeof(Boolean), &bestData);			break;		case pIndex:			theLong = GetWindowIndex(theWindow);			theErr = AECreateDesc(typeLongInteger, (void *)&theLong, sizeof(long), &bestData);			break;		case pIsFloating:			theBoolean = WindowKind(theWindow) == floatingWindowKind;			theErr = AECreateDesc(typeBoolean, (void *)&theBoolean, sizeof(Boolean), &bestData);			break;		case pIsZoomed:			theBoolean = IsZoomed(theWindow);			theErr = AECreateDesc(typeBoolean, (void *)&theBoolean, sizeof(Boolean), &bestData);			break;		case pFullScreen:			theImage = FindImage(theWindow);			theBoolean = (theImage) ? (Full(theImage) != 0) : false;			theErr = AECreateDesc(typeBoolean, (void *)&theBoolean, sizeof(Boolean), &bestData);			break;		case pIsModal:			theBoolean = (WindowKind(theWindow) == dialogKind);			theErr = AECreateDesc(typeBoolean, (void *)&theBoolean, sizeof(Boolean), &bestData);			break;		case pIsResizable:			theBoolean = (FindImage(theWindow) != nil);			theErr = AECreateDesc(typeBoolean, (void *)&theBoolean, sizeof(Boolean), &bestData);			break;		case pIsZoomable:			theBoolean = (FindImage(theWindow) != nil) || (theWindow == GetStatWindow());			theErr = AECreateDesc(typeBoolean, (void *)&theBoolean, sizeof(Boolean), &bestData);			break;		case pName:			GetWTitle(theWindow, theString);			theErr = AEMakeIntlDesc(theString, &bestData);			break;		case pSelection:			theErr = MakeSelectionObject(theWindow, &bestData);			break;		case pVisible:			theBoolean = WindowVisible(theWindow);			theErr = AECreateDesc(typeBoolean, (void *)&theBoolean, sizeof(Boolean), &bestData);			break;		case pWindowType:			theType = GetWRefCon(theWindow);			theErr = AECreateDesc(typeEnumerated, (void *)&theType, sizeof(DescType), &bestData);			break;		default:			return errAETypeError;	}	if (theErr != noErr) return theErr;	if ((bestData.descriptorType == typeWanted) ||		(typeWanted == typeWildCard) || (typeWanted == typeBest))		theErr = AEDuplicateDesc(&bestData, theData);	else theErr = AECoerceDesc(&bestData, typeWanted, theData);	AEDisposeDesc(&bestData);	return theErr;}/* * SetWinPropertyData(theWindow, theProperty, theData) * *     Purpose: Sets the data for the window's properties *      Inputs: theWindow = the associated window *				theProperty = the property wanted *				theData = the data to set it to *     Returns: an OSErr describing the result * */OSErr SetWinPropertyData(WindowPtr theWindow, DescType theProperty, AEDesc *theData){	WindowPtr behindWindow;	Boolean theBoolean;	WindowPtr window;	Str255 theString;	long theLong;	Rect theRect;	OSErr theErr;	switch (theProperty) {		case pBounds:			theErr = AEExtractQDRectangle(theData, 0, &theRect);			if (theErr == noErr) DoSetWindowBounds(theWindow, &theRect);			break;		case pFullScreen:			theErr = AEExtractBoolean(theData, 0, &theBoolean);			if ((theErr == noErr) && theWindow) DoSetWindowFull(theWindow, theBoolean);			break;		case pIndex:			theErr = AEExtractLong(theData, 0, &theLong);			if (theErr == noErr) {				behindWindow = IndexedWindow(theLong);				if ((behindWindow == GetFirstWindow()) ||					(behindWindow == FWFrontWindow())) ChangeActive(theWindow);				else {					for (window = GetFirstWindow(); 						 window && (NextWindow(window) != behindWindow);						 window = NextWindow(window));					if (window) {						SendBehind(theWindow, window);						PaintOne((WindowPeek)theWindow, GetStrucRgn(theWindow));						CalcVis((WindowPeek)theWindow);					} else theErr = errAEIndexTooLarge;				}			}			break;		case pIsZoomed:			theErr = AEExtractBoolean(theData, 0, &theBoolean);			if ((theErr == noErr) && theWindow) DoZoomWindow(theWindow, theBoolean);			break;		case pName:			theErr = AEExtractPString(theData, 0, theString);			if (theErr == noErr) theErr = DoSetWindowTitle(theWindow, theString);			break;		case pVisible:			theErr = AEExtractBoolean(theData, 0, &theBoolean);			if (theErr == noErr && theWindow) theErr = DoSetWindowVisible(theWindow, theBoolean);			break;		default:			theErr = errAEWriteDenied;			break;	}	return theErr;}/* * GetAbsoluteWindow(keyData, theList) * *     Purpose: Looks up a window from an absolute position *      Inputs: keyData = descriptor containing search key data *				theList = handle to the set of windows to search *     Returns: an OSErr describing the result * */OSErr GetAbsoluteWindow(AEDesc *keyData, ObjectTokenHandle *theList){	short count = (**theList)->count;	OSErr theErr = noErr;	Ptr theWindow;	long index;		index = *(long *)*keyData->dataHandle;	switch (keyData->descriptorType) {		case typeLongInteger:			if (!count) return errAENoSuchObject;    		if (index == 0) index++;    		if ((index < 1) || (index > count)) return errAENoSuchObject;    		theWindow = (**theList)->object[index - 1];    		DisposeHandle((Handle)*theList);    		if (!(*theList = MakeSingleObjectToken(theWindow))) return memFullErr;			break;		case typeAbsoluteOrdinal:			switch (index) {				case kAEFirst:					if (!count) return errAENoSuchObject;					theWindow = (**theList)->object[0];					break;				case kAELast: 					if (!count) return errAENoSuchObject;					theWindow = (**theList)->object[count - 1];					break;				case kAEMiddle:					if (!count) return errAENoSuchObject;					theWindow = (**theList)->object[count >> 1];					break;				case kAEAny: 					if (!count) return errAENoSuchObject;					theWindow = (**theList)->object[(count * Random()) >> 16];					break;				case kAEAll:					return noErr;				case kStatWindowID:					if (theWindow = (Ptr)GetStatWindow()) break;					return gBadWindowID = index, errAENoSuchObject;				case kCommentsWindowID:					if (theWindow = (Ptr)GetCommentsWindow()) break;					return gBadWindowID = index, errAENoSuchObject;				case kHelpWindowID:					if (theWindow = (Ptr)GetHelpWindow()) break;					return gBadWindowID = index, errAENoSuchObject;				case kSlideShowDialogID:					if (theWindow = (Ptr)GetSlideOptionsWindow()) break;					return gBadWindowID = index, errAENoSuchObject;				case kPrefsDialogID:					if (theWindow = (Ptr)GetPrefsWindow()) break;					return gBadWindowID = index, errAENoSuchObject;				default:					return errAEBadKeyForm;			}			DisposeHandle((Handle)*theList);			if (!(*theList = MakeSingleObjectToken(theWindow))) return memFullErr;			break;		default:			return errAEBadKeyForm;	}	return noErr;}/* * GetNamedWindow(keyData, theWindow) * *     Purpose: Looks up a window from a text string *      Inputs: keyData = descriptor containing search key data *				theList = handle to the set of windows to search *     Returns: an OSErr describing the result * */OSErr GetNamedWindow(AEDesc *keyData, ObjectTokenHandle *theList){	uchar theString[256], theTitle[256];	short i, count = (**theList)->count;	WindowPtr *window;	OSErr theErr;		if (!count) return errAENoSuchObject;	if ((theErr = AEExtractPString(keyData, 0, theString)) != noErr) return theErr;	for (i = 0, window = (WindowPtr *)(**theList)->object; i < count; i++, window++) {		GetWTitle(*window, theTitle);		if (EqualString(theTitle, theString, false, false)) break;	}	if (i == count) return errAENoSuchObject;	DisposeHandle((Handle)*theList);	if (!(*theList = MakeSingleObjectToken((Ptr)*window))) return memFullErr;	return noErr;}/* * CountWindows() * *     Purpose: Counts the number of windows in the application *      Inputs: none *     Returns: the number of windows found * */short CountWindows(void){	WindowPtr window;	short count = 0;		for (window = GetFirstWindow(); window; window = NextWindow(window)) count++;	return count;}/* * IndexedWindow(index) * *     Purpose: Returns the window associated with the given index *      Inputs: index = the index *     Returns: a pointer to the given window * */WindowPtr IndexedWindow(short index){	WindowPtr theWindow;	short count = 1;		theWindow = GetFirstWindow();	for (; theWindow && (count < index); theWindow = NextWindow(theWindow)) count++;	return (WindowPtr)theWindow;}/* * GetWindowIndex() * *     Purpose: Gets the index of the given window *      Inputs: theWindow = the window to look for *     Returns: the index of the window * */short GetWindowIndex(WindowPtr theWindow){	WindowPtr window;	short count = 1;		window = GetFirstWindow();	for (; window && (window != theWindow); window = NextWindow(window)) count++;	return count;}/* * MakeWinPropertyObject(theWindow, theProperty, theObject) * *     Purpose: Creates a window object descriptor for the given window's property *      Inputs: theWindow = pointer to the window, or nil for all windows *				theObject = pointer to an AEDesc to store the result *     Returns: an OSErr describing what went wrong * */OSErr MakeWinPropertyObject(WindowPtr theWindow, DescType theProperty, AEDesc *theObject){	AEDesc theKey, theContainer;	OSErr theErr = noErr;		theErr = MakeWindowObject(theWindow, &theContainer);	if (theErr == noErr) {		theErr = AECreateDesc(typeType, (void *)&theProperty, sizeof(DescType), &theKey);		if (theErr == noErr) {			theErr = CreateObjSpecifier(typeProperty, &theContainer, formPropertyID, 					 &theKey, false, theObject);			AEDisposeDesc(&theKey);		}		AEDisposeDesc(&theContainer);	}	return theErr;}/* * MakeWindowObject(theWindow, theObject) * *     Purpose: Creates a window object descriptor for the given window *      Inputs: theWindow = pointer to the window, or nil for all windows *				theObject = pointer to an AEDesc to store the result *     Returns: an OSErr describing what went wrong * */OSErr MakeWindowObject(WindowPtr theWindow, AEDesc *theObject){	AEDesc theKey, theContainer = { typeNull, nil };	OSErr theErr = noErr;	DescType theType;	long index;		switch ((long)theWindow) {		case kAEAll:			index = (long)theWindow;			theType = typeAbsoluteOrdinal;			break;		default:			theType = typeLongInteger;			index = GetWindowIndex(theWindow);			break;	}	theErr = AECreateDesc(theType, (void *)&index, sizeof(long), &theKey);	if (theErr == noErr) {		theErr = CreateObjSpecifier(cWindow, &theContainer, formAbsolutePosition, 				 &theKey, false, theObject);		AEDisposeDesc(&theKey);	}	return theErr;}/* * MakeAllWindowsToken() * *     Purpose: Creates a window token list specifying all open windows *      Inputs: none *     Returns: an ObjectTokenHandle containing the result * */ObjectTokenHandle MakeAllWindowsToken(void){	short count = CountWindows();	ObjectTokenHandle theList;	WindowPtr window, *item;	if (theList = (ObjectTokenHandle)NewHandle(sizeof(ObjectToken) + 												(count - 1) * sizeof(Ptr))) {		(*theList)->count = count;		item = (WindowPtr *)(*theList)->object;		for (window = GetFirstWindow(); window; window = NextWindow(window)) 			*item++ = window;	}	return theList;}/* * MakeSingleObjectToken(theObject) * *     Purpose: Creates a window token list specifying the given object only *      Inputs: theObject = pointer to the object record to specify *     Returns: an ObjectTokenHandle containing the result * */ObjectTokenHandle MakeSingleObjectToken(Ptr theObject){	ObjectTokenHandle theList;	if (theList = (ObjectTokenHandle)NewHandle(sizeof(ObjectToken))) {		(*theList)->count = 1;		(*theList)->object[0] = theObject;	}	return theList;}/* * DoSetWindowBounds(theWindow, bounds) * *     Purpose: Sets the bounds of the given window to the specified rectangle *      Inputs: theWindow = the window to move/resize *				bounds = the new bounds of the window *     Returns: an OSErr describing the result * */OSErr DoSetWindowBounds(WindowPtr theWindow, Rect *bounds){	ImageHandle theImage = FindImage(theWindow);	MonitorHandle theMonitor;	Rect newRect;	RgnHandle theRgn;		if (!EqualSizeRect(bounds, &theWindow->portRect)) {		if (theImage && !Full(theImage)) {			(*theImage)->wrect = *bounds;			ResizeWindow(theImage);		} else if (theWindow == GetHelpWindow()) {			ResizeHelp(bounds);		} else return errAEPrivilegeError;	}	newRect = theWindow->portRect;	OffsetRect(&newRect, bounds->left - newRect.left, bounds->top - newRect.top);	theMonitor = GetMostDevice(&newRect);	if (theRgn = NewRgn()) {		SetRectRgn(theRgn, newRect.left + 4, newRect.top - gTitleBarHeight + 4, 							newRect.right - 4, newRect.top - 4);		SectRgn(theRgn, GetGrayRgn(), theRgn);		if (EmptyRgn(theRgn)) NudgeWindowRect(&newRect, theMonitor);		FWMoveWindow(theWindow, newRect.left, newRect.top, false);		DisposeRgn(theRgn);	}	if (theImage) GetMinMaxMonitor(theImage);	return noErr;}/* * DoSetWindowFull(theWindow, full) * *     Purpose: Sets the full screen property of the given window *      Inputs: theWindow = the window to move/resize *				full = flag: true to make the window full-screen *     Returns: an OSErr describing the result * */OSErr DoSetWindowFull(WindowPtr theWindow, Boolean full){	ImageHandle theImage = FindImage(theWindow);	WindowPtr behindWindow;	GWorldPtr oldGWorld;	Rect oldWRect;	OSErr theErr;		if (!theImage) return errAENotModifiable;	oldWRect = (*theImage)->wrect;	oldGWorld = (*theImage)->gworld;	(*theImage)->gworld = nil;	if (theWindow == FWFrontWindow()) behindWindow = nil;	else for (behindWindow = GetFirstWindow(); behindWindow; behindWindow = NextWindow(behindWindow))		if (NextWindow(behindWindow) == theWindow) break;	if (full) (*theImage)->flags |= ifFull;	else (*theImage)->flags &= ~ifFull;	theErr = MakeNewWindow(theImage, true, gThePrefs.expandSmall);	(*theImage)->gworld = oldGWorld;	if (theErr == noErr) {		if ((*theImage)->palette) {			SetPalette((*theImage)->window, (*theImage)->palette, true);			ActivatePalette((*theImage)->window);		}		if (behindWindow) {			SendBehind((*theImage)->window, behindWindow);			FWShowWindow((*theImage)->window);		} else ChangeActive((*theImage)->window);		if (!EqualSizeRect(&(*theImage)->wrect, &oldWRect) && !GWOrigSize(theImage)) 			KillGWorld(theImage);	}	return theErr;}/* * DoSetWindowTitle(theWindow, theString) * *     Purpose: Sets the title of the given window *      Inputs: theWindow = the window to retitle *				theString = the string to set it to *     Returns: an OSErr describing the result * */OSErr DoSetWindowTitle(WindowPtr theWindow, StringPtr theString){	ImageHandle theImage = FindImage(theWindow);	MenuHandle windowMenu;	short item;		SetWTitle(theWindow, theString);	if (!theImage) return noErr;	item = GetDocumentIndex((*theImage)->window) + windowFirstItem - 1;	(*theImage)->num = 0;	if (windowMenu = GetMHandle(rWindowMenu)) SetItem(windowMenu, item, theString);	return noErr;}extern OSErr DoSetWindowVisible(WindowPtr theWindow, Boolean visible){	if (visible) {		FWShowWindow(theWindow);		if (theWindow == FWFrontWindow()) ChangeActive(theWindow);	} else FWHideWindow(theWindow);	return noErr;}